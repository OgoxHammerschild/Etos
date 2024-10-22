// © 2016 - 2017 Daniel Bortfeld

#pragma once

#include "Utility/EnumLibrary.h"
#include "Resource.generated.h"

/**
*
*/
USTRUCT(BlueprintType)
struct FResource
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
		EResource Type = EResource::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
		int32 Amount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
		UTexture2D* Icon = Util::GetDefaultTexture();

public:

	FResource(EResource Type = EResource::None, int32 Amount = 0, UTexture2D* Icon = Util::GetDefaultTexture())
	{
		if (Type != EResource::None)
		{
			this->Type = Type;
			this->Amount = Amount;
			this->Icon = Icon;
		}
	}
};
