// © 2016 - 2017 Daniel Bortfeld

#pragma once

#include "Buildings/Base/Building.h"
#include "Etos/Utility/Structs/ResidentNeeds.h"
#include "TownCenter.generated.h"

/**
 *  Is now called Market Place, need to refactor classname
 */
UCLASS()
class ETOS_API ATownCenter : public ABuilding
{
	GENERATED_BODY()

private:

	UPROPERTY(VisibleAnywhere)
		TMap<EResource, float> consumedResources;

	UPROPERTY(VisibleAnywhere)
		float payedTaxes;

	UPROPERTY(VisibleAnywhere)
		int32 registeredResidents;

	UPROPERTY(VisibleAnywhere)
		float taxPerResident = 1.17f;

	UPROPERTY(VisibleAnywhere)
		FResidentNeeds peasantNeeds;

	UPROPERTY()
		float taxesTimerTotal = 3;

	UPROPERTY()
		float taxesTimerPassed = 0;

public:

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;

	bool HasResource(EResource in resource);

	void PayTaxes(float DeltaTime);

	void ConsumeResources(float DeltaTime);

	void UpdatePopulation(EResidentLevel level, int32 delta);

	void UpdatePopulation(EResidentLevel from, EResidentLevel to, int32 residents);

protected:

	virtual void BindDelayAction() override;
};
