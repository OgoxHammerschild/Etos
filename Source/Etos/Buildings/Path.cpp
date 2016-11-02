// © 2016 - 2017 Daniel Bortfeld

#include "Etos.h"
#include "Path.h"

APath::APath()
{
	FoundationMesh->SetCanEverAffectNavigation(true);
}

//void APath::OnConstruction(const FTransform& Transform)
//{
//	PossibleConnections.Empty(4);
//	TArray<FHitResult> AllHitResults;
//	AllHitResults.Reserve(4);
//
//	FHitResult HitResult;
//
//	for (int32 i = 1; i < 5; i++)
//	{
//		TraceSingleForBuildings(TracePoints[0]->GetComponentLocation(), TracePoints[i]->GetComponentLocation(), HitResult);
//		AllHitResults.Add(HitResult);
//	}
//
//	for (FHitResult hit : AllHitResults)
//	{
//		ABuilding* building = dynamic_cast<ABuilding*, AActor> (&*hit.Actor);
//		if (building)
//		{
//			PossibleConnections.AddUnique(building);
//		}
//	}
//}

void APath::OnBuild()
{
	Connections = PossibleConnections;
	PossibleConnections.Empty();

	for (ABuilding* building : Connections)
	{
		APath* path = dynamic_cast<APath*, ABuilding>(building);
		if (path)
		{
			path->Connections.Add(this);
		}
		else if (building)
		{
			building->Data.PathConnections.Add(this);
		}
	}

	PrimaryActorTick.bCanEverTick = false;
	Data.bIsBuilt = true;
}

void APath::CreateTracePoints()
{
	TracePoints.Add(NewObject<USceneComponent>(this, USceneComponent::StaticClass(), TEXT("Trace Start")));
	TracePoints.Add(NewObject<USceneComponent>(this, USceneComponent::StaticClass(), TEXT("Trace Point Top")));
	TracePoints.Add(NewObject<USceneComponent>(this, USceneComponent::StaticClass(), TEXT("Trace Point Bot")));
	TracePoints.Add(NewObject<USceneComponent>(this, USceneComponent::StaticClass(), TEXT("Trace Point Left")));
	TracePoints.Add(NewObject<USceneComponent>(this, USceneComponent::StaticClass(), TEXT("Trace Point Right")));

	for (USceneComponent* point : TracePoints)
	{
		point->SetupAttachment(OccupiedBuildSpace);
		point->SetVisibility(false);
	}
}

void APath::RelocateTracePoints()
{
	float hight = 0;
	float offset = 100;

	FVector location = FVector(0, 0, hight);
	TracePoints[0]->SetRelativeLocation(location);

	location = FVector(offset, 0, hight);
	TracePoints[1]->SetRelativeLocation(location);

	location = FVector(-offset, 0, hight);
	TracePoints[2]->SetRelativeLocation(location);

	location = FVector(0, offset, hight);
	TracePoints[3]->SetRelativeLocation(location);

	location = FVector(0, -offset, hight);
	TracePoints[4]->SetRelativeLocation(location);
}

void APath::GetSurroundingBuildings()
{
	PossibleConnections.Empty(4);
	TArray<FHitResult> AllHitResults;
	AllHitResults.Reserve(4);

	FHitResult HitResult;

	for (int32 i = 1; i < 5; i++)
	{
		TraceSingleForBuildings(TracePoints[0]->GetComponentLocation(), TracePoints[i]->GetComponentLocation(), HitResult);
		AllHitResults.Add(HitResult);
	}

	for (FHitResult hit : AllHitResults)
	{
		ABuilding* building = dynamic_cast<ABuilding*, AActor> (&*hit.Actor);
		if (building)
		{
			PossibleConnections.AddUnique(building);
		}
	}
}
