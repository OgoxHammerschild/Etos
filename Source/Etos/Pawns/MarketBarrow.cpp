// © 2016 - 2017 Daniel Bortfeld

#include "Etos.h"
#include "MarketBarrow.h"
#include "AIController.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "Blueprint/AIAsyncTaskBlueprintProxy.h"

// Sets default values
AMarketBarrow::AMarketBarrow()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	UCapsuleComponent* CapsuleComponent = dynamic_cast<UCapsuleComponent*, USceneComponent>(RootComponent);
	CapsuleComponent->bDynamicObstacle = false;
	CapsuleComponent->AreaClass = nullptr;
}

AMarketBarrow * AMarketBarrow::Construct(UObject* WorldContextObject, TSubclassOf<AMarketBarrow> ClassToSpawn, const FVector & SpawnLocation, const FVector & TargetLocation, ABuilding * MyWorkplace, ABuilding * TargetBuilding, EResource OrderedResource, const FRotator & Rotation, const FActorSpawnParameters & SpawnParameters)
{
	check(WorldContextObject);
	check(ClassToSpawn);
	check(MyWorkplace);
	check(TargetBuilding);

	if (UWorld* const World = WorldContextObject->GetWorld())
	{
		AMarketBarrow * barrow = World->SpawnActor<AMarketBarrow>(ClassToSpawn, SpawnLocation, Rotation, SpawnParameters);

		if (barrow)
		{
			barrow->ResetBarrow(SpawnLocation, TargetLocation, MyWorkplace, TargetBuilding, OrderedResource, Rotation);

			barrow->StartWork();

			return barrow;
		}
	}

	return nullptr;
}

// called by garbage collection (default 60sec interval)
void AMarketBarrow::BeginDestroy()
{
	UE_LOG(LogTemp, Warning, TEXT("%s: Fuck this shit I'm out"), *GetName());

	CheckOut_Checked();

	Super::BeginDestroy();
}

void AMarketBarrow::ResetBarrow(const FVector & SpawnLocation, const FVector & TargetLocation, ABuilding * MyWorkplace, ABuilding * TargetBuilding, EResource OrderedResource, const FRotator & Rotation)
{
	if (!this->GetController())
	{
		this->SpawnDefaultController();
		if (this->GetAIController())
		{
			this->BindToOnMoveCompleted();
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("MarketBarrow->Construct: Could not spawn default controller"));
		}
	}

	this->SetActorLocationAndRotation(SpawnLocation, Rotation);
	this->bArrivedAtTarget = false;
	this->TargetBuilding = TargetBuilding;
	this->MyWorkplace = MyWorkplace;
	this->StartLocation = SpawnLocation;
	this->TargetLocation = TargetLocation;
	this->TargetBuilding->Data.bBarrowIsOnTheWay = true;
	this->bCheckedOut = false;
	this->Resource.Type = OrderedResource;
	this->Resource.Amount = 0;
	this->SetCanEverAffectNavigationOnComponents(false);
	this->SetActive(true);
	this->BPEvent_OnReset();
}

void AMarketBarrow::StartWork()
{
	//TODO: fade in

	// on fade in finished:
	this->MoveToTarget();
}

void AMarketBarrow::SetActive(bool isActive)
{
	bIsActive = isActive;
	SetActorHiddenInGame(!bIsActive);
}

FResource AMarketBarrow::GetTransportedResource()
{
	return Resource;
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
	case EPathFollowingResult::Aborted:
		if (bArrivedAtTarget)
		{
			if (AcceptanceRadius >= FVector::Dist(StartLocation, GetActorLocation()))
			{
				AddResourceToWorkplace();
				HaveLunchBreak();
			}
		}
		else if (AcceptanceRadius >= FVector::Dist(TargetLocation, GetActorLocation()))
		{
			bArrivedAtTarget = true;
			GetResource();
			MoveBackToWorkplace();
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
		if (!MyController->ReceiveMoveCompleted.IsBound())
		{
			MyController->ReceiveMoveCompleted.AddDynamic(this, &AMarketBarrow::OnMoveCompleted);
		}
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
		Resource = TargetBuilding->HandOutResource(Resource.Type);
		BPEvent_OnGetResource(Resource);
	}
}

FORCEINLINE void AMarketBarrow::MoveBackToWorkplace()
{
	if (GetAIController())
	{
		MyController->MoveToLocation(StartLocation, 5.0f, false);
	}
}

FORCEINLINE void AMarketBarrow::AddResourceToWorkplace()
{
	//TODO: take-wares-from-barrow animation

	if (MyWorkplace)
	{
		MyWorkplace->ReceiveResource(Resource);
	}
}

void AMarketBarrow::HaveLunchBreak()
{
	//TODO: fade out

	// on fade out finished:
	{
		CheckOut_Checked();

		if (MyWorkplace->TryReturningToPool(this))
		{
			SetActive(false);
		}
		else
		{
			Destroy();
		}
	}
}

FORCEINLINE void AMarketBarrow::SetCanEverAffectNavigationOnComponents(bool bRelevance)
{
	TInlineComponentArray<UActorComponent*> Components;
	GetComponents(Components);
	for (auto comp : Components)
	{
		comp->SetCanEverAffectNavigation(bRelevance);
	}
}

void AMarketBarrow::CheckOut_Checked()
{
	if (!bCheckedOut)
	{
		if (TargetBuilding/*, TEXT("%s's TargetBuilding was null."), *GetName()*/)
		{
			TargetBuilding->Data.bBarrowIsOnTheWay = false;
		}

		if (MyWorkplace/*, TEXT("%s's Warehouse was null."), *GetName()*/)
		{
			MyWorkplace->DecreaseBarrowsInUse();
		}
		bCheckedOut = true;
	}
}
