// © 2016 - 2017 Daniel Bortfeld

#include "Etos.h"
#include "InGameUI.h"
#include "BuildMenuButton.h"
#include "ResourceLayout.h"
#include "SatisfactionLayout.h"
#include "ResourceCostLayout.h"
#include "Etos/Game/EtosGameMode.h"
#include "Etos/Buildings/Base/Building.h"
#include "Etos/Buildings/Residence.h"

void UInGameUI::NativeConstruct()
{
	Super::NativeConstruct();
	UpdateResourceAmounts();
	CreateButtons();
}

void UInGameUI::SetGridPanel(UGridPanel* buttonPanel, UGridPanel* resourcePanel)
{
	buildButtonGridPanel = buttonPanel;
	resourceInfoGridPanel = resourcePanel;
}

void UInGameUI::SetResidenceInfoPanel(UUniformGridPanel * panel)
{
	residenceInfoPanel = panel;
}

void UInGameUI::SetBuildCostsPanel(UVerticalBox * panel)
{
	buildCostsPanel = panel;
}

void UInGameUI::LinkTextToResource(UTextBlock* text, EResource resource)
{
	resourceTexts.Add(resource, text);
}

void UInGameUI::SetPopulationTexts(UTextBlock* population, UTextBlock* peasants, UTextBlock* citizens)
{
	populationText = population;
	peasantText = peasants;
	citizenText = citizens;
}

void UInGameUI::SetBalanceTexts(UTextBlock * balance, UTextBlock * income, UTextBlock * upkeep)
{
	balanceText = balance;
	incomeText = income;
	upkeepText = upkeep;
}

#define LOCTEXT_NAMESPACE "InGameGUI" 
void UInGameUI::UpdateResourceAmounts()
{
	if (GetPlayerController())
	{
		for (auto& elem : resourceTexts)
		{
			int32 amount = playerController->GetResourceAmount(elem.Key);

			FFormatNamedArguments args;
			args.Add(TEXT("amount"), amount);

			//args.Add(TEXT("resourceName"), FText::FromString(Enum::ToString(elem.Key)));

			elem.Value->SetText(FText::Format(LOCTEXT("RESOURCE_AMOUNT", /*"{resourceName}: */"{amount}"), args));
		}
	}
}

void UInGameUI::UpdatePopulation(const int32& peasants, const int32& citizens)
{
	if (GetPlayerController())
	{
		int32 amount = playerController->GetTotalPopulation();

		FFormatNamedArguments args;
		args.Add(TEXT("population"), amount);
		args.Add(TEXT("peasants"), peasants);
		args.Add(TEXT("citizens"), citizens);

		populationText->SetText(FText::Format(LOCTEXT("POPULATION_AMOUNT", "Population: {population}"), args));
		peasantText->SetText(FText::Format(LOCTEXT("PEASANT_AMOUNT", "Peasants: {peasants}"), args));
		citizenText->SetText(FText::Format(LOCTEXT("CITIZEN_AMOUNT", "Citizens: {citizens}"), args));
	}
}

void UInGameUI::UpdateBalance(const int32& income, const int32& upkeep)
{
	FFormatNamedArguments args;
	args.Add(TEXT("income"), income);
	args.Add(TEXT("upkeep"), upkeep);
	args.Add(TEXT("balance"), income - upkeep);

	incomeText->SetText(FText::Format(LOCTEXT("INCOME_AMOUNT", "Income: {income}"), args));
	upkeepText->SetText(FText::Format(LOCTEXT("UPKEEP_AMOUNT", "Upkeep: {upkeep}"), args));
	balanceText->SetText(FText::Format(LOCTEXT("BALANCE_AMOUNT", "Balance: {balance}"), args));
}
#undef LOCTEXT_NAMESPACE 

void UInGameUI::ShowBuildingInfo(ABuilding * SelectedBuilding)
{
	this->selectedBuilding = SelectedBuilding;

	if (this->selectedBuilding->IsValidLowLevel())
	{
		BPEvent_ShowBuildingInfo(this->selectedBuilding->Data);
	}
	else
	{
		BPEvent_HideBuildingInfo();
	}
}

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
		layout.Value->SetPadding(FMargin(10));
	}
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
		button->SetPadding(FMargin(5));
	}
}

void UInGameUI::HideBuildButtons()
{
	for (auto button : buttons)
	{
		button->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UInGameUI::ShowResidenceInfo(AResidence * residence)
{
	checkf(SatisfactionLayoutBlueprint, TEXT("No Satisfaction Layout Blueprint was selected for InGameUI"));
	checkf(residenceInfoPanel, TEXT("No Residence Info Panel was linked for InGameUI"));

	if (AEtosPlayerController * const PlayerController = UUtilityFunctionLibrary::GetFirstEtosPlayerController(this))
	{
		if (residenceInfoPanel->HasAnyChildren())
		{
			residenceInfoPanel->ClearChildren();
		}

		TMap<EResource, float> ResourceSatisfaction;
		TMap<EResidentNeed, bool> NeedsSatisfaction;
		float TotalSatisfaction;
		residence->GetAllSatisfactions(ResourceSatisfaction, NeedsSatisfaction, TotalSatisfaction);
		selectedResidence = residence;

		int32 i = 0;
		for (auto& resource : ResourceSatisfaction)
		{
			if (Enum::IsValid(resource.Key))
			{
				auto layout = CreateWidget<USatisfactionLayout>(PlayerController, SatisfactionLayoutBlueprint);
				if (layout)
				{
					layout->Residence = residence;
					layout->MyResource = resource.Key;
					AddChildToGridPanel(residenceInfoPanel, layout, i % 2, i / 2);
					++i;
				}
			}
		}

		for (auto& need : NeedsSatisfaction)
		{
			if (Enum::IsValid(need.Key))
			{
				auto layout = CreateWidget<USatisfactionLayout>(PlayerController, SatisfactionLayoutBlueprint);
				if (layout)
				{
					layout->Residence = residence;
					layout->MyNeed = need.Key;
					AddChildToGridPanel(residenceInfoPanel, layout, i % 2, i / 2);
					++i;
				}
			}
		}
	}
	BPEvent_OnShowResidenceInfo();
}

void UInGameUI::HideResidenceInfo()
{
	BPEvent_OnHideResidenceInfo();
}

void UInGameUI::StartDemolishing()
{
	if (GetPlayerController())
	{
		playerController->StartDemolishMode();
	}
}

FName UInGameUI::GetSelectedBuildingName()
{
	if (selectedBuilding)
	{
		return selectedBuilding->Data.Name;
	}
	return FName();
}

UTexture2D * UInGameUI::GetSelectedBuilding_Icon()
{
	if (selectedBuilding)
	{
		return Util::EnsureTexture(selectedBuilding->Data.BuildingIcon);
	}
	return Util::GetDefaultTexture();
}

int32 UInGameUI::GetSelectedBuilding_Upkeep()
{
	if (selectedBuilding)
	{
		return selectedBuilding->Data.Upkeep;
	}
	return 0;
}

FResource UInGameUI::GetSelectedBuilding_NeededResource1()
{
	if (selectedBuilding)
	{
		return selectedBuilding->Data.NeededResource1;
	}
	return FResource();
}

FResource UInGameUI::GetSelectedBuilding_NeededResource2()
{
	if (selectedBuilding)
	{
		return selectedBuilding->Data.NeededResource2;
	}
	return FResource();
}

FResource UInGameUI::GetSelectedBuilding_ProducedResource()
{
	if (selectedBuilding)
	{
		return selectedBuilding->Data.ProducedResource;
	}
	return FResource();
}

int32 UInGameUI::GetSelectedBuilding_MaxStoredResources()
{
	if (selectedBuilding)
	{
		return selectedBuilding->Data.MaxStoredResources;
	}
	return 0;
}

FText UInGameUI::GetSelectedResidence_ResidentsText()
{
	if (selectedResidence)
	{
		FString text = Enum::ToString(selectedResidence->MyLevel);
		text.Append(": ");
		text.AppendInt(selectedResidence->Residents);
		text.Append("/");
		text.AppendInt(selectedResidence->MaxResidents);
		return FText::FromString(text);
	}
	return FText();
}

void UInGameUI::ShowBuildCost(FBuildingData const & BuildingData)
{
	checkf(ResourceCostLayoutBlueprint, TEXT("No Resource Cost Layout Blueprint was selected for InGameUI"));

	if (GetPlayerController())
	{
		for (auto& cost : BuildingData.BuildCost)
		{
			if (auto layout = CreateWidget<UResourceCostLayout>(playerController, ResourceCostLayoutBlueprint))
			{
				layout->Cost = cost;
				auto slot = buildCostsPanel->AddChildToVerticalBox(layout);
				slot->SetPadding(FMargin(4, 4, 4, 0));
			}
		}
	}
}

void UInGameUI::HideBuildCost(FBuildingData const& BuildingData)
{
	buildCostsPanel->ClearChildren();
}

AEtosPlayerController * UInGameUI::GetPlayerController()
{
	if (!playerController)
	{
		playerController = dynamic_cast<AEtosPlayerController*, APlayerController>(GetWorld()->GetFirstPlayerController());
	}
	return playerController;
}

void UInGameUI::CreateButtons()
{
	checkf(BuildMenuButtonBlueprint, TEXT("No Build Menu Button Blueprint was selected for InGameUI"));

	if (AEtosPlayerController * const PlayerController = UUtilityFunctionLibrary::GetFirstEtosPlayerController(this))
	{
		if (AEtosGameMode * const GameMode = UUtilityFunctionLibrary::GetEtosGameMode(this))
		{
			for (int32 buildingID = 1; buildingID < GameMode->GetBuildingAmount(); ++buildingID)
			{
				if (FPredefinedBuildingData* const preDefData = GameMode->GetPredefinedBuildingData(buildingID))
				{
					tempButton = CreateWidget<UBuildMenuButton>(PlayerController, BuildMenuButtonBlueprint);
					if (tempButton)
					{
						tempButton->Data = FBuildingData(*preDefData);
						tempButton->Enabled = preDefData->BuildingButtonEnabled;

						tempButton->IconTexture = tempButton->Data.BuildingIcon;

						tempButton->Building = preDefData->BuildingBlueprint;
						tempButton->SetPadding(FMargin(5));

						AddChildToGridPanel(buildButtonGridPanel, tempButton, (buildingID - 1) % ButtonsPerRow, (buildingID - 1) / ButtonsPerRow);

						tempButton->OnHovered.AddDynamic(this, &UInGameUI::ShowBuildCost);
						tempButton->OnUnhovered.AddDynamic(this, &UInGameUI::HideBuildCost);

						buttons.Add(tempButton);
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
		if (!Enum::IsValid(resource.Key) || resource.Key == EResource::Money)
			continue;

		if (auto layout = resources.FindOrAdd(resource.Key))
		{
			layout->Resource = resource.Key;
		}
		else if (ResourceLayoutBlueprint)
		{
			if (AEtosPlayerController * const PlayerController = Util::GetFirstEtosPlayerController(this))
			{
				tempLayout = CreateWidget<UResourceLayout>(PlayerController, ResourceLayoutBlueprint);
				if (tempLayout)
				{
					tempLayout->Resource = resource.Key;
					tempLayout->MyPlayerController = PlayerController;
					tempLayout->ToolTipText = FText::FromString(Enum::ToString(resource.Key));
					tempLayout->SetPadding(FMargin(10));

					AddChildToGridPanel(resourceInfoGridPanel, tempLayout, i % ResourcesPerRow, i / ResourcesPerRow);

					resources.FindOrAdd(resource.Key) = tempLayout;
				}
			}
		}
		++i;
	}
}

UGridSlot * UInGameUI::AddChildToGridPanel(UGridPanel * Panel, UWidget * Content, int32 Column, int32 Row)
{
	checkf(Panel, TEXT("The Panel Param was not valid. Maybe no Panel was linked in the InGameUI-BP?"));
	check(Content);

	UGridSlot* gridSlot = Panel->AddChildToGrid(Content);
	gridSlot->SetColumn(Column);
	gridSlot->SetRow(Row);
	gridSlot->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Left);
	gridSlot->SetVerticalAlignment(EVerticalAlignment::VAlign_Top);

	return gridSlot;
}

UUniformGridSlot * UInGameUI::AddChildToGridPanel(UUniformGridPanel * Panel, UWidget * Content, int32 Column, int32 Row)
{
	checkf(Panel, TEXT("The Panel Param was not valid. Maybe no Panel was linked in the InGameUI-BP?"));
	check(Content);

	UUniformGridSlot* gridSlot = Panel->AddChildToUniformGrid(Content);
	gridSlot->SetColumn(Column);
	gridSlot->SetRow(Row);
	gridSlot->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Left);
	gridSlot->SetVerticalAlignment(EVerticalAlignment::VAlign_Top);

	return gridSlot;
}
