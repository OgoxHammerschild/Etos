// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Blueprint/UserWidget.h"
#include "Etos/Game/EtosPlayerController.h"
#include "Etos/Buildings/Base/Building.h"
#include "InGameUI.generated.h"

/**
 *
 */
UCLASS()
class ETOS_API UInGameUI : public UUserWidget
{
	GENERATED_BODY()


private:

	AEtosPlayerController* playerController;

	TMap<EResource, UTextBlock* > texts;

public:

	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintCallable, Category = Penis)
		void LinkTextToResource(UTextBlock* text, EResource resource);

	UFUNCTION(BlueprintCallable, Category = Penis)
		void UpdateResourceAmounts();

private:

	AEtosPlayerController* GetPlayerController();
};
