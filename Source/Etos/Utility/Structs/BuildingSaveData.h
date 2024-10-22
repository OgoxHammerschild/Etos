// © 2016 - 2017 Daniel Bortfeld

#pragma once

#include "Etos/Utility/EnumLibrary.h"
#include "BuildingSaveData.generated.h"

/**
*
*/
USTRUCT()
struct FBuildingSaveData
{
	GENERATED_BODY()

public:

	// The name of the building
	UPROPERTY(SaveGame)
		FName Name = FName(TEXT("New Building"));

	UPROPERTY(SaveGame)
		FTransform Transform;

	// The first resource needed for production
	UPROPERTY(SaveGame)
		int32 NeededResource1Amount;
	// The second resource needed for production
	UPROPERTY(SaveGame)
		int32 NeededResource2Amount;
	// The resource this building produces (or already has produced)
	UPROPERTY(SaveGame)
		int32 ProducedResourceAmount;

	UPROPERTY(SaveGame)
		bool bIsActive;
};

/**
*
*/
USTRUCT()
struct FResidenceSaveData : public FBuildingSaveData
{
	GENERATED_BODY()

public:

	UPROPERTY(SaveGame)
		EResidentLevel Level;

	UPROPERTY(SaveGame)
		int32 Residents;

	UPROPERTY(SaveGame)
		int32 MaxResidents;

	UPROPERTY(SaveGame)
		TMap<EResource, float> ResourceSatisfaction;

	UPROPERTY(SaveGame)
		TMap<EResidentNeed, bool> NeedsSatisfaction;

	UPROPERTY(SaveGame)
		float TotalSatisfaction;
};
