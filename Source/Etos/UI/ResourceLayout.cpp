// © 2016 - 2017 Daniel Bortfeld

#include "Etos.h"
#include "ResourceLayout.h"
#include "Etos/Game/EtosPlayerController.h"

float UResourceLayout::GetStoredPercentage()
{
	if (MyPlayerController && MyPlayerController->IsValidLowLevel())
	{
		return (float)MyPlayerController->GetResourceAmount(Resource) / MyPlayerController->GetTotalStorage();
	}
	return 0;
}

int32 UResourceLayout::GetResourceAmount()
{
	if (MyPlayerController && MyPlayerController->IsValidLowLevel())
	{
		return MyPlayerController->GetResourceAmount(Resource);
	}
	return int32();
}
