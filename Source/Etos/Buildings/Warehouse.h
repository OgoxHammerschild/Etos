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

private:

	UPROPERTY()
		int32 storageSpace = 40;

public:

	virtual FResource HandOutResource(const EResource& resource = EResource::None) override;

	virtual void ReceiveResource(const FResource& resource) override;

	virtual	bool HasResource(EResource in resource) override;

protected:

	virtual void BindDelayAction() override;

	virtual void Build() override;

private:

	UFUNCTION()
		void SendMarketBarrows();
};
