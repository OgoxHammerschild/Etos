// � 2016 - 2017 Daniel Bortfeld

#include "Etos.h"
#include "InGameUI.h"
#include "Etos/UI/BuildMenuButton.h"
#include "Etos/UI/ResourceLayout.h"
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
				resourceName.Replace(TEXT("EResource::"), TEXT(""));
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

void UInGameUI::ShowResourceInfo(const TArray<TEnumAsByte<EResource>>& playerResources, const TArray<int32>& playerResourceAmounts)
{
	if (playerResources.Num() != playerResourceAmounts.Num())
		return;

	TMap<EResource, int32> resourceAmountMap = TMap<EResource, int32>();

	for (int32 i = 0; i < playerResources.Num(); i++)
	{
		resourceAmountMap.Add(playerResources[i], playerResourceAmounts[i]);
	}

	ShowResourceInfo(resourceAmountMap);
}

void UInGameUI::ShowResourceInfo(const TMap<EResource, int32>& playerResourceAmounts)
{
	checkf(ResourceLayoutBlueprint, TEXT("Please specify the ResourceLayout-Blueprint in the InGameUI-Blueprint first"));

	UpdateResourceLayouts(playerResourceAmounts);

	for (auto layout : resources)
	{
		layout.Value->SetVisibility(ESlateVisibility::Visible);
	}

	//TArray<EResource> resources = TArray<EResource>();
	//TArray<int32> amounts = TArray<int32>();

	//playerResourceAmounts.GenerateKeyArray(resources);
	//playerResourceAmounts.GenerateValueArray(amounts);

	//TArray<TEnumAsByte<EResource>> resourcesAsBytes = TArray<TEnumAsByte<EResource>>(resources);

	//BPEvent_OnShowResourceInfo(resourcesAsBytes, amounts);
}

void UInGameUI::HideResourceInfo()
{
	for (auto layout : resources)
	{
		layout.Value->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UInGameUI::ShowBuildButtons()
{
	for (auto button : buttons)
	{
		button->SetVisibility(ESlateVisibility::Visible);
	}
}

void UInGameUI::HideBuildButtons()
{
	for (auto button : buttons)
	{
		button->SetVisibility(ESlateVisibility::Hidden);
	}
}

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
	if (!BuildMenuButtonBlueprint)
		return;

	if (AEtosPlayerController * const PlayerController = UUtilityFunctionLibrary::GetFirstEtosPlayerController(this))
	{
		if (AEtosGameMode * const GameMode = UUtilityFunctionLibrary::GetEtosGameMode(this))
		{
			for (int32 buildingID = 1; buildingID < GameMode->GetBuildingAmount(); buildingID++)
			{
				if (FPredefinedBuildingData* const preDefData = GameMode->GetPredefinedBuildingData(buildingID))
				{
					if (UBuildMenuButton* const button = CreateWidget<UBuildMenuButton>(PlayerController, BuildMenuButtonBlueprint))
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

						AddChildToGridPanel(button, (buildingID -1) % ButtonsPerRow, (buildingID -1) / ButtonsPerRow);

						buttons.Add(button);
					}
				}
			}
		}
	}
}

void UInGameUI::UpdateResourceLayouts(const TMap<EResource, int32>& playerResourceAmounts)
{
	int32 i = 0;
	for (auto resource : playerResourceAmounts)
	{
		if (resource.Key == EResource::None)
			continue;

		if (auto layout = resources.FindOrAdd(resource.Key))
		{
			layout->Resource = FResource(resource.Key, resource.Value);
		}
		else if (ResourceLayoutBlueprint)
		{
			if (AEtosPlayerController *const PlayerController = Util::GetFirstEtosPlayerController(this))
			{
				if (UResourceLayout* const layout = CreateWidget<UResourceLayout>(PlayerController, ResourceLayoutBlueprint))
				{
					layout->Resource = FResource(resource.Key, resource.Value);
					layout->MaxStoredResources = 100;
					layout->SetDesiredSizeInViewport(FVector2D(100, 100));

					AddChildToGridPanel(layout, i % ButtonsPerRow, i / ButtonsPerRow);

					resources.FindOrAdd(resource.Key) = layout;
				}
			}
		}
		++i;
	}
}

UUniformGridSlot * UInGameUI::AddChildToGridPanel(UWidget * Content, int32 Column, int32 Row)
{
	check(gridPanel);
	check(Content);

	UUniformGridSlot* buttonSlot = gridPanel->AddChildToUniformGrid(Content);
	buttonSlot->SetColumn(Column);
	buttonSlot->SetRow(Row);
	buttonSlot->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Left);
	buttonSlot->SetVerticalAlignment(EVerticalAlignment::VAlign_Top);

	return buttonSlot;
}
