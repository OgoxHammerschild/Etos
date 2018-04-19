// © 2016 - 2017 Daniel Bortfeld

#pragma once

class UInGameUI;
class AResourcePopup;
class UWarning;

#include "GameFramework/HUD.h"
#include "EtosHUD.generated.h"

/**
 *
 */
UCLASS()
class ETOS_API AEtosHUD : public AHUD
{
	GENERATED_BODY()

public:

	UPROPERTY(EditDefaultsOnly, Category = "Widgets")
		TSubclassOf<UInGameUI> wInGameUI;

	UPROPERTY(EditDefaultsOnly, Category = "Widgets")
		TSubclassOf<UUserWidget> wWinScreen;

	UPROPERTY(EditDefaultsOnly, Category = "Widgets")
		TSubclassOf<UUserWidget> wLoseScreen;

	UPROPERTY(EditDefaultsOnly, Category = "Widgets")
		TSubclassOf<UUserWidget> wPausedScreen;

	UPROPERTY(EditDefaultsOnly, Category = "Widgets")
		TSubclassOf<UUserWidget> wGameMenu;

	UPROPERTY(EditDefaultsOnly, Category = "Widgets")
		TSubclassOf<AResourcePopup> TextPopupBlueprint;

	UPROPERTY(EditDefaultsOnly, Category = "Widgets")
		TSubclassOf<UWarning> wWarning;

private:

	UPROPERTY()
		UInGameUI* inGameUI;

	UPROPERTY()
		UUserWidget* winScreen;

	UPROPERTY()
		UUserWidget* loseScreen;

	UPROPERTY()
		UUserWidget* pausedScreen;

	UPROPERTY()
		UUserWidget* gameMenu;

public:

	UFUNCTION(BluePrintpure, Category = HUD)
		UInGameUI* GetInGameUI();

	UFUNCTION(BluePrintpure, Category = HUD)
		UUserWidget* GetWinScreen();

	UFUNCTION(BluePrintpure, Category = HUD)
		UUserWidget* GetLoseScreen();

	UFUNCTION(BluePrintpure, Category = HUD)
		UUserWidget* GetGameMenu();

	UFUNCTION(BluePrintpure, Category = HUD)
		UUserWidget* GetPausedScreen();

	virtual void BeginDestroy() override;
};
