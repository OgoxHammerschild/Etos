// Fill out your copyright notice in the Description page of Project Settings.

#include "Etos.h"
#include "Etos/UI/InGameUI.h"
#include "EtosHUD.h"

UInGameUI * AEtosHUD::GetInGameUI()
{
	if (inGameUI)
	{
		return inGameUI;
	}

	if (wInGameUI)
	{
		inGameUI = CreateWidget<UInGameUI>(GetOwningPlayerController(), wInGameUI);
	}

	if (inGameUI)
	{
		return inGameUI;
	}
	return nullptr;
}
