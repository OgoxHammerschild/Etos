// © 2016 - 2017 Daniel Bortfeld

#pragma once

#include "Etos/Utility/EnumLibrary.h"
#include "TaxData.generated.h"

USTRUCT(BlueprintType)
struct FTaxResourcePair
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		EResource Resource;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float Tax;
};

/**
 *
 */
USTRUCT(BlueprintType)
struct FTaxData : public FTableRowBase
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		EResidentLevel Level;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float BaseTax = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray < FTaxResourcePair> taxPerResource;
};
