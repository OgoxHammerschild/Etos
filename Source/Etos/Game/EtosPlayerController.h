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

	UPROPERTY(VisibleAnywhere)
		int32 totalStorage = 0;

	UPROPERTY()
		TMap<EResidentLevel, int32> populationPerLevel;

	UPROPERTY()
		TMap<EResidentLevel, int32> availablePromotions;

	UPROPERTY()
		TArray<ABuilding*> builtBuildings;

	UPROPERTY()
		TArray<AResidence*> builtResidences;

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

	void UpdatePopulation(const EResidentLevel& level, const int32& deltaPolulation);

	void UpdatePopulation(const EResidentLevel& from, const EResidentLevel& to, const int32& residents);

	int32 GetTotalPopulation() const;

	int32 GetPopulationAmount(const EResidentLevel& level);

	void UpdateUpkeep(int32 deltaUpkeep);

	int32 GetTotalUpkeep();

	void UpdateStorage(int32 deltaStorage);

	int32 GetTotalStorage();

	class UInGameUI* GetInGameUI();

	ABuilding* SpawnBuilding(ABuilding* Class, const FBuildingData& Data);

	ABuilding* SpawnBuilding(const TSubclassOf<ABuilding>& Subclass, const FBuildingData& Data);

	void Win();

	void Lose();

	bool HasEnoughResources(const TArray<FResource>& buildCost) const;

	void ReportDestroyedBuilding(ABuilding* destroyedBuilding);

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
		void OnBuildingDestroyed(AActor* DestroyedActor);

	void AddIncome(float DeltaTime);

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
