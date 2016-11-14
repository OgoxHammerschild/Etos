// © 2016 - 2017 Daniel Bortfeld

#pragma once

#include "GameFramework/GameMode.h"
#include "Etos/Buildings/Base/Building.h"
#include "Runtime/Engine/Classes/Engine/DataTable.h"
#include "Etos/Collision/SimpleCollisionManager.h"
#include "EtosGameMode.generated.h"

USTRUCT(BlueprintType)
struct FPredefinedBuildingData : public FTableRowBase
{
	GENERATED_BODY()

public:

	// The name of the building
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PredefinedBuildingData)
		FName Name = FName(TEXT("New Building"));
	// The blueprint of this building containing its mesh and its size
	// (should derive from ABuilding)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PredefinedBuildingData)
		TSubclassOf<ABuilding> BuildingBlueprint = nullptr;
	// The icon of the building
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PredefinedBuildingData)
		UTexture2D* BuildingIcon = nullptr;

	// The resources required to build this building
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PredefinedBuildingData)
		TArray<FResource> BuildCost = TArray<FResource>();

	// The first resource needed for production
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PredefinedBuildingData)
		EResource NeededResource1 = EResource::None;
	// The second resource needed for production
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PredefinedBuildingData)
		EResource NeededResource2 = EResource::None;
	// The resource this building sould produce
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PredefinedBuildingData)
		EResource ProducedResource = EResource::None;
	// The time it takes for a building to produce a resource 
	// (or call its respective delay action)
	// <= 0 means dont call a delay action
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PredefinedBuildingData)
		float ProductionTime = 0.f;
	// The maximum amount of each needed and produced resources this 
	// building can store
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PredefinedBuildingData)
		int32 MaxStoredResources = 5;

	// The radius of the building in cm
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PredefinedBuildingData)
		float Radius = 500;
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

	UPROPERTY()
		ASimpleCollisionManager* CollisionManager;

public:

	AEtosGameMode();

	FPredefinedBuildingData* GetPredefinedBuildingData(int32 buildingID);
};
