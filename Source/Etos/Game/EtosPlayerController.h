// © 2016 - 2017 Daniel Bortfeld

#pragma once

#include "Etos/ObjectPool/ObjectPool.h"
#include "Etos/Buildings/Base/Building.h"
#include "GameFramework/PlayerController.h"
#include "EtosPlayerController.generated.h"

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

	UPROPERTY(VisibleAnywhere)
		TMap<EResource, int32> resourceAmounts;

	UPROPERTY(VisibleAnywhere)
		int32 totalPopulation = 0;

	UPROPERTY()
		TMap<EResidentLevel, int32> populationPerLevel;

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
		FObjectPool pathPool = FObjectPool();

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

public:

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;

	virtual void SetupInputComponent() override;

	void AddResource(const FResource& resource);

	void AddResource(const EResource& resource, const int32& amount);

	void RemoveResource(const FResource& resource);

	void RemoveResource(const EResource& resource, const int32& amount);

	bool TryRemovingResource(const FResource& resource);

	bool TryRemovingResource(const EResource& resource, const int32& amount);

	int32 GetResourceAmount(const EResource& resource);

	void UpdatePolulation(int32 deltaPolulation);

	int32 GetPopulationAmount();

	void UpdateUpkeep(int32 deltaUpkeep);

	int32 GetTotalUpkeep();

	class UInGameUI* GetInGameUI();

	ABuilding* SpawnBuilding(ABuilding* Class, const FBuildingData& Data);
	ABuilding* SpawnBuilding(TSubclassOf<ABuilding> Subclass, const FBuildingData& Data);

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

	void AddIncome(float DeltaTime);

	bool HasEnoughResources(const TArray<FResource>& buildCost);

	//ABuilding* GetBuildingUnderCursor();

	void AddHUDToViewport();

	void Panic(const std::exception& e);

	void InitResourceMapping();

	void PayCostsOfBuilding(const TArray<FResource>& buildCost);

	ABuilding* SpawnBuilding_Internal(UClass* Class, const FBuildingData& Data);

	void BuildNewBuilding_Internal();

	void StartBuildingPath(APath* newPath);

	void UpdatePathPreview();

	void SpawnPathPreview(const FVector& spawnLocation, const int32& index, UWorld* const World);

	void DestroyPathPreview(APath* tempPath);

	void UpdatePopulationUI(const int32& peasants, const int32& citizens);

	void UpdateBalanceUI(const int32& income, const int32& upkeep);
};
