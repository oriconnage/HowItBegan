// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "InventoryComponent.h"
#include "Components/WidgetComponent.h"
#include "CraftComponent.generated.h"

/**
 * 
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class HOWITBEGAN_API UCraftComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	//Add an item to the inventory.
	UFUNCTION(BlueprintCallable, Category = "CraftComponent")
	FItemAddResult AddThisItem(TSubclassOf<class UItemsObject> Item);

	UFUNCTION(BlueprintPure, Category = "CraftComponent")
	FORCEINLINE TArray<class UItemsObject*> GetItems() const { return Items; }

private:
	UItemsObject* AddItem(class UItemsObject* Item);

	FItemAddResult TryAddItem_Internal(class UItemsObject* Item);

	UPROPERTY(VisibleAnywhere, Category = "CraftComponent")
	TArray<class UItemsObject*> Items;
	
};
