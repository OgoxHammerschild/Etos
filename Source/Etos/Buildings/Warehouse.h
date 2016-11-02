// Fill out your copyright notice in the Description page of Project Settings.

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

	UPROPERTY(EditDefaultsOnly)
		TSubclassOf<AMarketBarrow> BP_MarketBarrow;

private:

	UPROPERTY(VisibleAnywhere)
		int32 barrowsInUse = 0;
	UPROPERTY(EditAnywhere)
		int32 maxBarrows = 2;
	UPROPERTY()
		AEtosPlayerController* MyPlayerController;

public:

	void ReceiveResource(const FResource& resource);

	void DecreaseBarrowsInUse();

protected:

	virtual void BindDelayAction() override;

private:

	AEtosPlayerController* GetMyPlayerController();

	UFUNCTION()
	void SendMarketBarrows();
};
