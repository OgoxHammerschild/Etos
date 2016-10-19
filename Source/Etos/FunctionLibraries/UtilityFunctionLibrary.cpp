// Fill out your copyright notice in the Description page of Project Settings.

#include "Etos.h"
#include "Etos/Game/EtosGameMode.h"
#include "Etos/Game/EtosHUD.h"
#include "Etos/Game/EtosPlayerController.h"
#include "UtilityFunctionLibrary.h"

AEtosGameMode* UUtilityFunctionLibrary::GetEtosGameMode(UObject* WorldContextObject)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject);
	AEtosGameMode* gm = dynamic_cast<AEtosGameMode*, AGameMode>(World->GetAuthGameMode());

	if (gm)
	{
		return gm;
	}

	return nullptr;
}

AEtosHUD * UUtilityFunctionLibrary::GetEtosHUD(UObject* WorldContextObject, int32 PlayerIndex)
{
	AEtosHUD* hud = dynamic_cast<AEtosHUD*, AHUD>(GetEtosPlayerController(WorldContextObject, PlayerIndex)->GetHUD());

	if (hud)
	{
		return hud;
	}

	return nullptr;
}

AEtosPlayerController * UUtilityFunctionLibrary::GetFirstEtosPlayerController(UObject* WorldContextObject)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject);
	AEtosPlayerController* pc = dynamic_cast<AEtosPlayerController*, APlayerController>(World->GetFirstPlayerController());

	if (pc)
	{
		return pc;
	}

	return nullptr;
}

AEtosPlayerController * UUtilityFunctionLibrary::GetEtosPlayerController(UObject* WorldContextObject, int32 PlayerIndex)
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
