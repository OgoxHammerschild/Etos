// © 2016 - 2017 Daniel Bortfeld

#include "Etos.h"
#include "ObjectPool.h"

// Sets default values
UObjectPool::UObjectPool()
{
	pooledObjects = TArray<UObject*>();
	pooledObjects.Reserve(minPooledObjects);
	//pooledObjectsRef = pooledObjects;
}

bool UObjectPool::AddObjectToPool(UObject * Object)
{
	bool success = false;
	if (pooledObjects.Num() + 1 <= maxPooledObjects)
	{
		pooledObjects.Push(Object);
		success = true;
	}

	return success;
}

UObject * UObjectPool::GetPooledObject(bool & bIsValid)
{
	return GetPooledObject<UObject*>(bIsValid);
}

int32 UObjectPool::GetMinPooledObjectsAmount()
{
	return minPooledObjects;
}

void UObjectPool::SetMinPooledObjectsAmount(int32 MinPooledObjects)
{
	this->minPooledObjects = FMath::Clamp(MinPooledObjects, 0, maxPooledObjects);
}

int32 UObjectPool::GetMaxPooledObjectsAmount()
{
	return maxPooledObjects;
}

void UObjectPool::SetMaxPooledObjectsAmount(int32 MaxPooledObjects)
{
	this->maxPooledObjects = FMath::Max(minPooledObjects, MaxPooledObjects);
}
