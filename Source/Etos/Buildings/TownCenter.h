// © 2016 - 2017 Daniel Bortfeld

#pragma once

#include "Buildings/Base/Building.h"
#include "TownCenter.generated.h"

/**
 *
 */
UCLASS()
class ETOS_API ATownCenter : public ABuilding
{
	GENERATED_BODY()

private:

	UPROPERTY(VisibleAnywhere)
		TMap<EResource, float> usedResources;

	UPROPERTY(VisibleAnywhere)
		float payedTaxes;

public:

	bool GetResources(TMap<EResource, int32> resourcesPerMinute, float intervalTime, int32 residents);

	void PayTaxes(float taxes);

protected:

	virtual void BindDelayAction() override;

private:

	UFUNCTION()
		void TradeResourcesForMoney();

};
