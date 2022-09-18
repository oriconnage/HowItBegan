// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/TargetPoint.h"
#include <Engine/DataTable.h>
#include "itemspawn.generated.h"


USTRUCT(BlueprintType)
struct FLootTableRow : public FTableRowBase
{
	GENERATED_BODY()

public:

	//The item(s) to spawn
	UPROPERTY(EditDefaultsOnly, Category = "Loot")
	TArray<TSubclassOf<class UItemsObject>> Items;

	//The percentage chance of spawning this item if we hit it on the roll
	UPROPERTY(EditDefaultsOnly, Category = "Loot", meta = (ClampMin = 0.001, ClampMax = 1.0))
	float Probability = 1.f;

};


UCLASS(ClassGroup = (Items), Blueprintable, Abstract)
class HOWITBEGAN_API Aitemspawn : public ATargetPoint
{
	GENERATED_BODY()
	
public:
	// Sets default values for this actor's properties
	Aitemspawn();

	UPROPERTY(EditAnywhere, Category = "Loot")
	class UDataTable* LootTable;

	UPROPERTY(EditDefaultsOnly, Category = "Loot")
	TSubclassOf<class APickUp> PickupClass;

// Item respawn time range
	UPROPERTY(EditDefaultsOnly, Category = "Loot")
	FIntPoint RespawnRange;

protected:

	FTimerHandle TimerHandle_RespawnItem;

	UPROPERTY()
	TArray<AActor*> SpawnedPickups;

	virtual void BeginPlay() override;

	UFUNCTION()
	void SpawnItem();

	//This is bound to the item being destroyed, so we can queue up another item to be spawned in
	UFUNCTION()
	void OnItemTaken(AActor* DestroyedActor);
};
