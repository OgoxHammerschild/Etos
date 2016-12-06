// © 2016 - 2017 Daniel Bortfeld

#pragma once

#include "GameFramework/GameMode.h"
#include "Etos/Buildings/Base/Building.h"
#include "Runtime/Engine/Classes/Engine/DataTable.h"
#include "Etos/Collision/SimpleCollisionManager.h"
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

public:

	AEtosGameMode();

	FPredefinedBuildingData* GetPredefinedBuildingData(int32 buildingID);

	int32 GetBuildingAmount();

	FResidentNeeds GetPeasantNeeds();
};
