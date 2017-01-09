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

void AEtosPlayerController::BeginPlay()
{
	PrimaryActorTick.bCanEverTick = true;
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;

	AddHUDToViewport();
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
	if (bIsBulidingPath)
	{
		UpdatePathPreview();
	}
	AddIncome(DeltaTime);
}

void AEtosPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	InputComponent->BindAction("Build", IE_Pressed, this, &AEtosPlayerController::BuildNewBuilding);
	InputComponent->BindAction("Pause", IE_Pressed, this, &AEtosPlayerController::PauseGame);
	InputComponent->BindAction("Escape", IE_Pressed, this, &AEtosPlayerController::ShowGameMenu);
	InputComponent->BindAction("ClickRepeatedly", IE_Pressed, this, &AEtosPlayerController::ClickRepeatedly);
	InputComponent->BindAction("CancelBuilding", IE_Pressed, this, &AEtosPlayerController::CancelPlacementOfBuilding);
	InputComponent->BindAction("Select", IE_Pressed, this, &AEtosPlayerController::SelectBuilding);
	InputComponent->BindAction("Demolish", IE_Pressed, this, &AEtosPlayerController::DemolishBuilding);

#if WITH_EDITOR
	InputComponent->BindAction("QuickSave", IE_Pressed, this, &AEtosPlayerController::Save);
	InputComponent->BindAction("QuickLoad", IE_Pressed, this, &AEtosPlayerController::Load);
#endif

}

FORCEINLINE void AEtosPlayerController::AddResource(const FResource& resource)
{
	AddResource(resource.Type, resource.Amount);
}

void AEtosPlayerController::AddResource(const EResource & resource, const int32 & amount)
{
	if (Enum::IsValid(resource))
	{
		resourceAmounts[resource] += amount;
		GetInGameUI()->UpdateResourceAmounts();
	}
}

FORCEINLINE void AEtosPlayerController::RemoveResource(const FResource& resource)
{
	RemoveResource(resource.Type, resource.Amount);
}

void AEtosPlayerController::RemoveResource(const EResource & resource, const int32 & amount)
{
	if (Enum::IsValid(resource))
	{
		resourceAmounts[resource] -= amount;
		GetInGameUI()->UpdateResourceAmounts();
	}
}

bool AEtosPlayerController::TryRemovingResource(const FResource & resource)
{
	return TryRemovingResource(resource.Type, resource.Amount);
}

bool AEtosPlayerController::TryRemovingResource(const EResource & resource, const int32 & amount)
{
	if (GetResourceAmount(resource) > 0)
	{
		RemoveResource(resource, amount);
		return true;
	}

	return false;
}

FORCEINLINE int32 AEtosPlayerController::GetResourceAmount(const EResource& resource)
{
	return resourceAmounts.FindOrAdd(resource);
}

void AEtosPlayerController::UpdatePopulation(const EResidentLevel& level, const int32& deltaPolulation)
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

void AEtosPlayerController::UpdatePopulation(const EResidentLevel& from, const EResidentLevel& to, const int32& residents)
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

void AEtosPlayerController::ReportUpgrade(AResidence * upgradedResidence, const EResidentLevel & levelBeforeUpgrade, const EResidentLevel & levelAfterUpgrade)
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

int32 AEtosPlayerController::GetPopulationAmount(const EResidentLevel& level)
{
	return populationPerLevel.FindOrAdd(level);
}

void AEtosPlayerController::UpdateUpkeep(int32 deltaUpkeep)
{
	totalUpkeep += deltaUpkeep;
	UpdateBalanceUI(totalIncome, totalUpkeep);
}

int32 AEtosPlayerController::GetTotalUpkeep()
{
	return totalUpkeep;
}

void AEtosPlayerController::UpdateStorage(int32 deltaStorage)
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

int32 AEtosPlayerController::GetAvailablePromotions(const EResidentLevel & to)
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
}

FORCEINLINE ABuilding* AEtosPlayerController::SpawnBuilding(ABuilding* Class, const FBuildingData& Data)
{
	return SpawnBuilding_Internal(Class->GetClass(), Data);
}

FORCEINLINE ABuilding * AEtosPlayerController::SpawnBuilding(const TSubclassOf<ABuilding>& Subclass, const FBuildingData& Data)
{
	return SpawnBuilding_Internal(Subclass, Data);
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
			else UE_LOG(LogTemp, Warning, TEXT("Not enough resources"));
		}
		else UE_LOG(LogTemp, Warning, TEXT("Position is blocked"));
	}
}

FORCEINLINE void AEtosPlayerController::ShowGameMenu(FKey key)
{
}

FORCEINLINE void AEtosPlayerController::ClickRepeatedly(FKey key)
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
						GUI->BPEvent_ShowBuildingInfo(building->Data);
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
				building->Demolish();
			}
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

void AEtosPlayerController::AddIncome(float DeltaTime)
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

FORCEINLINE bool AEtosPlayerController::HasEnoughResources(const TArray<FResource>& buildCost) const
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

void AEtosPlayerController::ReportDestroyedBuilding(ABuilding * destroyedBuilding)
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

void AEtosPlayerController::Save()
{
	//UGameplayStatics::DoesSaveGameExist...
	UEtosSaveGame* SaveGameInstance = Cast<UEtosSaveGame>(UGameplayStatics::CreateSaveGameObject(UEtosSaveGame::StaticClass()));
	SaveGameInstance->PlayerName = TEXT("Player 1");
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

	UGameplayStatics::SaveGameToSlot(SaveGameInstance, SaveGameInstance->SaveSlotName, SaveGameInstance->UserIndex);
}

void AEtosPlayerController::Load()
{
	if (!UGameplayStatics::DoesSaveGameExist(TEXT("NewSaveGame"), 0))
	{
		UE_LOG(LogTemp, Warning, TEXT("NewSaveGame was not found"));
		return;
	}

	UEtosSaveGame* LoadGameInstance = Cast<UEtosSaveGame>(UGameplayStatics::CreateSaveGameObject(UEtosSaveGame::StaticClass()));
	LoadGameInstance = Cast<UEtosSaveGame>(UGameplayStatics::LoadGameFromSlot(LoadGameInstance->SaveSlotName, LoadGameInstance->UserIndex));

	if (LoadGameInstance->IsValidLowLevel())
	{
		if (LoadGameInstance->ResourceAmounts.Num() > 0)
		{
			this->resourceAmounts = LoadGameInstance->ResourceAmounts;
		}
		if (LoadGameInstance->PopulationPerLevel.Num() > 0)
		{
			//this->populationPerLevel = LoadGameInstance->PopulationPerLevel;
		}
		if (LoadGameInstance->UsedPromotions.Num() > 0)
		{
			this->usedPromotions = LoadGameInstance->UsedPromotions;
		}

		if (const auto GM = Util::GetEtosGameMode(this))
		{
			int32 count = GM->GetBuildingAmount();

			TMap<FName, FPredefinedBuildingData> BuildingData;

			for (int32 i = 0; i < count; ++i)
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

			if (auto const World = GetWorld())
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
	}
}

void AEtosPlayerController::StartDemolishMode()
{
	bIsInDemolishMode = true;
}

FORCEINLINE void AEtosPlayerController::AddHUDToViewport()
{
	try
	{
		UInGameUI* const GUI = GetInGameUI();
		if (GUI != nullptr)
		{
			GUI->AddToViewport();
		}
		else if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(1, 5, FColor(1, 1, 1, 1), TEXT("Gui was not found"));
		}
	}
	catch (const std::exception& e)
	{
		Panic(e);
	}
}

void AEtosPlayerController::Panic(const std::exception & wait)
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

FORCEINLINE void AEtosPlayerController::CancelPlacementOfBuilding(FKey key)
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
		bIsInDemolishMode = false;
	}
}

FORCEINLINE void AEtosPlayerController::PayCostsOfBuilding(const TArray<FResource>& buildCost)
{
	for (FResource resource : buildCost)
	{
		RemoveResource(resource);
	}
}

FORCEINLINE ABuilding * AEtosPlayerController::SpawnBuilding_Internal(UClass * Class, const FBuildingData & Data)
{
	if (bIsHoldingObject)
		CancelPlacementOfBuilding(FKey());

	bIsInDemolishMode = false;

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

FORCEINLINE void AEtosPlayerController::BuildNewBuilding_Internal(bool skipCosts)
{
	newBuilding->Data.bIsHeld = false;
	bIsHoldingObject = false;

	if (!skipCosts)
		PayCostsOfBuilding(newBuilding->Data.BuildCost);

	if (builtBuildings.AddUnique(newBuilding) != INDEX_NONE)
	{
		if (AResidence* residence = dynamic_cast<AResidence*, ABuilding>(newBuilding))
		{
			builtResidences.FindOrAdd(residence->MyLevel).Residences.AddUnique(residence);
		}

		newBuilding->OnDestroyed.AddDynamic(this, &AEtosPlayerController::OnBuildingDestroyed);

		newBuilding->OnBuild();
	}
}

void AEtosPlayerController::BuildLoadedBuilding(const FBuildingSaveData & Data)
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

FORCEINLINE void AEtosPlayerController::StartBuildingPath(APath* newPath)
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

FORCEINLINE void AEtosPlayerController::SpawnPathPreview(const FVector& spawnLocation, const int32& index, UWorld* const World)
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

void AEtosPlayerController::DestroyPathPreview(APath * tempPath)
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

void AEtosPlayerController::UpdatePopulationUI(const int32 & peasants, const int32 & citizens)
{
	if (auto* const GUI = GetInGameUI())
	{
		GUI->UpdatePopulation(peasants, citizens);
	}
}

void AEtosPlayerController::UpdateBalanceUI(const int32 & income, const int32 & upkeep)
{
	if (auto* const GUI = GetInGameUI())
	{
		GUI->UpdateBalance(income, upkeep);
	}
}
