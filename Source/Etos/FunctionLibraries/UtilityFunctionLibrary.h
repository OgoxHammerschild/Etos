// © 2016 - 2017 Daniel Bortfeld

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "UtilityFunctionLibrary.generated.h"

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

	UFUNCTION(BlueprintCallable, Category = "EtosUtilityFunctions", meta = (WorldContext = "WorldContextObject", HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject", UnsafeDuringActorConstruction = "true"))
		static bool TraceSingleForBuildings(UObject* WorldContextObject, const FVector& Start, const FVector& End, FHitResult& HitResult);

	UFUNCTION(BlueprintCallable, Category = "EtosUtilityFunctions", meta = (WorldContext = "WorldContextObject", HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject", UnsafeDuringActorConstruction = "true"))
		static bool TraceMultiForBuildings(UObject* WorldContextObject, const FVector& Start, const FVector& End, TArray<FHitResult>& HitResults);

	UFUNCTION(BlueprintCallable, Category = "EtosUtilityFunctions", meta = (WorldContext = "WorldContextObject", HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject", UnsafeDuringActorConstruction = "true"))
		static bool TraceSingleForFloor(UObject* WorldContextObject, const FVector& Start, const FVector& End, FHitResult& Hit);

	// Trace line at mouse position into the screen
	// Call this from a blueprint
	// @ObjectTypes = Considered ObjectTypes. Defaults to Floor.
	// @Range = How far the trace goes into the screen
	UFUNCTION(BlueprintCallable, DisplayName = "Trace Single At Mouse Position", Category = "EtosUtilityFunctions", meta = (WorldContext = "WorldContextObject", HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject", UnsafeDuringActorConstruction = "true", AutoCreateRefTerm = "ObjectTypes", AdvancedDisplay = "ObjectTypes, Range"))
		static bool BP_TraceSingleAtMousePosition(UObject* WorldContextObject, FHitResult& Hit, TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes, float Range = 100000.f);

	// Trace line at mouse position into the screen
	// Call this from code
	// @Range = How far the trace goes into the screen
	// @ObjectTypes = Considered ObjectTypes. Defaults to Floor.
	static bool TraceSingleAtMousePosition(UObject* WorldContextObject, FHitResult& Hit, float Range = 100000.f, TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes = TArray<TEnumAsByte<EObjectTypeQuery>>());

	UFUNCTION(BlueprintPure, Category = "Conversion")
		static FString ConvertBoolToString(const bool& b);

	// https://wiki.unrealengine.com/Enums_For_Both_C%2B%2B_and_BP#Get_Name_of_Enum_as_String
	template<typename TEnum>
	static FString ConvertEnumValueToString(const FString& EnumName, TEnum Value);

private:

	static TArray<TEnumAsByte<EObjectTypeQuery>> InitBuildingObjectType();
	static TArray<TEnumAsByte<EObjectTypeQuery>> InitFloorObjectType();
};

typedef UUtilityFunctionLibrary Util;
