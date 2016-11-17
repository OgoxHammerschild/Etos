// © 2016 - 2017 Daniel Bortfeld

#pragma once

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

	UPROPERTY(EditDefaultsOnly, Category = "Build Button")
		TSubclassOf<class UBuildMenuButton> BuildMenuButtonBlueprint;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build Button")
		int32 ButtonsPerRow = 4;


private:

	UPROPERTY()
		AEtosPlayerController* playerController;

	UPROPERTY()
		TMap<EResource, UTextBlock* > texts;

	UPROPERTY()
		UUniformGridPanel* gridPanel;

public:

	virtual void NativeConstruct() override;

	// Create a reference between the elements in the designer and the elements in C++
	UFUNCTION(BlueprintCallable, Category = "Referencing")
		void SetGridPanel(UPARAM(DisplayName = "Button Grid Panel") UUniformGridPanel* panel);

	// Create a reference between the elements in the designer and the elements in C++
	UFUNCTION(BlueprintCallable, Category = "Referencing")
		void LinkTextToResource(UTextBlock* text, EResource resource);

	UFUNCTION(BlueprintCallable, Category = "Update")
		void UpdateResourceAmounts();

	UFUNCTION(BlueprintImplementableEvent, Category = "Building-Info", meta = (DisplayName = "Show Building-Info"))
		void BPEvent_ShowBuildingInfo(const FBuildingData& buildingData);

	UFUNCTION(BlueprintImplementableEvent, Category = "Building-Info", meta = (DisplayName = "Hide Building-Info"))
		void BPEvent_HideBuildingInfo();

private:

	AEtosPlayerController* GetPlayerController();

	void CreateButtons();
};
