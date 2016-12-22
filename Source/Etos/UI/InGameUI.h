// © 2016 - 2017 Daniel Bortfeld

#pragma once

class UResourceLayout;
class UBuildMenuButton;
class USatisfactionLayout;

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build Buttons")
		int32 ButtonsPerRow = 4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Info|Resource")
		int32 ResourcesPerRow = 3;

private:

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
		UGridPanel* gridPanel;

	UPROPERTY()
		UUniformGridPanel* residenceInfoPanel;

	UPROPERTY()
		TArray<UBuildMenuButton*> buttons;

	UPROPERTY()
		TMap<EResource, UResourceLayout*> resources;

	UPROPERTY()
		UResourceLayout* tempLayout;

	UPROPERTY()
		UBuildMenuButton* tempButton;

public:

	virtual void NativeConstruct() override;

	// Create a reference between the elements in the designer and the elements in C++
	UFUNCTION(BlueprintCallable, Category = "Referencing")
		void SetGridPanel(UPARAM(DisplayName = "Button Grid Panel") UGridPanel* panel);

	// Create a reference between the elements in the designer and the elements in C++
	UFUNCTION(BlueprintCallable, Category = "Referencing")
		void SetResidenceInfoPanel(UUniformGridPanel* panel);

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

	UFUNCTION(BlueprintImplementableEvent, Category = "Info|Building", meta = (DisplayName = "Show Building-Info"))
		void BPEvent_ShowBuildingInfo(const FBuildingData& buildingData);

	UFUNCTION(BlueprintImplementableEvent, Category = "Info|Building", meta = (DisplayName = "Hide Building-Info"))
		void BPEvent_HideBuildingInfo();

	UFUNCTION(BlueprintImplementableEvent, Category = "Info|Residence", meta = (DisplayName = "OnShowResidenceInfo"))
		void BPEvent_OnShowResidenceInfo();

	UFUNCTION(BlueprintImplementableEvent, Category = "Info|Residence", meta = (DisplayName = "OnHideResidenceInfo"))
		void BPEvent_OnHideResidenceInfo();

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

	UFUNCTION(BlueprintCallable, Category = "Info|Residence", meta = (DisplayName = "Show Residence-Info"))
		void ShowResidenceInfo(AResidence* residence);

	UFUNCTION(BlueprintCallable, Category = "Info|Residence", meta = (DisplayName = "Hide Residence-Info"))
		void HideResidenceInfo();

private:

	AEtosPlayerController* GetPlayerController();

	void CreateButtons();

	void UpdateResourceLayouts(const TMap<EResource, int32>& playerResourceAmounts);

	UGridSlot* AddChildToGridPanel(UWidget* Content, int32 Column, int32 Row);

	UUniformGridSlot* AddChildToResidenceInfoPanel(UWidget* Content, int32 Column, int32 Row);
};
