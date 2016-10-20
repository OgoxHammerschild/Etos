// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/GameMode.h"
#include "Etos/Buildings/Base/Building.h"
#include "Runtime/Engine/Classes/Engine/DataTable.h"
#include "EtosGameMode.generated.h"

USTRUCT(BlueprintType)
struct FPredefinedBuildingData : public FTableRowBase
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PredefinedBuildingData)
		FName Name = FName(TEXT("New Building"));
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PredefinedBuildingData)
		UTexture2D* BuildingIcon = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PredefinedBuildingData)
		TArray<FResource> BuildCost = TArray<FResource>();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PredefinedBuildingData)
		EResource NeededResource1 = EResource::None;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PredefinedBuildingData)
		EResource NeededResource2 = EResource::None;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PredefinedBuildingData)
		EResource ProducedResource = EResource::None;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PredefinedBuildingData)
		float ProductionTime = 1.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PredefinedBuildingData)
		int32 MaxStoredResources = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PredefinedBuildingData)
		float Radius = 5;
};

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

public:

	FPredefinedBuildingData* GetPredefinedBuildingData(int32 buildingID);
};
