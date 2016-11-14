// © 2016 - 2017 Daniel Bortfeld

#include "Etos.h"
#include "Building.h"
#include "Etos/FunctionLibraries/BuildingFunctionLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Etos/Buildings/Path.h"
#include "Etos/Game/EtosPlayerController.h"
#include "Etos/UI/ResourcePopup.h"
#include "Etos/Collision/BoxCollider.h"

// Sets default values
ABuilding::ABuilding()
{
	// Set this actor to call Tick() every frame. You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent->SetMobility(EComponentMobility::Movable);

	BuildingMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Building Mesh"));
	BuildingMesh->SetupAttachment(RootComponent);
	BuildingMesh->SetCanEverAffectNavigation(false);
	BuildingMesh->CastShadow = false;

	FoundationMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Foundation"));
	FoundationMesh->SetupAttachment(RootComponent);
	FoundationMesh->SetCanEverAffectNavigation(false);
	FoundationMesh->CastShadow = false;
	ConstructorHelpers::FObjectFinder<UStaticMesh> quadFinder = ConstructorHelpers::FObjectFinder<UStaticMesh>(TEXT("StaticMesh'/Game/BasicMeshes/SM_Quad_1x1m.SM_Quad_1x1m'"));
	if (quadFinder.Succeeded())
	{
		FoundationMesh->SetStaticMesh(quadFinder.Object);
	}

	Radius = CreateDefaultSubobject<USphereComponent>(TEXT("Radius"));
	Radius->SetupAttachment(RootComponent);
	Radius->SetVisibility(false);
	Radius->SetCollisionProfileName(TEXT("OverlapBuildings"));
	Radius->OnComponentBeginOverlap.AddDynamic(this, &ABuilding::BuildingEnteredRadius);
	Radius->OnComponentEndOverlap.AddDynamic(this, &ABuilding::BuildingLeftRadius);
	Radius->SetSphereRadius(Data.Radius);
	Radius->SetCanEverAffectNavigation(false);

	if (!ResourcePopup)
	{
		ConstructorHelpers::FObjectFinder<UBlueprint> popupFinder = ConstructorHelpers::FObjectFinder<UBlueprint>(TEXT("Blueprint'/Game/Blueprints/UI/ResourcePopup/BP_ResourcePopup.BP_ResourcePopup'"));
		if (popupFinder.Succeeded())
		{
			ResourcePopup = (UClass*)popupFinder.Object->GeneratedClass;
		}
	}

	if (bUseCustomBoxCollider)
	{
		InitOccupiedBuildSpace_Custom();
	}
	else
	{
		InitOccupiedBuildSpace();
	}

	SetFoundationSize(1, 1);
}

void ABuilding::PostInitProperties()
{
	Super::PostInitProperties();
	CreateTracePoints();
	BindDelayAction();
}

// Called when the game starts or when spawned
void ABuilding::BeginPlay()
{
	Super::BeginPlay();
	RelocateTracePoints();
}

// Called every frame
void ABuilding::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (Data.bIsBuilt)
	{
		CallDelayAction(DeltaTime, Data.ProductionTime);
	}
	else
	{
		if (Data.bIsHeld)
		{
			MoveToMouseLocation();
		}
		GetSurroundingBuildings();
	}
}

void ABuilding::BeginDestroy()
{
	Super::BeginDestroy();
}

#if WITH_EDITOR
void ABuilding::PostEditChangeProperty(FPropertyChangedEvent & PropertyChangedEvent)
{
	FName PropertyName = (PropertyChangedEvent.Property != NULL) ? PropertyChangedEvent.Property->GetFName() : NAME_None;

	if ((PropertyName == GET_MEMBER_NAME_CHECKED(ABuilding, Width)) || (PropertyName == GET_MEMBER_NAME_CHECKED(ABuilding, Height)))
	{
		SetFoundationSize(Width, Height);
	}
}
#endif

void ABuilding::OnBuild()
{
	Data.PathConnections = Data.PossibleConnections;
	Data.PossibleConnections.Empty();

	Data.BuildingsInRadius = Data.PossibleBuildingsInRadius;
	Data.PossibleBuildingsInRadius.Empty();

	for (APath* path : Data.PathConnections)
	{
		if (path)
		{
			path->Connections.Add(this);
		}
	}

	BuildEvent.Broadcast(this);

	Data.bIsBuilt = true;
}

void ABuilding::ResetStoredResources()
{
	Data.ProducedResource.Amount = 0;
}

void ABuilding::SetFoundationSize(int32 width, int32 height)
{
	if (bUseCustomBoxCollider)
	{
		checkf(OccupiedBuildSpace_Custom, TEXT("OccupiedBuildSpace should not be null actually"));
	}
	else
	{
		checkf(OccupiedBuildSpace, TEXT("OccupiedBuildSpace should not be null actually"));
	}

	float halfTileSize = 50;
	Width = width;
	Height = height;
	FoundationMesh->SetWorldScale3D(FVector(width, height, 1));

	if (bUseCustomBoxCollider)
	{
		OccupiedBuildSpace_Custom->Collider->SetBoxExtent(FVector(halfTileSize * width - 1, halfTileSize * height - 1, halfTileSize - 1));
	}
	else
	{
		OccupiedBuildSpace->SetBoxExtent(FVector(halfTileSize * width - 1, halfTileSize * height - 1, halfTileSize - 1));
	}
}

FORCEINLINE bool ABuilding::operator<(ABuilding & B) const
{
	return Data.ProducedResource.Amount < B.Data.ProducedResource.Amount;
}

FORCEINLINE bool ABuilding::operator<(const ABuilding & B) const
{
	return Data.ProducedResource.Amount < B.Data.ProducedResource.Amount;
}

void ABuilding::InitOccupiedBuildSpace()
{
	OccupiedBuildSpace = CreateDefaultSubobject<UBoxComponent>(TEXT("OccupiedBuildSpace"));
	OccupiedBuildSpace->SetupAttachment(RootComponent);
	OccupiedBuildSpace->SetRelativeLocation(FVector(0, 0, 50));
	OccupiedBuildSpace->SetBoxExtent(FVector(49));
	OccupiedBuildSpace->SetVisibility(true);
	OccupiedBuildSpace->bHiddenInGame = true;
	OccupiedBuildSpace->SetEnableGravity(false);
	OccupiedBuildSpace->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	OccupiedBuildSpace->SetCollisionObjectType(ECollisionChannel::ECC_GameTraceChannel1 /*Buliding*/);
	OccupiedBuildSpace->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);

	OccupiedBuildSpace->OnComponentBeginOverlap.AddDynamic(this, &ABuilding::BuildSpace_OnBeginOverlap);
	OccupiedBuildSpace->OnComponentEndOverlap.AddDynamic(this, &ABuilding::BuildSpace_OnEndOverlap);

	checkf(OccupiedBuildSpace, TEXT("OccupiedBuildSpace should not be null actually"));
}

void ABuilding::InitOccupiedBuildSpace_Custom()
{
	checkf(!OccupiedBuildSpace_Custom, TEXT("OccupiedBuildSpace_Custom should be null actually"));

	OccupiedBuildSpace_Custom = CreateDefaultSubobject<UBoxCollider>(TEXT("OccupiedBuildSpace"));

	checkf(OccupiedBuildSpace_Custom, TEXT("OccupiedBuildSpace_Custom should not be null actually"));

	OccupiedBuildSpace_Custom->SetupAttachment(RootComponent);


	OccupiedBuildSpace_Custom->Collider->SetRelativeLocation(FVector(0, 0, 50));
	OccupiedBuildSpace_Custom->Collider->SetBoxExtent(FVector(49));
	OccupiedBuildSpace_Custom->Collider->SetVisibility(true);
	OccupiedBuildSpace_Custom->Collider->bHiddenInGame = true;
	OccupiedBuildSpace_Custom->Collider->SetEnableGravity(false);
	OccupiedBuildSpace_Custom->Collider->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	OccupiedBuildSpace_Custom->Collider->SetCollisionObjectType(ECollisionChannel::ECC_GameTraceChannel1 /*Buliding*/);
	OccupiedBuildSpace_Custom->Collider->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);

	OccupiedBuildSpace_Custom->OnTriggerEnter.AddDynamic(this, &ABuilding::BuildSpace_OnBeginOverlap_Custom);
	OccupiedBuildSpace_Custom->OnTriggerExit.AddDynamic(this, &ABuilding::BuildSpace_OnEndOverlap_Custom);

	checkf(OccupiedBuildSpace_Custom, TEXT("OccupiedBuildSpace_Custom should not be null actually"));
}

void ABuilding::CreateTracePoints()
{
	checkf(this, TEXT("this should not be null actually"));
	if (bUseCustomBoxCollider)
	{
		checkf(OccupiedBuildSpace_Custom, TEXT("OccupiedBuildSpace should not be null actually"));
	}
	else
	{
		checkf(OccupiedBuildSpace, TEXT("OccupiedBuildSpace should not be null actually"));
	}

	if (UWorld* const World = GetWorld())
	{
		TracePoints.Add(NewObject<USceneComponent>(this, USceneComponent::StaticClass(), TEXT("Trace Start Top")));
		TracePoints.Add(NewObject<USceneComponent>(this, USceneComponent::StaticClass(), TEXT("Trace End Top")));
		TracePoints.Add(NewObject<USceneComponent>(this, USceneComponent::StaticClass(), TEXT("Trace Start Bot")));
		TracePoints.Add(NewObject<USceneComponent>(this, USceneComponent::StaticClass(), TEXT("Trace End Bot")));
		TracePoints.Add(NewObject<USceneComponent>(this, USceneComponent::StaticClass(), TEXT("Trace Start Left")));
		TracePoints.Add(NewObject<USceneComponent>(this, USceneComponent::StaticClass(), TEXT("Trace End Left")));
		TracePoints.Add(NewObject<USceneComponent>(this, USceneComponent::StaticClass(), TEXT("Trace Start Right")));
		TracePoints.Add(NewObject<USceneComponent>(this, USceneComponent::StaticClass(), TEXT("Trace End Right")));

		for (USceneComponent* point : TracePoints)
		{
			if (bUseCustomBoxCollider)
			{
				point->SetupAttachment(OccupiedBuildSpace_Custom);
			}
			else
			{
				point->SetupAttachment(OccupiedBuildSpace);
			}
			point->RegisterComponentWithWorld(World);
			point->SetVisibility(false);
		}
	}
}

void ABuilding::RelocateTracePoints()
{
	checkf(OccupiedBuildSpace, TEXT("OccupiedBuildSpace should not be null actually"));

	FVector BoxExtend;

	if (bUseCustomBoxCollider)
	{
		checkf(OccupiedBuildSpace_Custom->Collider, TEXT("Collider should not be null actually"));

		BoxExtend = OccupiedBuildSpace_Custom->Collider->GetScaledBoxExtent();
	}
	else
	{
		BoxExtend = OccupiedBuildSpace->GetScaledBoxExtent();
	}

	FVector Start;
	FVector End;
	//top
	UBuildingFunctionLibrary::CalcVectors(BoxExtend.X, BoxExtend.Y, BoxExtend.Z, true, true, false, true, EOffsetDirections::TopRight, EOffsetDirections::TopLeft, Start, End);
	TracePoints[0]->SetRelativeLocation(Start);
	TracePoints[1]->SetRelativeLocation(End);
	//bot
	UBuildingFunctionLibrary::CalcVectors(BoxExtend.X, BoxExtend.Y, BoxExtend.Z, true, false, false, false, EOffsetDirections::BotRight, EOffsetDirections::BotLeft, Start, End);
	TracePoints[2]->SetRelativeLocation(Start);
	TracePoints[3]->SetRelativeLocation(End);
	//left
	UBuildingFunctionLibrary::CalcVectors(BoxExtend.X, BoxExtend.Y, BoxExtend.Z, true, true, true, false, EOffsetDirections::BotLeft, EOffsetDirections::TopLeft, Start, End);
	TracePoints[4]->SetRelativeLocation(Start);
	TracePoints[5]->SetRelativeLocation(End);
	//right
	UBuildingFunctionLibrary::CalcVectors(BoxExtend.X, BoxExtend.Y, BoxExtend.Z, false, true, false, false, EOffsetDirections::BotRight, EOffsetDirections::TopRight, Start, End);
	TracePoints[6]->SetRelativeLocation(Start);
	TracePoints[7]->SetRelativeLocation(End);
}

void ABuilding::GetSurroundingBuildings()
{
	Data.PossibleConnections.Empty();
	TArray<FHitResult> AllHitResults;
	TArray<FHitResult> HitResults;

	for (int32 i = 0; i < 8; i += 2)
	{
		Util::TraceMultiForBuildings(this, TracePoints[i]->GetComponentLocation(), TracePoints[1 + i]->GetComponentLocation(), HitResults);
		AllHitResults.Append(HitResults);
	}

	for (FHitResult hit : AllHitResults)
	{
		APath* path = dynamic_cast<APath*, AActor> (&*hit.Actor);
		if (path)
		{
			Data.PossibleConnections.AddUnique(path);
		}
	}
}

void ABuilding::BindDelayAction()
{
	Action.BindDynamic(this, &ABuilding::AddResource);
}

TArray<ABuilding*> collisions;

void ABuilding::BuildSpace_OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	if (bMovedOnce)
	{
		if (ABuilding* const other = dynamic_cast<ABuilding*, AActor>(OtherActor))
		{
			if (UBoxComponent* const box = dynamic_cast<UBoxComponent*, UPrimitiveComponent>(OtherComp))
			{
				if (!collisions.Contains(other))
				{
					UE_LOG(LogTemp, Warning, TEXT("%s collided with %s"), *GetName(), *OtherActor->GetName());

					Data.bPositionIsBlocked = true;

					collisions.AddUnique(other);
				}
			}
		}
	}
}

void ABuilding::BuildSpace_OnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (bMovedOnce)
	{
		if (ABuilding* const other = dynamic_cast<ABuilding*, AActor>(OtherActor))
		{
			if (UBoxComponent* const box = dynamic_cast<UBoxComponent*, UPrimitiveComponent>(OtherComp))
			{
				if (collisions.Contains(other))
				{
					UE_LOG(LogTemp, Warning, TEXT("%s and %s are no more colliding"), *GetName(), *OtherActor->GetName());

					Data.bPositionIsBlocked = false;

					collisions.Remove(other);

					UE_LOG(LogTemp, Warning, TEXT("There are %i collisions left"), collisionCount);
				}
			}
		}

	}
}

void ABuilding::BuildSpace_OnBeginOverlap_Custom(UBoxCollider* other)
{
	if (bMovedOnce)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s collided with %s"), *GetName(), *other->GetName());

		Data.bPositionIsBlocked = true;

		++collisionCount;
	}
}

void ABuilding::BuildSpace_OnEndOverlap_Custom(UBoxCollider* other)
{
	if (bMovedOnce)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s and %s are no more colliding"), *GetName(), *other->GetName());

		if (--collisionCount == 0)
		{
			Data.bPositionIsBlocked = false;
		}

		UE_LOG(LogTemp, Warning, TEXT("There are %i collisions left"), collisionCount);
	}
}

void ABuilding::BuildingEnteredRadius(UPrimitiveComponent * OverlappedComponent, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	if (dynamic_cast<APath*, AActor>(OtherActor))
	{
		// Don't consider paths as buildings in range
		// Paths are stored as connections
		return;
	}

	if (ABuilding * building = dynamic_cast<ABuilding*, AActor>(OtherActor))
	{
		if (building->Data.bIsBuilt)
		{
			if (Data.bIsHeld)
			{
				Data.PossibleBuildingsInRadius.AddUnique(building);
			}
		}
		else if (building->Data.bIsHeld)
		{
			if (Data.bIsBuilt)
			{
				building->BuildEvent.AddDynamic(this, &ABuilding::AddNewBuildingInRange);
			}
		}
	}
}

void ABuilding::BuildingLeftRadius(UPrimitiveComponent * OverlappedComponent, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex)
{
	if (dynamic_cast<APath*, AActor>(OtherActor))
	{
		// don't consider paths as bulidings in range
		// paths are stored as connections
		return;
	}

	if (ABuilding * building = dynamic_cast<ABuilding*, AActor>(OtherActor))
	{
		if (building->Data.bIsBuilt)
		{
			if (Data.bIsHeld)
			{
				Data.PossibleBuildingsInRadius.Remove(building);
			}
		}
		else if (building->Data.bIsHeld)
		{
			if (Data.bIsBuilt)
			{
				building->BuildEvent.RemoveDynamic(this, &ABuilding::AddNewBuildingInRange);
			}
		}
	}
}

void ABuilding::AddResource()
{
	if (Data.ProducedResource.Amount < Data.MaxStoredResources)
	{
		Data.ProducedResource.Amount++;
		SpawnResourcePopup();
	}
}

TArray<ABuilding*> ABuilding::GetBuildingsInRange()
{
	FVector Location = this->GetActorLocation();

	TArray<FHitResult> HitResults;

	TArray<ABuilding*> BuildingsInRange;

	if (UKismetSystemLibrary::SphereTraceMultiForObjects(this, Location, Location, Data.Radius, Util::BuildingObjectType, false, TArray<AActor*>(), EDrawDebugTrace::None, HitResults, true))
	{
		for (FHitResult hit : HitResults)
		{
			ABuilding * building = dynamic_cast<ABuilding*, AActor>(&*hit.Actor);
			if (building)
			{
				BuildingsInRange.Emplace(building);
			}
		}
	}

	return BuildingsInRange;
}

void ABuilding::CallDelayAction(float pastTime, float delayDuration)
{
	if (delayDuration <= 0)
		return;

	this->pastDelayTimerTime += pastTime;

	if (this->pastDelayTimerTime >= delayDuration)
	{
		this->pastDelayTimerTime = 0;
		this->Action.ExecuteIfBound();
	}
}

void ABuilding::SpawnResourcePopup(FVector offset)
{
	if (ResourcePopup)
	{
		if (UWorld* const World = GetWorld())
		{
			FActorSpawnParameters params = FActorSpawnParameters();
			params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			if (APlayerCameraManager* const PlayerCameraManager = Util::GetFirstEtosPlayerController(this)->PlayerCameraManager)
			{
				FVector cameraLocation = PlayerCameraManager->GetCameraLocation();

				FRotator rotation = UKismetMathLibrary::FindLookAtRotation(cameraLocation + PlayerCameraManager->GetCameraRotation().Vector(), cameraLocation);

				World->SpawnActor<AActor>(ResourcePopup, GetActorLocation() + offset, rotation, params);
			}
		}
	}
}

void ABuilding::AddNewBuildingInRange(ABuilding * buildingInRange)
{
	if (buildingInRange && !buildingInRange->IsPendingKillOrUnreachable())
	{
		Data.BuildingsInRadius.AddUnique(buildingInRange);
	}
}

void ABuilding::MoveToMouseLocation()
{
	FHitResult Hit;
	if (Util::TraceSingleAtMousePosition(this, Hit))
	{
		SetActorLocation(BFuncs::GetNextGridLocation(Hit.ImpactPoint, FVector2Di(Width, Height)));
		bMovedOnce = true;
	}
}

//DEPRICATED CONTENT
//void ABuilding::OnConstruction(const FTransform& Transform)
//{
//	Data.PossibleConnections.Empty();
//	TArray<FHitResult> AllHitResults;
//	TArray<FHitResult> HitResults;
//
//	for (int32 i = 0; i < 8; i += 2)
//	{
//		TraceMultiForBuildings(TracePoints[i]->GetComponentLocation(), TracePoints[1 + i]->GetComponentLocation(), HitResults);
//		AllHitResults.Append(HitResults);
//	}
//
//	for (FHitResult hit : AllHitResults)
//	{
//		APath* path = dynamic_cast<APath*, AActor> (&*hit.Actor);
//		if (path)
//		{
//			Data.PossibleConnections.AddUnique(path);
//		}
//	}
//}

// DEPRICATED, moved to Util
//bool ABuilding::TraceSingleForBuildings(FVector Start, FVector End, FHitResult& HitResult)
//{
//	return UKismetSystemLibrary::LineTraceSingleForObjects(this, Start, End, Util::BuildingObjectType, false, TArray<AActor*>(), EDrawDebugTrace::None, HitResult, true);
//}
//
//bool ABuilding::TraceMultiForBuildings(FVector Start, FVector End, TArray<FHitResult>& HitResults)
//{
//	return UKismetSystemLibrary::LineTraceMultiForObjects(this, Start, End, Util::BuildingObjectType, false, TArray<AActor*>(), EDrawDebugTrace::None, HitResults, true);
//}
//
//bool ABuilding::TraceSingleForFloor(FVector Start, FVector End, FHitResult & Hit)
//{
//	return UKismetSystemLibrary::LineTraceSingleForObjects(this, Start, End, Util::FloorObjectType, false, TArray<AActor*>(), EDrawDebugTrace::None, Hit, true);
//}
