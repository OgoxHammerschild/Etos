// © 2016 - 2017 Daniel Bortfeld

#pragma once

#include "ResourceConsumption.generated.h"

/**
*
*/
USTRUCT(BlueprintType)
struct FResourceConsumption : public FTableRowBase
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		EResource Resource;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int32 Residents;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float ConsumptionPerMinute;

public:

	FResourceConsumption() {}

	FResourceConsumption(EResource Resource, int32 Residents, float ConsumptionPerMinute)
	{
		this->Resource = Resource;
		this->Residents = Residents;
		this->ConsumptionPerMinute = ConsumptionPerMinute;
	}
};
