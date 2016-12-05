// © 2016 - 2017 Daniel Bortfeld

#include "Etos.h"
#include "TownCenter.h"
#include "Etos/Game/EtosPlayerController.h"

bool ATownCenter::GetResources(TMap<EResource, int32> resourcesPerMinute, float intervalTime, int32 residents)
{
	bool hasResources = true;

	if (MyPlayerController)
	{
		for (auto resource : resourcesPerMinute)
		{
			float deltaResources = (float)resource.Value * (intervalTime / 60.f) * ((float)residents / 200.f);
			if (MyPlayerController->GetResourceAmount(resource.Key) < deltaResources)
			{
				hasResources = false;
			}

			usedResources.FindOrAdd(resource.Key) += deltaResources;
		}
	}
	else
	{
		MyPlayerController = Util::GetFirstEtosPlayerController(this);
	}

	return hasResources;
}

void ATownCenter::PayTaxes(float taxes)
{
	payedTaxes += taxes;
}

void ATownCenter::BindDelayAction()
{
	if (Action.IsBound())
	{
		Action.Unbind();
	}

	Action.BindDynamic(this, &ATownCenter::TradeResourcesForMoney);
}

void ATownCenter::TradeResourcesForMoney()
{
	if (MyPlayerController)
	{
		int32 flooredAmount;
		for (auto resource : usedResources)
		{
			flooredAmount = resource.Value;
			usedResources[resource.Key] -= flooredAmount;

			if (MyPlayerController->GetResourceAmount(resource.Key) > 0)
			{
				MyPlayerController->RemoveResource(FResource(resource.Key, flooredAmount));
			}
		}

		flooredAmount = payedTaxes;
		payedTaxes -= flooredAmount;

		MyPlayerController->AddResource(FResource(EResource::Money, flooredAmount));
	}
	else
	{
		MyPlayerController = Util::GetFirstEtosPlayerController(this);
	}
}
