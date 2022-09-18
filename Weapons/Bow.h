// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MeleeDamage.h"
#include "Bow.generated.h"



class UAnimMontage;
class APlayerCharacter;
class UAudioComponent;
class UParticleSystemComponent;
class UCameraShake;
class UForceFeedbackEffect;
class USoundCue;

UENUM()
enum class EWeaponType
{
	Bow,
	CrossBow,
	Sword,
	Spear,
	null
};

UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	Idle,
	Firing,
	Reloading,
	Equipping
};

USTRUCT(BlueprintType)
struct FWeaponData
{
	GENERATED_USTRUCT_BODY()

		/** clip size */
		UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Ammo)
		int32 AmmoPerClip;

	/**The item that this weapon uses as ammo*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Ammo)
		TSubclassOf<class UAmmoItem> AmmoClass;

	/** time between two consecutive shots */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = WeaponStat)
		float TimeBetweenShots;

	/** defaults */
	FWeaponData()
	{
		AmmoPerClip = 20;
		TimeBetweenShots = 0.2f;
	}
};

USTRUCT()
struct FWeaponAnim
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
struct FHitScanConfiguration
{
	GENERATED_BODY()

		FHitScanConfiguration()
	{
		Distance = 10000.f;
		Damage = 25.f;
		Radius = 0.f;
		DamageType = UWeaponDamage::StaticClass();
		ClientSideHitLeeway = 300.f;
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

	/**client side hit leeway for BoundingBox check*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Trace Info")
		float ClientSideHitLeeway;

	/** type of damage */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Trace Info")
		TSubclassOf<UDamageType> DamageType;

};

UCLASS()
class HOWITBEGAN_API ABow : public AActor
{
	GENERATED_BODY()

		friend class APlayerCharacter;

public:
	// Sets default values for this actor's properties
	ABow();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void Destroyed() override;

protected:
	
	/** consume a bullet */
	void UseClipAmmo();

	/**consume ammo from the inventory */
	void ConsumeAmmo(const int32 Amount = 1);

	/**[server] return ammo to the inventory when the weapon is unequipped*/
	void ReturnAmmoToInventory();

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


	//////////////////////////////////////////////////////////////////////////
	// Input

	/** [local + server] start weapon fire */
	virtual void StartFire();

	/** [local + server] stop weapon fire */
	virtual void StopFire();

	/** [all] start weapon reload */
	virtual void StartReload(bool bFromReplication = false);

	/** [local + server] interrupt weapon reload */
	virtual void StopReload();

	/** [server] performs actual reload */
	virtual void ReloadWeapon();

	/** trigger reload from server */
	UFUNCTION(reliable, client)
	void ClientStartReload();

	bool CanFire() const;
	bool CanReload() const;

	UFUNCTION(BlueprintPure, Category = "Weapon")
		EWeaponState GetCurrentState() const;

	/** get current ammo amount (total) */
	UFUNCTION(BlueprintPure, Category = "Weapon")
		int32 GetCurrentAmmo() const;

	/** get current ammo amount (clip) */
	UFUNCTION(BlueprintPure, Category = "Weapon")
		int32 GetCurrentAmmoInClip() const;

	/** get clip size */
	int32 GetAmmoPerClip() const;

	/** get weapon mesh (needs pawn owner to determine variant) */
	UFUNCTION(BlueprintPure, Category = "Weapon")
		USkeletalMeshComponent* GetWeaponMesh() const;

	/** get pawn owner */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
		class APlayerCharacter* GetPawnOwner() const;

	/** set the weapon's owning pawn */
	void SetPawnOwner(APlayerCharacter* SurvivalCharacter);

	/** gets last time when this weapon was switched to */
	float GetEquipStartedTime() const;

	/** gets the duration of equipping weapon*/
	float GetEquipDuration() const;

protected:

	//The weapon item in the players inventory
	UPROPERTY(Replicated, BlueprintReadOnly, Transient)
		class UWeaponItem* Item;

	/** pawn owner */
	UPROPERTY(Transient, ReplicatedUsing = OnRep_PawnOwner)
		class APlayerCharacter* PawnOwner;

	/** weapon data */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Config)
		FWeaponData WeaponConfig;

	/**Line trace data. Will be used if projectile class is null*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Config)
		FHitScanConfiguration HitScanConfig;

public:

	/** weapon mesh*/
	UPROPERTY(EditAnywhere, Category = Components)
	USkeletalMeshComponent* WeaponMesh;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Weapon)
	TEnumAsByte<EWeaponType> weaponType;

	UFUNCTION(BlueprintCallable)
	bool isBow();
	UFUNCTION(BlueprintCallable)
	bool isSword();
	UFUNCTION(BlueprintCallable)
	bool isSpear();
	UFUNCTION(BlueprintCallable)
	bool isCrossbow();

	EWeaponType determineWeaponType();
protected:
	UPROPERTY(EditAnywhere, Category = "Components")
	uint32 NeedAmmo:1;

	EWeaponType thisWeaponType;
	/** Adjustment to handle frame rate affecting actual timer interval. */
	UPROPERTY(Transient)
		float TimerIntervalAdjustment;

	/** Whether to allow automatic weapons to catch up with shorter refire cycles */
	UPROPERTY(Config)
		bool bAllowAutomaticWeaponCatchup = true;

	/** firing audio (bLoopedFireSound set) */
	UPROPERTY(Transient)
		UAudioComponent* FireAC;

	/** name of bone/socket for muzzle in weapon mesh */
	UPROPERTY(EditDefaultsOnly, Category = Effects)
		FName MuzzleAttachPoint;

	/**Name of the socket to attach to the character on*/
	UPROPERTY(EditDefaultsOnly, Category = Effects)
		FName AttachSocket1P;

	/**Name of the socket to attach to the character on*/
	UPROPERTY(EditDefaultsOnly, Category = Effects)
		FName AttachSocket3P;

	/** FX for muzzle flash */
	UPROPERTY(EditDefaultsOnly, Category = Effects)
		UParticleSystem* MuzzleParticles;

	/** FX for impact particles */
	UPROPERTY(EditDefaultsOnly, Category = Effects)
		UParticleSystem* ImpactParticles;

	/** spawned component for muzzle FX */
	UPROPERTY(Transient)
		UParticleSystemComponent* MuzzlePSC;

	/** spawned component for second muzzle FX (Needed for split screen) */
	UPROPERTY(Transient)
		UParticleSystemComponent* MuzzlePSCSecondary;

	/** camera shake on firing */
	UPROPERTY(EditDefaultsOnly, Category = Effects)
		TSubclassOf<UCameraShake> FireCameraShake;

	//The time it takes to aim down sights, in seconds
	UPROPERTY(EditDefaultsOnly, Category = Weapon)
		float ADSTime;

	/**The amount of recoil to apply. We choose a random point from 0-1 on the curve and use it to drive recoil.
	This means designers get lots of control over the recoil pattern*/
	UPROPERTY(EditDefaultsOnly, Category = Recoil)
		class UCurveVector* RecoilCurve;

	//The speed at which the recoil bumps up per second
	UPROPERTY(EditDefaultsOnly, Category = Recoil)
		float RecoilSpeed;

	//The speed at which the recoil resets per second
	UPROPERTY(EditDefaultsOnly, Category = Recoil)
		float RecoilResetSpeed;

	/** force feedback effect to play when the weapon is fired */
	UPROPERTY(EditDefaultsOnly, Category = Effects)
		UForceFeedbackEffect* FireForceFeedback;

	/** single fire sound (bLoopedFireSound not set) */
	UPROPERTY(EditDefaultsOnly, Category = Sound)
		USoundCue* FireSound;

	/** looped fire sound (bLoopedFireSound set) */
	UPROPERTY(EditDefaultsOnly, Category = Sound)
		USoundCue* FireLoopSound;

	/** finished burst sound (bLoopedFireSound set) */
	UPROPERTY(EditDefaultsOnly, Category = Sound)
		USoundCue* FireFinishSound;

	/** out of ammo sound */
	UPROPERTY(EditDefaultsOnly, Category = Sound)
		USoundCue* OutOfAmmoSound;

	/** reload sound */
	UPROPERTY(EditDefaultsOnly, Category = Sound)
		USoundCue* ReloadSound;

	/** reload animations */
	UPROPERTY(EditDefaultsOnly, Category = Animation)
		FWeaponAnim ReloadAnim;

	/** equip sound */
	UPROPERTY(EditDefaultsOnly, Category = Sound)
		USoundCue* EquipSound;

	/** equip animations */
	UPROPERTY(EditDefaultsOnly, Category = Animation)
		FWeaponAnim EquipAnim;

	/** fire animations */
	UPROPERTY(EditDefaultsOnly, Category = Animation)
		FWeaponAnim FireAnim;

	/** fire animations */
	UPROPERTY(EditDefaultsOnly, Category = Animation)
		FWeaponAnim FireAimingAnim;

	/** is muzzle FX looped? */
	UPROPERTY(EditDefaultsOnly, Category = Effects)
		uint32 bLoopedMuzzleFX : 1;

	/** is fire sound looped? */
	UPROPERTY(EditDefaultsOnly, Category = Sound)
		uint32 bLoopedFireSound : 1;

	/** is fire animation looped? */
	UPROPERTY(EditDefaultsOnly, Category = Animation)
		uint32 bLoopedFireAnim : 1;

	/** is fire animation playing? */
	uint32 bPlayingFireAnim : 1;

	/** is weapon currently equipped? */
	uint32 bIsEquipped : 1;

	/** is weapon fire active? */
	uint32 bWantsToFire : 1;

	/** is reload animation playing? */
	UPROPERTY(Transient, ReplicatedUsing = OnRep_Reload)
		uint32 bPendingReload : 1;

	/** is equip animation playing? */
	uint32 bPendingEquip : 1;

	/** weapon is refiring */
	uint32 bRefiring;

	/** current weapon state */
	EWeaponState CurrentState;

	/** time of last successful weapon fire */
	float LastFireTime;

	/** last time when this weapon was switched to */
	float EquipStartedTime;

	/** how much time weapon needs to be equipped */
	float EquipDuration;

	UPROPERTY(Transient, ReplicatedUsing = OnRep_HitNotify)
		FVector HitNotify;

	UFUNCTION()
		void OnRep_HitNotify();

	/** current ammo - inside clip */
	UPROPERTY(Transient, Replicated)
		int32 CurrentAmmoInClip;

	/** burst counter, used for replicating fire events to remote clients */
	UPROPERTY(Transient, ReplicatedUsing = OnRep_BurstCounter)
		int32 BurstCounter;

	/** Handle for efficient management of OnEquipFinished timer */
	FTimerHandle TimerHandle_OnEquipFinished;

	/** Handle for efficient management of StopReload timer */
	FTimerHandle TimerHandle_StopReload;

	/** Handle for efficient management of ReloadWeapon timer */
	FTimerHandle TimerHandle_ReloadWeapon;

	/** Handle for efficient management of HandleFiring timer */
	FTimerHandle TimerHandle_HandleFiring;

	// Input - server side

	UFUNCTION(Reliable, Server, WithValidation)
		void ServerStartFire();

	UFUNCTION(Reliable, Server, WithValidation)
		void ServerStopFire();

	UFUNCTION(Reliable, Server, WithValidation)
		void ServerStartReload();

	UFUNCTION(Reliable, Server, WithValidation)
		void ServerStopReload();

	//////////////////////////////////////////////////////////////////////////
// Replication & effects
	UFUNCTION()
		void OnRep_PawnOwner();

	UFUNCTION()
		void OnRep_BurstCounter();

	UFUNCTION()
		void OnRep_Reload();

	/** Called in network play to do the cosmetic fx for firing */
	virtual void SimulateWeaponFire();

	/** Called in network play to stop cosmetic fx (e.g. for a looping shot). */
	virtual void StopSimulatingWeaponFire();

	//////////////////////////////////////////////////////////////////////////
// Weapon usage

	/** [local] weapon specific fire implementation */
	virtual void FireShot();

	/** [server] fire & update ammo */
	UFUNCTION(reliable, server, WithValidation)
		void ServerHandleFiring();

	/** [local + server] handle weapon refire, compensating for slack time if the timer can't sample fast enough */
	void HandleReFiring();

	/** [local + server] handle weapon fire */
	void HandleFiring();

	/** [local + server] firing started */
	virtual void OnBurstStarted();

	/** [local + server] firing finished */
	virtual void OnBurstFinished();

	/** update weapon state */
	void SetWeaponState(EWeaponState NewState);

	/** determine current weapon state */
	void DetermineWeaponState();

	/** attaches weapon mesh to pawn's mesh */
	void AttachMeshToPawn();


	//////////////////////////////////////////////////////////////////////////
	// Weapon usage helpers

	/** play weapon sounds */
	UAudioComponent* PlayWeaponSound(USoundCue* Sound);

	void PlayWeaponAnimation(const FWeaponAnim& Animation);

	void StopWeaponAnimation(const FWeaponAnim& Animation);
	UFUNCTION(reliable, server, WithValidation)
	/** play weapon animations */
	void Server_PlayWeaponAnimation(const FWeaponAnim& Animation);

	UFUNCTION(reliable, server, WithValidation)
	/** stop playing weapon animations */
	void Server_StopWeaponAnimation(const FWeaponAnim& Animation);

	/** Get the aim of the camera */
	FVector GetCameraAim() const;

	/** called in network play to do the cosmetic fx  */
	void SimulateInstantHit(const FVector& Origin);

	/** spawn effects for impact */
	void SpawnImpactEffects(const FHitResult& Impact);

public:

	float ThisAnimDuration;

	/** handle damage */
	void DealDamage(const FHitResult& Impact, const FVector& ShootDir);

	/** find hit */
	FHitResult WeaponTrace(const FVector& StartTrace, const FVector& EndTrace) const;

	void ProcessInstantHit(const FHitResult& Impact, const FVector& Origin, const FVector& ShootDir);

	/** server notified of hit from client to verify */
	UFUNCTION(Reliable, Server)
		void ServerNotifyHit(const FHitResult& Impact, FVector_NetQuantizeNormal ShootDir);

	/** server notified of miss to show trail FX */
	UFUNCTION(Unreliable, Server)
		void ServerNotifyMiss(FVector_NetQuantizeNormal ShootDir);

	/** continue processing the instant hit, as if it has been confirmed by the server */
	void ProcessInstantHit_Confirmed(const FHitResult& Impact, const FVector& Origin, const FVector& ShootDir);
};



