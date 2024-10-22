// © 2016 - 2017 Daniel Bortfeld

#include "Etos.h"
#include "EtosSaveGame.h"
#include "Etos/Buildings/Base/Building.h"
#include "Etos/Buildings/Residence.h"

UEtosSaveGame::UEtosSaveGame()
{
	SaveSlotName = TEXT("NewSaveGame");
	UserIndex = 0;
}

void UEtosSaveGame::AddBuilding(ABuilding * const building)
{
	FBuildingSaveData save = FBuildingSaveData();

	save.Name = building->Data.Name;
	save.bIsActive = building->IsActive();
	save.NeededResource1Amount = building->Data.NeededResource1.Amount;
	save.NeededResource2Amount = building->Data.NeededResource2.Amount;
	save.ProducedResourceAmount = building->Data.ProducedResource.Amount;
	save.Transform = building->GetActorTransform();

	BuiltBuildings.Add(save);
}

void UEtosSaveGame::AddResidence(AResidence * const residence)
{
	FResidenceSaveData save = FResidenceSaveData();

	save.Name = residence->Data.Name;
	save.bIsActive = residence->IsActive();
	save.NeededResource1Amount = residence->Data.NeededResource1.Amount;
	save.NeededResource2Amount = residence->Data.NeededResource2.Amount;
	save.ProducedResourceAmount = residence->Data.ProducedResource.Amount;
	save.Transform = residence->GetActorTransform();

	save.MaxResidents = residence->MaxResidents;
	save.Level = residence->MyLevel;
	save.Residents = residence->Residents;
	residence->GetAllSatisfactions(save.ResourceSatisfaction, save.NeedsSatisfaction, save.TotalSatisfaction);

	BuiltResidences.Add(save);
}
