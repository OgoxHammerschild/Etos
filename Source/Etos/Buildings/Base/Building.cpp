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
	ConstructorHelpers::FObjectFinder<UStaticMesh> quadFinder = ConstructorHelpers::FObjectFinder<UStaticMesh>(TEXT("StaticMesh'/Game/BasicMeshes/SM_Quad_1x1m.SM_Quad_1x1m'"));
	if (quadFinder.Succeeded())
	{
		FoundationMesh->SetStaticMesh(quadFinder.Object);
	}

	InitOccupiedBuildSpace();
	InitTracePoints();
	BindDelayAction();
	SetFoundationSize(1, 1);
}

// Called when the game starts or when spawned
void ABuilding::BeginPlay()
{
	Super::BeginPlay();

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

// Called every frame
void ABuilding::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (Data.bIsBuilt)
	{
		CallActionDelayed(DeltaTime, Data.ProductionTime);
	}
	else if (Data.bIsHeld)
	{
		MoveToMouseLocation();
	}
}

void ABuilding::OnConstruction(const FTransform& Transform)
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

	for (APath* path : Data.PathConnections)
	{
		if (path)
		{
			path->Connections.Add(this);
		}
	}

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

void ABuilding::InitTracePoints()
{
	TracePoints.Add(CreateDefaultSubobject<USceneComponent>(TEXT("Trace Start Top")));
	TracePoints.Add(CreateDefaultSubobject<USceneComponent>(TEXT("Trace End Top")));
	TracePoints.Add(CreateDefaultSubobject<USceneComponent>(TEXT("Trace Start Bot")));
	TracePoints.Add(CreateDefaultSubobject<USceneComponent>(TEXT("Trace End Bot")));
	TracePoints.Add(CreateDefaultSubobject<USceneComponent>(TEXT("Trace Start Left")));
	TracePoints.Add(CreateDefaultSubobject<USceneComponent>(TEXT("Trace End Left")));
	TracePoints.Add(CreateDefaultSubobject<USceneComponent>(TEXT("Trace Start Right")));
	TracePoints.Add(CreateDefaultSubobject<USceneComponent>(TEXT("Trace End Right")));

	for (USceneComponent* point : TracePoints)
	{
		point->SetupAttachment(OccupiedBuildSpace);
		point->SetVisibility(false);
	}
}

void ABuilding::BindDelayAction()
{
	Action.BindUFunction(this, TEXT("AddResource")); // reflection is probably fine
}

void ABuilding::BuildSpace_OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	Data.bPositionIsBlocked = true;
}

void ABuilding::BuildSpace_OnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	Data.bPositionIsBlocked = false;
}

void ABuilding::AddResource()
{
	if (Data.ProducedResource.Amount < Data.MaxStoredResources)
	{
		Data.ProducedResource.Amount++;
	}
}

bool ABuilding::TraceSingleForBuildings(FVector Start, FVector End, FHitResult& HitResult)
{
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(EObjectTypeQuery::ObjectTypeQuery1);//Building

	return UKismetSystemLibrary::LineTraceSingleForObjects(this, Start, End, ObjectTypes, false, TArray<AActor*>(), EDrawDebugTrace::None, HitResult, true);
}

//Daniel'sche Trace-Funktion
bool ABuilding::TraceMultiForBuildings(FVector Start, FVector End, TArray<FHitResult>& HitResults)
{
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(EObjectTypeQuery::ObjectTypeQuery1);//Building

	return UKismetSystemLibrary::LineTraceMultiForObjects(this, Start, End, ObjectTypes, false, TArray<AActor*>(), EDrawDebugTrace::None, HitResults, true);
}

bool ABuilding::TraceSingleForFloor(FVector Start, FVector End, FHitResult & Hit)
{
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(EObjectTypeQuery::ObjectTypeQuery2);//Floor

	return UKismetSystemLibrary::LineTraceSingleForObjects(this, Start, End, ObjectTypes, false, TArray<AActor*>(), EDrawDebugTrace::None, Hit, true);
}

void ABuilding::CallActionDelayed(float pastTime, float delayDuration)
{
	pastDelayTimerTime += pastTime;
	if (pastDelayTimerTime >= delayDuration)
	{
		pastDelayTimerTime = 0;
		Action.ExecuteIfBound();
	}
}

void ABuilding::MoveToMouseLocation()
{
	FVector MouseLocation;
	FVector MouseDirection;
	GetWorld()->GetFirstPlayerController()->DeprojectMousePositionToWorld(MouseLocation, MouseDirection);
	MouseDirection *= 100000;

	FHitResult Hit;
	TraceSingleForFloor(MouseLocation, MouseLocation + MouseDirection, Hit);

	float X = UKismetMathLibrary::Round(Hit.ImpactPoint.X / 100) * 100;
	float Y = UKismetMathLibrary::Round(Hit.ImpactPoint.Y / 100) * 100;
	float Z = Hit.ImpactPoint.Z;

	if (Width % 2 != 0)
	{
		X += 50;
	}

	if (Height % 2 != 0)
	{
		Y += 50;
	}

	SetActorLocation(FVector(X, Y, Z));
}
