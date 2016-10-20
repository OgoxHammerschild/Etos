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

	UPROPERTY()
		TMap<EResource, int32> resourceAmounts;

	UPROPERTY()
		ABuilding* newBuilding;

public:

	virtual void BeginPlay() override;

	virtual void SetupInputComponent() override;

	void AddResource(FResource resource);

	void RemoveResource(FResource resource);
	
	int32 GetResourceAmount(EResource resource);

	class UInGameUI* GetInGameUI();

	ABuilding* SpawnBuilding(ABuilding* Class, FBuildingData Data);

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
		void CancelBuilding(FKey key);

	bool HasEnoughResources(TArray<FResource> buildCost);

	//ABuilding* GetBuildingUnderCursor();

	void AddHUDToViewport();

	void InitResourceMapping();

	void CancelPlacementOfBuilding();

	void PayCostsOfBuilding(TArray<FResource> buildCost);
};
