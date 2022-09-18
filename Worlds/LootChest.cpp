// Fill out your copyright notice in the Description page of Project Settings.


#include "LootChest.h"
#include "../Components/InteractionComponet.h"
#include "../Components/InventoryComponent.h"
#include "../Player/PlayerCharacter.h"
#include <Internationalization/Internationalization.h>
#include "itemspawn.h"

#define LLOCTEXT_NAMESPACE "LootChest"

ALootChest::ALootChest()
{
	SetReplicates(true);

	LootableMesh = CreateDefaultSubobject<UStaticMeshComponent>("LootContainerMesh");
	SetRootComponent(LootableMesh);

	LootInteraction = CreateDefaultSubobject<UInteractionComponet>("LootInteraction");
	LootInteraction->InteractableActionText = NSLOCTEXT("LootActorText","LootChest", "Loot");
	LootInteraction->InteractableNameText = NSLOCTEXT("LootActorName","LootChest" , "Chest");
	LootInteraction->SetupAttachment(GetRootComponent());

	Inventory = CreateDefaultSubobject<UInventoryComponent>("Inventory");
	Inventory->SetCapacity(20);
	Inventory->SetWeightCapacity(80.f);

	LootRolls = FIntPoint(2, 8);

}

// Called when the game starts or when spawned
void ALootChest::BeginPlay()
{
	Super::BeginPlay();
	
	LootInteraction->OnInteract.AddDynamic(this, &ALootChest::OnInteract);

	if (HasAuthority() && LootTable) {
		TArray<FLootTableRow*>SpawnItems;
		LootTable->GetAllRows("", SpawnItems);

		int32 Rolls = FMath::RandRange(LootRolls.GetMin(), LootRolls.GetMax());

		for (int32 i = 0; i < Rolls; ++i)
		{
			const FLootTableRow* LootRow = SpawnItems[FMath::RandRange(0, SpawnItems.Num() - 1)];

			ensure(LootRow);

			float ProbabilityRoll = FMath::FRandRange(0.f, 1.f);

			while (ProbabilityRoll > LootRow->Probability)
			{
				LootRow = SpawnItems[FMath::RandRange(0, SpawnItems.Num() - 1)];
				ProbabilityRoll = FMath::FRandRange(0.f, 1.f);
			}

			if (LootRow && LootRow->Items.Num())
			{
				for (auto& ItemClass : LootRow->Items)
				{
					if (ItemClass)
					{
						const int32 Quantity = Cast<UItemsObject>(ItemClass->GetDefaultObject())->GetQuantity();
						Inventory->TryAddItemFromClass(ItemClass, Quantity);
					}
				}
			}
		}
	}
}



void ALootChest::OnInteract(class APlayerCharacter* Character)
{
	if (Character)
	{
		Character->SetLootSource(Inventory);
	}
}

#undef LOCTEXT_NAMESPACE
