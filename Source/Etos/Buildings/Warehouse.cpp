// © 2016 - 2017 Daniel Bortfeld

#include "Etos.h"
#include "Warehouse.h"
#include "Etos/Pawns/MarketBarrow.h"
#include "Etos/Game/EtosPlayerController.h"
#include "Etos/Buildings/Path.h"
#include "Etos/FunctionLibraries/BuildingFunctionLibrary.h"

FORCEINLINE void AWarehouse::ReceiveResource(const FResource& resource)
{
	if (GetMyPlayerController())
	{
		MyPlayerController->AddResource(resource);
	}
}

FORCEINLINE void AWarehouse::DecreaseBarrowsInUse()
{
	barrowsInUse--;
}

inline void AWarehouse::BindDelayAction()
{
	Action.BindDynamic(this, &AWarehouse::SendMarketBarrows);
}

FORCEINLINE AEtosPlayerController * AWarehouse::GetMyPlayerController()
{
	if (!MyPlayerController)
	{
		MyPlayerController = (AEtosPlayerController*)GetWorld()->GetFirstPlayerController();
	}
	return MyPlayerController;
}

inline void AWarehouse::SendMarketBarrows()
{
	if (BP_MarketBarrow)
	{
		Data.BuildingsInRadius.Sort([&](ABuilding& A, ABuilding& B) {return A.Data.ProducedResource.Amount > B.Data.ProducedResource.Amount; });
		for (ABuilding* building : Data.BuildingsInRadius)
		{
			if (barrowsInUse < maxBarrows)
			{
				if (building && building->Data.ProducedResource.Amount > 0)
				{
					if (!building->Data.bBarrowIsOnTheWay)
					{
						if (BFuncs::FindPath(this, building))
						{
							if (UWorld* World = GetWorld())
							{
								FActorSpawnParameters params = FActorSpawnParameters();
								params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

								// TODO: find closest path tiles
								if (Data.PathConnections.IsValidIndex(0) && building->Data.PathConnections.IsValidIndex(0))
								{
									AMarketBarrow* newMarketBarrow = AMarketBarrow::Construct(this, BP_MarketBarrow, Data.PathConnections[0]->GetActorLocation() + FVector(0, 0, 100), building->Data.PathConnections[0]->GetActorLocation(), this, building, FRotator(0, 0, 0), params);
									if (newMarketBarrow != nullptr)
									{
										barrowsInUse++;
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
