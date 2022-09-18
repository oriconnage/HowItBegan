// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryComponent.h"
#include "Net/UnrealNetwork.h"
#include "Engine/ActorChannel.h"
#include"../item/ItemsObject.h"


#define LOCTEXT_NAMESPACE "Inventory"

// Sets default values for this component's properties
UInventoryComponent::UInventoryComponent()
{
	OnItemAdded.AddDynamic(this, &UInventoryComponent::ItemAdded);
	OnItemRemoved.AddDynamic(this, &UInventoryComponent::ItemRemoved);

	SetIsReplicatedByDefault(true);
}


FItemAddResult UInventoryComponent::TryAddItem(class UItemsObject* Item)
{
	GEngine->AddOnScreenDebugMessage(-1, 15, FColor::Green, TEXT("Try add item"));
	return TryAddItem_Internal(Item);
}

FItemAddResult UInventoryComponent::TryAddItemFromClass(TSubclassOf<class UItemsObject> ItemClass, const int32 Quantity /*=1*/)
{
	UItemsObject* Item = NewObject<UItemsObject>(GetOwner(), ItemClass);
	Item->SetQuantity(Quantity);
	return TryAddItem_Internal(Item);
}

int32 UInventoryComponent::ConsumeItem(class UItemsObject* Item)
{
	if (Item)
	{
		ConsumeItem(Item, Item->GetQuantity());
	}
	return 0;
}

int32 UInventoryComponent::ConsumeItem(class UItemsObject* Item, const int32 Quantity)
{
	if (GetOwner() && GetOwner()->HasAuthority() && Item)
	{
		const int32 RemoveQuantity = FMath::Min(Quantity, Item->GetQuantity());

		//We shouldn't have a negative amount of the item after the drop
		ensure(!(Item->GetQuantity() - RemoveQuantity < 0));

		//We now have zero of this item, remove it from the inventory
		Item->SetQuantity(Item->GetQuantity() - RemoveQuantity);

		if (Item->GetQuantity() <= 0)
		{
			RemoveItem(Item);
		}
		else
		{
			ClientRefreshInventory();
		}

		return RemoveQuantity;
	}

	return 0;
}

bool UInventoryComponent::RemoveItem(class UItemsObject* Item)
{
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		if (Item)
		{
			Items.RemoveSingle(Item);
			OnItemRemoved.Broadcast(Item);

			OnRep_Items();

			ReplicatedItemsKey++;

			return true;
		}
	}

	return false;
}

bool UInventoryComponent::HasItem(TSubclassOf <class UItemsObject> ItemClass, const int32 Quantity /*= 1*/) const
{
	if (UItemsObject* ItemToFind = FindItemByClass(ItemClass))
	{
		return ItemToFind->GetQuantity() >= Quantity;
	}
	return false;
}

UItemsObject* UInventoryComponent::FindItem(class UItemsObject* Item) const
{
	if (Item)
	{
		for (auto& InvItem : Items)
		{
			if (InvItem && InvItem->GetClass() == Item->GetClass())
			{
				return InvItem;
			}
		}
	}
	return nullptr;
}

UItemsObject* UInventoryComponent::FindItemByClass(TSubclassOf<class UItemsObject> ItemClass) const
{
	for (auto& InvItem : Items)
	{
		if (InvItem && InvItem->GetClass() == ItemClass)
		{
			return InvItem;
		}
	}
	return nullptr;
}

TArray<UItemsObject*> UInventoryComponent::FindItemsByClass(TSubclassOf<class UItemsObject> ItemClass) const
{
	TArray<UItemsObject*> ItemsOfClass;

	for (auto& InvItem : Items)
	{
		if (InvItem && InvItem->GetClass()->IsChildOf(ItemClass))
		{
			
			ItemsOfClass.Add(InvItem);
		}
	}

	return ItemsOfClass;
}

float UInventoryComponent::GetCurrentWeight() const
{
	float Weight = 0.f;

	for (auto& Item : Items)
	{
		if (Item)
		{
			Weight += Item->GetStackWeight();
		}
	}

	return Weight;
}

void UInventoryComponent::SetWeightCapacity(const float NewWeightCapacity)
{
	WeightCapacity = NewWeightCapacity;
	OnInventoryUpdated.Broadcast();
}

void UInventoryComponent::SetCapacity(const int32 NewCapacity)
{
	Capacity = NewCapacity;
	OnInventoryUpdated.Broadcast();
}

void UInventoryComponent::ClientRefreshInventory_Implementation()
{
	OnInventoryUpdated.Broadcast();
}

void UInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UInventoryComponent, Items);
}

bool UInventoryComponent::ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	bool bUpdated = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

	//Check if the array of items needs to replicate
	if (Channel->KeyNeedsToReplicate(0, ReplicatedItemsKey))
	{
		for (auto& Item : Items)
		{
			if (Channel->KeyNeedsToReplicate(Item->GetUniqueID(), Item->RepKey))
			{
				bUpdated |= Channel->ReplicateSubobject(Item, *Bunch, *RepFlags);
			}
		}
	}

	return bUpdated;
}

UItemsObject* UInventoryComponent::AddItem(class UItemsObject* Item, const int32 Quantity)
{
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		UItemsObject* NewItem = NewObject<UItemsObject>(GetOwner(), Item->GetClass());
		NewItem->World = GetWorld();
		NewItem->SetQuantity(Quantity);
		NewItem->OwningInventory = this;
		NewItem->AddedToInventory(this);
		Items.Add(NewItem);
		NewItem->MarkDirtyForReplication();
		OnItemAdded.Broadcast(NewItem);
		OnRep_Items();
		GEngine->AddOnScreenDebugMessage(-1, 15, FColor::Green, TEXT("Has added item"));
		return NewItem;
	}

	return nullptr;
}

void UInventoryComponent::OnRep_Items()
{
	OnInventoryUpdated.Broadcast();

	for (auto& Item : Items)
	{
		//On the client the world won't be set initially, so it set if not
		if (!Item->World)
		{
			OnItemAdded.Broadcast(Item);
			Item->World = GetWorld();
		}
	}
}

FItemAddResult UInventoryComponent::TryAddItem_Internal(class UItemsObject* Item)
{
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		if (Item->bStackable)
		{
			//Somehow the items quantity went over the max stack size. This shouldn't ever happen
			ensure(Item->GetQuantity() <= Item->MaxStackSize);

			if (UItemsObject* ExistingItem = FindItem(Item))
			{
				if (ExistingItem->IsStackFull())
				{
					return FItemAddResult::AddedNone(Item->GetQuantity(), FText::Format(LOCTEXT("StackFullText", "Couldn't add {ItemName}. Tried adding items to a stack that was full."), Item->DisplayName));
				}
				else
				{
					//Find the maximum amount of the item we could take due to weight
					const int32 WeightMaxAddAmount = FMath::FloorToInt((WeightCapacity - GetCurrentWeight()) / Item->Weight);
					const int32 QuantityMaxAddAmount = FMath::Min(ExistingItem->MaxStackSize - ExistingItem->GetQuantity(), Item->GetQuantity());
					const int32 AddAmount = FMath::Min(WeightMaxAddAmount, QuantityMaxAddAmount);

					if (AddAmount <= 0)
					{
						//Already did full stack check, must not have enough weight
						return FItemAddResult::AddedNone(Item->GetQuantity(), FText::Format(LOCTEXT("StackWeightFullText", "Couldn't add {ItemName}, too much weight."), Item->DisplayName));
					}
					else
					{
						ExistingItem->SetQuantity(ExistingItem->GetQuantity() + AddAmount);
						return AddAmount >= Item->GetQuantity() ? FItemAddResult::AddedAll(Item->GetQuantity()) : FItemAddResult::AddedSome(Item->GetQuantity(), AddAmount, LOCTEXT("StackAddedSomeFullText", "Couldn't add all of stack to inventory."));
					}
				}
			}
			else //we want to add a stackable item that doesn't exist in the inventory
			{
				if (Items.Num() + 1 > GetCapacity())
				{
					return FItemAddResult::AddedNone(Item->GetQuantity(), FText::Format(LOCTEXT("InventoryCapacityFullText", "Couldn't add {ItemName} to Inventory. Inventory is full."), Item->DisplayName));
				}

				const int32 WeightMaxAddAmount = FMath::FloorToInt((WeightCapacity - GetCurrentWeight()) / Item->Weight);
				const int32 QuantityMaxAddAmount = FMath::Min(Item->MaxStackSize, Item->GetQuantity());
				const int32 AddAmount = FMath::Min(WeightMaxAddAmount, QuantityMaxAddAmount);

				AddItem(Item, AddAmount);

				return AddAmount >= Item->GetQuantity() ? FItemAddResult::AddedAll(Item->GetQuantity()) : FItemAddResult::AddedSome(Item->GetQuantity(), AddAmount, LOCTEXT("StackAddedSomeFullText", "Couldn't add all of stack to inventory."));
			}
		}
		else //item isnt stackable
		{
			if (Items.Num() + 1 > GetCapacity())
			{
				return FItemAddResult::AddedNone(Item->GetQuantity(), FText::Format(LOCTEXT("InventoryCapacityFullText", "Couldn't add {ItemName} to Inventory. Inventory is full."), Item->DisplayName));
			}

			//Items with a weight of zero dont require a weight check
			if (!FMath::IsNearlyZero(Item->Weight))
			{
				if (GetCurrentWeight() + Item->Weight > GetWeightCapacity())
				{
					return FItemAddResult::AddedNone(Item->GetQuantity(), FText::Format(LOCTEXT("StackWeightFullText", "Couldn't add {ItemName}, too much weight."), Item->DisplayName));
				}
			}

			//Non-stackables should always have a quantity of 1
			ensure(Item->GetQuantity() == 1);
			AddItem(Item, 1);

			return FItemAddResult::AddedAll(Item->GetQuantity());
		}
	}
	GEngine->AddOnScreenDebugMessage(-1, 15, FColor::Green, TEXT("Has no Authority"));
	return FItemAddResult::AddedNone(-1, LOCTEXT("ErrorMessage", ""));

}

void UInventoryComponent::ItemAdded(class UItemsObject* Item)
{
	FString RoleString = GetOwner()->HasAuthority() ? "server" : "client";
	UE_LOG(LogTemp, Warning, TEXT("Item added: %s on %s"), *GetNameSafe(Item), *RoleString);
}

void UInventoryComponent::ItemRemoved(class UItemsObject* Item)
{
	FString RoleString = GetOwner()->HasAuthority() ? "server" : "client";
	UE_LOG(LogTemp, Warning, TEXT("Item Removed: %s on %s"), *GetNameSafe(Item), *RoleString);
}

#undef LOCTEXT_NAMESPACE
