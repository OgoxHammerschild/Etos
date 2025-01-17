// © 2016 - 2017 Daniel Bortfeld

#pragma once

#include "EnumLibrary.generated.h"

/**
 *
 */
UENUM(BlueprintType)
enum class EResource : uint8
{
	None,
	Money,
	Wood,
	Tool,
	Stone,
	Fish,
	Most,
	Coal,
	IronOre,
	Iron,

	EResource_MAX
};

/**
*
*/
UENUM(BlueprintType)
enum class EResidentLevel : uint8
{
	None,
	Peasant,
	Citizen,

	EResidentLevel_MAX
};

/**
*
*/
UENUM(BlueprintType)
enum class EResidentNeed : uint8
{
	None,
	MarketPlace,
	Chapel,
	Tavern,

	EResidentNeed_MAX
};

UENUM(BlueprintType)
enum class EIconSize : uint8
{
	Smol,
	Big
};

UENUM(BlueprintType)
enum class EEnableBuilding : uint8
{
	Always,
	WarehouseBuilt,
	MarketBuilt,
	OnFirstCitizen
};

// ------------------------- Define new Enums above ---------------------------------

#define ENUM_TO_STRING_IMPLEMENTATION(TypeNameAsString, EnumType) \
static FString ToString(EnumType value) \
{ \
	const UEnum* enumPtr = FindObject<UEnum>(ANY_PACKAGE, TEXT(TypeNameAsString), true); \
	if (enumPtr) \
	{ \
		return enumPtr->GetNameStringByIndex((int32)value); \
	} \
 \
	return FString(); \
}

namespace Enum
{
	ENUM_TO_STRING_IMPLEMENTATION("EResource", EResource);

	ENUM_TO_STRING_IMPLEMENTATION("EResidentLevel", EResidentLevel);

	ENUM_TO_STRING_IMPLEMENTATION("EResidentNeed", EResidentNeed);

	ENUM_TO_STRING_IMPLEMENTATION("EIconSize", EIconSize);

	static bool IsValid(EResource resource)
	{
		return resource != EResource::None && resource != EResource::EResource_MAX;
	}

	static bool IsValid(EResidentLevel level)
	{
		return level != EResidentLevel::None && level != EResidentLevel::EResidentLevel_MAX;
	}

	static bool IsValid(EResidentNeed need)
	{
		return need != EResidentNeed::None && need != EResidentNeed::EResidentNeed_MAX;
	}
}
