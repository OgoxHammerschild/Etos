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

FPredefinedBuildingData* AEtosGameMode::GetPredefinedBuildingData(int32 buildingID)
{
	check(PredefinedBuildingData);

	// DataTable.h:97 variable swap error

	//if(buildingID is valid)
	//{
	FPredefinedBuildingData* preDefData = PredefinedBuildingData->FindRow<FPredefinedBuildingData>(*FString::FromInt(buildingID), FString(TEXT("GameMode")));

	return preDefData;
	//}
}
