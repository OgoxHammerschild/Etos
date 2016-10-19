// Fill out your copyright notice in the Description page of Project Settings.

#include "Etos.h"
#include "EtosGameMode.h"

 FPredefinedBuildingData* AEtosGameMode::GetPredefinedBuildingData(int32 buildingID)
{
	if (!PredefinedBuildingData)
	{
		static ConstructorHelpers::FObjectFinder<UDataTable> BP_PredefinedBuildingData(TEXT("DataTable'/Game/Blueprints/GameMode/PredefinedBuildingData.PredefinedBuildingData'"));
		PredefinedBuildingData = BP_PredefinedBuildingData.Object;
	}

	check(PredefinedBuildingData);

	// DataTable.h:97 variable swap error

	//if(buildingID is valid)
	{
		FPredefinedBuildingData* preDefData = PredefinedBuildingData->FindRow<FPredefinedBuildingData>(*FString::FromInt(buildingID), FString(TEXT("GameMode")));

		check(preDefData);

		return preDefData;
	}
}
