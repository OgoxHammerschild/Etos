// © 2016 - 2017 Daniel Bortfeld

#pragma once

#include "Buildings/Base/Building.h"
#include "Path.generated.h"

/**
 *
 */
UCLASS()
class ETOS_API APath : public ABuilding
{
	GENERATED_BODY()

public:

	APath();

	//virtual void OnConstruction(const FTransform& Transform) override;

	virtual void OnBuild() override;

protected:

	virtual void CreateTracePoints() override;

	virtual void RelocateTracePoints() override;

	virtual void GetSurroundingBuildings() override;

public:
	UPROPERTY(VisibleAnywhere)
		TArray<ABuilding*> PossibleConnections;
	UPROPERTY(VisibleAnywhere)
		TArray<ABuilding*> Connections;
};
