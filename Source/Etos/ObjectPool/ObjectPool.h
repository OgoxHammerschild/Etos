// © 2016 - 2017 Daniel Bortfeld

#pragma once

#include "GameFramework/Actor.h"
#include "ObjectPool.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FResetObjectDelegate, UObject*, ResetObject);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNullObjectDelegate, int32, amountOfNulledObjects);

UCLASS()
class ETOS_API UObjectPool : public UObject
{
	GENERATED_BODY()

public:

	// Use to reset the returned object
	UPROPERTY()
		FResetObjectDelegate OnGetValidObject;

	UPROPERTY()
		FNullObjectDelegate OnRemovedNullptrs;

private:

	UPROPERTY(VisibleAnywhere, Category = "Object Pool")
		int32 minPooledObjects = 1;
	UPROPERTY(VisibleAnywhere, Category = "Object Pool")
		int32 maxPooledObjects = 10;

	UPROPERTY(VisibleAnywhere, Transient, Category = "Object Pool")
		TArray<UObject*> pooledObjects;

	//UPROPERTY()
	//	TArray<UObject*> pooledObjectsRef;

public:

	UObjectPool();

	bool AddObjectToPool(UObject* Object);

	UObject* GetPooledObject(bool& bIsValid);

	template<class T>
	T GetPooledObject(bool& bIsValid);

	int32 GetMinPooledObjectsAmount();

	void SetMinPooledObjectsAmount(int32 minPooledObjects);

	int32 GetMaxPooledObjectsAmount();

	void SetMaxPooledObjectsAmount(int32 maxPooledObjects);
};

template<class T>
inline T UObjectPool::GetPooledObject(bool& bIsValid)
{
	bIsValid = false;

	const int32 pooledObjectsNum = pooledObjects.Num();
	if (pooledObjectsNum > 0)
	{
		bool allowShrinking = pooledObjectsNum - 1 > minPooledObjects;
		T pooledObject = dynamic_cast<T, UObject>(pooledObjects.Pop(allowShrinking));

		if (pooledObject && pooledObject->IsValidLowLevel())
		{
			bIsValid = true;
			if (OnGetValidObject.IsBound())
			{
				OnGetValidObject.Broadcast(pooledObject);
			}
		}
		else
		{
			int32 nullCounter = 1;
			for (auto obj : pooledObjects)
			{
				if (obj == nullptr)
				{
					++nullCounter;
				}
			}

			if (OnRemovedNullptrs.IsBound())
			{
				OnRemovedNullptrs.Broadcast(nullCounter);
			}

			pooledObjects.Shrink();
			return GetPooledObject<T>(bIsValid);
		}

		return pooledObject;
	}

	return nullptr;
}