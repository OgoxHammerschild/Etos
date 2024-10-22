// © 2016 - 2017 Daniel Bortfeld

#include "Etos.h"
#include "SaveLoadDialogueEntry.h"
#include "SaveLoadDialogue.h"
#include "Etos/Game/EtosMetaSaveGame.h"
#include "Etos/Game/EtosPlayerController.h"

void USaveLoadDialogue::SetEntryPanel(UScrollBox * panel)
{
	dialogueEntryPanel = panel;
}

void USaveLoadDialogue::SetEditableTextBox(UEditableTextBox * textBox)
{
	editableTextBox = textBox;
}

void USaveLoadDialogue::SetSelectedEntry(USaveLoadDialogueEntry * Entry)
{
	ResetSelectedEntry();
	SelectedEntry = Entry;

	if (editableTextBox && editableTextBox->IsValidLowLevel() && SelectedEntry && SelectedEntry->IsValidLowLevel())
	{
		editableTextBox->SetText(FText::FromString(SelectedEntry->SaveSlotName));
	}
}

void USaveLoadDialogue::ResetSelectedEntry()
{
	if (SelectedEntry && SelectedEntry->IsValidLowLevel())
	{
		SelectedEntry->Reset();
	}
}

void USaveLoadDialogue::NativeConstruct()
{
	Super::NativeConstruct();
	PopulatePanel();
}

void USaveLoadDialogue::NativeDestruct()
{
	dialogueEntryPanel->ClearChildren();
	Super::NativeDestruct();
}

void USaveLoadDialogue::PopulatePanel()
{
	checkf(dialogueEntryPanel, TEXT("Please set a valid load entry panel in the SaveLoadDialogue-WidgetBP first (see SetEntryPanel(UVerticalBox * panel))"));
	checkf(SaveLoadDialogueEntryBlueprint, TEXT("Please specify a default Save Load Dialogue Entry Blueprint in the SaveLoadDialogue-WidgetBP first"));

	if (auto* const PlayerController = Util::GetFirstEtosPlayerController(this))
	{
		FString slotName = TEXT("Meta");

		if (!UGameplayStatics::DoesSaveGameExist(slotName, 0))
		{
			UEtosMetaSaveGame* SaveGameInstance = Cast<UEtosMetaSaveGame>(UGameplayStatics::CreateSaveGameObject(UEtosMetaSaveGame::StaticClass()));
			SaveGameInstance->PlayerName = TEXT("Player 1");
			SaveGameInstance->SaveSlotName = slotName;
			SaveGameInstance->UserIndex = 0;

			UTextBlock* noSavesFound_TextBlock = CreateWidget<UTextBlock>(PlayerController);
			if (noSavesFound_TextBlock)
			{
				noSavesFound_TextBlock->SetText(FText::FromString(NoSavesFound_Text));
				noSavesFound_TextBlock->SetColorAndOpacity(FLinearColor::White);

				dialogueEntryPanel->ClearChildren();
				dialogueEntryPanel->AddChild(noSavesFound_TextBlock);
			}

			//USaveLoadDialogueEntry* savegameEntry = CreateWidget<USaveLoadDialogueEntry>(PlayerController, SaveLoadDialogueEntryBlueprint);
			//if (savegameEntry)
			//{
			//	savegameEntry->SaveSlotName = NoSavesFound_Text;
			//	savegameEntry->SaveTime = FDateTime::Now();
			//	savegameEntry->MySaveLoadDialogue = this;

			//	dialogueEntryPanel->ClearChildren();
			//	dialogueEntryPanel->AddChild(savegameEntry);
			//}
			return;
		}

		UEtosMetaSaveGame* LoadGameInstance = Cast<UEtosMetaSaveGame>(UGameplayStatics::LoadGameFromSlot(slotName, 0));

		TArray<FDateTime> saveDates;
		LoadGameInstance->SaveSlots.GenerateValueArray(saveDates);
		saveDates.Sort();

		TArray<FString> invalidSaveSlots;

		dialogueEntryPanel->ClearChildren();
		for (int i = saveDates.Num() - 1; i >= 0; --i)
		{
			FString currentSaveSlotName = *LoadGameInstance->SaveSlots.FindKey(saveDates[i]);

			if (UGameplayStatics::DoesSaveGameExist(currentSaveSlotName, 0))
			{
				USaveLoadDialogueEntry* savegameEntry = CreateWidget<USaveLoadDialogueEntry>(PlayerController, SaveLoadDialogueEntryBlueprint);
				if (savegameEntry)
				{
					savegameEntry->SaveSlotName = currentSaveSlotName;
					savegameEntry->SaveTime = saveDates[i];
					savegameEntry->MySaveLoadDialogue = this;

					dialogueEntryPanel->AddChild(savegameEntry);
				}
			}
			else
			{
				invalidSaveSlots.Add(currentSaveSlotName);
			}
		}
		PlayerController->RemoveInvalidSaveGamesFromMeta(invalidSaveSlots);
	}
}
