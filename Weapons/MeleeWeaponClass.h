// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MeleeDamage.h"
#include "GameFramework/Actor.h"
#include "MeleeWeaponClass.generated.h"


class USoundCue;
UENUM(BlueprintType)
enum class EMeleeWeaponState : uint8
{
	Idle,
	Attack,
	Equipped
};

USTRUCT(BlueprintType)
struct FMeleeWeaponConfig
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	float LastMeleeAttackTime;

	UPROPERTY(EditDefaultsOnly, Category = Melee)
	float MeleeAttackDistance;

	UPROPERTY(EditDefaultsOnly, Category = Melee)
	float MeleeAttackDamage;

	UPROPERTY(EditDefaultsOnly, Category = "Trace Info")
	TMap<FName, float> BoneDamageModifiers;

	UPROPERTY(EditDefaultsOnly, Category = Melee)
	TSubclassOf<class UDamageType> MeleeDamageType;

	UPROPERTY()
	float StaminaCost;

	FMeleeWeaponConfig() {
		LastMeleeAttackTime = 0;
		MeleeAttackDistance = 150.f;
		MeleeAttackDamage = 20;
		MeleeDamageType = UWeaponDamage::StaticClass();
		StaminaCost = 20;
	}
};

USTRUCT()
struct FMeleeWeaponAnim
{
	GENERATED_USTRUCT_BODY()

		/** animation played on pawn (1st person view) */
		UPROPERTY(EditDefaultsOnly, Category = Animation)
		UAnimMontage* Pawn1P;

	/** animation played on pawn (3rd person view) */
	UPROPERTY(EditDefaultsOnly, Category = Animation)
		UAnimMontage* Pawn3P;
};

UCLASS()
class HOWITBEGAN_API AMeleeWeaponClass : public AActor
{
	GENERATED_BODY()

	friend class APlayerCharacter;
public:	
	// Sets default values for this actor's properties
	AMeleeWeaponClass();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;
	virtual void Destroyed() override;
	virtual void PostInitializeComponents() override;
protected:
	
	/** weapon is being equipped by owner pawn */
	virtual void OnEquip();

	/** weapon is now equipped by owner pawn */
	virtual void OnEquipFinished();

	/** weapon is holstered by owner pawn */
	virtual void OnUnEquip();

	/** check if it's currently equipped */
	bool IsEquipped() const;

	/** last time when this weapon was switched to */
	float EquipStartedTime;

	/** how much time weapon needs to be equipped */
	float EquipDuration;

	/** check if mesh is already attached */
	bool IsAttachedToPawn() const;

	/** attaches weapon mesh to pawn's mesh */
	void AttachMeshToPawn();

	// Check if player has Stamina to perform a certain attack
	bool HasStaminaToAttack() const;

	virtual void StartAttack();
	UFUNCTION(Reliable, Server, WithValidation)
	void ServerStartAttack();

	//Checks the player can attack
	bool CanAttack()const;

	void DetermineWeaponState();


	/** get weapon mesh (needs pawn owner to determine variant) */
	UFUNCTION(BlueprintPure, Category = "Weapon")
		USkeletalMeshComponent* GetWeaponMesh() const;

	/** get pawn owner */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
		class APlayerCharacter* GetPawnOwner() const;

	/** set the weapon's owning pawn */
	void SetPawnOwner(APlayerCharacter* Character);

	
	EMeleeWeaponState CurrentState;

public:
	//The weapon item in the players inventory
	UPROPERTY(Replicated, BlueprintReadOnly, Transient)
	class UEquippableMeleeWeapon* Item;

	/** pawn owner */
	UPROPERTY(Transient, ReplicatedUsing = OnRep_PawnOwner)
	class APlayerCharacter* PawnOwner;

	/** weapon data */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Config)
	FMeleeWeaponConfig WeaponConfig;

	/** weapon mesh*/
	UPROPERTY(EditAnywhere, Category = Components)
	USkeletalMeshComponent* WeaponMesh;

	/**Name of the socket to attach to the character on*/
	UPROPERTY(EditDefaultsOnly, Category = Effects)
	FName AttachSocket1P;

	UPROPERTY(EditDefaultsOnly, Category = Effects)
	FName AttachSocket3P;
protected:

	/** equip sound */
	UPROPERTY(EditDefaultsOnly, Category = Sound)
	USoundCue* EquipSound;
	/** Attack sound  */
	UPROPERTY(EditDefaultsOnly, Category = Sound)
	USoundCue* AttackSound;

	/** Attack animations */
	UPROPERTY(EditDefaultsOnly, Category = Animation)
	FMeleeWeaponAnim AttackAnim;

	/** is Attack animation playing? */
	uint32 bPlayingAttackAnim : 1;

	uint32 bIsEquipped : 1;

	uint32 bIsPendingEquip : 1;
	// Is player attacking
	uint32 bIsAttacking : 1;


	virtual void Attack();


	UPROPERTY(Transient, ReplicatedUsing = OnRep_HitNotify)
	FVector HitNotify;

	UFUNCTION()
	void OnRep_HitNotify();

	// Replication & effects
	UFUNCTION()
	void OnRep_PawnOwner();

	/** [server] Attacks */
	//UFUNCTION(reliable, server, WithValidation)
	//void ServerHandleAttack();


	/** play weapon sounds */
	UAudioComponent* PlayWeaponSound(USoundCue* Sound);

	/** play weapon animations */
	float PlayWeaponAnimation(const FMeleeWeaponAnim& Animation);

	public:
		/** handle damage */
	void DealDamage(const FHitResult& Impact, const FVector& ShootDir);

		/** find hit */
	FHitResult Trace(const FVector& StartTrace, const FVector& EndTrace) const;

	void processHit(const FHitResult& Impact, const FVector& start, const FVector& dir);

		/** server notified of hit from client to verify */
	UFUNCTION(Reliable, Server)
	void ServerNotifyHit(const FHitResult& Impact, FVector_NetQuantizeNormal ShootDir);

		/** server notified of miss to show trail FX */
	UFUNCTION(Unreliable, Server)
	void ServerNotifyMiss(FVector_NetQuantizeNormal dir);

		/** continue processing the instant hit, as if it has been confirmed by the server */
	void processHit_Confirmed(const FHitResult& Impact, const FVector& Origin, const FVector& end);
};

