// Fill out your copyright notice in the Description page of Project Settings.

#include "Etos.h"
#include "MarketBarrow.h"
#include "AIController.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "Blueprint/AIAsyncTaskBlueprintProxy.h"

// Sets default values
AMarketBarrow::AMarketBarrow()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AMarketBarrow::BeginPlay()
{
	Super::BeginPlay();
	TargetBuilding->Data.bBarrowIsOnTheWay = true;

	//TODO: fade in

	MoveToTarget();
}

// Called every frame
void AMarketBarrow::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AMarketBarrow::SetupPlayerInputComponent(class UInputComponent* InputComponent)
{
	Super::SetupPlayerInputComponent(InputComponent);

}

void AMarketBarrow::Destroyed()
{
	Super::Destroyed();

	TargetBuilding->Data.bBarrowIsOnTheWay = false;
	MyWarehouse->DecreaseBarrowsInUse();
}

void AMarketBarrow::OnMoveCompleted(FAIRequestID RequestID, EPathFollowingResult::Type MovementResult)
{
	switch (MovementResult)
	{
	case EPathFollowingResult::Success:
		if (bArrivedAtTarget)
		{
			AddResourceToPlayer();
			HaveLunchBreak();
		}
		else
		{
			bArrivedAtTarget = true;
			GetResource();
			MoveToWarehouse();
		}
		break;
	default:
		HaveLunchBreak();
		break;
	}
}

FORCEINLINE AAIController * AMarketBarrow::GetAIController()
{
	if (!MyController)
	{
		MyController = (AAIController*)GetController();
	}
	return MyController;
}

FORCEINLINE void AMarketBarrow::MoveToTarget()
{
	if (GetAIController())
	{
		MyController->MoveToLocation(TargetLocation, 5.0f, false);
		MyController->ReceiveMoveCompleted.AddDynamic(this, &AMarketBarrow::OnMoveCompleted);
	}
}

FORCEINLINE void AMarketBarrow::GetResource()
{
	//TODO: put-wares-on-barrow animation

	if (TargetBuilding)
	{
		Resource = TargetBuilding->Data.ProducedResource;
		TargetBuilding->ResetStoredResources();
	}
}

FORCEINLINE void AMarketBarrow::MoveToWarehouse()
{
	if (GetAIController())
	{
		MyController->MoveToLocation(StartLocation, 5.0f, false);
	}
}

FORCEINLINE void AMarketBarrow::AddResourceToPlayer()
{
	//TODO: take-wares-from-barrow animation

	if (MyWarehouse)
	{
		MyWarehouse->ReceiveResource(Resource);
	}
}

FORCEINLINE void AMarketBarrow::HaveLunchBreak()
{
	//TODO: fade out

	Destroy();
}
