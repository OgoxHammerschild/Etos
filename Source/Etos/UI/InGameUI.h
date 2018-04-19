// © 2016 - 2017 Daniel Bortfeld

#pragma once

class UResourceLayout;
class UBuildMenuButton;
class USatisfactionLayout;
class UResourceCostLayout;

#include "UMG.h"
#include "UMGStyle.h"
#include "Slate/SObjectWidget.h"
#include "IUMGModule.h"
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

	UPROPERTY(EditDefaultsOnly, Category = "Info|Residence")
		TSubclassOf<USatisfactionLayout> SatisfactionLayoutBlueprint;

	UPROPERTY(EditDefaultsOnly, Category = "Info|Build Cost")
		TSubclassOf<UResourceCostLayout> ResourceCostLayoutBlueprint;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build Buttons")
		int32 ButtonsPerRow = 4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Info|Resource")
		int32 ResourcesPerRow = 3;

private:

	UPROPERTY()
		ABuilding* selectedBuilding;

	UPROPERTY()
		AResidence* selectedResidence;

	UPROPERTY()
		AEtosPlayerController* playerController;

	UPROPERTY()
		// the texts in the top left of the screen
		TMap<EResource, UTextBlock*> resourceTexts;

	UPROPERTY()
		UTextBlock* populationText;

	UPROPERTY()
		UTextBlock* peasantText;

	UPROPERTY()
		UTextBlock* citizenText;

	UPROPERTY()
		UTextBlock* balanceText;

	UPROPERTY()
		UTextBlock* incomeText;

	UPROPERTY()
		UTextBlock* upkeepText;

	UPROPERTY()
		UGridPanel* buildButtonGridPanel;

	UPROPERTY()
		UGridPanel* resourceInfoGridPanel;

	UPROPERTY()
		UUniformGridPanel* residenceInfoPanel;

	UPROPERTY()
		UVerticalBox* buildCostsPanel;

	UPROPERTY()
		TArray<UBuildMenuButton*> buttons;

	UPROPERTY()
		TMap<EResource, UResourceLayout*> resources;

	UPROPERTY()
		UResourceLayout* tempLayout;

	UPROPERTY()
		UBuildMenuButton* tempButton;

	UPROPERTY()
		TMap<EEnableBuilding, FText> buttonToolTipTexts;

public:

	virtual void NativeConstruct() override;

	// Create a reference between the elements in the designer and the elements in C++
	UFUNCTION(BlueprintCallable, Category = "Referencing")
		void SetGridPanel(UPARAM(DisplayName = "Build Button Grid Panel") UGridPanel* buttonPanel, UPARAM(DisplayName = "Resource Info Grid Panel") UGridPanel* resourcePanel);

	// Create a reference between the elements in the designer and the elements in C++
	UFUNCTION(BlueprintCallable, Category = "Referencing")
		void SetResidenceInfoPanel(UUniformGridPanel* panel);

	// Create a reference between the elements in the designer and the elements in C++
	UFUNCTION(BlueprintCallable, Category = "Referencing")
		void SetBuildCostsPanel(UVerticalBox* panel);

	// Create a reference between the elements in the designer and the elements in C++
	UFUNCTION(BlueprintCallable, Category = "Referencing")
		void LinkTextToResource(UTextBlock* text, EResource resource);

	// Create a reference between the elements in the designer and the elements in C++
	UFUNCTION(BlueprintCallable, Category = "Referencing")
		void SetPopulationTexts(UPARAM(DisplayName = "Total Population") UTextBlock* population, UTextBlock* peasants, UTextBlock* citizens);

	// Create a reference between the elements in the designer and the elements in C++
	UFUNCTION(BlueprintCallable, Category = "Referencing")
		void SetBalanceTexts(UTextBlock* balance, UTextBlock* income, UTextBlock* upkeep);

	UFUNCTION(BlueprintCallable, Category = "Update")
		void UpdateResourceAmounts();

	UFUNCTION(BlueprintCallable, Category = "Update")
		void UpdatePopulation(const int32& peasants, const int32& citizens);

	UFUNCTION(BlueprintCallable, Category = "Update")
		void UpdateBalance(const int32& income, const int32& upkeep);

	UFUNCTION(BlueprintCallable, Category = "Info|Building")
		void ShowBuildingInfo(ABuilding* InSelectedBuilding);

	// Don't call this directly, use ShowBuildingInfo(ABuilding* InSelectedBuilding) instead
	UFUNCTION(BlueprintImplementableEvent, Category = "Info|Building", meta = (DisplayName = "Show Building-Info"))
		void BPEvent_ShowBuildingInfo(const FBuildingData& buildingData);

	UFUNCTION(BlueprintImplementableEvent, Category = "Info|Building", meta = (DisplayName = "Hide Building-Info"))
		void BPEvent_HideBuildingInfo();

	// Don't call this directly, use ShowResidenceInfo(AResidence* residence) instead
	UFUNCTION(BlueprintImplementableEvent, Category = "Info|Residence", meta = (DisplayName = "OnShowResidenceInfo"))
		void BPEvent_OnShowResidenceInfo();

	// Don't call this directly, use HideResidenceInfo() instead
	UFUNCTION(BlueprintImplementableEvent, Category = "Info|Residence", meta = (DisplayName = "OnHideResidenceInfo"))
		void BPEvent_OnHideResidenceInfo();

	//Note: Make to FResource-array ?
	UFUNCTION(BlueprintCallable, Category = "Info|Resource", meta = (DisplayName = "Show Resource-Info"))
		void ShowResourceInfo(const TArray<EResource>& playerResources, const  TArray<int32>& playerResourceAmounts);

	void ShowResourceInfo(const TMap<EResource, int32>& playerResourceAmounts);

	UFUNCTION(BlueprintCallable, Category = "Info|Resource", meta = (DisplayName = "Hide Resource-Info"))
		void HideResourceInfo();

	UFUNCTION(BlueprintCallable, Category = "Menu Actions|Build Buttons", meta = (DisplayName = "Show Build Buttons"))
		void ShowBuildButtons();

	UFUNCTION(BlueprintCallable, Category = "Menu Actions|Build Buttons", meta = (DisplayName = "Hide Build Buttons"))
		void HideBuildButtons();

	UFUNCTION(BlueprintCallable, Category = "Info|Residence", meta = (DisplayName = "Show Residence-Info"))
		void ShowResidenceInfo(AResidence* residence);

	UFUNCTION(BlueprintCallable, Category = "Info|Residence", meta = (DisplayName = "Hide Residence-Info"))
		void HideResidenceInfo();

	UFUNCTION(BlueprintCallable, Category = "Menu Actions|Demolish")
		void StartDemolishing();

	UFUNCTION(BlueprintPure, Category = "Info|Building")
		FName GetSelectedBuildingName();

	// is ensured
	UFUNCTION(BlueprintPure, Category = "Info|Building")
		UTexture2D* GetSelectedBuilding_Icon();

	UFUNCTION(BlueprintPure, Category = "Info|Building")
		int32 GetSelectedBuilding_Upkeep();

	UFUNCTION(BlueprintPure, Category = "Info|Building")
		FResource GetSelectedBuilding_NeededResource1();

	UFUNCTION(BlueprintPure, Category = "Info|Building")
		FResource GetSelectedBuilding_NeededResource2();

	UFUNCTION(BlueprintPure, Category = "Info|Building")
		FResource GetSelectedBuilding_ProducedResource();

	UFUNCTION(BlueprintPure, Category = "Info|Building")
		int32 GetSelectedBuilding_MaxStoredResources();

	UFUNCTION(BlueprintPure, Category = "Info|Residence")
		FText GetSelectedResidence_ResidentsText();

private:

	UFUNCTION()
		void ShowBuildCost(FBuildingData const& BuildingData);

	UFUNCTION()
		void HideBuildCost(FBuildingData const& BuildingData);

	AEtosPlayerController* GetPlayerController();

	void CreateButtons();

	void UpdateResourceLayouts(const TMap<EResource, int32>& PlayerResourceAmounts);

	UGridSlot* AddChildToGridPanel(UGridPanel* Panel, UWidget* Content, int32 Column, int32 Row);

	UUniformGridSlot* AddChildToGridPanel(UUniformGridPanel* Panel, UWidget* Content, int32 Column, int32 Row);
};
