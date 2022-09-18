// Fill out your copyright notice in the Description page of Project Settings.


#include "ItemsObject.h"
#include "Net/UnrealNetwork.h"
#include "../Components/InventoryComponent.h"
#include "HowItBegan/Player/PlayerCharacter.h"
#include "HowItBegan/Player/MyPlayerController.h"

#define LOCTEXT_NAMESPACE "Item"

void UItemsObject::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UItemsObject, Quantity);
}

bool UItemsObject::IsSupportedForNetworking() const
{
	return true;
}

UWorld* UItemsObject::GetWorld() const
{
	return World;
}

#if WITH_EDITOR
void UItemsObject::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	FName ChangedPropertyName = PropertyChangedEvent.Property ? PropertyChangedEvent.Property->GetFName() : NAME_None;

	//UPROPERTY clamping doesn't support using a variable to clamp so we do in here instead
	if (ChangedPropertyName == GET_MEMBER_NAME_CHECKED(UItemsObject, Quantity))
	{
		Quantity = FMath::Clamp(Quantity, 1, bStackable ? MaxStackSize : 1);
	}
	else if (ChangedPropertyName == GET_MEMBER_NAME_CHECKED(UItemsObject, bStackable))
	{
		if (!bStackable)
		{
			Quantity = 1;
		}
	}

}
#endif

UItemsObject::UItemsObject()
{
	DisplayName = LOCTEXT("ItemName", "Item");
	UseActionText = LOCTEXT("ItemUseActionText", "Use");
	Weight = 0.f;
	bStackable = true;
	bIsCraftable = false;
	Quantity = 1;
	MaxStackSize = 2;
	RepKey = 0;

}

void UItemsObject::OnRep_Quantity()
{
	OnItemModified.Broadcast();
}

void UItemsObject::SetQuantity(const int32 NewQuantity)
{
	if (NewQuantity != Quantity)
	{
		Quantity = FMath::Clamp(NewQuantity, 0, bStackable ? MaxStackSize : 1);
		MarkDirtyForReplication();
	}
}

bool UItemsObject::ShouldShowInInventory() const
{
	return true;
}


void UItemsObject::Use(APlayerCharacter* Character)
{

}

void UItemsObject::Craft(APlayerCharacter* Character)
{
	if (Character)
	{
		if (!bIsCraftable)
		{
		//	GEngine->AddOnScreenDebugMessage(-1, 15, FColor::Green, TEXT("Not craftable"));
			return;
		}
			if (UInventoryComponent* Inventory = Character->PlayerInventory)
			{
				if (Inventory->HasItem(MaterialItems.MaterialClass, MaterialItems.Quantiy))
				{
					const FItemAddResult AddResult = Inventory->TryAddItem(this);
				//	GEngine->AddOnScreenDebugMessage(-1, 15, FColor::Green, TEXT("Crafting"));
					
					if (AddResult.AmountGiven < this->GetQuantity())
					{
						this->SetQuantity(this->GetQuantity() - AddResult.AmountGiven);
						Materialitem = MaterialItems.MaterialClass->GetDefaultObject<UItemsObject>();
						Inventory->ConsumeItem(Inventory->FindItemByClass(MaterialItems.MaterialClass), MaterialItems.Quantiy);
					}
					if (!AddResult.ErrorText.IsEmpty())
					{
						if (AMyPlayerController* PC = Cast<AMyPlayerController>(Character->GetController()))
						{
							PC->ClientShowNotification(AddResult.ErrorText);
						}
					}
				}
			}
	}
}

void UItemsObject::AddedToInventory(UInventoryComponent* Inventory)
{

}

void UItemsObject::AddedToCraftInventory(UCraftComponent* Inventory)
{
	
}

void UItemsObject::MarkDirtyForReplication()
{
	//Mark this object for replication
	++RepKey;

	//Mark the array for replication
	if (OwningInventory)
	{
		++OwningInventory->ReplicatedItemsKey;
	}
}
#undef LOCTEXT_NAMESPACE