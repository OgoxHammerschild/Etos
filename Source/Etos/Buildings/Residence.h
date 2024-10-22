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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Residents")
		EResidentLevel MyLevel = EResidentLevel::Peasant;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Residents")
		int32 Residents = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Residents")
		int32 MaxResidents = 8;

private:

	UPROPERTY(VisibleAnywhere)
		TMap<EResource, float> resourceSatisfaction;

	UPROPERTY(VisibleAnywhere)
		TMap<EResidentNeed, bool> needsSatisfaction;

	UPROPERTY(VisibleAnywhere)
		float totalSatisfaction = 0.5;

	UPROPERTY(VisibleAnywhere)
		ATownCenter* myTownCenter;

	UPROPERTY()
		FTimerHandle ResidentUpdateTimerHandle;

	UPROPERTY()
		float residentTimerTotal = 28;

	UPROPERTY()
		float residentTimerPassed = 0;

	UPROPERTY()
		FTimerHandle NeedsUpdateTimerHandle;

	UPROPERTY()
		float needsTimerTotal = 4;

	UPROPERTY()
		float needsTimerPassed = 0;

public:

	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void Tick(float DeltaTime) override;

	virtual void Demolish() override;

	void UpgradeToCitizen();

	void Upgrade_Internal(const FName& UpgradeName);

	float GetSatisfaction(const EResource& Resource);

	float GetSatisfaction(const EResidentNeed& Need);

	// for loading savegame only
	void SetAllSatisfactions(const TMap<EResource, float>& ResourceSatisfaction, const TMap<EResidentNeed, bool>& NeedsSatisfaction, const float& TotalSatisfaction);

	void GetAllSatisfactions(TMap<EResource, float>& ResourceSatisfaction, TMap<EResidentNeed, bool>& NeedsSatisfaction, float& TotalSatisfaction);

protected:

	virtual void Build() override;

	virtual void BindDelayAction() override;

private:

	void MoveResidents(float DeltaTime);

	void UpdateSatisfaction(float DeltaTime);

	UFUNCTION()
		void MoveResidents();

	UFUNCTION()
		void UpdateSatisfaction();

	template<class TBuilding>
	bool IsConnectedTo();
};
