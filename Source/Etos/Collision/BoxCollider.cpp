// © 2016 - 2017 Daniel Bortfeld

#include "Etos.h"
#include "BoxCollider.h"
#include "SimpleCollisionManager.h"

// Sets default values for this component's properties
UBoxCollider::UBoxCollider()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	bWantsBeginPlay = true;
	PrimaryComponentTick.bCanEverTick = false;

	Collider = CreateDefaultSubobject<UBoxComponent>(TEXT("Box Collider"));
	Collider->SetupAttachment(this);
	Collider->ResetRelativeTransform();
	Collider->SetCanEverAffectNavigation(false);
	Collider->AreaClass = nullptr;

	Collider->bHiddenInGame = false;
	Collider->SetVisibility(true);
	bHiddenInGame = false;
	SetVisibility(true, true);
}


// Called when the game starts
void UBoxCollider::BeginPlay()
{
	Super::BeginPlay();

	if (ASimpleCollisionManager* const CollisionManger = Util::GetEtosCollisionManager(this))
	{
		CollisionManger->RegisterCollider(this);
	}
}


// Called every frame
void UBoxCollider::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UBoxCollider::BeginDestroy()
{
	bGenerateCollisionEvents = false;

	if (ASimpleCollisionManager* const CollisionManger = Util::GetEtosCollisionManager(this))
	{
		CollisionManger->UnregisterCollider(this);
	}

	Super::BeginDestroy();
}

void UBoxCollider::CheckCollision(UBoxCollider * other)
{
	if ((!bGenerateCollisionEvents && !other->bGenerateCollisionEvents)
		|| !FBoxSphereBounds::BoxesIntersect(Collider->Bounds, other->Collider->Bounds))
	{// no collision

		if (collisions.Contains(other))
		{// no more colliding

			if (OnTriggerExit.IsBound())
			{
				OnTriggerExit.Broadcast(other);
			}

			collisions.Remove(other);
		}
		return;
	}

	if (collisions.Contains(other))
	{// still colliding

		if (OnTriggerStay.IsBound())
		{
			OnTriggerStay.Broadcast(other);
		}
		return;
	}

	// new collision
	collisions.Add(other);
	if (OnTriggerEnter.IsBound())
	{
		OnTriggerEnter.Broadcast(other);
	}
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
			collider->CheckCollision(this);
		}
		collisions.Empty();
	}
}

