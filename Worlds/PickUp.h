// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PickUp.generated.h"

UCLASS()
class HOWITBEGAN_API APickUp : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APickUp();

	//Takes the item to represent and creates the pickup from it. Done on BeginPlay and when a player drops an item on the ground.
	void InitializePickup(const TSubclassOf<class UItemsObject> ItemClass, const int32 Quantity);

	//Align pickups rotation with ground rotation. 
	UFUNCTION(BlueprintImplementableEvent)
	void AlignWithGround();

	//This is used as a template to create the pickup when spawned in
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced)
	class UItemsObject* ItemTemplate;
protected:

	//The item that will be added to the inventory when this pickup is taken
	UPROPERTY(BlueprintReadWrite, ReplicatedUsing = OnRep_Item)
	class UItemsObject* Item;

	UFUNCTION()
	void OnRep_Item();

	//If some property on the item is modified, we bind this to OnItemModified and refresh the UI if the item gets modified.
	UFUNCTION()
	void OnItemModified();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags) override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif 

	//Called when a player takes the pickup
	UFUNCTION()
	void OnTakePickup(class APlayerCharacter* Taker);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components")
	class UStaticMeshComponent* PickupMesh;

	UPROPERTY(EditDefaultsOnly, Category = "Components")
	class UInteractionComponet* InteractionComponent;
};
