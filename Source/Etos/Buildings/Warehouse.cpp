// Fill out your copyright notice in the Description page of Project Settings.

#include "Etos.h"
#include "Warehouse.h"
#include "MarketBarrow.h"
#include "Etos/Game/EtosPlayerController.h"
#include "Etos/Buildings/Path.h"

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
	for (ABuilding* building : Data.BuildingsInRadius)
	{
		if (barrowsInUse < maxBarrows)
		{
			if (building && building->Data.ProducedResource.Amount > 0)
			{
				if (!building->Data.bBarrowIsOnTheWay)
				{
					// TODO: search path

					if (BP_MarketBarrow)
					{
						if (Data.PathConnections.IsValidIndex(0))
						{
							if (UWorld* World = GetWorld())
							{
								FActorSpawnParameters params = FActorSpawnParameters();
								params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

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
						else UE_LOG(LogTemp, Warning, TEXT("no path connection in warehouse"))
					}
				}
			}
		}
	}
}
