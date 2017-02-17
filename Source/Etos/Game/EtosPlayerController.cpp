// © 2016 - 2017 Daniel Bortfeld

#include "Etos.h"
#include "EtosPlayerController.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Etos/Utility/FunctionLibraries/BuildingFunctionLibrary.h"
#include "Etos/UI/InGameUI.h"
#include "Etos/Game/EtosHUD.h"
#include "Etos/Buildings/Path.h"
#include "Etos/Collision/BoxCollider.h"
#include "Etos/Buildings/Warehouse.h"
#include "Etos/Buildings/Residence.h"
#include "EtosGameMode.h"
#include "Etos/Pawns/MarketBarrow.h"
#include "Etos/Game/EtosSaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "Etos/UI/ResourcePopup.h"
#include "EtosMetaSaveGame.h"
#include "Etos/Utility/FunctionLibraries/WarningDialogueFunctions.h"

void AEtosPlayerController::BeginPlay()
{
	PrimaryActorTick.bCanEverTick = true;
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;

	AddGUIToViewport();
	UpdatePopulation(EResidentLevel::Peasant, 0); // updates UI
	UpdateBalanceUI(totalIncome, totalUpkeep);
	InitResourceMapping();

	pathPool = NewObject<UObjectPool>();

	if (pathPool)
	{
		pathPool->SetMinPooledObjectsAmount(8);
		pathPool->SetMaxPooledObjectsAmount(256);
	}

	// ### start resources - difficulty: easy ###
	AddResource(FResource(EResource::Money, 10000));
	AddResource(FResource(EResource::Wood, 40));
	AddResource(FResource(EResource::Tool, 40));
	AddResource(FResource(EResource::Fish, 30));
	// ##########################################

	// ############## for testing ###############
	AddResource(FResource(EResource::Iron, 20));
	AddResource(FResource(EResource::Stone, 20));
	//###########################################
}

void AEtosPlayerController::Tick(float DeltaTime)
{
	if (IsPaused())
		return;

	if (bIsBulidingPath)
	{
		UpdatePathPreview();
	}
	AddIncome(DeltaTime);
}

void AEtosPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	InputComponent->BindAction("Build", IE_Pressed, this, &AEtosPlayerController::BuildNewBuilding); // LMB
	InputComponent->BindAction("Pause", IE_Pressed, this, &AEtosPlayerController::PauseGame).bExecuteWhenPaused = true; // P
	InputComponent->BindAction("Escape", IE_Pressed, this, &AEtosPlayerController::ShowGameMenu).bExecuteWhenPaused = true; // ESC
	InputComponent->BindAction("ClickRepeatedly", IE_Pressed, this, &AEtosPlayerController::ClickRepeatedly); // Shift + LMB
	InputComponent->BindAction("CancelBuilding", IE_Pressed, this, &AEtosPlayerController::CancelPlacementOfBuilding).bConsumeInput = false; // RMB
	InputComponent->BindAction("Select", IE_Pressed, this, &AEtosPlayerController::SelectBuilding); // LMB
	InputComponent->BindAction("Demolish", IE_Pressed, this, &AEtosPlayerController::DemolishBuilding); // LMB
	InputComponent->BindAction("RotateBuilding", IE_Pressed, this, &AEtosPlayerController::RotateHeldBuilding); // ,

#if WITH_EDITOR
	InputComponent->BindAction("QuickSave", IE_Pressed, this, &AEtosPlayerController::QuickSave); // F5
	InputComponent->BindAction("QuickLoad", IE_Pressed, this, &AEtosPlayerController::QuickLoad); // F9
#endif
}

FORCEINLINE void AEtosPlayerController::AddResource(FResource in resource)
{
	AddResource(resource.Type, resource.Amount);
}

void AEtosPlayerController::AddResource(EResource in resource, int32 in amount)
{
	if (Enum::IsValid(resource))
	{
		resourceAmounts[resource] += amount;
		GetInGameUI()->UpdateResourceAmounts();
	}
}

FORCEINLINE void AEtosPlayerController::RemoveResource(FResource in resource)
{
	RemoveResource(resource.Type, resource.Amount);
}

void AEtosPlayerController::RemoveResource(EResource in resource, int32 in amount)
{
	if (Enum::IsValid(resource))
	{
		resourceAmounts[resource] -= amount;
		GetInGameUI()->UpdateResourceAmounts();
	}
}

bool AEtosPlayerController::TryRemovingResource(FResource in resource)
{
	return TryRemovingResource(resource.Type, resource.Amount);
}

bool AEtosPlayerController::TryRemovingResource(EResource in resource, int32 in amount)
{
	if (GetResourceAmount(resource) > 0)
	{
		RemoveResource(resource, amount);
		return true;
	}

	return false;
}

FORCEINLINE int32 AEtosPlayerController::GetResourceAmount(EResource in resource)
{
	return resourceAmounts.FindOrAdd(resource);
}

void AEtosPlayerController::UpdatePopulation(EResidentLevel in level, int32 in deltaPolulation)
{
	if (!Enum::IsValid(level))
		return;

	totalPopulation += deltaPolulation;
	populationPerLevel.FindOrAdd(level) += deltaPolulation;

	if (auto* const GUI = GetInGameUI())
	{
		GUI->UpdatePopulation(populationPerLevel.FindOrAdd(EResidentLevel::Peasant), populationPerLevel.FindOrAdd(EResidentLevel::Citizen));
	}
}

void AEtosPlayerController::UpdatePopulation(EResidentLevel in from, EResidentLevel in to, int32 in residents)
{
	if (!Enum::IsValid(from) || !Enum::IsValid(to))
		return;

	populationPerLevel.FindOrAdd(from) -= residents;
	populationPerLevel.FindOrAdd(to) += residents;

	if (auto* const GUI = GetInGameUI())
	{
		GUI->UpdatePopulation(populationPerLevel.FindOrAdd(EResidentLevel::Peasant), populationPerLevel.FindOrAdd(EResidentLevel::Citizen));
	}
}

void AEtosPlayerController::ReportUpgrade(AResidence * in upgradedResidence, EResidentLevel in levelBeforeUpgrade, EResidentLevel in levelAfterUpgrade)
{
	if (Enum::IsValid(levelBeforeUpgrade) && Enum::IsValid(levelAfterUpgrade))
	{
		++usedPromotions.FindOrAdd(levelAfterUpgrade);
		builtResidences.FindOrAdd(levelAfterUpgrade).Residences.AddUnique(upgradedResidence);
		builtResidences.FindOrAdd(levelBeforeUpgrade).Residences.Remove(upgradedResidence);
	}
}

int32 AEtosPlayerController::GetTotalPopulation() const
{
	return totalPopulation;
}

int32 AEtosPlayerController::GetPopulationAmount(EResidentLevel in level)
{
	return populationPerLevel.FindOrAdd(level);
}

void AEtosPlayerController::UpdateUpkeep(int32 in deltaUpkeep)
{
	totalUpkeep += deltaUpkeep;
	UpdateBalanceUI(totalIncome, totalUpkeep);
}

int32 AEtosPlayerController::GetTotalUpkeep()
{
	return totalUpkeep;
}

void AEtosPlayerController::UpdateStorage(int32 in deltaStorage)
{
	totalStorage += deltaStorage;
}

int32 AEtosPlayerController::GetTotalStorage()
{
	return totalStorage;
}

FORCEINLINE UInGameUI * AEtosPlayerController::GetInGameUI()
{
	return UUtilityFunctionLibrary::GetEtosHUD(this)->GetInGameUI();
}

int32 AEtosPlayerController::GetAvailablePromotions(EResidentLevel in to)
{
	switch (to)
	{
	case EResidentLevel::Citizen:
		return (float)builtResidences.FindOrAdd(EResidentLevel::Peasant).Residences.Num() * 0.8f - usedPromotions.FindOrAdd(to);
	default:
		return 0;
	}
}

FORCEINLINE void AEtosPlayerController::PauseGame(FKey key)
{
	if (bIsInMainMenu || bIsInGameMenu)
		return;

	ServerPause();

	if (IsPaused())
	{
		if (auto* const GUI = GetInGameUI())
		{
			GUI->RemoveFromParent();
			if (auto* const HUD = Util::GetEtosHUD(this))
			{
				if (auto* const PausedScreen = HUD->GetPausedScreen())
				{
					PausedScreen->AddToViewport();
				}
			}
		}
	}
	else
	{
		if (auto* const World = GetWorld())
		{
			if (auto* const HUD = Util::GetEtosHUD(this))
			{
				if (auto* const PausedScreen = HUD->GetPausedScreen())
				{
					PausedScreen->RemoveFromParent();
				}
			}
		}

		if (auto* const GUI = GetInGameUI())
		{
			GUI->AddToViewport();
		}
	}
}

FORCEINLINE ABuilding* AEtosPlayerController::SpawnBuilding(ABuilding* in Class, FBuildingData in Data)
{
	return SpawnBuilding_Internal(Class->GetClass(), Data);
}

FORCEINLINE ABuilding * AEtosPlayerController::SpawnBuilding(TSubclassOf<ABuilding> in Subclass, FBuildingData in Data)
{
	return SpawnBuilding_Internal(Subclass, Data);
}

AResourcePopup * AEtosPlayerController::SpawnTextPopup(FVector in Position, FText in Text, FLinearColor in TextColor)
{
	if (auto* const HUD = Util::GetEtosHUD(this))
	{
		checkf(HUD->TextPopupBlueprint, TEXT("No default Text Popup Actor Blueprint was selected for EtosHUD"));

		if (UWorld* const World = GetWorld())
		{
			FActorSpawnParameters params = FActorSpawnParameters();
			params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			FVector cameraLocation = PlayerCameraManager->GetCameraLocation();

			FRotator rotation = UKismetMathLibrary::FindLookAtRotation(cameraLocation + PlayerCameraManager->GetCameraRotation().Vector(), cameraLocation);

			AResourcePopup* popup = World->SpawnActor<AResourcePopup>(HUD->TextPopupBlueprint, Position, rotation, params);
			popup->BPEvent_UpdateWidget(nullptr, Text, FSlateColor(TextColor));
			return popup;
		}
	}
	return nullptr;
}

void AEtosPlayerController::Win()
{
	ServerPause();

	if (auto* HUD = Util::GetEtosHUD(this))
	{
		auto WinScreen = HUD->GetWinScreen();

		if (WinScreen)
		{
			WinScreen->AddToViewport();
		}
	}
}

void AEtosPlayerController::Lose()
{
	if (auto* HUD = Util::GetEtosHUD(this))
	{
		auto LoseScreen = HUD->GetLoseScreen();

		if (LoseScreen)
		{
			LoseScreen->AddToViewport();
		}
	}
}

inline void AEtosPlayerController::BuildNewBuilding(FKey key)
{
	static bool bSkipStartBuildingPath;

	if (newBuilding && (newBuilding->Data.bIsHeld || bIsBulidingPath))
	{
		if (!newBuilding->Data.bPositionIsBlocked)
		{
			if (HasEnoughResources(newBuilding->Data.BuildCost))
			{
				if (bIsBulidingPath)
				{
					bSkipStartBuildingPath = true;
					bIsBulidingPath = false;
					for (APath* path : tempPaths)
					{
						path->Data.bIsHeld = true;
						newBuilding = path;
						BuildNewBuilding(key);
					}
					tempPaths.Empty();
					bSkipStartBuildingPath = false;
				}
				else
				{
					if (bSkipStartBuildingPath)
					{
						BuildNewBuilding_Internal();
					}
					else if (APath* newPath = dynamic_cast<APath*, ABuilding>(newBuilding))
					{
						StartBuildingPath(newPath);
					}
					else
					{
						BuildNewBuilding_Internal();
					}
				}
			}
			else SpawnTextPopup(newBuilding->GetActorLocation() + FVector(0,0,300), FText::FromName(TEXT("Not enough resources"))); 
		}
		else SpawnTextPopup(newBuilding->GetActorLocation() + FVector(0, 0, 300), FText::FromName(TEXT("Position is blocked"))); 
	}
}

void AEtosPlayerController::ShowGameMenu(FKey key)
{
	if (bIsInMainMenu)
		return;

	if (bIsInGameMenu)
	{
		bIsInGameMenu = false;
		if (IsPaused())
		{
			PauseGame(key);
		}
	}
	else
	{
		if (!IsPaused())
		{
			PauseGame(key);
		}
		bIsInGameMenu = true;
	}

	if (auto* const HUD = Util::GetEtosHUD(this))
	{
		if (auto* const GameMenu = HUD->GetGameMenu())
		{
			if (IsPaused())
			{
				GameMenu->AddToViewport();
			}
			else
			{
				GameMenu->RemoveFromParent();
			}
		}
	}
}

void AEtosPlayerController::ClickRepeatedly(FKey key)
{
	if (bIsHoldingObject)
	{
		FBuildingData data = FBuildingData(newBuilding->Data);
		BuildNewBuilding(key);
		if (!bIsHoldingObject)
		{
			SpawnBuilding(newBuilding, data);
		}
	}
}

void AEtosPlayerController::SelectBuilding(FKey key)
{
	if (bIsHoldingObject)
		return;

	FHitResult Hit = FHitResult();
	if (Util::TraceSingleAtMousePosition(this, Hit, 100000, Util::BuildingObjectType))
	{
		if (UInGameUI* const GUI = GetInGameUI())
		{
			if (dynamic_cast<APath*, AActor>(&*Hit.Actor) == nullptr)
			{
				if (ABuilding* const building = dynamic_cast<ABuilding*, AActor> (&*Hit.Actor))
				{
					if (AResidence* const residence = dynamic_cast<AResidence*, ABuilding>(building))
					{
						GUI->ShowResidenceInfo(residence);
						GUI->BPEvent_HideBuildingInfo();
						GUI->ShowBuildButtons();
						GUI->HideResourceInfo();
					}
					else
					{
						GUI->ShowBuildingInfo(building);
						GUI->HideResidenceInfo();

						if (AWarehouse* const warehouse = dynamic_cast<AWarehouse*, ABuilding> (building))
						{
							GUI->HideBuildButtons();
							GUI->ShowResourceInfo(resourceAmounts);
						}
						else
						{
							GUI->ShowBuildButtons();
							GUI->HideResourceInfo();
						}
					}
				}
			}
		}
	}
	else if (UInGameUI* const GUI = GetInGameUI())
	{
		GUI->BPEvent_HideBuildingInfo();
		GUI->HideResourceInfo();
		GUI->HideResidenceInfo();
		GUI->ShowBuildButtons();
	}
}

void AEtosPlayerController::DemolishBuilding(FKey key)
{
	if (bIsInDemolishMode)
	{
		FHitResult Hit = FHitResult();
		if (Util::TraceSingleAtMousePosition(this, Hit, 100000.f, Util::BuildingObjectType))
		{
			if (ABuilding* const building = dynamic_cast<ABuilding*, AActor>(&*Hit.Actor))
			{
				if (OnDemolish.IsBound())
				{
					OnDemolish.Broadcast(building);
				}
				building->Demolish();
			}
		}
	}
}

void AEtosPlayerController::RotateHeldBuilding(FKey key)
{
	if (bIsHoldingObject)
	{
		if (!newBuilding->Data.bIsBuilt)
		{
			FRotator rotation = newBuilding->GetActorRotation();
			newBuilding->SetActorRotation(FRotator(0, FMath::RoundToInt(rotation.Yaw) % 360 + 90, 0));
		}
	}
}

void AEtosPlayerController::OnBuildingDestroyed(AActor * DestroyedActor)
{
	if (ABuilding* const building = dynamic_cast<ABuilding*, AActor>(DestroyedActor))
	{
		ReportDestroyedBuilding(building);
	}
}

void AEtosPlayerController::SaveToTempSlot()
{
	Save(tempSaveSlotName);
}

void AEtosPlayerController::LoadFromTempSlot()
{
	Load(tempSaveSlotName);
}

void AEtosPlayerController::LoadLatestSaveGame_Wrapper()
{
	LoadLatestSaveGame();
}

void AEtosPlayerController::AddIncome(float in DeltaTime)
{
	incomeTimerPassed += DeltaTime;
	if (incomeTimerPassed >= incomeTimerTotal)
	{
		incomeTimerPassed = 0;

		if (auto GM = Util::GetEtosGameMode(this))
		{
			float taxPerResident = 0;
			float population = 0;
			float currentTaxIncome = 0;
			totalIncome = 0;

			for (uint8 i = 1; i < (uint8)EResidentLevel::EResidentLevel_MAX; ++i)
			{
				EResidentLevel level = (EResidentLevel)i;

				auto taxData = GM->GetTaxData(level);

				taxPerResident = 0;

				int32 j = 0;
				bool removeNullptrs = false;
				for (AResidence* residence : builtResidences.FindOrAdd(level).Residences)
				{
					if (residence == nullptr)
					{
						removeNullptrs = true;
						continue;
					}

					if (residence->MyLevel == level)
					{
						for (auto& pair : taxData->taxPerResource)
						{
							taxPerResident += residence->GetSatisfaction(pair.Resource) * pair.Tax;
						}
						++j;
					}
				}

				if (removeNullptrs)
				{
					builtResidences[level].Residences.Shrink();
				}

				if (j > 0)
				{
					taxPerResident /= j;
				}

				taxPerResident += taxData->BaseTax;

				population = populationPerLevel.FindOrAdd(level);
				currentTaxIncome += population * taxPerResident;
			}

			totalIncome = currentTaxIncome;

			payedIncome += currentTaxIncome * ((float)incomeTimerTotal / 60.f);
			payedIncome -= (float)totalUpkeep * ((float)incomeTimerTotal / 60.f);

			int32 income = payedIncome;
			payedIncome -= income;

			AddResource(EResource::Money, income);

			UpdateBalanceUI(totalIncome, totalUpkeep);
		}
	}
}

FORCEINLINE bool AEtosPlayerController::HasEnoughResources(TArray<FResource> in buildCost) const
{
	for (const FResource& cost : buildCost)
	{
		if (cost.Amount > resourceAmounts[cost.Type])
		{
			return false;
		}
	}
	return true;
}

void AEtosPlayerController::ReportDestroyedBuilding(ABuilding * in destroyedBuilding)
{
	if (destroyedBuilding)
	{
		if (builtBuildings.Contains(destroyedBuilding))
		{
			builtBuildings.Remove(destroyedBuilding);
		}

		if (AResidence* residence = dynamic_cast<AResidence*, ABuilding>(destroyedBuilding))
		{
			if (builtResidences.FindOrAdd(residence->MyLevel).Residences.Contains(residence))
			{
				builtResidences[residence->MyLevel].Residences.Remove(residence);
			}
		}
	}
}

bool AEtosPlayerController::Save(FString SaveSlotName)
{
	//if (UGameplayStatics::DoesSaveGameExist(SaveSlotName, 0))
	//{
		// display warning
		//if (warning says no)
		//{
		//	return false;
		//}
	//}

	UEtosSaveGame* SaveGameInstance = Cast<UEtosSaveGame>(UGameplayStatics::CreateSaveGameObject(UEtosSaveGame::StaticClass()));
	SaveGameInstance->PlayerName = TEXT("Player 1");
	SaveGameInstance->SaveSlotName = SaveSlotName;
	SaveGameInstance->UserIndex = 0;

	SaveGameInstance->ResourceAmounts = this->resourceAmounts;
	SaveGameInstance->PopulationPerLevel = this->populationPerLevel;
	SaveGameInstance->UsedPromotions = this->usedPromotions;

	for (const auto& building : this->builtBuildings)
	{
		if (building->IsValidLowLevel())
		{
			if (AResidence * const residence = dynamic_cast<AResidence*, ABuilding>(building))
			{
				SaveGameInstance->AddResidence(residence);
			}
			else
			{
				SaveGameInstance->AddBuilding(building);
			}
		}
	}

	if (auto const World = GetWorld())
	{
		for (TActorIterator<AMarketBarrow> ActorItr(World); ActorItr; ++ActorItr)
		{
			if (ActorItr->IsValidLowLevel())
			{
				auto resource = ActorItr->GetTransportedResource();
				if (resource.Amount > 0)
				{
					SaveGameInstance->ResourcesOnTransit.Add(resource);
				}
			}
		}
	}

	if (UGameplayStatics::SaveGameToSlot(SaveGameInstance, SaveGameInstance->SaveSlotName, SaveGameInstance->UserIndex))
	{
		return AddSlotNameToMeta(SaveGameInstance->SaveSlotName);
	}
	return false;
}

void AEtosPlayerController::SaveWithWarning(FString SaveSlotName)
{
	tempSaveSlotName = SaveSlotName;

	if (auto HUD = Util::GetEtosHUD(this))
	{
		checkf(HUD->wWarning, TEXT("No default Warning Widget was selected for EtosHUD"));

		FWarning::ShowWarningDialogue(AEtosPlayerController, UObject, this, HUD->wWarning, this, &AEtosPlayerController::SaveToTempSlot, nullptr, nullptr);
	}
}

bool AEtosPlayerController::Load(FString SaveSlotName)
{
	if (!UGameplayStatics::DoesSaveGameExist(SaveSlotName, 0))
	{
		UE_LOG(LogTemp, Warning, TEXT("%s was not found"), *SaveSlotName);
		RemoveSlotNameFromMeta(SaveSlotName);
		return false;
	}

	UEtosSaveGame* LoadGameInstance = Cast<UEtosSaveGame>(UGameplayStatics::CreateSaveGameObject(UEtosSaveGame::StaticClass()));
	LoadGameInstance = Cast<UEtosSaveGame>(UGameplayStatics::LoadGameFromSlot(SaveSlotName, LoadGameInstance->UserIndex));

	if (LoadGameInstance->IsValidLowLevel())
	{
		UWorld* const World = GetWorld();

		if (World)
		{
			// clear all buildings of current game
			for (TActorIterator<ABuilding> ActorItr(World); ActorItr; ++ActorItr)
			{
				ActorItr->Destroy();
				ActorItr->ConditionalBeginDestroy();
			}

			for (TActorIterator<AMarketBarrow> ActorItr(World); ActorItr; ++ActorItr)
			{
				ActorItr->Destroy();
				ActorItr->ConditionalBeginDestroy();
			}
		}

		totalUpkeep = 0;
		totalPopulation = 0;
		for (auto& counter : this->populationPerLevel)
		{
			counter.Value = 0;
		}
		UpdatePopulationUI(0, 0);

		if (LoadGameInstance->ResourceAmounts.Num() > 0)
		{
			this->resourceAmounts = LoadGameInstance->ResourceAmounts;
			if (auto* const GUI = GetInGameUI())
			{
				GUI->UpdateResourceAmounts();
			}
		}
		if (LoadGameInstance->UsedPromotions.Num() > 0)
		{
			this->usedPromotions = LoadGameInstance->UsedPromotions;
		}

		if (const auto GM = Util::GetEtosGameMode(this))
		{
			int32 count = GM->GetBuildingAmount();

			TMap<FName, FPredefinedBuildingData> BuildingData;

			for (int32 i = 1; i < count; ++i)
			{
				if (auto data = GM->GetPredefinedBuildingData(i))
					BuildingData.Add(data->Name, *data);
			}

			builtBuildings.Reset();
			builtResidences.Reset();

			if (LoadGameInstance->BuiltBuildings.Num() > 0)
			{
				for (auto& buildingData : LoadGameInstance->BuiltBuildings)
				{
					auto data = BuildingData[buildingData.Name];
					SpawnBuilding(data.BuildingBlueprint, FBuildingData(data));

					if (newBuilding->IsValidLowLevel())
					{
						BuildLoadedBuilding(buildingData);
					}
				}
			}

			if (LoadGameInstance->BuiltResidences.Num() > 0)
			{
				auto data = BuildingData[TEXT("Farmhouse")]; // maybe make variable in case the name changes in predefData or localization
				for (auto& residenceData : LoadGameInstance->BuiltResidences)
				{
					SpawnBuilding(data.BuildingBlueprint, FBuildingData(data));

					if (newBuilding->IsValidLowLevel())
					{
						if (AResidence * const residence = dynamic_cast<AResidence*, ABuilding>(newBuilding))
						{
							residence->MyLevel = residenceData.Level;
							residence->Residents = residenceData.Residents;
							residence->MaxResidents = residenceData.MaxResidents;
							BuildLoadedBuilding(residenceData);
							if (residence->MyLevel == EResidentLevel::Citizen)
							{
								residence->UpgradeToCitizen();
							}
							residence->SetAllSatisfactions(residenceData.ResourceSatisfaction, residenceData.NeedsSatisfaction, residenceData.TotalSatisfaction);
						}
					}
				}
			}

			if (World)
			{
				for (TActorIterator<APath> ActorItr(World); ActorItr; ++ActorItr)
				{
					ActorItr->ReconnectToSurroundings();
				}
			}

			if (LoadGameInstance->ResourcesOnTransit.Num() > 0)
			{
				for (auto& resource : LoadGameInstance->ResourcesOnTransit)
				{
					AddResource(resource);
				}
			}
		}
		return true;
	}
	return false;
}

void AEtosPlayerController::LoadWithWarning(FString SaveSlotName)
{
	tempSaveSlotName = SaveSlotName;

	if (auto HUD = Util::GetEtosHUD(this))
	{
		checkf(HUD->wWarning, TEXT("No default Warning Widget was selected for EtosHUD"));

		FWarning::ShowWarningDialogue(AEtosPlayerController, UObject, this, HUD->wWarning, this, &AEtosPlayerController::LoadFromTempSlot, nullptr, nullptr);
	}
}

bool AEtosPlayerController::LoadLatestSaveGame()
{
	if (auto* MetaSaveGameInstance = GetMetaSaveGame())
	{
		if (MetaSaveGameInstance->SaveSlots.Num() <= 0)
			return false;

		TArray<FDateTime> saveDates;
		MetaSaveGameInstance->SaveSlots.GenerateValueArray(saveDates);
		saveDates.Sort();

		FString latestSlotName = *MetaSaveGameInstance->SaveSlots.FindKey(saveDates.Last());

		if (UGameplayStatics::DoesSaveGameExist(latestSlotName, 0))
		{
			return Load(latestSlotName);
		}
		else
		{
			RemoveSlotNameFromMeta(latestSlotName);
		}
	}

	return false;
}

void AEtosPlayerController::LoadLatestSaveGameWithWarning()
{
	if (auto HUD = Util::GetEtosHUD(this))
	{
		checkf(HUD->wWarning, TEXT("No default Warning Widget was selected for EtosHUD"));

		FWarning::ShowWarningDialogue(AEtosPlayerController, UObject, this, HUD->wWarning, this, &AEtosPlayerController::LoadLatestSaveGame_Wrapper, nullptr, nullptr);
	}
}

void AEtosPlayerController::QuickSave()
{
	Save(TEXT("QuickSave"));
}

void AEtosPlayerController::QuickLoad()
{
	Load(TEXT("QuickSave"));
}

void AEtosPlayerController::TogglePause()
{
	PauseGame(FKey());
}

void AEtosPlayerController::ToggleGameMenu()
{
	ShowGameMenu(FKey());
}

void AEtosPlayerController::StartDemolishMode()
{
	SetDemolishMode(true);
}

bool AEtosPlayerController::GetWarehouseWasBuilt()
{
	return bWarehouseWasBuilt;
}

bool AEtosPlayerController::GetMarketWasBuilt()
{
	return bMarketWasBuilt;
}

bool AEtosPlayerController::RemoveInvalidSaveGamesFromMeta(TArray<FString>in invalidSaveSlots)
{
	bool bRemovedAny = false;

	if (invalidSaveSlots.Num() > 0)
	{
		for (auto& slotName : invalidSaveSlots)
		{
			if (!UGameplayStatics::DoesSaveGameExist(slotName, 0))
			{
				if (RemoveSlotNameFromMeta(slotName))
				{
					bRemovedAny = true;
				}
			}
		}
	}

	return bRemovedAny;
}

FORCEINLINE void AEtosPlayerController::AddGUIToViewport()
{
#if WITH_EDITOR
	try
	{
#endif // WITH_EDITOR

		UInGameUI* const GUI = GetInGameUI();
		if (GUI != nullptr)
		{
			GUI->AddToViewport();
		}
		else if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(1, 5, FColor(1, 1, 1, 1), TEXT("Gui was not found"));
		}

#if WITH_EDITOR
	}
	catch (const std::exception& e)
	{
		Panic(e);
	}
#endif // WITH_EDITOR
}

void AEtosPlayerController::Panic(std::exception in wait)
{
	UE_LOG(LogTemp, Error, TEXT("A %s occured. Oh Shit!"), wait.what());
}

FORCEINLINE void AEtosPlayerController::InitResourceMapping()
{
	uint8 max = static_cast<uint8>(EResource::EResource_MAX);

	for (uint8 i = 0; i < max; i++)
	{
		resourceAmounts.Add(static_cast<EResource>(i));
	}
}

void AEtosPlayerController::CancelPlacementOfBuilding(FKey key)
{
	if (bIsHoldingObject)
	{
		if (tempPaths.Num() > 0)
		{
			for (int32 i = tempPaths.Num() - 1; i >= 0; --i)
			{
				DestroyPathPreview(tempPaths[i]);
			}
			tempPaths.Empty(1);
			bIsHoldingObject = false;
			bIsBulidingPath = false;
			UE_LOG(LogTemp, Warning, TEXT("building canceled"));
		}
		else if (newBuilding->IsValidLowLevel())
		{
			if (newBuilding->OccupiedBuildSpace_Custom)
			{
				newBuilding->OccupiedBuildSpace_Custom->SetGenerateCollisionEvents(false);
			}
			newBuilding->Data.bIsHeld = false;
			newBuilding->Destroy();
			newBuilding->ConditionalBeginDestroy();
			bIsHoldingObject = false;
			UE_LOG(LogTemp, Warning, TEXT("building canceled"));
		}
	}

	if (bIsInDemolishMode)
	{
		SetDemolishMode(false);
	}
}

void AEtosPlayerController::PayCostsOfBuilding(ABuilding* in building)
{
	auto* popupList = building->SpawnResourcePopupList(FVector(0, 0, 200));

	for (FResource const& cost : building->Data.BuildCost)
	{
		RemoveResource(cost);

		FString Text = TEXT("-");
		Text.AppendInt(cost.Amount);

		if (popupList)
		{
			popupList->BPEvent_UpdateWidget(cost.Icon, FText::FromString(Text), FLinearColor::Red);
		}
	}
}

ABuilding * AEtosPlayerController::SpawnBuilding_Internal(UClass * in Class, FBuildingData in Data)
{
	if (bIsHoldingObject)
		CancelPlacementOfBuilding(FKey());

	SetDemolishMode(false);

	if (!Class)
		return nullptr;

	FBuildingData data = Data;

	if (UWorld* World = GetWorld())
	{
		newBuilding = World->SpawnActor<ABuilding>(Class);
		newBuilding->Data = data;
		newBuilding->Data.bIsHeld = true;
		newBuilding->Data.bPositionIsBlocked = false;
		newBuilding->Radius->SetSphereRadius(data.Radius);
		newBuilding->SetFoundationSize(newBuilding->Width, newBuilding->Height);
		bIsHoldingObject = true;

		return newBuilding;
	}
	return nullptr;
}

void AEtosPlayerController::BuildNewBuilding_Internal(bool in skipCosts)
{
	newBuilding->Data.bIsHeld = false;
	bIsHoldingObject = false;

	if (!skipCosts)
		PayCostsOfBuilding(newBuilding);

	if (builtBuildings.AddUnique(newBuilding) != INDEX_NONE)
	{
		if (AResidence* residence = dynamic_cast<AResidence*, ABuilding>(newBuilding))
		{
			builtResidences.FindOrAdd(residence->MyLevel).Residences.AddUnique(residence);
		}
		else if (!bWarehouseWasBuilt)
		{
			bWarehouseWasBuilt = dynamic_cast<AWarehouse*, ABuilding>(newBuilding) != nullptr;
		}
		else if (!bMarketWasBuilt)
		{
			bMarketWasBuilt = dynamic_cast<ATownCenter*, ABuilding>(newBuilding) != nullptr;
		}

		newBuilding->OnDestroyed.AddDynamic(this, &AEtosPlayerController::OnBuildingDestroyed);

		newBuilding->Build();
	}
}

void AEtosPlayerController::BuildLoadedBuilding(FBuildingSaveData in Data)
{
	newBuilding->SetActorTransform(Data.Transform);
	newBuilding->Data.bPositionIsBlocked = false;
	BuildNewBuilding_Internal(true);
	newBuilding->SetBarrowsInUse(0);
	newBuilding->Data.bBarrowIsOnTheWay = false;
	newBuilding->Data.NeededResource1.Amount = Data.NeededResource1Amount;
	newBuilding->Data.NeededResource2.Amount = Data.NeededResource2Amount;
	newBuilding->Data.ProducedResource.Amount = Data.ProducedResourceAmount;
	newBuilding->SetActive(Data.bIsActive);
}

FORCEINLINE void AEtosPlayerController::StartBuildingPath(APath* in newPath)
{
	tempPaths.Add(newPath);
	newPath->Data.bIsHeld = false;
	bIsBulidingPath = true;
	previousMouseLocation = newPath->GetActorLocation();
}

inline void AEtosPlayerController::UpdatePathPreview()
{
	FHitResult Hit;
	if (Util::TraceSingleAtMousePosition(this, Hit))
	{
		if (FVector::Dist(Hit.ImpactPoint, previousMouseLocation) > mouseMoveThreshold)
		{
			previousMouseLocation = Hit.ImpactPoint;

			if (UWorld* const World = GetWorld())
			{
				for (int32 i = tempPaths.Num() - 1; i > 0; --i)
				{
					DestroyPathPreview(tempPaths[i]);
				}

				FVector mouseGridLocation = BFuncs::GetNextGridLocation(Hit.ImpactPoint, FVector2Di(1, 1));
				FVector startLocation = tempPaths[0]->GetActorLocation();

				FVector offsetX = startLocation.X < mouseGridLocation.X ? FVector(100, 0, 0) : FVector(-100, 0, 0);

				APath* p = tempPaths[0];
				tempPaths.Empty(UKismetMathLibrary::Abs(UKismetMathLibrary::Abs(mouseGridLocation.X) - UKismetMathLibrary::Abs(startLocation.X)) / 100);
				tempPaths.Insert(p, 0);

				int32 i = 1;
				FVector currentLocation;
				for (currentLocation = BFuncs::GetNextGridLocation(startLocation + offsetX, FVector2Di(1, 1), 0); (startLocation.X < mouseGridLocation.X) ? (currentLocation.X <= mouseGridLocation.X) : (currentLocation.X >= mouseGridLocation.X); currentLocation = BFuncs::GetNextGridLocation(currentLocation + offsetX, FVector2Di(1, 1), 0))
				{
					SpawnPathPreview(currentLocation, i++, World);
				}

				startLocation = currentLocation - offsetX;
				FVector offsetY = currentLocation.Y < mouseGridLocation.Y ? FVector(0, 100, 0) : FVector(0, -100, 0);

				for (currentLocation = BFuncs::GetNextGridLocation(startLocation + offsetY, FVector2Di(1, 1), 0); (startLocation.Y < mouseGridLocation.Y) ? (currentLocation.Y <= mouseGridLocation.Y) : (currentLocation.Y >= mouseGridLocation.Y); currentLocation = BFuncs::GetNextGridLocation(currentLocation + offsetY, FVector2Di(1, 1), 0))
				{
					SpawnPathPreview(currentLocation, i++, World);
				}
			}
		}
	}
}

FORCEINLINE void AEtosPlayerController::SpawnPathPreview(FVector in spawnLocation, int32 in index, UWorld* in World)
{
	bool bGotPathFromPool = false;
	APath* newPath = pathPool->GetPooledObject<APath*>(bGotPathFromPool);

	if (bGotPathFromPool)
	{
		newPath->SetActive(true);
		newPath->SetActorLocation(spawnLocation);
	}
	else
	{
		newPath = World->SpawnActor<APath>(newBuilding->GetClass(), FTransform(spawnLocation));
	}

	tempPaths.Insert(newPath, index);
	tempPaths[index]->Data = newBuilding->Data;
	tempPaths[index]->Data.bIsHeld = false;
	tempPaths[index]->Data.bPositionIsBlocked = false;
	tempPaths[index]->SetFoundationSize(tempPaths[index]->Width, tempPaths[index]->Height);
}

void AEtosPlayerController::DestroyPathPreview(APath * in tempPath)
{
	if (pathPool->AddObjectToPool(tempPath))
	{
		tempPath->SetActive(false);
	}
	else
	{
		if (tempPath->OccupiedBuildSpace_Custom)
		{
			tempPath->OccupiedBuildSpace_Custom->UnregisterCollider();
		}
		tempPath->Destroy();
	}
}

void AEtosPlayerController::UpdatePopulationUI(int32 in peasants, int32 in citizens)
{
	if (auto* const GUI = GetInGameUI())
	{
		GUI->UpdatePopulation(peasants, citizens);
	}
}

void AEtosPlayerController::UpdateBalanceUI(int32 in income, int32 in upkeep)
{
	if (auto* const GUI = GetInGameUI())
	{
		GUI->UpdateBalance(income, upkeep);
	}
}

bool AEtosPlayerController::AddSlotNameToMeta(FString in slotName)
{
	UEtosMetaSaveGame* MetaSaveGameInstance = GetMetaSaveGame();

	MetaSaveGameInstance->SaveSlots.Add(slotName, FDateTime::Now());

	return UGameplayStatics::SaveGameToSlot(MetaSaveGameInstance, MetaSaveGameInstance->SaveSlotName, 0);
}

bool AEtosPlayerController::RemoveSlotNameFromMeta(FString in slotName)
{
	UEtosMetaSaveGame* MetaSaveGameInstance = GetMetaSaveGame();

	MetaSaveGameInstance->SaveSlots.Remove(slotName);

	return UGameplayStatics::SaveGameToSlot(MetaSaveGameInstance, MetaSaveGameInstance->SaveSlotName, 0);
}

void AEtosPlayerController::SetDemolishMode(bool in newState)
{
	if (newState != bIsInDemolishMode)
	{
		bIsInDemolishMode = newState;

		if (bIsInDemolishMode)
		{
			CurrentMouseCursor = EMouseCursor::Crosshairs;
		}
		else
		{
			CurrentMouseCursor = EMouseCursor::Default;
		}
	}
}

UEtosMetaSaveGame * AEtosPlayerController::GetMetaSaveGame()
{
	FString metaName = TEXT("Meta");
	UEtosMetaSaveGame* MetaSaveGameInstance = Cast<UEtosMetaSaveGame>(UGameplayStatics::CreateSaveGameObject(UEtosMetaSaveGame::StaticClass()));
	if (UGameplayStatics::DoesSaveGameExist(metaName, 0))
	{
		MetaSaveGameInstance = Cast<UEtosMetaSaveGame>(UGameplayStatics::LoadGameFromSlot(metaName, 0));
	}
	else
	{
		MetaSaveGameInstance->PlayerName = TEXT("Player 1");
		MetaSaveGameInstance->SaveSlotName = metaName;
		MetaSaveGameInstance->UserIndex = 0;
	}

	return MetaSaveGameInstance;
}
