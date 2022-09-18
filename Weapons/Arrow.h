// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Arrow.generated.h"

UCLASS()
class HOWITBEGAN_API AArrow : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AArrow();

	//Takes the item to represent and creates the pickup from it. Done on BeginPlay and when a player drops an item on the ground.
	void InitializeArrow(const TSubclassOf<class UAmmoItem> AmmoClass, const int32 Quantity);

	//This is used as a template so we can fire and pick it up later
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced)
	class UAmmoItem* AmmoTemplate;

	/** Projectile movement component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement)
	class UProjectileMovementComponent* ProjectileMovement;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	/** Sphere collision component */
	UPROPERTY(VisibleDefaultsOnly, Category = Projectile)
	class USphereComponent* CollisionComp;

	/**Because pickups use a Blueprint base, we use a UPROPERTY to select it*/
	UPROPERTY(EditDefaultsOnly, Category = "Loot")
	TSubclassOf<class APickUp> PickupClass;


	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components")
	class UStaticMeshComponent* PickupMesh;

	UPROPERTY(EditAnywhere, Category = "Components")
	class USkeletalMeshComponent* ArrowMesh;

	UPROPERTY(EditDefaultsOnly, Category = "Components")
	class UInteractionComponet* InteractionComponent;

};
