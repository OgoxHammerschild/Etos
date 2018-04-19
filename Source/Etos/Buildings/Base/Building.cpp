// © 2016 - 2017 Daniel Bortfeld

#include "Etos.h"
#include "Building.h"
#include "Etos/Utility/FunctionLibraries/BuildingFunctionLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Etos/Buildings/Path.h"
#include "Etos/Game/EtosPlayerController.h"
#include "Etos/UI/ResourcePopup.h"
#include "Etos/Collision/BoxCollider.h"
#include "Etos/Pawns/MarketBarrow.h"

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
	BuildingMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	FoundationMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Foundation"));
	FoundationMesh->SetupAttachment(RootComponent);
	FoundationMesh->SetCanEverAffectNavigation(false);
	FoundationMesh->CastShadow = false;
	//FoundationMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
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

	OnBuild_ParticleSystem = CreateDefaultSubobject<UParticleSystemComponent>("OnBuild ParticleSystem");
	OnBuild_ParticleSystem->SetupAttachment(RootComponent);
	OnBuild_ParticleSystem->bAutoActivate = false;

	if (!ResourcePopup)
	{
		ConstructorHelpers::FObjectFinder<UClass> popupFinder = ConstructorHelpers::FObjectFinder<UClass>(TEXT("Class'/Game/Blueprints/UI/ResourcePopup/BP_ResourcePopup.BP_ResourcePopup_C'"));
		if (popupFinder.Succeeded())
		{
			ResourcePopup = popupFinder.Object;
		}
	}

	if (!ResourcePopupList)
	{
		ConstructorHelpers::FObjectFinder<UClass> popupFinder = ConstructorHelpers::FObjectFinder<UClass>(TEXT("Class'/Game/Blueprints/UI/ResourcePopup/BP_ResourcePopupList.BP_ResourcePopupList_C'"));
		if (popupFinder.Succeeded())
		{
			ResourcePopupList = popupFinder.Object;
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

	//OnDestroyed.AddDynamic(this, &ABuilding::OnBuildingDestroyed);

	//MarketBarrowPool = NewObject<UObjectPool>();

	//if (MarketBarrowPool)
	//{
	//	MarketBarrowPool->SetMaxPooledObjectsAmount(MaxBarrows);
	//}
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
	SetFoundationSize(Width, Height);
	GetMyPlayerController();
}

void ABuilding::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* const World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(DelayActionTimerHandle);
	}
	Super::EndPlay(EndPlayReason);
}

// Called every frame
void ABuilding::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (Data.bIsBuilt)
	{
		CallDelayAction(DeltaTime, Data.ProductionTime);
		SetActorTickEnabled(false);
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

void ABuilding::Demolish()
{
	for (auto& building : Data.PathConnections)
	{
		if (building->IsValidLowLevel())
		{
			building->Connections.Remove(this);
		}
	}

	if (GetMyPlayerController())
	{
		MyPlayerController->UpdateUpkeep(-Data.Upkeep);

		auto* popupList = SpawnResourcePopupList(FVector(0, 0, 200));

		for (auto& cost : Data.BuildCost)
		{
			// give back full cost in easy mode
			MyPlayerController->AddResource(cost);

			FString Text = TEXT("+");
			Text.AppendInt(cost.Amount);

			popupList->BPEvent_UpdateWidget(cost.Icon, FText::FromString(Text));
		}
	}

	Data.bIsBuilt = false;

	Destroy();
	ConditionalBeginDestroy();
}

void ABuilding::Destroyed()
{
	if (bUseCustomBoxCollider && OccupiedBuildSpace_Custom->IsValidLowLevel())
	{
		OccupiedBuildSpace_Custom->SetGenerateCollisionEvents(false);
		OccupiedBuildSpace_Custom->UnregisterCollider();
	}
	Super::Destroyed();
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

void ABuilding::Build()
{
	if (auto PC = Util::GetFirstEtosPlayerController(this))
	{
		PC->UpdateUpkeep(Data.Upkeep);
	}

	FVector start = GetActorLocation() - (FVector::UpVector * 100.f);
	FVector end = start + (FVector::UpVector * 300.f);
	FHitResult Hit;
	if (bUseCustomBoxCollider)
	{
		if (Util::TraceBoxForBuildings(this, start, end, OccupiedBuildSpace_Custom->Collider->GetScaledBoxExtent(), Hit, GetActorRotation()))
		{
			Demolish();
			return;
		}
	}
	else
	{
		if (Util::TraceBoxForBuildings(this, start, end, OccupiedBuildSpace->GetScaledBoxExtent(), Hit))
		{
			Demolish();
			return;
		}
	}

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

	if (bUseCustomBoxCollider)
	{
		OccupiedBuildSpace_Custom->SetGenerateCollisionEvents(false);
		OccupiedBuildSpace_Custom->SetMobilityType(EComponentMobility::Static);
	}

	OnBuild_ParticleSystem->Activate(true);

	OnBuilt.Broadcast(this);

	Data.bIsBuilt = true;
}

FResource ABuilding::HandOutResource(EResource in resource)
{
	if (EResource::None == resource)
	{
		if (Data.ProducedResource.Type != EResource::None)
		{
			FResource res = Data.ProducedResource;
			ResetStoredResources();
			return res;
		}
	}
	else if (resource == Data.ProducedResource.Type)
	{
		return HandOutResource();
	}
	return FResource();
}

void ABuilding::ReceiveResource(FResource in resource)
{
	if (Data.NeededResource1.Type == resource.Type)
	{
		Data.NeededResource1.Amount += resource.Amount;
		UKismetMathLibrary::Clamp(Data.NeededResource1.Amount, 0, Data.MaxStoredResources);
	}
	else if (Data.NeededResource2.Type == resource.Type)
	{
		Data.NeededResource2.Amount += resource.Amount;
		UKismetMathLibrary::Clamp(Data.NeededResource2.Amount, 0, Data.MaxStoredResources);
	}
}

void ABuilding::ResetStoredResources()
{
	Data.ProducedResource.Amount = 0;
}

void ABuilding::SetFoundationSize(int32 in width, int32 in height)
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

int32 ABuilding::GetBarrowsInUse()
{
	return BarrowsInUse;
}

FORCEINLINE bool ABuilding::operator<(ABuilding in B) const
{
	return Data.ProducedResource.Amount < B.Data.ProducedResource.Amount;
}

inline void ABuilding::InitOccupiedBuildSpace()
{
	OccupiedBuildSpace = CreateDefaultSubobject<UBoxComponent>(TEXT("OccupiedBuildSpace"));
	OccupiedBuildSpace->SetupAttachment(RootComponent);

	InitOccupiedBuildSpace_Internal(OccupiedBuildSpace);

	OccupiedBuildSpace->OnComponentBeginOverlap.AddDynamic(this, &ABuilding::BuildSpace_OnBeginOverlap);
	OccupiedBuildSpace->OnComponentEndOverlap.AddDynamic(this, &ABuilding::BuildSpace_OnEndOverlap);

	checkf(OccupiedBuildSpace, TEXT("OccupiedBuildSpace should not be null actually"));
}

inline void ABuilding::InitOccupiedBuildSpace_Custom()
{
	checkf(!OccupiedBuildSpace_Custom, TEXT("OccupiedBuildSpace_Custom should be null actually"));

	OccupiedBuildSpace_Custom = CreateDefaultSubobject<UBoxCollider>(TEXT("OccupiedBuildSpace"));

	checkf(OccupiedBuildSpace_Custom, TEXT("OccupiedBuildSpace_Custom should not be null actually"));

	OccupiedBuildSpace_Custom->SetupAttachment(RootComponent);

	InitOccupiedBuildSpace_Internal(OccupiedBuildSpace_Custom->Collider);

	OccupiedBuildSpace_Custom->OnTriggerEnter.AddDynamic(this, &ABuilding::BuildSpace_OnBeginOverlap_Custom);
	OccupiedBuildSpace_Custom->OnTriggerExit.AddDynamic(this, &ABuilding::BuildSpace_OnEndOverlap_Custom);

	checkf(OccupiedBuildSpace_Custom, TEXT("OccupiedBuildSpace_Custom should not be null actually"));
}

inline void ABuilding::InitOccupiedBuildSpace_Internal(UBoxComponent * collider)
{
	checkf(collider, TEXT("@param Collider should not be null"));

	collider->SetRelativeLocation(FVector(0, 0, 50));
	collider->SetBoxExtent(FVector(49));
	collider->SetVisibility(true);
	collider->bHiddenInGame = true;
	collider->SetEnableGravity(false);
	collider->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	collider->SetCollisionObjectType(ECollisionChannel::ECC_GameTraceChannel1 /*Buliding*/);
	collider->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
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
				point->SetupAttachment(OccupiedBuildSpace_Custom->Collider);
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
	FVector BoxExtend;

	if (bUseCustomBoxCollider)
	{
		checkf(OccupiedBuildSpace_Custom, TEXT("OccupiedBuildSpace should not be null actually"));
		checkf(OccupiedBuildSpace_Custom->Collider, TEXT("Collider should not be null actually"));

		SetFoundationSize(Width, Height);
		BoxExtend = OccupiedBuildSpace_Custom->Collider->GetScaledBoxExtent();
	}
	else
	{
		checkf(OccupiedBuildSpace, TEXT("OccupiedBuildSpace should not be null actually"));

		SetFoundationSize(Width, Height);
		BoxExtend = OccupiedBuildSpace->GetScaledBoxExtent();
	}

	float heightOffset = 25;
	FVector Start;
	FVector End;
	//top
	UBuildingFunctionLibrary::CalcVectors(BoxExtend.X, BoxExtend.Y, BoxExtend.Z, true, true, false, true, EOffsetDirections::TopRight, EOffsetDirections::TopLeft, Start, End, heightOffset);
	TracePoints[0]->SetRelativeLocation(Start);
	TracePoints[1]->SetRelativeLocation(End);
	//bot
	UBuildingFunctionLibrary::CalcVectors(BoxExtend.X, BoxExtend.Y, BoxExtend.Z, true, false, false, false, EOffsetDirections::BotRight, EOffsetDirections::BotLeft, Start, End, heightOffset);
	TracePoints[2]->SetRelativeLocation(Start);
	TracePoints[3]->SetRelativeLocation(End);
	//left
	UBuildingFunctionLibrary::CalcVectors(BoxExtend.X, BoxExtend.Y, BoxExtend.Z, true, true, true, false, EOffsetDirections::BotLeft, EOffsetDirections::TopLeft, Start, End, heightOffset);
	TracePoints[4]->SetRelativeLocation(Start);
	TracePoints[5]->SetRelativeLocation(End);
	//right
	UBuildingFunctionLibrary::CalcVectors(BoxExtend.X, BoxExtend.Y, BoxExtend.Z, false, true, false, false, EOffsetDirections::BotRight, EOffsetDirections::TopRight, Start, End, heightOffset);
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
	if (Action.IsBound())
	{
		Action.Unbind();
	}

	Action.BindDynamic(this, &ABuilding::AddResource);
}

void ABuilding::SpendUpkeep(float in DeltaTime)
{
	if (Data.Upkeep <= 0)
		return;

	if (MyPlayerController)
	{
		currentUpkeepDebts += Data.Upkeep * (DeltaTime / 60); // save the fragment of the upkeep
		int32 upkeep = currentUpkeepDebts; // floor the upkeep
		if (upkeep >= 1) // if upkeep is at least 1 full coin...
		{
			MyPlayerController->RemoveResource(FResource(EResource::Money, upkeep)); // ... spend it ...
			currentUpkeepDebts -= upkeep; // ... and update the fragmented debt
		}
	}
	else
	{
		GetMyPlayerController();
	}
}

void ABuilding::SendMarketBarrow_Internal(ABuilding* targetBuilding, EResource in orderedResource, FVector in spawnLocation, FVector in targetLocation)
{
	bool isValid = false;
	UPROPERTY()
		AMarketBarrow* newMarketBarrow = nullptr;

	//if (MarketBarrowPool)
	//{
	//	newMarketBarrow = MarketBarrowPool->GetPooledObject<AMarketBarrow*>(isValid);
	//}

	if (newMarketBarrow && isValid)
	{
		newMarketBarrow->ResetBarrow(
			spawnLocation, // spawn location
			targetLocation, // target location
			this, // workplace
			targetBuilding, // target
			orderedResource);

		newMarketBarrow->StartWork();
	}
	else
	{
		FActorSpawnParameters params = FActorSpawnParameters();
		params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		newMarketBarrow = AMarketBarrow::Construct(
			this, // world context 
			BP_MarketBarrow, // class to spawn
			spawnLocation, // spawn location
			targetLocation, // target location
			this, // workplace
			targetBuilding, // target
			orderedResource,
			FRotator(0, 0, 0),
			params);
	}

	if (newMarketBarrow && newMarketBarrow->IsValidLowLevel())
	{
		++BarrowsInUse;
	}
}

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
		if (!other->IsValidLowLevel())
			return;

		UE_LOG(LogTemp, Warning, TEXT("%s collided with %s"), *GetName(), *other->GetAttachmentRootActor()->GetName());

		Data.bPositionIsBlocked = true;

		++collisionCount;

		UE_LOG(LogTemp, Warning, TEXT("There are %i collisions"), collisionCount);

	}
}

void ABuilding::BuildSpace_OnEndOverlap_Custom(UBoxCollider* other)
{
	if (bMovedOnce)
	{
		if (!other->IsValidLowLevel())
			return;

		UE_LOG(LogTemp, Warning, TEXT("%s and %s are no more colliding"), *GetName(), *other->GetAttachmentRootActor()->GetName());

		if (--collisionCount <= 0)
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
			if (this->Data.bIsHeld)
			{
				this->Data.PossibleBuildingsInRadius.AddUnique(building);
			}
		}
		else if (building->Data.bIsHeld)
		{
			if (this->Data.bIsBuilt)
			{
				building->OnBuilt.AddDynamic(this, &ABuilding::AddNewBuildingInRange);
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
			if (this->Data.bIsHeld)
			{
				this->Data.PossibleBuildingsInRadius.Remove(building);
			}
		}
		else if (building->Data.bIsHeld)
		{
			if (this->Data.bIsBuilt)
			{
				building->OnBuilt.RemoveDynamic(this, &ABuilding::AddNewBuildingInRange);
			}
		}
	}
}

void ABuilding::AddResource()
{
	GetNeededResources();

	if (Data.ProducedResource.Amount < Data.MaxStoredResources)
	{
		if (HasNeededResources())
		{
			RemoveANeededResource();
			Data.ProducedResource.Amount++;
			SpawnResourcePopup();
		}
	}
}

TArray<ABuilding*> ABuilding::GetBuildingsInRange()
{
	FVector Location = this->GetActorLocation();

	TArray<FHitResult> HitResults;

	TArray<ABuilding*> BuildingsInRange = TArray<ABuilding*>();

	if (UKismetSystemLibrary::SphereTraceMultiForObjects(this, Location, Location, Data.Radius, Util::BuildingObjectType, true, TArray<AActor*>(), EDrawDebugTrace::None, HitResults, true))
	{
		for (FHitResult hit : HitResults)
		{
			if (dynamic_cast<APath*, AActor>(&*hit.Actor))
				continue;

			ABuilding * const building = dynamic_cast<ABuilding*, AActor>(&*hit.Actor);
			if (building && building->IsValidLowLevel() && building->Data.bIsBuilt)
			{
				BuildingsInRange.Add(building);
			}
		}
	}

	return BuildingsInRange;
}

inline void ABuilding::CallDelayAction(float in pastTime, float in delayDuration)
{
	if (delayDuration <= 0)
		return;

	if (UWorld* const World = GetWorld())
	{
		World->GetTimerManager().SetTimer(DelayActionTimerHandle, Action, delayDuration, true);
	}

	//this->pastDelayTimerTime += pastTime;

	//if (this->pastDelayTimerTime >= delayDuration)
	//{
	//	this->pastDelayTimerTime = 0;
	//	this->Action.ExecuteIfBound();
	//}
}

void ABuilding::SpawnResourcePopup(FVector in offset)
{
	if (auto popup = SpawnResourcePopup_Internal(offset))
	{
		popup->BPEvent_UpdateWidget(Data.ProducedResource.Icon);
	}
}

void ABuilding::SpawnResourcePopup(FVector in offset, UTexture2D * ResourceIcon, FText in Text, FLinearColor in TextColor)
{
	if (auto popup = SpawnResourcePopup_Internal(offset))
	{
		popup->BPEvent_UpdateWidget(ResourceIcon, Text, FSlateColor(TextColor));
	}
}

AResourcePopup * ABuilding::SpawnResourcePopupList(FVector in Offset)
{
	if (ResourcePopupList)
	{
		return SpawnResourcePopup_Internal(Offset, ResourcePopupList);
	}
	return nullptr;
}

FORCEINLINE AEtosPlayerController * ABuilding::GetMyPlayerController()
{
	if (!MyPlayerController)
	{
		MyPlayerController = (AEtosPlayerController*)GetWorld()->GetFirstPlayerController();
	}
	return MyPlayerController;
}

void ABuilding::RefreshBuildingsInRadius(bool useSphereTrace)
{
	TArray<ABuilding*> OverlappingBulidings;

	if (useSphereTrace)
	{
		OverlappingBulidings = GetBuildingsInRange();
	}
	else
	{
		GetOverlappingBulidings(OverlappingBulidings);
	}

	if (Data.BuildingsInRadius.Num() != OverlappingBulidings.Num())
	{
		Data.BuildingsInRadius = OverlappingBulidings;
	}
}

FVector2Di ABuilding::GetRotatedSize()
{
	FRotator rotation = GetActorRotation();
	return FMath::Abs(FMath::RoundToInt(rotation.Yaw) % 180) == 90 ? FVector2Di(Height, Width) : FVector2Di(Width, Height);
}

void ABuilding::OnBuildingDestroyed(AActor * DestroyedActor)
{
	if (this == DestroyedActor)
	{
		if (GetMyPlayerController())
		{
			MyPlayerController->ReportDestroyedBuilding(this);
		}
	}
}

void ABuilding::AddNewBuildingInRange(ABuilding * buildingInRange)
{
	if (buildingInRange->IsValidLowLevel())
	{
		Data.BuildingsInRadius.AddUnique(buildingInRange);
	}
}

bool ABuilding::HasNeededResources()
{
	if (Data.NeededResource1.Type != EResource::None)
	{// need res1
		if (Data.NeededResource2.Type == EResource::None)
		{// dont need res2
			if (Data.NeededResource1.Amount > 0)
			{// only need res1 and have res1
				return true;
			}
			else
			{// only need res1 but don't have res1
				return false;
			}
		}
		else if (Data.NeededResource1.Amount > 0 && Data.NeededResource2.Amount > 0)
		{// need res 1 and res2 and have both
			return true;
		}
		else
		{// need res 1 and res2 but don't have one or both of them
			return false;
		}
	}
	else if (Data.NeededResource2.Type != EResource::None)
	{// only need res2
		if (Data.NeededResource2.Amount > 0)
		{// need res2 and have res2
			return true;
		}
		else
		{// need res2 but don't have res2
			return false;
		}
	}

	// don't need res1 and res2
	return true;
}

void ABuilding::RemoveANeededResource()
{
	if (Data.NeededResource1.Type != EResource::None && Data.NeededResource1.Amount > 0)
	{
		--Data.NeededResource1.Amount;
	}
	if (Data.NeededResource2.Type != EResource::None && Data.NeededResource2.Amount > 0)
	{
		--Data.NeededResource2.Amount;
	}
}

void ABuilding::GetNeededResources()
{
	if (Data.NeededResource1.Amount < 1 || Data.NeededResource2.Amount < 1)
	{
		if (BP_MarketBarrow)
		{
			RefreshBuildingsInRadius(false);

			Data.BuildingsInRadius.Sort([](ABuilding in A, ABuilding in B)
			{
				return A.Data.ProducedResource.Amount > B.Data.ProducedResource.Amount;
			});

			for (ABuilding* const building : Data.BuildingsInRadius)
			{
				if (BarrowsInUse < MaxBarrows)
				{
					if (building && HasNeededResources(building))
					{
						if (!building->Data.bBarrowIsOnTheWay)
						{
							APath* start; APath* goal;
							if (BFuncs::FindPath(this, building, start, goal))
							{
								if (start && goal)
								{
									EResource orderedResource = EResource::None;
									DetermineOrderedResource(orderedResource, building);

									if (orderedResource != EResource::None)
									{
										SendMarketBarrow_Internal(
											building, // target
											orderedResource,
											start->GetActorLocation() + FVector(0, 0, 100), // spawn location
											goal->GetActorLocation()); // target location
									}
								}
							}
						}
					}
				} // (BarrowsInUse < MaxBarrows)
				else
				{
					break;
				}
			}
		}
	}
}

void ABuilding::MoveToMouseLocation()
{
	FHitResult Hit;
	if (Util::TraceSingleAtMousePosition(this, Hit))
	{
		SetActorLocation(BFuncs::GetNextGridLocation(Hit.ImpactPoint, GetRotatedSize()));
		bMovedOnce = true;
	}
}

void ABuilding::DetermineOrderedResource(EResource out OrderedResource, ABuilding* TargetBuilding)
{
	OrderedResource = EResource::None;
	FResource res1 = Data.NeededResource1;
	FResource res2 = Data.NeededResource2;

	if (res1.Type != EResource::None && res1.Amount < 1 && TargetBuilding->HasResource(res1.Type))
	{
		OrderedResource = res1.Type;
	}
	else if (res2.Type != EResource::None && res2.Amount < 1 && TargetBuilding->HasResource(res2.Type))
	{
		OrderedResource = res2.Type;
	}
}

AResourcePopup* ABuilding::SpawnResourcePopup_Internal(FVector in Offset, TSubclassOf<AResourcePopup> in AlternativeBlueprint)
{
	if (AlternativeBlueprint || ResourcePopup)
	{
		if (UWorld* const World = GetWorld())
		{
			FActorSpawnParameters params = FActorSpawnParameters();
			params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			if (APlayerCameraManager* const PlayerCameraManager = Util::GetFirstEtosPlayerController(this)->PlayerCameraManager)
			{
				FVector cameraLocation = PlayerCameraManager->GetCameraLocation();

				FRotator rotation = UKismetMathLibrary::FindLookAtRotation(cameraLocation + PlayerCameraManager->GetCameraRotation().Vector(), cameraLocation);

				return World->SpawnActor<AResourcePopup>(AlternativeBlueprint ? AlternativeBlueprint : ResourcePopup, GetActorLocation() + Offset, rotation, params);
			}
		}
	}
	return nullptr;
}

void ABuilding::SetBarrowsInUse(int32 in amount)
{
	BarrowsInUse = amount;
}

FORCEINLINE void ABuilding::DecreaseBarrowsInUse()
{
	--BarrowsInUse;
}

bool ABuilding::HasNeededResources(ABuilding * other)
{
	if (dynamic_cast<AWarehouse*, AActor>(other))
	{
		if (GetMyPlayerController())
		{
			return (Data.NeededResource1.Amount < 1 && MyPlayerController->GetResourceAmount(Data.NeededResource1.Type) >= 5)
				|| (Data.NeededResource2.Amount < 1 && MyPlayerController->GetResourceAmount(Data.NeededResource2.Type) >= 5);
		}
		else
		{
			return false;
		}
	}

	return ((Data.NeededResource1.Amount < 1 && other->Data.ProducedResource.Type == Data.NeededResource1.Type)
		|| (Data.NeededResource2.Amount < 1 && other->Data.ProducedResource.Type == Data.NeededResource2.Type))
		&& other->Data.ProducedResource.Amount >= 5;
}

bool ABuilding::HasResource(ABuilding * other, EResource in resource)
{
	return other->Data.ProducedResource.Type == resource && other->Data.ProducedResource.Amount > 0;
}

bool ABuilding::HasResource(EResource in resource)
{
	return HasResource(this, resource);
}

bool ABuilding::IsActive()
{
	return bIsActive;
}

void ABuilding::SetActive(bool in isActive)
{
	bIsActive = isActive;
	SetActorHiddenInGame(!bIsActive);
	SetActorEnableCollision(bIsActive);
	Data.bIsBuilt = bIsActive;

	if (bUseCustomBoxCollider)
	{
		if (OccupiedBuildSpace_Custom)
		{
			OccupiedBuildSpace_Custom->SetGenerateCollisionEvents(bIsActive);
			bIsActive ?
				OccupiedBuildSpace_Custom->RegisterCollider() :
				OccupiedBuildSpace_Custom->UnregisterCollider();
		}
	}
}

bool ABuilding::TryReturningToPool(AMarketBarrow * barrow)
{
	//if (MarketBarrowPool)
	//{
	//	return MarketBarrowPool->AddObjectToPool(barrow);;
	//}
	return false;
}

void ABuilding::GetOverlappingBulidings(TArray<ABuilding*> out OverlappingBuildings)
{
	TArray<AActor*> actors;
	Radius->GetOverlappingActors(actors, TSubclassOf<ABuilding>());

	actors.RemoveAll([](AActor* actor) { return dynamic_cast<APath*, AActor>(actor) != nullptr; });

	for (AActor* building : actors)
	{
		if (this == building)
			continue;

		OverlappingBuildings.Add((ABuilding*)building);
	}
}

void ABuilding::TestFunction()
{
	UE_LOG(LogTemp, Warning, TEXT("I was called on %s"), *GetName());
}
