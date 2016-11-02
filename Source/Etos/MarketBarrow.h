// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

class UAIAsyncTaskBlueprintProxy;
#include "GameFramework/Character.h"
#include "Etos/Buildings/Base/Building.h"
#include "Etos/Buildings/Warehouse.h"
#include "Navigation/PathFollowingComponent.h"
#include "AIController.h"
#include "MarketBarrow.generated.h"

UCLASS()
class ETOS_API AMarketBarrow : public ACharacter
{
	GENERATED_BODY()

public:

	UPROPERTY()
		FVector StartLocation;
	UPROPERTY()
		FVector TargetLocation;
	UPROPERTY()
		AWarehouse* MyWarehouse;
	UPROPERTY()
		ABuilding* TargetBuilding;

private:

	UPROPERTY()
		AAIController* MyController;
	UPROPERTY()
		FResource Resource;
	UPROPERTY()
		bool bArrivedAtTarget = false;


public:
	// Sets default values for this character's properties
	AMarketBarrow();

	static AMarketBarrow* Construct(UObject* WorldContextObject, TSubclassOf<AMarketBarrow> ClassToSpawn, const FVector& SpawnLocation, const FVector& TargetLocation, AWarehouse* MyWarehouse, ABuilding* TargetBuilding, const FRotator& Rotation = FRotator(), const FActorSpawnParameters& SpawnParameters = FActorSpawnParameters());

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called every frame
	virtual void Tick(float DeltaSeconds) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;

	virtual void BeginDestroy() override;

private:

	UFUNCTION()
		void OnMoveCompleted(FAIRequestID RequestID, EPathFollowingResult::Type MovementResult);

	AAIController* GetAIController();

	void BindToOnMoveCompleted();

	void MoveToTarget();

	void GetResource();

	void MoveToWarehouse();

	void AddResourceToPlayer();

	void HaveLunchBreak();
};
