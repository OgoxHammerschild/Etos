// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

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
		bool bIsHoldingObject;

	UPROPERTY(VisibleAnywhere)
		TMap<EResource, int32> resourceAmounts;

	UPROPERTY(VisibleAnywhere)
		ABuilding* newBuilding;

public:

	virtual void BeginPlay() override;

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

	bool HasEnoughResources(const TArray<FResource>& buildCost);

	//ABuilding* GetBuildingUnderCursor();

	void AddHUDToViewport();

	void InitResourceMapping();

	void PayCostsOfBuilding(const TArray<FResource>& buildCost);

	ABuilding* SpawnBuilding_Internal(UClass* Class, const FBuildingData& Data);
};
