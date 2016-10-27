// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

class AEtosPlayerController;
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
		int32 barrowsInUse = 0;
	UPROPERTY()
		int32 maxBarrows = 1;
	UPROPERTY()
		AEtosPlayerController* MyPlayerController;

public:

	void ReceiveResource(FResource resource);

	void DecreaseBarrowsInUse();

protected:

	virtual void BindDelayAction() override;

private:

	AEtosPlayerController* GetMyPlayerController();

	UFUNCTION()
	void SendMarketBarrows();
};
