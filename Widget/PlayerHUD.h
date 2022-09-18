// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "PlayerHUD.generated.h"

/**
 * 
 */
UCLASS()
class HOWITBEGAN_API APlayerHUD : public AHUD
{
	GENERATED_BODY()
public:
	APlayerHUD();
	virtual void BeginPlay() override;

	void CreateGameplayWidget();

	//Pinging notifcation
	void CreateNotificationWidget();
	void ShowNotification(const FText& NotificationText);
	void ShowDeathWidget(class APlayerCharacter* Killer);

	//inventory
	void OpenInventoryWidget();
	bool IsInventoryOpen() const;

	void OpenLootWidget();
	void CloseLootWidget();

	void ShowHitmarker();

	void OpenBuildingWidget();
	bool IsBuildingWidgetOpen() const;
	void CloseBuildWidget();
protected:
	/**Widget classes and refs*/
	UPROPERTY(EditDefaultsOnly, Category = "Widgets")
	TSubclassOf<class UGamePlayWidget> GameplayWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "Widgets")
	TSubclassOf<class UInventoryWidget> InventoryWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "Widgets")
	TSubclassOf<class ULootWidget> LootWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "Widgets")
	TSubclassOf<class UNotificationWidget> NotificationWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "Widgets")
	TSubclassOf<class UBuildingWidget> BuildingWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "Widgets")
		TSubclassOf<class UDeathWidget> DeathWidgetClass;
public:
	UPROPERTY(BlueprintReadOnly, Category = "Widgets")
	class UInventoryWidget* InventoryWidget;

	UPROPERTY(BlueprintReadOnly, Category = "Widgets")
	class UNotificationWidget* NotificationWidget;

	UPROPERTY(BlueprintReadOnly, Category = "Widgets")
	class UGamePlayWidget* GameplayWidget;

	UPROPERTY(BlueprintReadOnly, Category = "Widgets")
	class ULootWidget* LootWidget;

	UPROPERTY(BlueprintReadOnly, Category = "Widgets")
	class UBuildingWidget* BuildingWidget;

	UPROPERTY(BlueprintReadOnly, Category = "Widgets")
	class UDeathWidget* DeathWidget;
};


