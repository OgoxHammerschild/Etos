// Fill out your copyright notice in the Description page of Project Settings.

#include "Etos.h"
#include "Path.h"

void APath::OnConstruction(const FTransform& Transform)
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

void APath::InitTracePoints()
{
	TracePoints.Add(CreateDefaultSubobject<USceneComponent>(TEXT("Trace Start")));
	TracePoints.Add(CreateDefaultSubobject<USceneComponent>(TEXT("Trace Point Top")));
	TracePoints.Add(CreateDefaultSubobject<USceneComponent>(TEXT("Trace Point Bot")));
	TracePoints.Add(CreateDefaultSubobject<USceneComponent>(TEXT("Trace Point Left")));
	TracePoints.Add(CreateDefaultSubobject<USceneComponent>(TEXT("Trace Point Right")));

	for (USceneComponent* point : TracePoints)
	{
		point->SetupAttachment(OccupiedBuildSpace);
		point->SetVisibility(false);
	}
}
