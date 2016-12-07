// © 2016 - 2017 Daniel Bortfeld

#include "Etos.h"
#include "EtosGameMode.h"
#include "Runtime/Engine/Public/GameDelegates.h"

AEtosGameMode::AEtosGameMode()
{
	if (!PredefinedBuildingData)
	{
		static ConstructorHelpers::FObjectFinder<UDataTable> BP_PredefinedBuildingData(TEXT("DataTable'/Game/Blueprints/GameMode/PredefinedBuildingData.PredefinedBuildingData'"));
		PredefinedBuildingData = BP_PredefinedBuildingData.Object;
	}

	if (UWorld* const World = GetWorld())
	{
		CollisionManager = nullptr;

		for (TActorIterator<ASimpleCollisionManager> ActorItr(World); ActorItr; ++ActorItr)
		{
			if (*ActorItr != nullptr)
			{
				CollisionManager = *ActorItr;
				break;
			}
		}

		if (!CollisionManager)
		{
			UE_LOG(LogTemp, Error, TEXT("There is no Collision Manager in the level."));
		}
	}

	taxPerResidentPerMinute.Add(EResidentLevel::Peasant, 1.17f);

	FGameDelegates::Get().GetEndPlayMapDelegate().AddLambda([&] 
	{
		//PredefinedBuildingData = nullptr;
		CollisionManager = nullptr;
	});
}

FPredefinedBuildingData* AEtosGameMode::GetPredefinedBuildingData(const int32& buildingID)
{
	check(PredefinedBuildingData);

	// DataTable.h:97 variable swap error
	// UObjectGlobals.h typo in NewObject assert

	//if(buildingID is valid)
	//{
	FPredefinedBuildingData* preDefData = PredefinedBuildingData->FindRow<FPredefinedBuildingData>(*FString::FromInt(buildingID), FString(TEXT("GameMode")));

	return preDefData;
	//}
}

int32 AEtosGameMode::GetBuildingAmount()
{
	return PredefinedBuildingData ? PredefinedBuildingData->GetRowNames().Num() : 0;
}

FResidentNeeds AEtosGameMode::GetPeasantNeeds()
{
	FResidentNeeds needs = FResidentNeeds(EResidentLevel::Peasant);
	
	needs.ResidentNeeds.Add(EResidentNeed::TownCenter);
	//needs.ResidentNeeds.Add(EResidentNeed::Chapel);

	needs.ResourceConsumptions.Add(FResourceConsumption(EResource::Fish, 200, 2));
	//needs.ResourceConsumptions.Add(FResourceConsumption(EResource::Most, 340, 1.5));

	return needs;
}

float AEtosGameMode::GetTaxForResident(const EResidentLevel& level)
{
	float tax = 0;

	if (Enum::IsValid(level))
	{
		tax = taxPerResidentPerMinute.FindOrAdd(level);
	}

	return tax;
}
