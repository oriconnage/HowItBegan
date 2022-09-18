// Fill out your copyright notice in the Description page of Project Settings.


#include "itemspawn.h"
#include "../item/ItemsObject.h"
#include"PickUp.h"
#include <GameFramework/Actor.h>

Aitemspawn::Aitemspawn()
{
	PrimaryActorTick.bCanEverTick = false;
	bNetLoadOnClient = false;

	RespawnRange = FIntPoint(10, 30);
}
void Aitemspawn::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		SpawnItem();
	}
}

void Aitemspawn::SpawnItem()
{	

	if (HasAuthority() && LootTable)
	{
		TArray<FLootTableRow*> SpawnItems;
		LootTable->GetAllRows("", SpawnItems);

		const FLootTableRow* LootRow = SpawnItems[FMath::RandRange(0, SpawnItems.Num() - 1)];

		ensure(LootRow);

		float ProbabilityRoll = FMath::FRandRange(0.f, 1.f);

		while (ProbabilityRoll > LootRow->Probability)
		{
			LootRow = SpawnItems[FMath::RandRange(0, SpawnItems.Num() - 1)];
			ProbabilityRoll = FMath::FRandRange(0.f, 1.f);
		}

		if (LootRow && LootRow->Items.Num() && PickupClass)
		{
			float Angle = 0.f;

			for (auto& ItemClass : LootRow->Items)
			{
				if (ItemClass)
				{
					const FVector LocationOffset = FVector(FMath::Cos(Angle), FMath::Sin(Angle), 0.f) * 50.f;

					FActorSpawnParameters SpawnParams;
					SpawnParams.bNoFail = true;
					SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

					const int32 ItemQuantity = ItemClass->GetDefaultObject<UItemsObject>()->GetQuantity();

					FTransform SpawnTransform = GetActorTransform();
					SpawnTransform.AddToTranslation(LocationOffset);

					APickUp* Pickup = GetWorld()->SpawnActor<APickUp>(PickupClass, SpawnTransform, SpawnParams);
					Pickup->InitializePickup(ItemClass, ItemQuantity);
					Pickup->OnDestroyed.AddUniqueDynamic(this, &Aitemspawn::OnItemTaken);

					SpawnedPickups.Add(Pickup);

					Angle += (PI * 2.f) / LootRow->Items.Num();
				}
			}
		}
	}

}

void Aitemspawn::OnItemTaken(AActor* DestroyedActor)
{
	if (HasAuthority())
	{
		SpawnedPickups.Remove(DestroyedActor);

		//If all pickups were taken queue a respawn
		if (SpawnedPickups.Num() <= 0)
		{
			GetWorldTimerManager().SetTimer(TimerHandle_RespawnItem, this, &Aitemspawn::SpawnItem, FMath::RandRange(RespawnRange.GetMin(), RespawnRange.GetMax()), false);
		}
	}
}
