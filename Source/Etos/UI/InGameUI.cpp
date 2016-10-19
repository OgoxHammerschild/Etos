// Fill out your copyright notice in the Description page of Project Settings.

#include "Etos.h"
#include "InGameUI.h"

void UInGameUI::NativeConstruct()
{
	Super::NativeConstruct();
	UpdateResourceAmounts();
}

void UInGameUI::LinkTextToResource(UTextBlock* text, EResource resource)
{
	texts.Emplace(resource, text);
}

#define LOCTEXT_NAMESPACE "InGameUI" 
void UInGameUI::UpdateResourceAmounts()
{
	if (GetPlayerController())
	{
		for (auto& elem : texts)
		{
			int32 amount = playerController->GetResourceAmount(elem.Key);

			FFormatNamedArguments args;
			args.Add(TEXT("amount"), amount);

			elem.Value->SetText(FText::Format(LOCTEXT("", "{amount}"), args));
		}
	}
}
#undef LOCTEXT_NAMESPACE 

AEtosPlayerController * UInGameUI::GetPlayerController()
{
	if (playerController)
	{
		return playerController;
	}

	playerController = dynamic_cast<AEtosPlayerController*, APlayerController>(GetWorld()->GetFirstPlayerController());

	if (playerController)
	{
		return playerController;
	}

	return nullptr;
}
