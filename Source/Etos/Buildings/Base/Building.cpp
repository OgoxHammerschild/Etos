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

	InitOccupiedBuildSpace();
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
	else if (Data.bIsHeld)
	{
		MoveToMouseLocation();
		GetSurroundingBuildings();
	}
}

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
	float halfTileSize = 50;
	Width = width;
	Height = height;
	FoundationMesh->SetWorldScale3D(FVector(width, height, 1));
	OccupiedBuildSpace->Collider->SetBoxExtent(FVector(halfTileSize * width - 1, halfTileSize * height - 1, halfTileSize - 1));
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
	OccupiedBuildSpace = CreateDefaultSubobject<UBoxCollider>(TEXT("Occupied Build Space"));
	OccupiedBuildSpace->SetupAttachment(RootComponent);
	OccupiedBuildSpace->Collider->SetRelativeLocation(FVector(0, 0, 50));
	OccupiedBuildSpace->Collider->SetBoxExtent(FVector(49));
	OccupiedBuildSpace->Collider->SetVisibility(true);
	OccupiedBuildSpace->Collider->bHiddenInGame = true;
	OccupiedBuildSpace->Collider->SetEnableGravity(false);
	OccupiedBuildSpace->Collider->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	OccupiedBuildSpace->Collider->SetCollisionObjectType(ECollisionChannel::ECC_GameTraceChannel1 /*Buliding*/);
	OccupiedBuildSpace->Collider->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);

	OccupiedBuildSpace->OnTriggerEnter.AddDynamic(this, &ABuilding::BuildSpace_OnBeginOverlap);
	OccupiedBuildSpace->OnTriggerExit.AddDynamic(this, &ABuilding::BuildSpace_OnEndOverlap);
}

void ABuilding::CreateTracePoints()
{
	if (UWorld* World = GetWorld())
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
			point->SetupAttachment(OccupiedBuildSpace);
			point->RegisterComponentWithWorld(World);
			point->SetVisibility(false);
		}
	}
}

void ABuilding::RelocateTracePoints()
{
	if (!OccupiedBuildSpace || !OccupiedBuildSpace->Collider)
	{
		UE_LOG(LogTemp, Error, TEXT("*explodes*"));
		return;
	}

	FVector BoxExtend = OccupiedBuildSpace->Collider->GetScaledBoxExtent();
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

//void ABuilding::BuildSpace_OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
void ABuilding::BuildSpace_OnBeginOverlap(UBoxCollider* other)
{
	if (bMovedOnce)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s collided with %s"), *GetName(), *other->GetName());

		Data.bPositionIsBlocked = true;

		++collisions;
	}
}

//void ABuilding::BuildSpace_OnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
void ABuilding::BuildSpace_OnEndOverlap(UBoxCollider* other)
{
	if (bMovedOnce)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s and %s are no more colliding"), *GetName(), *other->GetName());

		if (--collisions == 0)
		{
			Data.bPositionIsBlocked = false;
		}

		UE_LOG(LogTemp, Warning, TEXT("There are %i collisions left"), collisions);
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

void ABuilding::CallDelayAction(float pastTime, float delayDuration)
{
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
