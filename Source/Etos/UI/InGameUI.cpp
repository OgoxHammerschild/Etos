// © 2016 - 2017 Daniel Bortfeld

#include "Etos.h"
#include "InGameUI.h"
#include "Etos/UI/BuildMenuButton.h"
#include "Etos/UI/ResourceLayout.h"
#include "Etos/Game/EtosGameMode.h"
#include "Etos/Buildings/Base/Building.h"

void UInGameUI::NativeConstruct()
{
	Super::NativeConstruct();
	UpdateResourceAmounts();
	CreateButtons();
}

void UInGameUI::SetGridPanel(UGridPanel* panel)
{
	gridPanel = panel;
}

void UInGameUI::LinkTextToResource(UTextBlock* text, EResource resource)
{
	resourceTexts.Emplace(resource, text);
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

			const UEnum* enumPtr = FindObject<UEnum>(ANY_PACKAGE, TEXT("EResource"), true);
			if (enumPtr)
			{
				FString resourceName = enumPtr->GetEnumName((int32)elem.Key);
				resourceName.Replace(TEXT("EResource::"), TEXT(""));
				args.Add(TEXT("resourceName"), FText::FromString(resourceName));

				elem.Value->SetText(FText::Format(LOCTEXT("RESOURCE_AMOUNT", "{resourceName}: {amount}"), args));
			}
			else
			{
				elem.Value->SetText(FText::Format(LOCTEXT("RESOURCE_AMOUNT", "{amount}"), args));
			}
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
						button->SetPadding(FMargin(5));

						AddChildToGridPanel(button, (buildingID - 1) % ButtonsPerRow, (buildingID - 1) / ButtonsPerRow);

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
		if (!Enum::IsValid(resource.Key) || resource.Key == EResource::Money)
			continue;

		if (auto layout = resources.FindOrAdd(resource.Key))
		{
			layout->Resource = FResource(resource.Key, resource.Value);
		}
		else if (ResourceLayoutBlueprint)
		{
			if (AEtosPlayerController * const PlayerController = Util::GetFirstEtosPlayerController(this))
			{
				if (UResourceLayout * const layout = CreateWidget<UResourceLayout>(PlayerController, ResourceLayoutBlueprint))
				{
					layout->Resource = FResource(resource.Key, resource.Value);
					layout->MaxStoredResources = 100;
					layout->SetPadding(FMargin(10));

					AddChildToGridPanel(layout, i % ResourcesPerRow, i / ResourcesPerRow);

					resources.FindOrAdd(resource.Key) = layout;
				}
			}
		}
		++i;
	}
}

UGridSlot * UInGameUI::AddChildToGridPanel(UWidget * Content, int32 Column, int32 Row)
{
	check(gridPanel);
	check(Content);

	UGridSlot* gridSlot = gridPanel->AddChildToGrid(Content);
	gridSlot->SetColumn(Column);
	gridSlot->SetRow(Row);
	gridSlot->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Left);
	gridSlot->SetVerticalAlignment(EVerticalAlignment::VAlign_Top);

	return gridSlot;
}
