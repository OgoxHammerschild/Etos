// © 2016 - 2017 Daniel Bortfeld

#pragma once

#include "Blueprint/UserWidget.h"
#include "BuildMenuButton.generated.h"

/**
 *
 */
UCLASS()
class ETOS_API UBuildMenuButton : public UUserWidget
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadOnly, Category = "UIElements")
		UButton* BuildButton;

	UPROPERTY(BlueprintReadOnly, Category = "UIElements")
		UImage* BuildingIcon;

	UPROPERTY()
		class ABuilding* Building;

	UPROPERTY()
		struct FBuildingData Data;

public:

	virtual void NativeConstruct() override;

	// Create a reference between the elements in the designer and the elements in C++
	UFUNCTION(BlueprintCallable, Category = "UIElements")
		void SetButtonAndIcon(UPARAM(DisplayName = "Bulid Button") UButton* button, UPARAM(DisplayName = "Building Icon")  UImage* icon);

private:

	UFUNCTION()
		void SpawnBuilding();
};
