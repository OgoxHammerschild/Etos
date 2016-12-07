// © 2016 - 2017 Daniel Bortfeld

#pragma once

#include "GameFramework/GameMode.h"
#include "Runtime/Engine/Classes/Engine/DataTable.h"
#include "Etos/Collision/SimpleCollisionManager.h"
#include "Etos/FunctionLibraries/EnumLibrary.h"
#include "EtosGameMode.generated.h"

/**
 *
 */
UCLASS()
class ETOS_API AEtosGameMode : public AGameMode
{
	GENERATED_BODY()

public:

	UPROPERTY()
		UDataTable* PredefinedBuildingData;

	UPROPERTY()
		TArray<FPredefinedBuildingData> Buildings;

	UPROPERTY()
		ASimpleCollisionManager* CollisionManager;

private:

	UPROPERTY()
		TMap<EResidentLevel, float> taxPerResidentPerMinute;

public:

	AEtosGameMode();

	FPredefinedBuildingData* GetPredefinedBuildingData(const int32& buildingID);

	int32 GetBuildingAmount();

	FResidentNeeds GetPeasantNeeds();

	// Returns the payed Tax per Resident per Minute for the specified ResidentLevel
	float GetTaxForResident(const EResidentLevel& level);
};
