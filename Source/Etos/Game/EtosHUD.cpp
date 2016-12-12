// © 2016 - 2017 Daniel Bortfeld

#include "Etos.h"
#include "Etos/UI/InGameUI.h"
#include "EtosHUD.h"

UInGameUI * AEtosHUD::GetInGameUI()
{
	if (inGameUI == nullptr)
	{
		if (wInGameUI != nullptr)
		{
			inGameUI = CreateWidget<UInGameUI>(GetOwningPlayerController(), wInGameUI);
		}
	}

	return inGameUI;
}

UUserWidget * AEtosHUD::GetWinScreen()
{
	if (winScreen == nullptr)
	{
		if (wWinScreen != nullptr)
		{
			winScreen = CreateWidget<UUserWidget>(GetOwningPlayerController(), wWinScreen);
		}
	}
	return winScreen;
}

UUserWidget * AEtosHUD::GetLoseScreen()
{
	if (loseScreen == nullptr)
	{
		if (wLoseScreen != nullptr)
		{
			loseScreen = CreateWidget<UUserWidget>(GetOwningPlayerController(), wLoseScreen);
		}
	}
	return loseScreen;
}

void AEtosHUD::BeginDestroy()
{
	wInGameUI = nullptr;
	inGameUI = nullptr;

	Super::BeginDestroy();
}
