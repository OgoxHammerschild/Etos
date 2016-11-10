// © 2016 - 2017 Daniel Bortfeld

#include "Etos.h"
#include "Etos/UI/InGameUI.h"
#include "EtosHUD.h"

UInGameUI * AEtosHUD::GetInGameUI()
{
	if (inGameUI)
	{
		return inGameUI;
	}

	if (wInGameUI != nullptr)
	{
		inGameUI = CreateWidget<UInGameUI>(GetOwningPlayerController(), wInGameUI);
	}

	if (inGameUI)
	{
		return inGameUI;
	}
	return nullptr;
}

void AEtosHUD::BeginDestroy()
{
	wInGameUI = nullptr;
	inGameUI = nullptr;

	Super::BeginDestroy();
}
