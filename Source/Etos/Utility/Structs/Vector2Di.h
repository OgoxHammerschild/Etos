// © 2016 - 2017 Daniel Bortfeld

#pragma once

#include "Vector2Di.generated.h"

/**
 * 
 */

USTRUCT(BlueprintType)
struct FVector2Di
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int32 X;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int32 Y;

public:

	FVector2Di(int32 X = 0, int32 Y = 0);

};
