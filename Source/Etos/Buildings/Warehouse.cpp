// © 2016 - 2017 Daniel Bortfeld

#include "Etos.h"
#include "Warehouse.h"
#include "Etos/Pawns/MarketBarrow.h"
#include "Etos/Game/EtosPlayerController.h"
#include "Etos/Buildings/Path.h"
#include "Etos/Utility/FunctionLibraries/BuildingFunctionLibrary.h"

FResource AWarehouse::HandOutResource(const EResource & resource)
{
	if (resource != EResource::None)
	{
		if (GetMyPlayerController())
		{
			FResource orderedResource = FResource(resource);
			for (int32 i = 0; i < 5; i++)
			{
				if (MyPlayerController->GetResourceAmount(resource) > 0)
				{
					orderedResource.Amount++;
					MyPlayerController->RemoveResource(FResource(resource, 1));
				}
				else
				{
					return orderedResource;
				}
			}
			return orderedResource;
		}
	}

	return FResource();
}

void AWarehouse::ReceiveResource(const FResource& resource)
{
	if (MyPlayerController)
	{
		MyPlayerController->AddResource(resource);
	}
	else
	{
		GetMyPlayerController();
	}
}

bool AWarehouse::HasResource(EResource in resource)
{
	if (MyPlayerController)
	{
		return MyPlayerController->GetResourceAmount(resource) > 0;
	}
	else
	{
		GetMyPlayerController();
		return false;
	}
}

void AWarehouse::BindDelayAction()
{
	Action.BindDynamic(this, &AWarehouse::SendMarketBarrows);
}

void AWarehouse::OnBuild()
{
	if (GetMyPlayerController())
	{
		MyPlayerController->UpdateStorage(storageSpace);
	}

	Super::OnBuild();
}

void AWarehouse::SendMarketBarrows()
{
	if (BP_MarketBarrow)
	{
		RefreshBuildingsInRadius();

		Data.BuildingsInRadius.Sort([](ABuilding in A, ABuilding in B)
		{
			return A.Data.ProducedResource.Amount > B.Data.ProducedResource.Amount;
		});

		if (GetMyPlayerController())
		{
			int32 totalStorage = MyPlayerController->GetTotalStorage();

			for (ABuilding* building : Data.BuildingsInRadius)
			{
				if (BarrowsInUse < MaxBarrows)
				{
					if (building)
					{
						int32 producedAmount = building->Data.ProducedResource.Amount;
						if (producedAmount > 0 && MyPlayerController->GetResourceAmount(building->Data.ProducedResource.Type) + producedAmount <= totalStorage)
						{
							if (!building->Data.bBarrowIsOnTheWay)
							{
								APath* start; APath* goal;
								if (BFuncs::FindPath(this, building, start, goal))
								{
									if (start && goal)
									{
										SendMarketBarrow_Internal(building, // target building
											building->Data.ProducedResource.Type, // ordered resource
											start->GetActorLocation() + FVector(0, 0, 100), // spawn location
											goal->GetActorLocation()); // target location
									}
								}
							}
						}
					}
				}
			}
		}
	}
}
