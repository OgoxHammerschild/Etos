// Fill out your copyright notice in the Description page of Project Settings.

#include "Etos.h"
#include "InGameUI.h"
#include "Etos/UI/BuildMenuButton.h"
#include "Etos/Game/EtosGameMode.h"

void UInGameUI::NativeConstruct()
{
	Super::NativeConstruct();
	UpdateResourceAmounts();
}

void UInGameUI::SetGridPanel(UUniformGridPanel* panel)
{
	gridPanel = panel;
}

void UInGameUI::LinkTextToResource(UTextBlock* text, EResource resource)
{
	texts.Emplace(resource, text);
}

#define LOCTEXT_NAMESPACE "InGameGUI" 
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

void UInGameUI::CreateButtons()
{
	if (UWorld * World = GetWorld())
	{
		if (AEtosGameMode * GameMode = UUtilityFunctionLibrary::GetEtosGameMode(this))
		{
			for (int32 buildingID = 0; buildingID < 10/* amount of buildings*/; buildingID++)
			{
				if (UBuildMenuButton* button = CreateWidget<UBuildMenuButton>(World))
				{
					FPredefinedBuildingData preDefData = *GameMode->GetPredefinedBuildingData(buildingID);
					FBuildingData data = FBuildingData();
					data.BuildCost = preDefData.BuildCost;
					data.BuildingIcon = preDefData.BuildingIcon;
					data.MaxStoredResources = preDefData.MaxStoredResources;
					data.Name = preDefData.Name;
					data.NeededResource1.Type = preDefData.NeededResource1;
					data.NeededResource2.Type = preDefData.NeededResource2;
					data.ProducedResource.Type = preDefData.ProducedResource;
					data.ProductionTime = preDefData.ProductionTime;
					data.Radius = preDefData.Radius;

					button->Data = data;
					button->BuildingIcon->SetBrushFromTexture(preDefData.BuildingIcon);
					//button->Building =

				}
			}
		}
	}
}
