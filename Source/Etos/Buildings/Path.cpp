// © 2016 - 2017 Daniel Bortfeld

#include "Etos.h"
#include "Path.h"
#include "Etos/Collision/BoxCollider.h"

APath::APath()
{
	FoundationMesh->SetCanEverAffectNavigation(false);
}

void APath::Build()
{
	FVector start = GetActorLocation();
	FVector end = start + (FVector::UpVector * 10000.f);
	FHitResult Hit;
	if (Util::TraceSingleForBuildings(this, start, end, Hit))
	{
		Demolish();
		return;
	}

	FoundationMesh->SetCanEverAffectNavigation(true);
	
	Connections = PossibleConnections;
	PossibleConnections.Empty();

	for (ABuilding* building : Connections)
	{
		APath* path = dynamic_cast<APath*, ABuilding>(building);
		if (path)
		{
			path->Connections.AddUnique(this);
		}
		else if (building)
		{
			building->Data.PathConnections.AddUnique(this);
		}
	}

	if (bUseCustomBoxCollider)
	{
		OccupiedBuildSpace_Custom->SetGenerateCollisionEvents(false);
		OccupiedBuildSpace_Custom->SetMobilityType(EComponentMobility::Static);
	}

	OnBuilt.Broadcast(this);

	Data.bIsBuilt = true;
}

void APath::ReconnectToSurroundings()
{
	GetSurroundingBuildings();
	Build();
}

void APath::CreateTracePoints()
{
	if (UWorld* World = GetWorld())
	{
		TracePoints.Add(NewObject<USceneComponent>(this, USceneComponent::StaticClass(), TEXT("Trace Start")));
		TracePoints.Add(NewObject<USceneComponent>(this, USceneComponent::StaticClass(), TEXT("Trace Point Top")));
		TracePoints.Add(NewObject<USceneComponent>(this, USceneComponent::StaticClass(), TEXT("Trace Point Bot")));
		TracePoints.Add(NewObject<USceneComponent>(this, USceneComponent::StaticClass(), TEXT("Trace Point Left")));
		TracePoints.Add(NewObject<USceneComponent>(this, USceneComponent::StaticClass(), TEXT("Trace Point Right")));

		for (USceneComponent* point : TracePoints)
		{
			if (bUseCustomBoxCollider)
			{
				point->SetupAttachment((USceneComponent*)OccupiedBuildSpace_Custom);
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

void APath::RelocateTracePoints()
{
	float height = 25;
	float offset = 100;

	FVector location = FVector(0, 0, height);
	TracePoints[0]->SetRelativeLocation(location);

	location = FVector(offset, 0, height);
	TracePoints[1]->SetRelativeLocation(location);

	location = FVector(-offset, 0, height);
	TracePoints[2]->SetRelativeLocation(location);

	location = FVector(0, offset, height);
	TracePoints[3]->SetRelativeLocation(location);

	location = FVector(0, -offset, height);
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
		Util::TraceSingleForBuildings(this, TracePoints[0]->GetComponentLocation(), TracePoints[i]->GetComponentLocation(), HitResult);
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
