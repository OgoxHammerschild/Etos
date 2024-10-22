// © 2016 - 2017 Daniel Bortfeld

#pragma once

#include "Utility/EnumLibrary.h"
#include "ResourceIconPair.generated.h"

/**
 *
 */
USTRUCT(BlueprintType)
struct FResourceIconPair : public FTableRowBase
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		EResource Resource;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UTexture2D* Icon_Smol;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UTexture2D* Icon_Big;
};

USTRUCT()
struct FIcon
{
	GENERATED_BODY()

public:

	UPROPERTY()
		UTexture2D* Icon_Smol;

	UPROPERTY()
		UTexture2D* Icon_Big;

public:

	FIcon() {}

	FIcon(UTexture2D* in_Icon_Smol, UTexture2D* in_Icon_Big) : Icon_Smol(in_Icon_Smol), Icon_Big(in_Icon_Big) {}
};
