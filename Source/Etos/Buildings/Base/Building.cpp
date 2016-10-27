// Fill out your copyright notice in the Description page of Project Settings.

#include "Etos.h"
#include "Building.h"
#include "Etos/FunctionLibraries/BuildingFunctionLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Etos/Buildings/Path.h"

// Sets default values
ABuilding::ABuilding()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent->SetMobility(EComponentMobility::Movable);

	BuildingMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Building Mesh"));
	BuildingMesh->SetupAttachment(RootComponent);

	FoundationMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Foundation"));
	FoundationMesh->SetupAttachment(RootComponent);
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
	ReloacteTracePoints();
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

void ABuilding::Destroyed()
{
	Super::Destroyed();
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
	OccupiedBuildSpace->SetBoxExtent(FVector(halfTileSize * width - 1, halfTileSize * height - 1, halfTileSize - 1));
}

void ABuilding::InitOccupiedBuildSpace()
{
	OccupiedBuildSpace = CreateDefaultSubobject<UBoxComponent>(TEXT("Occupied Build Space"));
	OccupiedBuildSpace->SetupAttachment(RootComponent);
	OccupiedBuildSpace->SetRelativeLocation(FVector(0, 0, 50));
	OccupiedBuildSpace->SetBoxExtent(FVector(49));
	OccupiedBuildSpace->SetVisibility(true);
	OccupiedBuildSpace->bHiddenInGame = true;
	OccupiedBuildSpace->SetEnableGravity(false);
	OccupiedBuildSpace->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	OccupiedBuildSpace->SetCollisionObjectType(ECollisionChannel::ECC_GameTraceChannel1 /*Buliding*/);
	OccupiedBuildSpace->OnComponentBeginOverlap.AddDynamic(this, &ABuilding::BuildSpace_OnBeginOverlap);
	OccupiedBuildSpace->OnComponentEndOverlap.AddDynamic(this, &ABuilding::BuildSpace_OnEndOverlap);
}

void ABuilding::CreateTracePoints()
{
	TracePoints.Add(NewObject<USceneComponent>(this, TEXT("Trace Start Top")));
	TracePoints.Add(NewObject<USceneComponent>(this, TEXT("Trace End Top")));
	TracePoints.Add(NewObject<USceneComponent>(this, TEXT("Trace Start Bot")));
	TracePoints.Add(NewObject<USceneComponent>(this, TEXT("Trace End Bot")));
	TracePoints.Add(NewObject<USceneComponent>(this, TEXT("Trace Start Left")));
	TracePoints.Add(NewObject<USceneComponent>(this, TEXT("Trace End Left")));
	TracePoints.Add(NewObject<USceneComponent>(this, TEXT("Trace Start Right")));
	TracePoints.Add(NewObject<USceneComponent>(this, TEXT("Trace End Right")));

	for (USceneComponent* point : TracePoints)
	{
		point->SetupAttachment(OccupiedBuildSpace);
		point->SetVisibility(false);
	}
}

void ABuilding::ReloacteTracePoints()
{
	FVector BoxExtend = OccupiedBuildSpace->GetScaledBoxExtent();
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
		TraceMultiForBuildings(TracePoints[i]->GetComponentLocation(), TracePoints[1 + i]->GetComponentLocation(), HitResults);
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

void ABuilding::BuildSpace_OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	Data.bPositionIsBlocked = true;
}

void ABuilding::BuildSpace_OnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	Data.bPositionIsBlocked = false;
}

void ABuilding::BuildingEnteredRadius(UPrimitiveComponent * OverlappedComponent, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	ABuilding * building = dynamic_cast<ABuilding*, AActor>(OtherActor);
	if (building)
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
	ABuilding * building = dynamic_cast<ABuilding*, AActor>(OtherActor);
	if (building)
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

bool ABuilding::TraceSingleForBuildings(FVector Start, FVector End, FHitResult& HitResult)
{
	return UKismetSystemLibrary::LineTraceSingleForObjects(this, Start, End, Util::BuildingObjectType, false, TArray<AActor*>(), EDrawDebugTrace::ForDuration, HitResult, true);
}

bool ABuilding::TraceMultiForBuildings(FVector Start, FVector End, TArray<FHitResult>& HitResults)
{
	return UKismetSystemLibrary::LineTraceMultiForObjects(this, Start, End, Util::BuildingObjectType, false, TArray<AActor*>(), EDrawDebugTrace::None, HitResults, true);
}

bool ABuilding::TraceSingleForFloor(FVector Start, FVector End, FHitResult & Hit)
{
	return UKismetSystemLibrary::LineTraceSingleForObjects(this, Start, End, Util::FloorObjectType, false, TArray<AActor*>(), EDrawDebugTrace::None, Hit, true);
}

void ABuilding::CallDelayAction(float pastTime, float delayDuration)
{
	this->pastDelayTimerTime += pastTime;
	//UE_LOG(LogTemp, Warning, TEXT("timer: %f duration : %f delta time: %f"), this->pastDelayTimerTime, delayDuration, pastTime);
	if (this->pastDelayTimerTime >= delayDuration)
	{
		this->pastDelayTimerTime = 0;
		this->Action.ExecuteIfBound();
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
	if (UWorld* World = GetWorld())
	{
		if (APlayerController* PlayerController = World->GetFirstPlayerController())
		{
			FVector MouseLocation;
			FVector MouseDirection;
			PlayerController->DeprojectMousePositionToWorld(MouseLocation, MouseDirection);
			MouseDirection *= 100000;

			FHitResult Hit;
			if (TraceSingleForFloor(MouseLocation, MouseLocation + MouseDirection, Hit))
			{

				float X = UKismetMathLibrary::Round(Hit.ImpactPoint.X / 100) * 100;
				float Y = UKismetMathLibrary::Round(Hit.ImpactPoint.Y / 100) * 100;
				float Z = Hit.ImpactPoint.Z;

				if (Width % 2 == 0)
				{
					X += 50;
				}

				if (Height % 2 == 0)
				{
					Y += 50;
				}

				SetActorLocation(FVector(X, Y, Z));
			}
		}
	}
}
