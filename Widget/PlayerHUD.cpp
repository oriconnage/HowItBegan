// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerHUD.h"

#include "BuildingWidget.h"
#include "DeathWidget.h"
#include"InventoryWidget.h"
#include "NotificationWidget.h"
#include "GamePlayWidget.h"
#include "LootWidget.h"

APlayerHUD::APlayerHUD() 
{

}
void APlayerHUD::BeginPlay()
{
	Super::BeginPlay();

	//Create the inventory component before we even open it so it updates before we open it
	if (!InventoryWidget && InventoryWidgetClass && PlayerOwner)
	{
		InventoryWidget = CreateWidget<UInventoryWidget>(PlayerOwner, InventoryWidgetClass);
	}
	CreateGameplayWidget();
}

void APlayerHUD::CreateGameplayWidget()
{
	//If death widget was on screen remove it
	if (DeathWidget)
	{
		DeathWidget->RemoveFromViewport();
	}

	// create an Notification widget.
	if (!NotificationWidget)
	{
		CreateNotificationWidget();
	}

	//Create Gameplay widget if it doesn't exist
	if (!GameplayWidget && GameplayWidgetClass && PlayerOwner)
	{
		GameplayWidget = CreateWidget<UGamePlayWidget>(PlayerOwner, GameplayWidgetClass);

		PlayerOwner->bShowMouseCursor = false;
		PlayerOwner->SetInputMode(FInputModeGameOnly());
	}

	//Add to viewport
	if (GameplayWidget && !GameplayWidget->IsInViewport())
	{
		//Keep Gameplay widget on top by using high Z order
		GameplayWidget->AddToViewport();
	}
}

void APlayerHUD::ShowDeathWidget(APlayerCharacter* Killer)
{
	//If death widget was on screen remove it
	if (GameplayWidget)
	{
		GameplayWidget->RemoveFromViewport();
	}

	//Create Death widget if it doesn't exist
	if (!DeathWidget && DeathWidgetClass && PlayerOwner)
	{
		DeathWidget = CreateWidget<UDeathWidget>(PlayerOwner, DeathWidgetClass);
	}

	//Add to viewport
	if (DeathWidget)
	{
		DeathWidget->killer = Killer;
		//Keep Death widget on top by using high Z order
		DeathWidget->AddToViewport();
	}
}

void APlayerHUD::OpenInventoryWidget()
{
	//Create inventory widget if it doesn't exist
	if (!InventoryWidget && InventoryWidgetClass && PlayerOwner)
	{
		InventoryWidget = CreateWidget<UInventoryWidget>(PlayerOwner, InventoryWidgetClass);
	}

	//Add to viewport
	if (InventoryWidget && !InventoryWidget->IsInViewport())
	{
		InventoryWidget->AddToViewport();
		InventoryWidget->SetKeyboardFocus();

		FInputModeUIOnly UIInput;
		UIInput.SetWidgetToFocus(InventoryWidget->TakeWidget());

		PlayerOwner->bShowMouseCursor = true;
		PlayerOwner->SetInputMode(UIInput);
	}
}


bool APlayerHUD::IsInventoryOpen() const
{
	return InventoryWidget ? InventoryWidget->IsInViewport() : false;
}

void APlayerHUD::OpenLootWidget()
{
	//Create Loot widget if it doesn't exist
	if (!LootWidget && LootWidgetClass && PlayerOwner)
	{
		LootWidget = CreateWidget<ULootWidget>(PlayerOwner, LootWidgetClass);
	}

	//Add to viewport
	if (LootWidget)
	{
		LootWidget->AddToViewport();

		FInputModeUIOnly UIInput;
		UIInput.SetWidgetToFocus(LootWidget->TakeWidget());

		PlayerOwner->bShowMouseCursor = true;
		PlayerOwner->SetInputMode(UIInput);
	}
}

void APlayerHUD::CloseLootWidget()
{
	if (LootWidget && LootWidget->IsInViewport())
	{
		LootWidget->RemoveFromViewport();

		PlayerOwner->bShowMouseCursor = true;
		PlayerOwner->SetInputMode(FInputModeGameOnly());
	}
}

void APlayerHUD::ShowHitmarker()
{
	if (GameplayWidget) {
		GameplayWidget->ShowHitmarker();
	}
}

void APlayerHUD::OpenBuildingWidget()
{
	//Create Loot widget if it doesn't exist
	if (!BuildingWidget && BuildingWidgetClass && PlayerOwner)
	{
		BuildingWidget = CreateWidget<UBuildingWidget>(PlayerOwner, BuildingWidgetClass);
	}
	//Add to viewport
	if (BuildingWidget)
	{
		BuildingWidget->AddToViewport();

		FInputModeUIOnly UIInput;
		UIInput.SetWidgetToFocus(BuildingWidget->TakeWidget());

		PlayerOwner->bShowMouseCursor = true;
		PlayerOwner->SetInputMode(UIInput);
	}
}

bool APlayerHUD::IsBuildingWidgetOpen() const
{
	return BuildingWidget ? BuildingWidget->IsInViewport() : false;
}

void APlayerHUD::CloseBuildWidget()
{
}

void APlayerHUD::CreateNotificationWidget()
{
	//Create Notification widget if it doesn't exist
	if (!NotificationWidget && NotificationWidgetClass && PlayerOwner)
	{
		NotificationWidget = CreateWidget<UNotificationWidget>(PlayerOwner, NotificationWidgetClass);
	}

	//Add to viewport
	if (NotificationWidget)
	{
		//Keep notification widget on top by using high Z order
		NotificationWidget->AddToViewport(1);
	}
}

void APlayerHUD::ShowNotification(const FText& NotificationText)
{
	if (NotificationWidget)
	{
		NotificationWidget->ShowNotification(NotificationText);
	}
}