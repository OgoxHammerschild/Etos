// © 2016 - 2017 Daniel Bortfeld

#pragma once

//#include "Runtime/Core/Public/Serialization/Archive.h"
//#include "Runtime/CoreUObject/Public/Serialization/ObjectAndNameAsStringProxyArchive.h"

#include "Etos/Utility/Structs/BuildingSaveData.h"
#include "Etos/Utility/EnumLibrary.h"
#include "GameFramework/SaveGame.h"
#include "EtosSaveGame.generated.h"

/**
 *
 */
UCLASS()
class ETOS_API UEtosSaveGame : public USaveGame
{
	GENERATED_BODY()

public:

	UPROPERTY(VisibleAnywhere, Category = Basic)
		FString PlayerName;

	UPROPERTY(VisibleAnywhere, Category = Basic)
		FString SaveSlotName;

	UPROPERTY(VisibleAnywhere, Category = Basic)
		uint32 UserIndex;


	UPROPERTY()
		TMap<EResource, int32> ResourceAmounts;

	UPROPERTY()
		TMap<EResidentLevel, int32> PopulationPerLevel;

	UPROPERTY()
		// @key = level promoted to
		TMap<EResidentLevel, int32> UsedPromotions;

	UPROPERTY()
		TArray<FResidenceSaveData> BuiltResidences;

	UPROPERTY()
		TArray<FBuildingSaveData> BuiltBuildings;

	UPROPERTY()
		TArray<struct FResource> ResourcesOnTransit;

public:

	UEtosSaveGame();

	void AddBuilding(class ABuilding* const building);

	void AddResidence(class AResidence* const residence);
};
