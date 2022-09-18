// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InventoryComponent.generated.h"


//Called when the inventory is changed and the UI needs an update. 
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryUpdated);

/**Called on server when an item is added to this inventory*/
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemAdded, class UItemsObject*, Item);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemRemoved, class UItemsObject*, Item);

UENUM(BlueprintType)
enum class EItemAddResult : uint8
{
	IAR_NoItemsAdded UMETA(DisplayName = "No items added"),
	IAR_SomeItemsAdded UMETA(DisplayName = "Some items added"),
	IAR_AllItemsAdded UMETA(DisplayName = "All items added")
};
//Represents the result of adding an item to the inventory.
USTRUCT(BlueprintType)
struct FItemAddResult
{

	GENERATED_BODY()

public:

	FItemAddResult() {};
	FItemAddResult(int32 InItemQuantity) : AmountToGive(InItemQuantity), AmountGiven(0) {};
	FItemAddResult(int32 InItemQuantity, int32 InQuantityAdded) : AmountToGive(InItemQuantity), AmountGiven(InQuantityAdded) {};

	//The amount of the item that we tried to add
	UPROPERTY(BlueprintReadOnly, Category = "Item Add Result")
	int32 AmountToGive = 0;

	//The amount of the item that was actually added in the end. Maybe we tried adding 10 items, but only 8 could be added because of capacity/weight
	UPROPERTY(BlueprintReadOnly, Category = "Item Add Result")
	int32 AmountGiven = 0;

	//The result
	UPROPERTY(BlueprintReadOnly, Category = "Item Add Result")
	EItemAddResult Result = EItemAddResult::IAR_NoItemsAdded;

	//If something went wrong, like we didnt have enough capacity or carrying too much weight this contains the reason why
	UPROPERTY(BlueprintReadOnly, Category = "Item Add Result")
	FText ErrorText = FText::GetEmpty();

	//Helpers
	static FItemAddResult AddedNone(const int32 InItemQuantity, const FText& ErrorText)
	{
		FItemAddResult AddedNoneResult(InItemQuantity);
		AddedNoneResult.Result = EItemAddResult::IAR_NoItemsAdded;
		AddedNoneResult.ErrorText = ErrorText;

		return AddedNoneResult;
	}

	static FItemAddResult AddedSome(const int32 InItemQuantity, const int32 ActualAmountGiven, const FText& ErrorText)
	{
		FItemAddResult AddedSomeResult(InItemQuantity, ActualAmountGiven);

		AddedSomeResult.Result = EItemAddResult::IAR_SomeItemsAdded;
		AddedSomeResult.ErrorText = ErrorText;

		return AddedSomeResult;
	}

	static FItemAddResult AddedAll(const int32 InItemQuantity)
	{
		FItemAddResult AddAllResult(InItemQuantity, InItemQuantity);

		AddAllResult.Result = EItemAddResult::IAR_AllItemsAdded;

		return AddAllResult;
	}

};


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HOWITBEGAN_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

	friend class UItemsObject;
public:	
	// Sets default values for this component's properties
	UInventoryComponent();

	//Add an item to the inventory.
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	FItemAddResult TryAddItem(class UItemsObject* Item);

	//Add an item to the inventory using the item class instead of an item instance.
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	FItemAddResult TryAddItemFromClass(TSubclassOf<class UItemsObject> ItemClass, const int32 Quantity = 1);

	/** Take some quantity away from the item, and remove it from the inventory when quantity reaches zero.
Useful for things like eating food, using ammo, etc.*/
	int32 ConsumeItem(class UItemsObject* Item);
	int32 ConsumeItem(class UItemsObject* Item, const int32 Quantity);
	/** Remove the item from the inventory*/
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool RemoveItem(class UItemsObject* Item);

	/**Return true if we have a given amount of an item*/
	UFUNCTION(BlueprintPure, Category = "Inventory")
	bool HasItem(TSubclassOf <class UItemsObject> ItemClass, const int32 Quantity = 1) const;

	/**Return the first item with the same class as a given Item*/
	UFUNCTION(BlueprintPure, Category = "Inventory")
	UItemsObject* FindItem(class UItemsObject* Item) const;

	/**Return the first item with the same class as ItemClass.*/
	UFUNCTION(BlueprintPure, Category = "Inventory")
	UItemsObject* FindItemByClass(TSubclassOf<class UItemsObject> ItemClass) const;

	/**Get all inventory items that are a child of ItemClass. Useful for grabbing all weapons, all food, etc*/
	UFUNCTION(BlueprintPure, Category = "Inventory")
	TArray<UItemsObject*> FindItemsByClass(TSubclassOf<class UItemsObject> ItemClass) const;

	//Get the current weight of the inventory. To get the amount of items in the inventory, just do GetItems().Num()
	UFUNCTION(BlueprintPure, Category = "Inventory")
	float GetCurrentWeight() const;

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SetWeightCapacity(const float NewWeightCapacity);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SetCapacity(const int32 NewCapacity);

	UFUNCTION(BlueprintPure, Category = "Inventory")
	FORCEINLINE float GetWeightCapacity() const { return WeightCapacity; };

	UFUNCTION(BlueprintPure, Category = "Inventory")
	FORCEINLINE int32 GetCapacity() const { return Capacity; }

	UFUNCTION(BlueprintPure, Category = "Inventory")
	FORCEINLINE TArray<class UItemsObject*> GetItems() const { return Items; }

	UFUNCTION(Client, Reliable)
	void ClientRefreshInventory();

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnInventoryUpdated OnInventoryUpdated;

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnItemAdded OnItemAdded;

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnItemRemoved OnItemRemoved;

protected:

	//The maximum weight the inventory can hold. For players, backpacks and other items increase this limit
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory")
	float WeightCapacity;

	//The maximum number of items the inventory can hold. For players, backpacks and other items increase this limit
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory", meta = (ClampMin = 0, ClampMax = 200))
	int32 Capacity;

	/**The items currently in our inventory*/
	UPROPERTY(ReplicatedUsing = OnRep_Items, VisibleAnywhere, Category = "Inventory")
	TArray<class UItemsObject*> Items;

	/**each time the clients items OnRep, we cache them so we can see which items were added/removed since last OnRep*/
	UPROPERTY()
	TArray<class UItemsObject*> ClientLastReceivedItems;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags) override;

private:

	/**Don't call Items.Add() directly, use this function instead, as it handles replication and ownership*/
	UItemsObject* AddItem(class UItemsObject* Item, const int32 Quantity);

	UFUNCTION()
	void OnRep_Items();

	UPROPERTY()
	int32 ReplicatedItemsKey;

	//Internal, non-BP exposed add item function. Don't call this directly, use TryAddItem(), or TryAddItemFromClass() instead.
	FItemAddResult TryAddItem_Internal(class UItemsObject* Item);

	UFUNCTION()
	void ItemAdded(class UItemsObject* Item);

	UFUNCTION()
	void ItemRemoved(class UItemsObject* Item);

		
};
