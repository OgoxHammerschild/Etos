// © 2016 - 2017 Daniel Bortfeld

#include "Etos.h"
#include "EtosPlayerController.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Etos/FunctionLibraries/UtilityFunctionLibrary.h"
#include "Etos/UI/InGameUI.h"
#include "Etos/Game/EtosHUD.h"
#include "Etos/Buildings/Path.h"

void AEtosPlayerController::BeginPlay()
{
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;

	AddHUDToViewport();
	InitResourceMapping();

	// for testing ############
	FResource money = FResource();
	money.Type = EResource::Money;
	money.Amount = 10000;

	FResource wood = FResource();
	wood.Type = EResource::Wood;
	wood.Amount = 50;

	FResource tools = FResource();
	tools.Type = EResource::Tool;
	tools.Amount = 50;

	AddResource(money);
	AddResource(wood);
	AddResource(tools);
	//#########################
}

void AEtosPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	InputComponent->BindAction("Build", IE_Pressed, this, &AEtosPlayerController::BuildNewBuilding);
	InputComponent->BindAction("Pause", IE_Pressed, this, &AEtosPlayerController::PauseGame);
	InputComponent->BindAction("Escape", IE_Pressed, this, &AEtosPlayerController::ShowGameMenu);
	InputComponent->BindAction("ClickRepeatedly", IE_Pressed, this, &AEtosPlayerController::ClickRepeatedly);
	InputComponent->BindAction("CancelBuilding", IE_Pressed, this, &AEtosPlayerController::CancelPlacementOfBuilding);
}

FORCEINLINE void AEtosPlayerController::AddResource(const FResource& resource)
{
	if (resource.Type != EResource::None)
	{
		resourceAmounts[resource.Type] += resource.Amount;
		GetInGameUI()->UpdateResourceAmounts();
	}
}

FORCEINLINE void AEtosPlayerController::RemoveResource(const FResource& resource)
{
	if (resource.Type != EResource::None)
	{
		resourceAmounts[resource.Type] -= resource.Amount;
		GetInGameUI()->UpdateResourceAmounts();
	}
}

FORCEINLINE int32 AEtosPlayerController::GetResourceAmount(const EResource& resource)
{
	return resourceAmounts.FindOrAdd(resource);
}

FORCEINLINE UInGameUI * AEtosPlayerController::GetInGameUI()
{
	return UUtilityFunctionLibrary::GetEtosHUD(this)->GetInGameUI();
}

FORCEINLINE void AEtosPlayerController::PauseGame(FKey key)
{
}

FORCEINLINE void AEtosPlayerController::BuildNewBuilding(FKey key)
{
	if (newBuilding && newBuilding->Data.bIsHeld)
	{
		if (!newBuilding->Data.bPositionIsBlocked)
		{
			if (HasEnoughResources(newBuilding->Data.BuildCost))
			{
				if (APath* newPath = dynamic_cast<APath*, ABuilding>(newBuilding))
				{
					UE_LOG(LogTemp, Warning, TEXT("It's a path!"));
				}

				newBuilding->Data.bIsHeld = false;
				bIsHoldingObject = false;
				PayCostsOfBuilding(newBuilding->Data.BuildCost);

				//TODO: make building opaque

				newBuilding->OnBuild();
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
		SpawnBuilding(newBuilding, data);
	}
}

FORCEINLINE bool AEtosPlayerController::HasEnoughResources(const TArray<FResource>& buildCost)
{
	for (FResource cost : buildCost)
	{
		if (cost.Amount > resourceAmounts[cost.Type])
		{
			return false;
		}
	}
	return true;
}

FORCEINLINE void AEtosPlayerController::AddHUDToViewport()
{
	if (UInGameUI * GUI = GetInGameUI())
	{
		GUI->AddToViewport();
	}
	else if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(1, 5, FColor(1, 1, 1, 1), TEXT("Gui was not found"));
	}
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
		if (newBuilding)
		{
			newBuilding->Destroy();
			bIsHoldingObject = false;
			UE_LOG(LogTemp, Warning, TEXT("building canceled"));
		}
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
		return nullptr;

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
		bIsHoldingObject = true;

		//TODO: make building transparent
		return newBuilding;
	}
	return nullptr;
}

FORCEINLINE ABuilding* AEtosPlayerController::SpawnBuilding(ABuilding* Class, const FBuildingData& Data)
{
	return SpawnBuilding_Internal(Class->GetClass(), Data);
}

FORCEINLINE ABuilding * AEtosPlayerController::SpawnBuilding(TSubclassOf<ABuilding> Subclass, const FBuildingData& Data)
{
	return SpawnBuilding_Internal(Subclass, Data);
}
