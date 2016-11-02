// © 2016 - 2017 Daniel Bortfeld

#include "Etos.h"
#include "Etos/Game/EtosGameMode.h"
#include "Etos/Game/EtosHUD.h"
#include "Etos/Game/EtosPlayerController.h"
#include "UtilityFunctionLibrary.h"

TArray<TEnumAsByte<EObjectTypeQuery>> UUtilityFunctionLibrary::BuildingObjectType = InitBuildingObjectType();
TArray<TEnumAsByte<EObjectTypeQuery>> UUtilityFunctionLibrary::FloorObjectType = InitFloorObjectType();

FORCEINLINE AEtosGameMode* UUtilityFunctionLibrary::GetEtosGameMode(UObject* WorldContextObject)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject);
	AEtosGameMode* gm = dynamic_cast<AEtosGameMode*, AGameMode>(World->GetAuthGameMode());

	if (gm)
	{
		return gm;
	}

	return nullptr;
}

FORCEINLINE AEtosHUD * UUtilityFunctionLibrary::GetEtosHUD(UObject* WorldContextObject, int32 PlayerIndex)
{
	AEtosHUD* hud = dynamic_cast<AEtosHUD*, AHUD>(GetEtosPlayerController(WorldContextObject, PlayerIndex)->GetHUD());

	if (hud)
	{
		return hud;
	}

	return nullptr;
}

FORCEINLINE AEtosPlayerController * UUtilityFunctionLibrary::GetFirstEtosPlayerController(UObject* WorldContextObject)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject);
	AEtosPlayerController* pc = dynamic_cast<AEtosPlayerController*, APlayerController>(World->GetFirstPlayerController());

	if (pc)
	{
		return pc;
	}

	return nullptr;
}

FORCEINLINE AEtosPlayerController * UUtilityFunctionLibrary::GetEtosPlayerController(UObject* WorldContextObject, int32 PlayerIndex)
{
	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject))
	{
		uint32 Index = 0;
		for (FConstPlayerControllerIterator Iterator = World->GetPlayerControllerIterator(); Iterator; ++Iterator)
		{
			APlayerController* PlayerController = *Iterator;
			if (Index == PlayerIndex)
			{
				AEtosPlayerController* pc = dynamic_cast<AEtosPlayerController*, APlayerController>(PlayerController);
				if (pc)
				{
					return pc;
				}
				else
				{
					return nullptr;
				}
			}
			Index++;
		}
	}
	return nullptr;
}

FORCEINLINE FString UUtilityFunctionLibrary::ConvertBoolToString(const bool & b)
{
	return b ? FString(TEXT("true")) : FString(TEXT("false"));
}

template<typename TEnum>
FORCEINLINE FString UUtilityFunctionLibrary::ConvertEnumValueToString(const FString& EnumName, TEnum Value)
{
	const UEnum* enumPtr = FindObject<UEnum>(ANY_PACKAGE, *EnumName, true);
	if (!enumPtr)
	{
		return FString("Invalid");
	}

	return enumPtr->GetEnumName((int32)Value);
}

FORCEINLINE TArray<TEnumAsByte<EObjectTypeQuery>> UUtilityFunctionLibrary::InitBuildingObjectType()
{
	TArray<TEnumAsByte<EObjectTypeQuery>> buildingObjectType;
	buildingObjectType.Init(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_GameTraceChannel1/*Building*/), 1);

	return buildingObjectType;
}

FORCEINLINE TArray<TEnumAsByte<EObjectTypeQuery>> UUtilityFunctionLibrary::InitFloorObjectType()
{
	TArray<TEnumAsByte<EObjectTypeQuery>> floorObjectType;
	floorObjectType.Init(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_GameTraceChannel2/*Floor*/), 1);

	return floorObjectType;
}
