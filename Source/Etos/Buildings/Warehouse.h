// © 2016 - 2017 Daniel Bortfeld

#pragma once

class AEtosPlayerController;
class AMarketBarrow;
#include "Buildings/Base/Building.h"
#include "Warehouse.generated.h"

/**
 *
 */
UCLASS()
class ETOS_API AWarehouse : public ABuilding
{
	GENERATED_BODY()

public:

	virtual FResource HandOutResource(const EResource& resource = EResource::None) override;

	virtual void ReceiveResource(const FResource& resource) override;

protected:

	virtual void BindDelayAction() override;

private:

	UFUNCTION()
		void SendMarketBarrows();
};
