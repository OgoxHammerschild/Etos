// © 2016 - 2017 Daniel Bortfeld

#include "Etos.h"
#include "Warehouse.h"
#include "Etos/Pawns/MarketBarrow.h"
#include "Etos/Game/EtosPlayerController.h"
#include "Etos/Buildings/Path.h"
#include "Etos/FunctionLibraries/BuildingFunctionLibrary.h"

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

FORCEINLINE void AWarehouse::ReceiveResource(const FResource& resource)
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

bool AWarehouse::HasResource(EResource resource)
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

inline void AWarehouse::BindDelayAction()
{
	Action.BindDynamic(this, &AWarehouse::SendMarketBarrows);
}

inline void AWarehouse::SendMarketBarrows()
{
	if (BP_MarketBarrow)
	{
		RefreshBuildingsInRadius();

		Data.BuildingsInRadius.Sort([](const ABuilding& A, const ABuilding& B)
		{
			return A.Data.ProducedResource.Amount > B.Data.ProducedResource.Amount; 
		});

		for (ABuilding* building : Data.BuildingsInRadius)
		{
			if (BarrowsInUse < MaxBarrows)
			{
				if (building && building->Data.ProducedResource.Amount > 0)
				{
					if (!building->Data.bBarrowIsOnTheWay)
					{
						if (BFuncs::FindPath(this, building))
						{
							// TODO: find closest path tiles
							if (Data.PathConnections.IsValidIndex(0) && building->Data.PathConnections.IsValidIndex(0))
							{
								bool isValid;
								AMarketBarrow* newMarketBarrow = MarketBarrowPool.GetPooledObject<AMarketBarrow*>(isValid);

								if (isValid && newMarketBarrow)
								{
									newMarketBarrow->ResetBarrow(
										Data.PathConnections[0]->GetActorLocation() + FVector(0, 0, 100),
										building->Data.PathConnections[0]->GetActorLocation(),
										this,
										building,
										building->Data.ProducedResource.Type,
										FRotator(0, 0, 0));

									newMarketBarrow->StartWork();
								}
								else
								{
									FActorSpawnParameters params = FActorSpawnParameters();
									params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

									newMarketBarrow = AMarketBarrow::Construct(
										this,
										BP_MarketBarrow,
										Data.PathConnections[0]->GetActorLocation() + FVector(0, 0, 100),
										building->Data.PathConnections[0]->GetActorLocation(),
										this,
										building,
										building->Data.ProducedResource.Type,
										FRotator(0, 0, 0),
										params);
								}

								if (newMarketBarrow && newMarketBarrow->IsValidLowLevel())
								{
									++BarrowsInUse;
								}
							}
						}
					}
				}
			}
		}
	}
}
