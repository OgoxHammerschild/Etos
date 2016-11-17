// © 2016 - 2017 Daniel Bortfeld

#include "Etos.h"
#include "InGameUI.h"
#include "Etos/UI/BuildMenuButton.h"
#include "Etos/Game/EtosGameMode.h"
#include "Etos/FunctionLibraries/UtilityFunctionLibrary.h"
#include "Etos/Buildings/Base/Building.h"

void UInGameUI::NativeConstruct()
{
	Super::NativeConstruct();
	UpdateResourceAmounts();
	CreateButtons();
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

			const UEnum* enumPtr = FindObject<UEnum>(ANY_PACKAGE, TEXT("EResource"), true);
			if (enumPtr)
			{
				FString resourceName = enumPtr->GetEnumName((int32)elem.Key);
				args.Add(TEXT("resourceName"), FText::FromString(resourceName));

				elem.Value->SetText(FText::Format(LOCTEXT("", "{resourceName}: {amount}"), args));
			}
			else
			{
				elem.Value->SetText(FText::Format(LOCTEXT("", "{amount}"), args));
			}
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
	if (AEtosPlayerController * PlayerController = UUtilityFunctionLibrary::GetFirstEtosPlayerController(this))
	{
		if (AEtosGameMode * GameMode = UUtilityFunctionLibrary::GetEtosGameMode(this))
		{
			for (int32 buildingID = 1; buildingID < 7/* amount of buildings*/; buildingID++)
			{
				if (FPredefinedBuildingData* preDefData = GameMode->GetPredefinedBuildingData(buildingID))
				{
					if (UBuildMenuButton* button = CreateWidget<UBuildMenuButton>(PlayerController, BuildMenuButtonBlueprint))
					{
						FBuildingData data = FBuildingData();
						data.BuildCost = preDefData->BuildCost;
						data.BuildingIcon = preDefData->BuildingIcon;
						data.MaxStoredResources = preDefData->MaxStoredResources;
						data.Name = preDefData->Name;
						data.NeededResource1.Type = preDefData->NeededResource1;
						data.NeededResource1.Amount = 0;
						data.NeededResource2.Type = preDefData->NeededResource2;
						data.NeededResource2.Amount = 0;
						data.ProducedResource.Type = preDefData->ProducedResource;
						data.ProducedResource.Amount = 0;
						data.ProductionTime = preDefData->ProductionTime;
						data.Radius = preDefData->Radius;
						data.Upkeep = preDefData->Upkeep;

						button->Data = data;

						check(button->BuildingIcon);
						button->BuildingIcon->SetBrushFromTexture(preDefData->BuildingIcon);
						button->Building = preDefData->BuildingBlueprint;

						check(gridPanel);

						UUniformGridSlot* buttonSlot = gridPanel->AddChildToUniformGrid(button);
						buttonSlot->SetColumn(buildingID % ButtonsPerRow);
						buttonSlot->SetRow(buildingID / ButtonsPerRow);
						buttonSlot->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Left);
						buttonSlot->SetVerticalAlignment(EVerticalAlignment::VAlign_Top);
					}
				}
			}
		}
	}
}
