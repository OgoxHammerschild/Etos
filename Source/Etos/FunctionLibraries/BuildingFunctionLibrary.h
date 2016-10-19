// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "BuildingFunctionLibrary.generated.h"

UENUM(BlueprintType)
enum class EOffsetDirections
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

	UFUNCTION(BlueprintCallable, Category = "BuildingUtilFunctions", meta=(DisplayName="Calc Locations For Road-Detection Rays", AdvancedDisplay="ZHeight", Keywords="raycast"))
		static void CalcVectors(float XExtend, float YExtend, float ZExtend, bool negateX1, bool negateY1, bool negateX2, bool negateY2, EOffsetDirections StartOffset, EOffsetDirections GoalOffset, FVector& OutStart, FVector& OutGoal, float ZHeight = 25);
	
	static FVector MakeVector(float X, float Y, FVector2D offset, float Z = 25);
};
