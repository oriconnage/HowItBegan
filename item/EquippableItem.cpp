// Fill out your copyright notice in the Description page of Project Settings.


#include "EquippableItem.h"
#include "Net/UnrealNetwork.h"
#include "../Components/InventoryComponent.h"
#include"../Player/PlayerCharacter.h"


#define LOCTEXT_NAMESPACE "EquippableItem"
UEquippableItem::UEquippableItem()
{
	bStackable = false;
	bEquipped = false;
	UseActionText = LOCTEXT("EquipText", "Equip");
}
void UEquippableItem::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UEquippableItem, bEquipped);
}

void UEquippableItem::Use(class APlayerCharacter* Character)
{
	if (Character && Character->HasAuthority())
	{
		if (Character->GetEquippedItems().Contains(Slot) && !bEquipped)
		{
			UEquippableItem* AlreadyEquippedItem = *Character->GetEquippedItems().Find(Slot);

			AlreadyEquippedItem->SetEquipped(false);
		}

		SetEquipped(!IsEquipped());
	}
}

bool UEquippableItem::Equip(class APlayerCharacter* Character)
{
	if (Character)
	{
		return Character->EquipItem(this);
	}
	return false;
}

bool UEquippableItem::UnEquip(class APlayerCharacter* Character)
{
	if (Character)
	{
		return Character->UnEquipItem(this);
	}
	return false;
}

bool UEquippableItem::ShouldShowInInventory() const
{
	return !bEquipped;
}

void UEquippableItem::AddedToInventory(class UInventoryComponent* Inventory)
{
	//If the player looted an item don't equip it
	if (APlayerCharacter* Character = Cast<APlayerCharacter>(Inventory->GetOwner()))
	{
		if (Character && !Character->IsLooting())
		{
			/**If we take an equippable, and don't have an item equipped at its slot, then auto equip it*/
			if (!Character->GetEquippedItems().Contains(Slot))
			{
				SetEquipped(true);
			}
		}
	}
}

void UEquippableItem::SetEquipped(bool bNewEquipped)
{
	bEquipped = bNewEquipped;
	EquipStatusChanged();
	MarkDirtyForReplication();
}

void UEquippableItem::EquipStatusChanged()
{
	if (APlayerCharacter* Character = Cast<APlayerCharacter>(GetOuter()))
	{
		UseActionText = bEquipped ? LOCTEXT("UnequipText", "Unequip") : LOCTEXT("EquipText", "Equip");

		if (bEquipped)
		{
			Equip(Character);
		}
		else
		{
			UnEquip(Character);
		}
	}

	//Tell ui to update
	OnItemModified.Broadcast();
}

#undef LOCTEXT_NAMESPACE 