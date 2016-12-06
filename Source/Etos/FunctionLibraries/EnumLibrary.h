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

	ECitizenNeed_MAX
};

/**
*
*/
UENUM(BlueprintType)
enum class EResidentNeed : uint8
{
	None,
	TownCenter,
	Chapel,
	Tavern,

	ECitizenNeed_MAX
};
