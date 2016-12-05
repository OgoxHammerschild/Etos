// © 2016 - 2017 Daniel Bortfeld

#include "Etos.h"
#include "Residence.h"
#include "TownCenter.h"
#include "Etos/Game/EtosPlayerController.h"

void AResidence::BeginPlay()
{
	Super::BeginPlay();

	needsPerMinute.Add(EResource::Food, 2);
}

void AResidence::BindDelayAction()
{
	if (Action.IsBound())
	{
		Action.Unbind();
	}

	Action.BindDynamic(this, &AResidence::ProduceMoney);
}

void AResidence::ProduceMoney()
{
	if (myTownCenter)
	{
		// check for resources
		if (myTownCenter->GetResources(needsPerMinute, Data.ProductionTime, Residents))
		{
			for (int32 i = 0; i < 5 && happynessPercentage < 1; i++)
			{
				happynessPercentage += 0.01;
			}
		}
		else
		{
			for (int32 i = 0; i < 5 && happynessPercentage > 0; i++)
			{
				happynessPercentage -= 0.01;
			}
		}
		// end ~ check for resources

		// move residents in or out
		int32 deltaCitizens = 0;
		if (Residents < MaxResidents && happynessPercentage > 0.80)
		{
			if (happynessPercentage > 0.95)
			{
				for (int32 i = 0; i < 3 && Residents + deltaCitizens < MaxResidents; i++)
					deltaCitizens++;
			}
			else if (happynessPercentage >= 0.90)
			{
				for (int32 i = 0; i < 2 && Residents + deltaCitizens < MaxResidents; i++)
					deltaCitizens++;
			}
			else
			{
				deltaCitizens++;
			}
		}
		else if (Residents > 0 && happynessPercentage < 0.55)
		{
			if (happynessPercentage < 0.15)
			{
				for (int32 i = 0; i < 5 && Residents + deltaCitizens >= 0; i++)
					deltaCitizens--;
			}
			else if (happynessPercentage < 0.30)
			{
				for (int32 i = 0; i < 3 && Residents + deltaCitizens >= 0; i++)
					deltaCitizens--;
			}
			else if (happynessPercentage < 0.45)
			{
				for (int32 i = 0; i < 2 && Residents + deltaCitizens >= 0; i++)
					deltaCitizens--;
			}
			else
			{
				deltaCitizens--;
			}
		}

		if (MyPlayerController)
		{
			MyPlayerController->UpdateCitizens(deltaCitizens);
		}
		Residents += deltaCitizens;
		// end ~ move residents in or out

		// pay taxes
		float taxes = TaxPerResident * Residents * happynessPercentage * (Data.ProductionTime / 60);
		myTownCenter->PayTaxes(taxes);
		// end ~ pay taxes
	}
	else
	{
		RefreshBuildingsInRadius();
		if (!Data.BuildingsInRadius.FindItemByClass<ATownCenter>(&myTownCenter))
		{
			for (int32 i = 0; i < 10 && happynessPercentage > 0; i++)
			{
				happynessPercentage -= 0.01;
			}
		}
	}
}
