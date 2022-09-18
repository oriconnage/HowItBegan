// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "CharacterBaseClass.h"
#include "GameFramework/Character.h"
#include "../item/ItemsObject.h"
#include "PlayerCharacter.generated.h"

UENUM(BlueprintType)
enum class EOverlayState: uint8
{
	Bow,
	Crossbow,
	Sword,
	Torch,
	Spear,
	Axe,
	Default
};



USTRUCT()
struct FInteractionData
{
	GENERATED_BODY()

		FInteractionData()
	{
		ViewedInteractionComponent = nullptr;
		LastInteractionCheckTime = 0.f;
		bInteractHeld = false;
	}

	//The current interactable component we're viewing, if there is one
	UPROPERTY()
		class UInteractionComponet* ViewedInteractionComponent;

	//The time when we last checked for an interactable
	UPROPERTY()
		float LastInteractionCheckTime;

	//Whether the local player is holding the interact key
	UPROPERTY()
		bool bInteractHeld;

};


DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEquippedItemsChanged, const EEquippableSlot, Slot, const UEquippableItem*, Item);

UCLASS()
class HOWITBEGAN_API APlayerCharacter : public ACharacterBaseClass
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	APlayerCharacter();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class USpringArmComponent* SpringArmComponent;

	UPROPERTY(EditAnywhere, Category = "Components")
		class UCameraComponent* CameraComponent;


	//The mesh to have equipped if we dont have an item equipped - ie the bare skin meshes
	UPROPERTY(BlueprintReadOnly, Category = Mesh)
		TMap<EEquippableSlot, USkeletalMesh*> NakedMeshes;

	//The players body meshes.
	UPROPERTY(BlueprintReadOnly, Category = Mesh)
		TMap<EEquippableSlot, USkeletalMeshComponent*> PlayerMeshes;

	/**Our players inventory*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
		class UInventoryComponent* PlayerInventory;

	/**Interaction component used to allow other players to loot us when we have died*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
		class UInteractionComponet* LootPlayerInteraction;

	UPROPERTY(EditAnywhere, Category = "Components")
		class USkeletalMeshComponent* HelmetMesh;

	UPROPERTY(EditAnywhere, Category = "Components")
		class USkeletalMeshComponent* ChestMesh;

	UPROPERTY(EditAnywhere, Category = "Components")
		class USkeletalMeshComponent* ArmMesh;
	
	UPROPERTY(EditAnywhere, Category = "Components")
		class USkeletalMeshComponent* LegsMesh;

	UPROPERTY(EditAnywhere, Category = "Components")
		class USkeletalMeshComponent* SkirtMesh;

	UPROPERTY(EditAnywhere, Category = "Components")
		class USkeletalMeshComponent* CloakMesh;

	UPROPERTY(EditAnywhere, Category = "Components")
		class USkeletalMeshComponent* PauldronMesh;

	UPROPERTY(EditAnywhere, Category = "Components")
		class USkeletalMeshComponent* HandsMesh;

	UPROPERTY(EditAnywhere, Category = "Components")
		class USkeletalMeshComponent* BackpackMesh;


	UPROPERTY(EditAnywhere, Category = "Components")
		class USkeletalMeshComponent* ArrowMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
		class USceneCaptureComponent2D* InventoryPlayerCapture;

	/* Player Stats component*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerStats")
		class UPlayerStatsComponent* PLayerStatsComp;

	//Build component
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Building")
	class UBuilderComponent* BuildComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Crafting")
	class UCraftComponent* CraftComponent;

	UFUNCTION(BlueprintPure, Category = "State")
	EOverlayState GetCurrentState() const { return PlayerOverlay; };

	UPROPERTY(EditDefaultsOnly, Category = "Item")
	EOverlayState SetCurrentState;

	UPROPERTY(EditAnywhere, Category = "Components")
	bool isRespawning;
protected:
	// Called when the game starts or when spawned	
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void Tick(float DeltaTime) override;
	virtual void Restart() override;
	virtual float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
	virtual void SetActorHiddenInGame(bool bNewHidden) override;

	// player state 
	EOverlayState PlayerOverlay;

public:
	
	void Damage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
	{
		TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);
	};

	UFUNCTION(BlueprintCallable)
		void SetLootSource(class UInventoryComponent* NewLootSource);

	UFUNCTION(BlueprintPure, Category = "Looting")
		bool IsLooting() const;

	UFUNCTION(BlueprintPure, Category = "State")
	EOverlayState SetOverlayState(EOverlayState i);

protected:

	//Begin being looted by a player
	UFUNCTION()
	void BeginLootingPlayer(class APlayerCharacter* Character);

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable)
	void ServerSetLootSource(class UInventoryComponent* NewLootSource);

	/**The inventory that we are currently looting from. */
	UPROPERTY(ReplicatedUsing = OnRep_LootSource, BlueprintReadOnly)
		UInventoryComponent* LootSource;

	UFUNCTION()
		void OnLootSourceOwnerDestroyed(AActor* DestroyedActor);

	UFUNCTION()
		void OnRep_LootSource();

public:

	UFUNCTION(BlueprintCallable, Category = "Looting")
	void LootItem(class UItemsObject* ItemToGive);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerLootItem(class UItemsObject* ItemToLoot);

	//How often in seconds to check for an interactable object. Set this to zero if you want to check every tick.
	UPROPERTY(EditDefaultsOnly, Category = "Interaction")
	float InteractionCheckFrequency;

	//How far we'll trace when we check if the player is looking at an interactable object
	UPROPERTY(EditDefaultsOnly, Category = "Interaction")
		float InteractionCheckDistance;

	void PerformInteractionCheck();

	void CouldntFindInteractable();
	void FoundNewInteractable(UInteractionComponet* Interactable);

	void BeginInteract();
	void EndInteract();

	UFUNCTION(Server, Reliable, WithValidation)
		void ServerBeginInteract();

	UFUNCTION(Server, Reliable, WithValidation)
		void ServerEndInteract();

	void Interact();

	//Information about the current state of the players interaction
	UPROPERTY()
		FInteractionData InteractionData;

	//Helper function to make grabbing interactable faster
	FORCEINLINE class UInteractionComponet* GetInteractable() const { return InteractionData.ViewedInteractionComponent; }

	FTimerHandle TimerHandle_Interact;

public:

	//True if we're interacting with an item that has an interaction time (for example a lamp that takes 2 seconds to turn on)
	bool IsInteracting() const;

	//Get the time till we interact with the current interactable
	float GetRemainingInteractTime() const;


	//Items

/**[Server] Use an item from our inventory.*/
	UFUNCTION(BlueprintCallable, Category = "Items")
		void UseItem(class UItemsObject* Item);

	UFUNCTION(Server, Reliable, WithValidation)
		void ServerUseItem(class UItemsObject* Item);

	/**[Server] Drop an item*/
	UFUNCTION(BlueprintCallable, Category = "Items")
		void DropItem(class UItemsObject* Item, const int32 Quantity);

	UFUNCTION(Server, Reliable, WithValidation)
		void ServerDropItem(class UItemsObject* Item, const int32 Quantity);

	/**[Server] Drop an item*/
	UFUNCTION(BlueprintCallable, Category = "Items")
	void CraftItem(class UItemsObject* Item);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerCraftItem(class UItemsObject* Item);
	
	UFUNCTION()
		void ItemAddedToInventory(class UItemsObject* Item);

	UFUNCTION()
		void ItemRemovedFromInventory(class UItemsObject* Item);

	/**needed this because the pickups use a blueprint base class*/
	UPROPERTY(EditDefaultsOnly, Category = "Item")
		TSubclassOf<class APickUp> PickupClass;

public:

	/**Handle equipping an equippable item*/
	bool EquipItem(class UEquippableItem* Item);
	bool UnEquipItem(class UEquippableItem* Item);

	/**These should never be called directly - UGearItem and UWeaponItem call these on top of EquipItem*/
	void EquipGear(class UGearItem* Gear);
	void UnEquipGear(const EEquippableSlot Slot);
	void EquipWeapon(class UWeaponItem* WeaponItem);
	void UnEquipWeapon();
	void EquipMeleeWeapon(class UEquippableMeleeWeapon* WeaponItem);
	void UnEquipMeleeWeapon();

	
	/**Called to update the inventory*/
	UPROPERTY(BlueprintAssignable, Category = "Items")
		FOnEquippedItemsChanged OnEquippedItemsChanged;

	/**Return the skeletal mesh component for a given slot*/
	class USkeletalMeshComponent* GetSlotSkeletalMeshComponent(const EEquippableSlot Slot);

	UFUNCTION(BlueprintPure)
	FORCEINLINE TMap<EEquippableSlot, UEquippableItem*> GetEquippedItems() const { return EquippedItems; }

	UFUNCTION(BlueprintCallable, Category = "Weapons")
	FORCEINLINE class ABow* GetEquippedWeapon() const { return RangedWeapon; }

	UFUNCTION(BlueprintCallable, Category = "Melee Weapons")
	FORCEINLINE class AMeleeWeaponClass* GetEquippedMeleeWeapon() const { return MeleeWeapon; }
protected:
	//Allows for efficient access of equipped items
	UPROPERTY(VisibleAnywhere, Category = "Items")
	TMap<EEquippableSlot, UEquippableItem*> EquippedItems;
	
	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_EquippedWeapon)
	class ABow* RangedWeapon;

	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_EquippedWeapon)
	class AMeleeWeaponClass* MeleeWeapon;

	UFUNCTION()
	void OnRep_EquippedWeapon();

	void StartFire();
	void StopFire();

	void BeginMeleeAttack();

	UFUNCTION(Server, Reliable)
		void ServerProcessMeleeHit(const FHitResult& MeleeHit);

	UFUNCTION(NetMulticast, Unreliable)
		void MulticastPlayMeleeFX();

	UPROPERTY()
		float LastMeleeAttackTime;

	UPROPERTY(EditDefaultsOnly, Category = Melee)
		float MeleeAttackDistance;

	UPROPERTY(EditDefaultsOnly, Category = Melee)
		float MeleeAttackDamage;

	UPROPERTY(EditDefaultsOnly, Category = Melee)
		class UAnimMontage* MeleeAttackMontage;

	//Called when killed by the player, or killed by something else like the environment
	void OnOtherDeath(struct FDamageEvent const& DamageEvent, const AActor* DamageCauser);
	void KilledByPlayer(struct FDamageEvent const& DamageEvent, class APlayerCharacter* Character, const AActor* DamageCauser);

	UPROPERTY(ReplicatedUsing = OnRep_Killer)
	class APlayerCharacter* Killer;

	UFUNCTION()
	void OnRep_Killer();

	UFUNCTION(BlueprintImplementableEvent)
	void OnDeath();

public:

	UPROPERTY(EditDefaultsOnly, Category = Movement)
	float SprintSpeed;

	UPROPERTY()
	float WalkSpeed;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = Movement)
	bool bSprinting;

	bool CanSprint() const;

	//[local] start and stop sprinting functions
	void StartSprinting();
	void StopSprinting();

	/**[server + local] set sprinting*/
	void SetSprinting(const bool bNewSprinting);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerSetSprinting(const bool bNewSprinting);

	void StartCrouching();
	void StopCrouching();

	void MoveForward(float Val);
	void MoveRight(float Val);

	void LookUp(float Val);
	void Turn(float Val);

protected:

	bool CanAim() const;

	void StartAiming();
	void StopAiming();

	//Aiming
	void SetAiming(const bool bNewAiming);

	UFUNCTION(Server, Reliable)
		void ServerSetAiming(const bool bNewAiming);

	UPROPERTY(Transient, Replicated)
		bool bIsAiming;

public:

	void StartReload();

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BlueprintPure)
		FORCEINLINE bool IsAlive() const { return Killer == nullptr; };

	UFUNCTION(BlueprintPure, Category = "Weapons")
		FORCEINLINE bool IsAiming() const { return bIsAiming; }
};