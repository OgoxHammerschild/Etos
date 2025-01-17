// © 2016 - 2017 Daniel Bortfeld

#include "Etos.h"
#include "BoxCollider.h"
#include "SimpleCollisionManager.h"

// Sets default values for this component's properties
UBoxCollider::UBoxCollider()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame. You can turn these features
	// off to improve performance if you don't need them.
	//bWantsBeginPlay = true;
	PrimaryComponentTick.bCanEverTick = false;

	Collider = CreateDefaultSubobject<UBoxComponent>(TEXT("Box Collider"));
	Collider->SetupAttachment(this);
	Collider->ResetRelativeTransform();
	Collider->SetCanEverAffectNavigation(false);
	Collider->AreaClass = nullptr;
}

// Called when the game starts
void UBoxCollider::BeginPlay()
{
	Super::BeginPlay();

	RegisterCollider();
}

void UBoxCollider::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	bGenerateCollisionEvents = false;

	UnregisterCollider();

	Super::EndPlay(EndPlayReason);
}

bool UBoxCollider::CheckCollision(UBoxCollider * other)
{
	if (!bGenerateCollisionEvents														 // if I shouldn't collide or..
		//|| (!bIsActive || !other->bIsActive)
		|| (!FBoxSphereBounds::BoxesIntersect(Collider->Bounds, other->Collider->Bounds) // .. if I don't intersect with other and..
			&& !IsInside(other) && !other->IsInside(this)))							 // .. if I'm not inside other and other isn't inside me
	{// no collision

		if (collisions.Contains(other))
		{// no more colliding

			if (OnTriggerExit.IsBound())
			{
				OnTriggerExit.Broadcast(other);
			}

			collisions.Remove(other);
			collisions.Shrink();
		}
		return false;
	}

	if (collisions.Contains(other))
	{// still colliding

		if (OnTriggerStay.IsBound())
		{
			OnTriggerStay.Broadcast(other);
		}
		return true;
	}

	// new collision
	collisions.AddUnique(other);
	if (OnTriggerEnter.IsBound())
	{
		OnTriggerEnter.Broadcast(other);
	}
	return true;
}

FORCEINLINE bool UBoxCollider::GetGenerateCollisionEvents()
{
	return bGenerateCollisionEvents;
}

FORCEINLINE void UBoxCollider::SetGenerateCollisionEvents(bool shouldGenerate)
{
	if (bGenerateCollisionEvents == shouldGenerate)
		return;

	bGenerateCollisionEvents = shouldGenerate;

	if (!bGenerateCollisionEvents)
	{
		for (UBoxCollider* collider : collisions)
		{
			if (collider && collider->IsValidLowLevel())
			{
				collider->CheckCollision(this);
			}
		}
		collisions.Empty();
	}
}

void UBoxCollider::RegisterCollider()
{
	if (ASimpleCollisionManager* const CollisionManager = Util::GetEtosCollisionManager(this))
	{
		CollisionManager->RegisterCollider(this);
	}
}

void UBoxCollider::UnregisterCollider()
{
	if (ASimpleCollisionManager* const CollisionManager = Util::GetEtosCollisionManager(this))
	{
		CollisionManager->UnregisterCollider(this);
	}
}

void UBoxCollider::SetMobilityType(EComponentMobility::Type newMobilityType)
{
	if (newMobilityType == mobilityType)
		return;

	mobilityType = newMobilityType;

	if (ASimpleCollisionManager* const CollisionManager = Util::GetEtosCollisionManager(this))
	{
		if (CollisionManager->IsRegistered(this))
		{
			UnregisterCollider();
			RegisterCollider();
		}
	}
}

EComponentMobility::Type UBoxCollider::GetMobilityType()
{
	return mobilityType;
}

bool UBoxCollider::IsInside(UBoxCollider* other)
{
	return Collider->Bounds.GetBox().IsInside(other->Collider->Bounds.GetBox());
}