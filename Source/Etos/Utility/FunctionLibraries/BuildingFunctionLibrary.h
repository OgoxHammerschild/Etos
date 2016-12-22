// © 2016 - 2017 Daniel Bortfeld

#pragma once

class ABuilding;

#include "Etos/Utility/Structs/Vector2Di.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "BuildingFunctionLibrary.generated.h"

UENUM(BlueprintType)
enum class EOffsetDirections : uint8
{
	TopLeft,
	TopRight,
	BotLeft,
	BotRight
};

/**
 *
 */
UCLASS()
class ETOS_API UBuildingFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "BuildingUtilFunctions", meta = (DisplayName = "Calc Locations For Road-Detection Rays", AdvancedDisplay = "ZHeight", Keywords = "raycast"))
		static void CalcVectors(float XExtend, float YExtend, float ZExtend, bool negateX1, bool negateY1, bool negateX2, bool negateY2, EOffsetDirections StartOffset, EOffsetDirections GoalOffset, FVector& OutStart, FVector& OutGoal, float ZHeight = 25);

	UFUNCTION(BlueprintCallable, Category = "BuildingUtilFunctions")
		static bool FindPath(const ABuilding* Source, const ABuilding* Target);

	UFUNCTION(BlueprintPure, Category = "BuildingUtilFunctions")
		static FVector GetNextGridLocation(const FVector& location, const FVector2Di& size, const float& heightOffset = 2);

	static FVector MakeVector(float X, float Y, FVector2D offset, float Z = 25);
};

typedef UBuildingFunctionLibrary BFuncs;
