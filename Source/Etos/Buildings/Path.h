// Fill out your copyright notice in the Description page of Project Settings.

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

	virtual void OnConstruction(const FTransform& Transform) override;

	virtual void OnBuild() override;

protected:

	UFUNCTION()
		virtual void InitTracePoints() override;

public:
	UPROPERTY()
		TArray<ABuilding*> PossibleConnections;
	UPROPERTY()
		TArray<ABuilding*> Connections;
};
