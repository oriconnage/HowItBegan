// Fill out your copyright notice in the Description page of Project Settings.


#include "CraftComponent.h"

#include "HowItBegan/item/ItemsObject.h"

#define LOCTEXT_NAMESPACE "CraftInventory"
FItemAddResult UCraftComponent::AddThisItem(TSubclassOf<class UItemsObject> Item)
{
	UItemsObject* item = NewObject<UItemsObject>(GetOwner(), Item);
	ensure(item->GetQuantity() == 1);
	return TryAddItem_Internal(item);
}

UItemsObject* UCraftComponent::AddItem(UItemsObject* Item)
{
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		UItemsObject* NewItem = NewObject<UItemsObject>(GetOwner(), Item->GetClass());
		NewItem->World = GetWorld();
		NewItem->OwningCraftComponent = this;
		NewItem->AddedToCraftInventory(this);
		Items.Add(NewItem);
		NewItem->MarkDirtyForReplication();
	
		return NewItem;
	}

	return nullptr;
}

FItemAddResult UCraftComponent::TryAddItem_Internal(UItemsObject* Item)
{
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		ensure(Item->GetQuantity() == 1);
		AddItem(Item);
		return FItemAddResult::AddedAll(Item->GetQuantity());
	}
	return FItemAddResult::AddedNone(-1, LOCTEXT("ErrorMessage", ""));
}
#undef LOCTEXT_NAMESPACE