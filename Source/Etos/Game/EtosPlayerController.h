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

public:

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;

	virtual void SetupInputComponent() override;

	void AddResource(const FResource& resource);

	void RemoveResource(const FResource& resource);

	int32 GetResourceAmount(const EResource& resource);

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
};
