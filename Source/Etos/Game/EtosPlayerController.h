// © 2016 - 2017 Daniel Bortfeld

#pragma once

class AResidence;
class UEtosMetaSaveGame;

#include "Etos/Utility/InOut.h"
#include "Etos/ObjectPool/ObjectPool.h"
#include "Etos/Buildings/Base/Building.h"
#include "GameFramework/PlayerController.h"
#include "EtosPlayerController.generated.h"

USTRUCT()
struct FResidenceArray
{
	GENERATED_BODY()

public:

	UPROPERTY()
		TArray<AResidence*> Residences;
};

/**
 *
 */
UCLASS()
class ETOS_API AEtosPlayerController : public APlayerController
{
	GENERATED_BODY()

private:

	UPROPERTY()
		bool bIsHoldingObject = false;

	UPROPERTY()
		bool bIsInDemolishMode = false;

	UPROPERTY(VisibleAnywhere)
		TMap<EResource, int32> resourceAmounts;

	UPROPERTY(VisibleAnywhere)
		int32 totalPopulation = 0;

	UPROPERTY(VisibleAnywhere)
		int32 totalStorage = 0;

	UPROPERTY()
		TMap<EResidentLevel, int32> populationPerLevel;

	UPROPERTY()
		// @key = level promoted to
		TMap<EResidentLevel, int32> usedPromotions;

	UPROPERTY()
		TArray<ABuilding*> builtBuildings;

	UPROPERTY()
		TMap<EResidentLevel, FResidenceArray> builtResidences;

	UPROPERTY(VisibleAnywhere)
		ABuilding* newBuilding;

	UPROPERTY(VisibleAnywhere)
		TArray<APath*> tempPaths;

	UPROPERTY(VisibleAnywhere)
		bool bIsBulidingPath = false;

	UPROPERTY(VisibleAnywhere)
		FVector previousMouseLocation;

	UPROPERTY(EditDefaultsOnly)
		float mouseMoveThreshold = 20.f;

	UPROPERTY()
		UObjectPool* pathPool;

	UPROPERTY(VisibleAnywhere)
		int32 totalUpkeep = 0;

	UPROPERTY(VisibleAnywhere)
		int32 totalIncome = 0;

	UPROPERTY()
		int32 incomeTimerTotal = 3;

	UPROPERTY()
		float incomeTimerPassed = 0;

	UPROPERTY(VisibleAnywhere)
		float payedIncome = 0;

	UPROPERTY()
		FString tempSaveSlotName;

public:

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;

	virtual void SetupInputComponent() override;

	void AddResource(FResource in resource);

	void AddResource(EResource in resource, int32 in amount);

	void RemoveResource(FResource in resource);

	void RemoveResource(EResource in resource, int32 in amount);

	bool TryRemovingResource(FResource in resource);

	bool TryRemovingResource(EResource in resource, int32 in amount);

	int32 GetResourceAmount(EResource in resource);

	void UpdatePopulation(EResidentLevel in level, int32 in deltaPolulation);

	void UpdatePopulation(EResidentLevel in from, EResidentLevel in to, int32 in residents);

	void ReportUpgrade(AResidence* in upgradedResidence, EResidentLevel in levelBeforeUpgrade, EResidentLevel in levelAfterUpgrade);

	int32 GetTotalPopulation() const;

	int32 GetPopulationAmount(EResidentLevel in level);

	void UpdateUpkeep(int32 in deltaUpkeep);

	int32 GetTotalUpkeep();

	void UpdateStorage(int32 in deltaStorage);

	int32 GetTotalStorage();

	class UInGameUI* GetInGameUI();

	int32 GetAvailablePromotions(EResidentLevel in to);

	ABuilding* SpawnBuilding(ABuilding* in Class, FBuildingData in Data);

	ABuilding* SpawnBuilding(TSubclassOf<ABuilding> in Subclass, FBuildingData in Data);

	AResourcePopup* SpawnTextPopup(FVector in Position, FText in Text, FLinearColor in TextColor = FLinearColor::Red);

	void Win();

	void Lose();

	bool HasEnoughResources(TArray<FResource> in buildCost) const;

	void ReportDestroyedBuilding(ABuilding* in destroyedBuilding);

	void StartDemolishMode();

	// returns: whether any save games were removed from meta
	bool RemoveInvalidSaveGamesFromMeta(TArray<FString> in invalidSaveSlots);

	UFUNCTION(BlueprintCallable, Category = "Save / Load")
		bool Save(FString SaveSlotName = TEXT("NewSaveGame"));

	UFUNCTION(BlueprintCallable, Category = "Save / Load")
		void SaveWithWarning(FString SaveSlotName = TEXT("NewSaveGame"));

	UFUNCTION(BlueprintCallable, Category = "Save / Load")
		bool Load(FString SaveSlotName = TEXT("NewSaveGame"));

	UFUNCTION(BlueprintCallable, Category = "Save / Load")
		void LoadWithWarning(FString SaveSlotName = TEXT("NewSaveGame"));

	UFUNCTION(BlueprintCallable, Category = "Save / Load")
		bool LoadLatestSaveGame();

	UFUNCTION(BlueprintCallable, Category = "Save / Load")
		void LoadLatestSaveGameWithWarning();

	UFUNCTION(BlueprintCallable, Category = "Save / Load")
		void QuickSave();

	UFUNCTION(BlueprintCallable, Category = "Save / Load")
		void QuickLoad();

	UFUNCTION(BlueprintCallable, Category = "Pause")
		void TogglePause();

	UFUNCTION(BlueprintCallable, Category = "Pause")
		void ToggleGameMenu();

private:

	UFUNCTION()
		void BuildNewBuilding(FKey key);

	UFUNCTION()
		void PauseGame(FKey key);

	UFUNCTION()
		void ShowGameMenu(FKey key);

	UFUNCTION()
		void ClickRepeatedly(FKey key);

	UFUNCTION()
		void CancelPlacementOfBuilding(FKey key);

	UFUNCTION()
		void SelectBuilding(FKey key);

	UFUNCTION()
		void DemolishBuilding(FKey key);

	UFUNCTION()
		void RotateHeldBuilding(FKey key);

	UFUNCTION()
		void OnBuildingDestroyed(AActor* DestroyedActor);

	UFUNCTION()
		void SaveToTempSlot();

	UFUNCTION()
		void LoadFromTempSlot();

	UFUNCTION()
		void LoadLatestSaveGame_Wrapper();

		void AddIncome(float in DeltaTime);

	//ABuilding* GetBuildingUnderCursor();

	void AddGUIToViewport();

	void Panic(std::exception in e);

	void InitResourceMapping();

	void PayCostsOfBuilding(ABuilding* in building);

	ABuilding* SpawnBuilding_Internal(UClass* in Class, FBuildingData in Data);

	void BuildNewBuilding_Internal(bool in skipCosts = false);

	void BuildLoadedBuilding(struct FBuildingSaveData in Data);

	void StartBuildingPath(APath* in newPath);

	void UpdatePathPreview();

	void SpawnPathPreview(FVector in spawnLocation, int32 in index, UWorld* in World);

	void DestroyPathPreview(APath* in tempPath);

	void UpdatePopulationUI(int32 in peasants, int32 in citizens);

	void UpdateBalanceUI(int32 in income, int32 in upkeep);

	bool AddSlotNameToMeta(FString in slotName);

	bool RemoveSlotNameFromMeta(FString in slotName);

	UEtosMetaSaveGame* GetMetaSaveGame();
};
