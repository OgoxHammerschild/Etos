// © 2016 - 2017 Daniel Bortfeld

#pragma once

#include "GameFramework/GameMode.h"
#include "Runtime/Engine/Classes/Engine/DataTable.h"
#include "Etos/Collision/SimpleCollisionManager.h"
#include "Etos/Utility/EnumLibrary.h"
#include "Etos/Utility/Structs/PredefinedBuildingData.h"
#include "Etos/Utility/Structs/UpgradeData.h"
#include "Etos/Utility/Structs/TaxData.h"
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
		UDataTable* UpgradeData;

	UPROPERTY()
		UDataTable* TaxData;

	UPROPERTY()
		TArray<FPredefinedBuildingData> Buildings;

	UPROPERTY()
		ASimpleCollisionManager* CollisionManager;

private:

	static UTexture2D* defaultTexture;

	UPROPERTY()
		TMap<EResidentLevel, float> taxPerResidentPerMinute;

	UPROPERTY()
		float checkWinTimerTotal;

	UPROPERTY()
		float checkWinTimerPassed;

	UPROPERTY()
		int32 populationWinAmount = 200;

public:

	AEtosGameMode();

	static UTexture2D* GetDefaultTexture();

	virtual void Tick(float DeltaTime) override;

	FPredefinedBuildingData* GetPredefinedBuildingData(const int32& buildingID);

	FUpgradeData* GetUpgradeData(const FName& upgrade /*row name*/);

	FTaxData* GetTaxData(const EResidentLevel& level);

	int32 GetBuildingAmount();

	FResidentNeeds GetPeasantNeeds();

	FResidentNeeds GetCitizenNeeds();

	// Returns the payed Tax per Resident per Minute for the specified ResidentLevel
	float GetBaseTaxForResident(const EResidentLevel& level);
};
