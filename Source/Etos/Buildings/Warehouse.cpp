// Fill out your copyright notice in the Description page of Project Settings.

#include "Etos.h"
#include "Warehouse.h"
#include "MarketBarrow.h"
#include "Etos/Game/EtosPlayerController.h"
#include "Etos/Buildings/Path.h"

void AWarehouse::ReceiveResource(FResource resource)
{
	if (GetMyPlayerController())
	{
		//MyPlayerController->addresou
	}
}

FORCEINLINE void AWarehouse::DecreaseBarrowsInUse()
{
	barrowsInUse--;
}

void AWarehouse::BindDelayAction()
{
	Action.BindDynamic(this, &AWarehouse::SendMarketBarrows);
}

AEtosPlayerController * AWarehouse::GetMyPlayerController()
{
	if (!MyPlayerController)
	{
		MyPlayerController = (AEtosPlayerController*)GetWorld()->GetFirstPlayerController();
	}
	return MyPlayerController;
}

void AWarehouse::SendMarketBarrows()
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

					AMarketBarrow* newMarketBarrow = GetWorld()->SpawnActor<AMarketBarrow>(Data.PathConnections[0]->GetActorLocation() + FVector(0, 0, 25), FRotator());

					// TODO: init barrow

					if (!newMarketBarrow->GetController())
					{
						newMarketBarrow->SpawnDefaultController();
					}

					barrowsInUse++;
				}
			}
		}
	}
}
