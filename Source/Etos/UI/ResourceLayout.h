// © 2016 - 2017 Daniel Bortfeld

#pragma once

#include "Etos/Buildings/Base/Building.h"
#include "Blueprint/UserWidget.h"
#include "ResourceLayout.generated.h"

/**
 *
 */
UCLASS()
class ETOS_API UResourceLayout : public UUserWidget
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, Category = "LayoutData")
		EResource Resource;

	UPROPERTY(BlueprintReadWrite, Category = "LayoutData")
		AEtosPlayerController* MyPlayerController;

public:

	UFUNCTION(BlueprintPure, Category = "LayoutData")
		float GetStoredPercentage();

	UFUNCTION(BlueprintPure, Category = "LayoutData")
		int32 GetResourceAmount();
};

