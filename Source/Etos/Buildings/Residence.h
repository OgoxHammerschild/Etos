// © 2016 - 2017 Daniel Bortfeld

#pragma once

class ATownCenter;
#include "Buildings/Base/Building.h"
#include "Residence.generated.h"

/**
 *
 */
UCLASS()
class ETOS_API AResidence : public ABuilding
{
	GENERATED_BODY()

public:

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Residents")
		int32 Residents = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Residents")
		int32 MaxResidents = 8;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Residents")
		int32 TaxPerResident = 1.5f;

private:

	UPROPERTY(VisibleAnywhere)
		TMap<EResource, int32> needsPerMinute;

	UPROPERTY(VisibleAnywhere)
		float happynessPercentage = 0.5;

	UPROPERTY(VisibleAnywhere)
		ATownCenter* myTownCenter;

public:

	virtual void BeginPlay() override;

protected:

	virtual void BindDelayAction() override;

private:

	UFUNCTION()
		void ProduceMoney();
};
