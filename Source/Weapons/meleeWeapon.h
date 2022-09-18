// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "MeleeDamage.h"
#include "GameFramework/Actor.h"
#include "meleeWeapon.generated.h"

class UAnimMontage;
class APlayerCharacter;
class UAudioComponent;
class UParticleSystemComponent;
class UCameraShake;
class UForceFeedbackEffect;
class USoundCue;

UENUM(BlueprintType)
enum class EMeleeState: uint8
{
	Idle,
	Attack,
	Block,
	Equipping
}; 


USTRUCT()
struct FMeleeAnim
{
	GENERATED_USTRUCT_BODY()

	/** animation played on pawn (1st person view) */
	UPROPERTY(EditDefaultsOnly, Category = Animation)
	UAnimMontage* Pawn1P;

	/** animation played on pawn (3rd person view) */
	UPROPERTY(EditDefaultsOnly, Category = Animation)
	UAnimMontage* Pawn3P;
};

USTRUCT(BlueprintType)
struct FHitConfiguration
{
	GENERATED_BODY()

	FHitConfiguration()
	{
		Distance = 150.f;
		Damage = 25.f;
		Radius = 0.f;
		DamageType = UMeleeDamage::StaticClass();
		ClientSideHitLeeway = 300.f;
		StaminaCost = 30;
		TimeBetweenAttack = 1.0f;
	}

	/**A map of bone -> damage amount. If the bone is a child of the given bone, it will use this damage amount.
	A value of 2 would mean double damage etc */
	UPROPERTY(EditDefaultsOnly, Category = "Trace Info")
	TMap<FName, float> BoneDamageModifiers;

	/**How far the hitscan traces for a hit*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Trace Info")
	float Distance;

	/**The amount of damage to deal when we hit a player with the hitscan*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Trace Info")
	float Damage;

	/**Optional trace radius. A value of zero is just a linetrace, anything higher is a spheretrace*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Trace Info")
	float Radius;

	/**Optional trace radius. A value of zero is just a linetrace, anything higher is a spheretrace*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Trace Info")
	float StaminaCost;
	/**client side hit leeway for BoundingBox check*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Trace Info")
	float ClientSideHitLeeway;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "WeaponStat")
	float TimeBetweenAttack;
	/** type of damage */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Trace Info")
	TSubclassOf<UDamageType> DamageType;

};
UCLASS()
class HOWITBEGAN_API AmeleeWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AmeleeWeapon();

	friend class APlayerCharacter;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;
	virtual void Destroyed() override;


	

	/** weapon mesh*/
	UPROPERTY(EditAnywhere, Category = Components)
	USkeletalMeshComponent* WeaponMesh;
	
	/** weapon is being equipped by owner pawn */
	virtual void OnEquip();

	/** weapon is now equipped by owner pawn */
	virtual void OnEquipFinished();

	/** weapon is holstered by owner pawn */
	virtual void OnUnEquip();

	/** check if it's currently equipped */
	bool IsEquipped() const;

	/** check if mesh is already attached */
	bool IsAttachedToPawn() const;

	/** FX for impact particles */
	UPROPERTY(EditDefaultsOnly, Category = Effects)
	UParticleSystem* ImpactParticles;


	/**Name of the socket to attach to the character on*/
	UPROPERTY(EditDefaultsOnly, Category = Effects)
	FName AttachSocket1P;
	UPROPERTY(EditDefaultsOnly, Category = Effects)
	FName AttachSocket3P;

	/** [local + server] start weapon fire */
	virtual void StartAttack();

	/** [local + server] stop weapon fire */
	virtual void StopAttack();

	
	virtual void StartBlock();
	virtual void StopBlock();
	
	bool CanAttack() const;
	bool CanBlock() const;
	
	//Current Weapon state 
	EMeleeState CurrentState;

	//Get the weapon state
	EMeleeState GetCurrentState()const { return CurrentState;}

	//The weapon item in the players inventory
	UPROPERTY(Replicated, BlueprintReadOnly, Transient)
	class UEquippableMeleeWeapon* Item;
	/** get weapon mesh (needs pawn owner to determine variant) */
	UFUNCTION(BlueprintPure, Category = "Weapon")
	USkeletalMeshComponent* GetWeaponMesh() const;

	/** pawn owner */
	UPROPERTY(Transient, ReplicatedUsing = OnRep_PawnOwner)
	class APlayerCharacter* PawnOwner;
	/** get pawn owner */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	class APlayerCharacter* GetPawnOwner() const;

	/** set the weapon's owning pawn */
	void SetPawnOwner(APlayerCharacter* Character);

	/**Line trace data. Will be used if projectile class is null*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Config)
	FHitConfiguration HitConfig;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	float GetEquipStartedTime() const;
	float GetEquipDuration()const;


protected:
	/** equip sound */
	UPROPERTY(EditDefaultsOnly, Category = Sound)
	USoundCue* EquipSound;

	/** equip animations */
	UPROPERTY(EditDefaultsOnly, Category = Animation)
	FMeleeAnim EquipAnim;

	/** Block sound */
	UPROPERTY(EditDefaultsOnly, Category = Sound)
	USoundCue* BlockSound;

	/** Block animations */
	UPROPERTY(EditDefaultsOnly, Category = Animation)
	FMeleeAnim BlockAnim;
	
	/** slash sound */
	UPROPERTY(EditDefaultsOnly, Category = Sound)
	USoundCue* SlashSound;

	UPROPERTY(EditDefaultsOnly, Category = Sound)
	USoundCue* FinshedSound;

	/** slash animations */
	UPROPERTY(EditDefaultsOnly, Category = Animation)
	FMeleeAnim slashAnim;

	/** firing audio */
	UPROPERTY(Transient)
	UAudioComponent* AttackAC;

	
	/** is weapon currently equipped? */
	uint32 bIsEquipped : 1;

	/** is slash animation playing? */
	uint32 bPlayingslashAnim : 1;

	
	/** is reload animation playing? */
	uint32 bPlayingBlockAnim : 1;

	/** is equip animation playing? */
	uint32 bPendingEquip : 1;

	// Is Player attacking ? 
	uint32 bWantToAttack : 1;
	
	//Checking if player re attacking
	uint32 bReAttacking : 1;

	
	// is Block Anim Playing?
		
	UPROPERTY(Transient, ReplicatedUsing = OnRep_Block)
	uint32 bPendingBlock:1;

	/** time of last successful weapon fire */
	float LastAttackTime;

	/** last time when this weapon was switched to */
	float EquipStartedTime;

	/** how much time weapon needs to be equipped */
	float EquipDuration;

	/** Adjustment to handle frame rate affecting actual timer interval. */
	UPROPERTY(Transient)
	float TimerIntervalAdjustment;

	UPROPERTY(Transient, ReplicatedUsing = OnRep_HitNotify)
	FVector HitNotify;

	UFUNCTION()
	void OnRep_HitNotify();

	void OnBurstStarted();
	void OnBurstFinished();
	
	/** burst counter, used for replicating fire events to remote clients */
	UPROPERTY(Transient, ReplicatedUsing = OnRep_BurstCounter)
	int32 BurstCounter;
	UFUNCTION()
	void OnRep_BurstCounter();

	UFUNCTION()
	void OnRep_Block();
	UFUNCTION(Reliable, Server, WithValidation)
	void ServerStartAttack();

	UFUNCTION(Reliable, Server, WithValidation)
	void ServerStopAttack();

	UFUNCTION(Reliable, Server, WithValidation)
	void ServerStartBlock();

	UFUNCTION(Reliable, Server, WithValidation)
	void ServerStopBlock();

	UFUNCTION()
	void OnRep_PawnOwner();

	/** [local] weapon specific fire implementation */
	virtual void meleeAttack();

	
	UFUNCTION(reliable, server, WithValidation)
	void ServerHandleAttack();

	


	/** Handle for efficient management of OnEquipFinished timer */
	FTimerHandle TimerHandle_OnEquipFinished;
	FTimerHandle TimerHandle_HandleAttack;

	/** play weapon sounds */
	UAudioComponent* PlayWeaponSound(USoundCue* Sound);

	/** play weapon animations */
	float PlayWeaponAnimation(const FMeleeAnim& Animation);

	/** stop playing weapon animations */
	void StopWeaponAnimation(const FMeleeAnim& Animation);

	/** spawn effects for impact */
	void SpawnImpactEffects(const FHitResult& Impact);

public:
	void SetWeaponState(EMeleeState state);

	/** handle damage */
	void DealDamage(const FHitResult& Impact, const FVector& ShootDir);

	/** find hit */
	FHitResult WeaponTrace(const FVector& StartTrace, const FVector& EndTrace) const;

	void ProcessHit(const FHitResult& Impact, const FVector& Origin, const FVector& ShootDir);

	void SimulateHit(const FVector& Origin);
	void SpawnImpactEffect(const FHitResult& impact);
	void SimulateMeleeAttack();
	void StopSimulatingMeleeAttack();
	/** server notified of hit from client to verify */
	UFUNCTION(Reliable, Server)
	void ServerNotifyHit(const FHitResult& Impact, FVector_NetQuantizeNormal ShootDir);

	/** server notified of miss to show trail FX */
	UFUNCTION(Unreliable, Server)
	void ServerNotifyMiss(FVector_NetQuantizeNormal ShootDir);
	
	void AttachMeshToPawn();

	/** determine current weapon state */
	void DetermineWeaponState();
	/** continue processing the instant hit, as if it has been confirmed by the server */
	void ProcessInstantHit_Confirmed(const FHitResult& Impact, const FVector& Origin, const FVector& ShootDir);

	/** [local + server] handle weapon fire */
	void HandleAttack();
};
