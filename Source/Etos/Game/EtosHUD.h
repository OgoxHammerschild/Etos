// © 2016 - 2017 Daniel Bortfeld

#pragma once

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets")
		TSubclassOf<class UInGameUI> wInGameUI;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets")
		TSubclassOf<UUserWidget> wWinScreen;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets")
		TSubclassOf<UUserWidget> wLoseScreen;

private:

	UPROPERTY()
		UInGameUI* inGameUI;

	UPROPERTY()
		UUserWidget* winScreen;

	UPROPERTY()
		UUserWidget* loseScreen;

public:

	UInGameUI* GetInGameUI();

	UUserWidget* GetWinScreen();

	UUserWidget* GetLoseScreen();

	virtual void BeginDestroy() override;
};
