// © 2016 - 2017 Daniel Bortfeld

#include "Etos.h"
#include "MarketBarrow.h"
#include "AIController.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "Blueprint/AIAsyncTaskBlueprintProxy.h"
//#include "Etos/FunctionLibraries/UtilityFunctionLibrary.h"

// Sets default values
AMarketBarrow::AMarketBarrow()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	UCapsuleComponent* CapsuleComponent = dynamic_cast<UCapsuleComponent*, USceneComponent>(RootComponent);
	CapsuleComponent->bDynamicObstacle = false;
	CapsuleComponent->AreaClass = nullptr;
}

AMarketBarrow * AMarketBarrow::Construct(UObject* WorldContextObject, TSubclassOf<AMarketBarrow> ClassToSpawn, const FVector & SpawnLocation, const FVector & TargetLocation, AWarehouse * MyWarehouse, ABuilding * TargetBuilding, const FRotator & Rotation, const FActorSpawnParameters & SpawnParameters)
{
	check(WorldContextObject);
	check(ClassToSpawn);
	check(MyWarehouse);
	check(TargetBuilding);

	if (UWorld* World = WorldContextObject->GetWorld())
	{
		AMarketBarrow * barrow = World->SpawnActor<AMarketBarrow>(ClassToSpawn, SpawnLocation, Rotation, SpawnParameters);
		if (barrow)
		{
			if (!barrow->GetController())
			{
				barrow->SpawnDefaultController();
				if (!barrow->GetAIController())
				{
					UE_LOG(LogTemp, Error, TEXT("MarketBarrow->Construct: Could not spawn default controller"));
				}
			}

			barrow->TargetBuilding = TargetBuilding;
			barrow->MyWarehouse = MyWarehouse;
			barrow->StartLocation = SpawnLocation;
			barrow->TargetLocation = TargetLocation;
			barrow->TargetBuilding->Data.bBarrowIsOnTheWay = true;
			barrow->SetCanEverAffectNavigationOnComponents(false);

			//TODO: fade in

			barrow->BindToOnMoveCompleted();

			// on fade in finished:
			barrow->MoveToTarget();

			return barrow;
		}
	}

	return nullptr;
}

// Called when the game starts or when spawned
void AMarketBarrow::BeginPlay()
{
	Super::BeginPlay();

	//UE_LOG(LogTemp, Warning, TEXT("%s: I live!"), *GetName());
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

// called by garbage collection (default 60sec interval)
void AMarketBarrow::BeginDestroy()
{
	//UE_LOG(LogTemp, Warning, TEXT("%s: Goodbye cruel world. D:"), *GetName());
	Super::BeginDestroy();

	//if (!this)
	//{
	//	UE_LOG(LogTemp, Warning, TEXT("%s's was null."), *GetName());
	//}
}

#if WITH_EDITOR
namespace EPathFollowingResult
{
	// https://wiki.unrealengine.com/Enums_For_Both_C%2B%2B_and_BP#Get_Name_of_Enum_as_String
	FString ToString(EPathFollowingResult::Type EnumValue)
	{
		const UEnum* EnumPtr = FindObject<UEnum>(ANY_PACKAGE, TEXT("EPathFollowingResult"), true);
		if (!EnumPtr) return FString("InvalidEnum");

		return EnumPtr->GetEnumName((int32)EnumValue);
	}
}
#endif

void AMarketBarrow::OnMoveCompleted(FAIRequestID RequestID, EPathFollowingResult::Type MovementResult)
{
	//UE_LOG(LogTemp, Warning, TEXT("MovementResult: %s"), *EPathFollowingResult::ToString(MovementResult));

	switch (MovementResult)
	{
	case EPathFollowingResult::Success:
		if (bArrivedAtTarget)
		{
			if (AcceptanceRadius < FVector::Dist(TargetLocation, GetActorLocation()))
			{
				AddResourceToPlayer();
				HaveLunchBreak();
			}
		}
		else if (AcceptanceRadius < FVector::Dist(StartLocation, GetActorLocation()))
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

FORCEINLINE void AMarketBarrow::BindToOnMoveCompleted()
{
	if (GetAIController())
	{
		MyController->ReceiveMoveCompleted.AddDynamic(this, &AMarketBarrow::OnMoveCompleted);
	}
	else UE_LOG(LogTemp, Warning, TEXT("%s: I have no AI :("), *GetName());
}

FORCEINLINE void AMarketBarrow::MoveToTarget()
{
	if (GetAIController())
	{
		MyController->MoveToLocation(TargetLocation, 5.0f, false);
	}
	else UE_LOG(LogTemp, Warning, TEXT("%s: I have no AI :("), *GetName());
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

	if (TargetBuilding)
	{
		TargetBuilding->Data.bBarrowIsOnTheWay = false;
	}
	else UE_LOG(LogTemp, Warning, TEXT("%s's TargetBuilding was null."), *GetName());

	if (MyWarehouse)
	{
		MyWarehouse->DecreaseBarrowsInUse();
	}
	else UE_LOG(LogTemp, Warning, TEXT("%s's Warehouse was null."), *GetName());

	// on fade out finished:
	Destroy();
}

FORCEINLINE void AMarketBarrow::SetCanEverAffectNavigationOnComponents(bool bRelevance)
{
	TArray<UActorComponent*> comps;
	GetComponents(comps);
	for (auto comp : comps)
	{
		comp->SetCanEverAffectNavigation(bRelevance);
	}
}
