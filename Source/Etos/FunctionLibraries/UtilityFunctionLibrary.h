// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "UtilityFunctionLibrary.generated.h"


USTRUCT(BlueprintType)
struct FVector2Di
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int32 X;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int32 Y;
};

/**
 *
 */
UCLASS()
class ETOS_API UUtilityFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

		public:

	static TArray<TEnumAsByte<EObjectTypeQuery>> BuildingObjectType;
	static TArray<TEnumAsByte<EObjectTypeQuery>> FloorObjectType;

public:

	UFUNCTION(BlueprintPure, Category = "EtosUtilityFunctions", meta = (WorldContext = "WorldContextObject", HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject", UnsafeDuringActorConstruction = "true"))
		static class AEtosGameMode* GetEtosGameMode(UObject* WorldContextObject);

	UFUNCTION(BlueprintPure, Category = "EtosUtilityFunctions", meta = (WorldContext = "WorldContextObject", HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject", UnsafeDuringActorConstruction = "true"))
		static class AEtosHUD* GetEtosHUD(UObject* WorldContextObject, int32 PlayerIndex = 0);

	UFUNCTION(BlueprintPure, Category = "EtosUtilityFunctions", meta = (WorldContext = "WorldContextObject", HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject", UnsafeDuringActorConstruction = "true"))
		static class AEtosPlayerController* GetFirstEtosPlayerController(UObject* WorldContextObject);

	UFUNCTION(BlueprintPure, Category = "EtosUtilityFunctions", meta = (WorldContext = "WorldContextObject", HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject", UnsafeDuringActorConstruction = "true"))
		static AEtosPlayerController* GetEtosPlayerController(UObject* WorldContextObject, int32 PlayerIndex);

private:

	static TArray<TEnumAsByte<EObjectTypeQuery>> InitBuildingObjectType();
	static TArray<TEnumAsByte<EObjectTypeQuery>> InitFloorObjectType();
};

typedef UUtilityFunctionLibrary Util;
