// © 2016 - 2017 Daniel Bortfeld

#pragma once

class UResourceLayout;
class UBuildMenuButton;
#include "Blueprint/UserWidget.h"
#include "Etos/Game/EtosPlayerController.h"
#include "Etos/Buildings/Base/Building.h"
#include "InGameUI.generated.h"

/**
 *
 */
UCLASS()
class ETOS_API UInGameUI : public UUserWidget
{
	GENERATED_BODY()

public:

	UPROPERTY(EditDefaultsOnly, Category = "Info|Resource")
		TSubclassOf<UResourceLayout> ResourceLayoutBlueprint;

	UPROPERTY(EditDefaultsOnly, Category = "Build Buttons")
		TSubclassOf<UBuildMenuButton> BuildMenuButtonBlueprint;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build Buttons")
		int32 ButtonsPerRow = 4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Info|Resource")
		int32 ResourcesPerRow = 3;

private:

	UPROPERTY()
		AEtosPlayerController* playerController;

	UPROPERTY()
		TMap<EResource, UTextBlock* > texts;

	UPROPERTY()
		UGridPanel* gridPanel;

	UPROPERTY()
		TArray<UBuildMenuButton*> buttons;

	UPROPERTY()
		TMap<EResource, UResourceLayout*> resources;

public:

	virtual void NativeConstruct() override;

	// Create a reference between the elements in the designer and the elements in C++
	UFUNCTION(BlueprintCallable, Category = "Referencing")
		void SetGridPanel(UPARAM(DisplayName = "Button Grid Panel") UGridPanel* panel);

	// Create a reference between the elements in the designer and the elements in C++
	UFUNCTION(BlueprintCallable, Category = "Referencing")
		void LinkTextToResource(UTextBlock* text, EResource resource);

	UFUNCTION(BlueprintCallable, Category = "Update")
		void UpdateResourceAmounts();

	UFUNCTION(BlueprintImplementableEvent, Category = "Info|Building", meta = (DisplayName = "Show Building-Info"))
		void BPEvent_ShowBuildingInfo(const FBuildingData& buildingData);

	UFUNCTION(BlueprintImplementableEvent, Category = "Info|Building", meta = (DisplayName = "Hide Building-Info"))
		void BPEvent_HideBuildingInfo();

	//Note: Make to FResource-array ?
	UFUNCTION(BlueprintCallable, Category = "Info|Resource", meta = (DisplayName = "Show Resource-Info"))
		void ShowResourceInfo(const TArray<TEnumAsByte<EResource>>& playerResources, const  TArray<int32>& playerResourceAmounts);

	void ShowResourceInfo(const TMap<EResource, int32>& playerResourceAmounts);

	UFUNCTION(BlueprintCallable, Category = "Info|Resource", meta = (DisplayName = "Hide Resource-Info"))
		void HideResourceInfo();

	UFUNCTION(BlueprintCallable, Category = "Build Buttons", meta = (DisplayName = "Show Build Buttons"))
		void ShowBuildButtons();

	UFUNCTION(BlueprintCallable, Category = "Build Buttons", meta = (DisplayName = "Hide Build Buttons"))
		void HideBuildButtons();

private:

	AEtosPlayerController* GetPlayerController();

	void CreateButtons();

	void UpdateResourceLayouts(const TMap<EResource, int32>& playerResourceAmounts);

	UGridSlot* AddChildToGridPanel(UWidget* Content, int32 Column, int32 Row);
};
