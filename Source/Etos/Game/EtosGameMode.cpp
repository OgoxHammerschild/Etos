// © 2016 - 2017 Daniel Bortfeld

#include "Etos.h"
#include "EtosGameMode.h"

AEtosGameMode::AEtosGameMode()
{
	if (!PredefinedBuildingData)
	{
		static ConstructorHelpers::FObjectFinder<UDataTable> BP_PredefinedBuildingData(TEXT("DataTable'/Game/Blueprints/GameMode/PredefinedBuildingData.PredefinedBuildingData'"));
		PredefinedBuildingData = BP_PredefinedBuildingData.Object;
	}
}

void AEtosGameMode::StartPlay()
{
	if (UWorld* const World = GetWorld())
	{
		FActorSpawnParameters params = FActorSpawnParameters();
		params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		CollisionManager = World->SpawnActor<ASimpleCollisionManager>(params);
	}

	Super::StartPlay();
}

void AEtosGameMode::BeginDestroy()
{
	PredefinedBuildingData = nullptr;
	CollisionManager = nullptr;

	Super::BeginDestroy();
}

FPredefinedBuildingData* AEtosGameMode::GetPredefinedBuildingData(int32 buildingID)
{
	check(PredefinedBuildingData);

	// DataTable.h:97 variable swap error
	// UObjectGlobals.h typo in NewObject assert

	//if(buildingID is valid)
	//{
	FPredefinedBuildingData* preDefData = PredefinedBuildingData->FindRow<FPredefinedBuildingData>(*FString::FromInt(buildingID), FString(TEXT("GameMode")));

	return preDefData;
	//}
}
