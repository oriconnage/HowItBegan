// Fill out your copyright notice in the Description page of Project Settings.


#include "Bow.h"
#include "Net/UnrealNetwork.h"
#include "Components/SkeletalMeshComponent.h"
#include "../Components/InventoryComponent.h"
#include "../Player/MyPlayerController.h"
#include "../Player/PlayerCharacter.h"
#include <GameFramework/Actor.h>
#include "../item/AmmoItem.h"
#include <Engine/EngineTypes.h>
#include "Arrow.h"
#include "../HelperFile.h"
#include "Sound/SoundCue.h"
#include "Curves/CurveVector.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "../HowitBegan.h"
#include "Particles/ParticleSystemComponent.h"

#define COLISION_WEAPON ECC_GameTraceChannel1

static const FName NAME_MuzzleSocket("Muzzle");
// Sets default values
ABow::ABow()
{
	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->bReceivesDecals = false;
	WeaponMesh->SetCollisionObjectType(ECC_WorldDynamic);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	RootComponent = WeaponMesh;

	bLoopedMuzzleFX = false;
	bLoopedFireAnim = false;
	bPlayingFireAnim = false;
	bIsEquipped = false;
	bWantsToFire = false;
	bPendingReload = false;
	bPendingEquip = false;
	CurrentState = EWeaponState::Idle;
	AttachSocket1P = FName("GripPoint");
	AttachSocket3P = FName("GripPoint");

	CurrentAmmoInClip = 0;
	BurstCounter = 0;
	LastFireTime = 0.0f;

	ADSTime = 0.5f;
	RecoilResetSpeed = 5.f;
	RecoilSpeed = 10.f;

	NeedAmmo = true;
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PrePhysics;
	bReplicates = true;
	bNetUseOwnerRelevancy = true;
}

void ABow::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABow, PawnOwner);

	//DOREPLIFETIME_CONDITION(ABow, bPlayingFireAnim, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(ABow, CurrentAmmoInClip, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(ABow, BurstCounter, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(ABow, bPendingReload, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(ABow, Item, COND_InitialOnly);
}

void ABow::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

void ABow::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		PawnOwner = Cast<APlayerCharacter>(GetOwner());
	}
}


void ABow::Destroyed()
{
	Super::Destroyed();

	StopSimulatingWeaponFire();
}

void ABow::UseClipAmmo()
{
	if (HasAuthority() && NeedAmmo)
	{
		--CurrentAmmoInClip;
	}
}

void ABow::ConsumeAmmo(const int32 Amount)
{
	if (HasAuthority() && PawnOwner && NeedAmmo)
	{
		if (UInventoryComponent* Inventory = PawnOwner->PlayerInventory)
		{
			if (UItemsObject* AmmoItem = Inventory->FindItemByClass(WeaponConfig.AmmoClass))
			{
				Inventory->ConsumeItem(AmmoItem, Amount);
			}
		}
	}
}

void ABow::ReturnAmmoToInventory()
{
	//When the weapon is unequipped, try return the players ammo to their inventory
	if (HasAuthority())
	{
		if (PawnOwner && NeedAmmo && CurrentAmmoInClip > 0)
		{
			if (UInventoryComponent* Inventory = PawnOwner->PlayerInventory)
			{
				Inventory->TryAddItemFromClass(WeaponConfig.AmmoClass, CurrentAmmoInClip);
			}
		}
	}
}


void ABow::OnEquip()
{
	AttachMeshToPawn();

	bPendingEquip = true;
	DetermineWeaponState();

	OnEquipFinished();


	if (PawnOwner && PawnOwner->IsLocallyControlled())
	{
		PlayWeaponSound(EquipSound);
	}
}

void ABow::OnEquipFinished()
{
	AttachMeshToPawn();

	bIsEquipped = true;
	bPendingEquip = false;

	// Determine the state so that the can reload checks will work
	DetermineWeaponState();

	if (PawnOwner)
	{
		// try to reload empty clip
		if (PawnOwner->IsLocallyControlled() &&
			CanReload())
		{
			StartReload();
		}
	}
}


void ABow::OnUnEquip()
{
	bIsEquipped = false;
	StopFire();

	if (bPendingReload)
	{
		StopWeaponAnimation(ReloadAnim);
		bPendingReload = false;

		GetWorldTimerManager().ClearTimer(TimerHandle_StopReload);
		GetWorldTimerManager().ClearTimer(TimerHandle_ReloadWeapon);
	}

	if (bPendingEquip)
	{
		StopWeaponAnimation(EquipAnim);
		bPendingEquip = false;

		GetWorldTimerManager().ClearTimer(TimerHandle_OnEquipFinished);
	}

	ReturnAmmoToInventory();
	DetermineWeaponState();
}

bool ABow::IsEquipped() const
{
	return bIsEquipped;
}

bool ABow::IsAttachedToPawn() const
{
	return bIsEquipped || bPendingEquip;
}


void ABow::StartFire()
{
	if (!HasAuthority())
	{
		ServerStartFire();
	}

	if (!bWantsToFire)
	{
		bWantsToFire = true;
		DetermineWeaponState();
	}
}

void ABow::StopFire()
{
	if (!HasAuthority() && PawnOwner && PawnOwner->IsLocallyControlled())
	{
		ServerStopFire();
	}

	if (bWantsToFire)
	{
		bWantsToFire = false;
		DetermineWeaponState();
	}
}

void ABow::StartReload(bool bFromReplication /*= false*/)
{
	if (!bFromReplication && !HasAuthority())
	{
		ServerStartReload();
	}

	if (bFromReplication || CanReload())
	{
		bPendingReload = true;
		DetermineWeaponState();
		PlayWeaponAnimation(ReloadAnim);
		float AnimDuration = ThisAnimDuration; 
		if (AnimDuration <= 0.0f)
		{
			AnimDuration = .5f;
		}

		GetWorldTimerManager().SetTimer(TimerHandle_StopReload, this, &ABow::StopReload, AnimDuration, false);
		if (HasAuthority())
		{
			GetWorldTimerManager().SetTimer(TimerHandle_ReloadWeapon, this, &ABow::ReloadWeapon, FMath::Max(0.1f, AnimDuration - 0.1f), false);
		}

		if (PawnOwner && PawnOwner->IsLocallyControlled())
		{
			PlayWeaponSound(ReloadSound);
		}
	}
}

void ABow::StopReload()
{
	if (CurrentState == EWeaponState::Reloading)
	{
		bPendingReload = false;
		DetermineWeaponState();
		StopWeaponAnimation(ReloadAnim);
	}
}

void ABow::ReloadWeapon()
{
	const int32 ClipDelta = FMath::Min(WeaponConfig.AmmoPerClip - CurrentAmmoInClip, GetCurrentAmmo());

	if (ClipDelta > 0)
	{
		CurrentAmmoInClip += ClipDelta;
		ConsumeAmmo(ClipDelta);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Didnt have enough ammo for a reload"));
	}
}

bool ABow::CanFire() const
{
	bool bCanFire = PawnOwner != nullptr;
	bool bStateOKToFire = ((CurrentState == EWeaponState::Idle) || (CurrentState == EWeaponState::Firing));
	return ((bCanFire == true) && (bStateOKToFire == true) && (bPendingReload == false));
}

bool ABow::CanReload() const
{
	bool bWeaponNeedammo = NeedAmmo == true;
	bool bCanReload = PawnOwner != nullptr;
	bool bGotAmmo = (CurrentAmmoInClip < WeaponConfig.AmmoPerClip) && (GetCurrentAmmo() > 0);
	bool bStateOKToReload = ((CurrentState == EWeaponState::Idle) || (CurrentState == EWeaponState::Firing));
	return ((bCanReload == true) && (bGotAmmo == true) && (bStateOKToReload == true)&& bWeaponNeedammo == true);
}

EWeaponState ABow::GetCurrentState() const
{
	return CurrentState;
}

int32 ABow::GetCurrentAmmo() const
{
	if (PawnOwner)
	{
		if (UInventoryComponent* Inventory = PawnOwner->PlayerInventory)
		{
			if (UItemsObject* Ammo = Inventory->FindItemByClass(WeaponConfig.AmmoClass))
			{
				return Ammo->GetQuantity();
			}
		}
	}

	return 0;
}

int32 ABow::GetCurrentAmmoInClip() const
{
	return CurrentAmmoInClip;
}

int32 ABow::GetAmmoPerClip() const
{
	return WeaponConfig.AmmoPerClip;
}

USkeletalMeshComponent* ABow::GetWeaponMesh() const
{
	return WeaponMesh;
}

class APlayerCharacter* ABow::GetPawnOwner() const
{
	return PawnOwner;
}

void ABow::SetPawnOwner(APlayerCharacter* PlayerCharacter)
{
	if (PawnOwner != PlayerCharacter)
	{
		SetInstigator(PlayerCharacter);
		PawnOwner = PlayerCharacter;
		// net owner for RPC APlayerCharacter
		SetOwner(PlayerCharacter);
	}
}

float ABow::GetEquipStartedTime() const
{
	return EquipStartedTime;
}

float ABow::GetEquipDuration() const
{
	return EquipDuration;
}

bool ABow::isBow( )
{
	return (weaponType.GetValue() == EWeaponType::Bow) ? true : false;
}

bool ABow::isSword()
{
	return (weaponType.GetValue() == EWeaponType::Sword) ? true : false;
}

bool ABow::isSpear()
{
	return (weaponType.GetValue() == EWeaponType::Spear) ? true : false;
}

bool ABow::isCrossbow()
{
	return (weaponType.GetValue() == EWeaponType::CrossBow) ? true : false;
}

EWeaponType ABow::determineWeaponType()
{
	return weaponType.GetValue();
}

void ABow::ClientStartReload_Implementation()
{
	StartReload();
}

void ABow::ServerStartFire_Implementation()
{
	StartFire();
}

bool ABow::ServerStartFire_Validate()
{
	return true;
}

void ABow::ServerStopFire_Implementation()
{
	StopFire();
}

bool ABow::ServerStopFire_Validate()
{
	return true;
}

void ABow::ServerStartReload_Implementation()
{
	StartReload();
}

bool ABow::ServerStartReload_Validate()
{
	return true;
}

void ABow::ServerStopReload_Implementation()
{
	StopReload();
}

bool ABow::ServerStopReload_Validate()
{
	return true;
}

void ABow::OnRep_HitNotify()
{
	SimulateInstantHit(HitNotify);
}

void ABow::OnRep_PawnOwner()
{

}

void ABow::OnRep_BurstCounter()
{
	if (GetNetMode() != NM_DedicatedServer)
	{
		if (BurstCounter > 0)
		{
			SimulateWeaponFire();
		}
		else
		{
			StopSimulatingWeaponFire();
		}
	}
}

void ABow::OnRep_Reload()
{
	if (bPendingReload)
	{
		StartReload();
	}
	else
	{
		StopReload();
	}
}

void ABow::SimulateWeaponFire()
{
	if (CurrentState != EWeaponState::Firing)
	{
		return;
	}

	if (MuzzleParticles)
	{
		if (!bLoopedMuzzleFX || MuzzlePSC == NULL)
		{
			// Split screen requires we create 2 effects. One that we see and one that the other player sees.
			if ((PawnOwner != NULL) && (PawnOwner->IsLocallyControlled() == true))
			{
				AController* PlayerCon = PawnOwner->GetController();
				if (PlayerCon != NULL)
				{
					WeaponMesh->GetSocketLocation(MuzzleAttachPoint);
					MuzzlePSC = UGameplayStatics::SpawnEmitterAttached(MuzzleParticles, WeaponMesh, MuzzleAttachPoint);
					MuzzlePSC->bOwnerNoSee = false;
					MuzzlePSC->bOnlyOwnerSee = true;
				}
			}
			else
			{
				MuzzlePSC = UGameplayStatics::SpawnEmitterAttached(MuzzleParticles, WeaponMesh, MuzzleAttachPoint);
			}
		}
	}


	if (!bLoopedFireAnim || !bPlayingFireAnim)
	{
		FWeaponAnim AnimToPlay = FireAnim; //PawnOwner->IsAiming() || PawnOwner->IsLocallyControlled() ? FireAimingAnim : FireAnim;
		PlayWeaponAnimation(FireAnim);
		bPlayingFireAnim = true;
	}

	if (bLoopedFireSound)
	{
		if (FireAC == NULL)
		{
			FireAC = PlayWeaponSound(FireLoopSound);
		}
	}
	else
	{
		PlayWeaponSound(FireSound);
	}

	AMyPlayerController* PC = (PawnOwner != NULL) ? Cast<AMyPlayerController>(PawnOwner->Controller) : NULL;
	if (PC != NULL && PC->IsLocalController())
	{
		if (RecoilCurve)
		{
			const FVector2D RecoilAmount(RecoilCurve->GetVectorValue(FMath::RandRange(0.f, 1.f)).X, RecoilCurve->GetVectorValue(FMath::RandRange(0.f, 1.f)).Y);
			PC->ApplyRecoil(RecoilAmount, RecoilSpeed, RecoilResetSpeed);
		}

		if (FireCameraShake != NULL)
		{
			PC->ClientPlayCameraShake(FireCameraShake, 1);
		}
		if (FireForceFeedback != NULL)
		{
			FForceFeedbackParameters FFParams;
			FFParams.Tag = "Weapon";
			PC->ClientPlayForceFeedback(FireForceFeedback, FFParams);
		}
	}
}


void ABow::StopSimulatingWeaponFire()
{
	if (bLoopedMuzzleFX)
	{
		if (MuzzlePSC != NULL)
		{
			MuzzlePSC->DeactivateSystem();
			MuzzlePSC = NULL;
		}
		if (MuzzlePSCSecondary != NULL)
		{
			MuzzlePSCSecondary->DeactivateSystem();
			MuzzlePSCSecondary = NULL;
		}
	}

	if (bLoopedFireAnim && bPlayingFireAnim)
	{
		StopWeaponAnimation(FireAimingAnim);
		StopWeaponAnimation(FireAnim);
		bPlayingFireAnim = false;
	}

	if (FireAC)
	{
		FireAC->FadeOut(0.1f, 0.0f);
		FireAC = NULL;

		PlayWeaponSound(FireFinishSound);
	}
}

void ABow::HandleReFiring()
{
	UWorld* MyWorld = GetWorld();

	float SlackTimeThisFrame = FMath::Max(0.0f, (MyWorld->TimeSeconds - LastFireTime) - WeaponConfig.TimeBetweenShots);

	if (bAllowAutomaticWeaponCatchup)
	{
		TimerIntervalAdjustment -= SlackTimeThisFrame;
	}

	HandleFiring();
}

void ABow::HandleFiring()
{
	if (NeedAmmo != NeedAmmo)
	{
		if (GetNetMode() != NM_DedicatedServer)
		{
			SimulateWeaponFire();
		}

		if (PawnOwner && PawnOwner->IsLocallyControlled())
		{
			FireShot();
			//UseClipAmmo();

			// update firing FX on remote clients if function was called on server
			BurstCounter++;
			OnRep_BurstCounter();
		}
	}
		if (NeedAmmo ||(CurrentAmmoInClip > 0) && CanFire())
		{
			if (GetNetMode() != NM_DedicatedServer)
			{
				SimulateWeaponFire();
			}

			if (PawnOwner && PawnOwner->IsLocallyControlled())
			{
				FireShot();
				//UseClipAmmo();

				// update firing FX on remote clients if function was called on server
				BurstCounter++;
				OnRep_BurstCounter();
			}
		}

		else if (CanReload())
		{
			StartReload();
		}
		else if (PawnOwner && PawnOwner->IsLocallyControlled()&& NeedAmmo)
		{
			if (GetCurrentAmmo() == 0 && !bRefiring)
			{
				PlayWeaponSound(OutOfAmmoSound);
				AMyPlayerController* MyPC = Cast<AMyPlayerController>(PawnOwner->Controller);
			}

			// stop weapon fire FX, but stay in Firing state
			if (BurstCounter > 0)
			{
				OnBurstFinished();
			}
		}

		if (PawnOwner && PawnOwner->IsLocallyControlled())
		{
			// local client will notify server
			if (!HasAuthority())
			{
				ServerHandleFiring();
			}
			else
			{
				const bool bShouldUpdateAmmo = (CurrentAmmoInClip > 0 && CanFire()&& NeedAmmo);

				if (bShouldUpdateAmmo)
				{
					// update ammo
					UseClipAmmo();

					// update firing FX on remote clients
					BurstCounter++;
					OnRep_BurstCounter();
				}
			}

			// reload after firing last round
			if (CurrentAmmoInClip <= 0 && CanReload())
			{
				StartReload();
			}

			// setup refire timer
			bRefiring = (CurrentState == EWeaponState::Firing && WeaponConfig.TimeBetweenShots > 0.0f);
			if (bRefiring)
			{
				GetWorldTimerManager().SetTimer(TimerHandle_HandleFiring, this, &ABow::HandleReFiring, FMath::Max<float>(WeaponConfig.TimeBetweenShots + TimerIntervalAdjustment, SMALL_NUMBER), false);
				TimerIntervalAdjustment = 0.f;
			}
		}

		LastFireTime = GetWorld()->GetTimeSeconds();
	
}

void ABow::OnBurstStarted()
{
	// start firing, can be delayed to satisfy TimeBetweenShots
	const float GameTime = GetWorld()->GetTimeSeconds();
	if (LastFireTime > 0 && WeaponConfig.TimeBetweenShots > 0.0f &&
		LastFireTime + WeaponConfig.TimeBetweenShots > GameTime)
	{
		GetWorldTimerManager().SetTimer(TimerHandle_HandleFiring, this, &ABow::HandleFiring, LastFireTime + WeaponConfig.TimeBetweenShots - GameTime, false);
	}
	else
	{
		HandleFiring();
	}
}

void ABow::OnBurstFinished()
{
	// stop firing FX on remote clients
	BurstCounter = 0;
	OnRep_BurstCounter();

	// stop firing FX locally, unless it's a dedicated server
	if (GetNetMode() != NM_DedicatedServer)
	{
		StopSimulatingWeaponFire();
	}

	GetWorldTimerManager().ClearTimer(TimerHandle_HandleFiring);
	bRefiring = false;

	// reset firing interval adjustment
	TimerIntervalAdjustment = 0.0f;
}


void ABow::SetWeaponState(EWeaponState NewState)
{
	const EWeaponState PrevState = CurrentState;

	if (PrevState == EWeaponState::Firing && NewState != EWeaponState::Firing)
	{
		OnBurstFinished();
	}

	CurrentState = NewState;

	if (PrevState != EWeaponState::Firing && NewState == EWeaponState::Firing)
	{
		OnBurstStarted();
	}
}


void ABow::DetermineWeaponState()
{
	EWeaponState NewState = EWeaponState::Idle;

	if (bIsEquipped)
	{
		if (bPendingReload)
		{
			if (CanReload() == false)
			{
				NewState = CurrentState;
			}
			else
			{
				NewState = EWeaponState::Reloading;
			}
		}
		else if ((bPendingReload == false) && (bWantsToFire == true) && (CanFire() == true))
		{
			NewState = EWeaponState::Firing;
		}
	}
	else if (bPendingEquip)
	{
		NewState = EWeaponState::Equipping;
	}

	SetWeaponState(NewState);
}

void ABow::AttachMeshToPawn()
{
	if (PawnOwner)
	{
		if (const USkeletalMeshComponent* PawnMesh = PawnOwner->GetMesh())
		{
			const FName AttachSocket = PawnOwner->IsLocallyControlled() ? AttachSocket1P : AttachSocket3P;
			AttachToComponent(PawnOwner->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, AttachSocket);
		}
	}
}

UAudioComponent* ABow::PlayWeaponSound(USoundCue* Sound)
{
	UAudioComponent* AC = NULL;
	if (Sound && PawnOwner)
	{
		AC = UGameplayStatics::SpawnSoundAttached(Sound, PawnOwner->GetRootComponent());
	}

	return AC;
}

void ABow::PlayWeaponAnimation(const FWeaponAnim& Animation)
{

	float Duration = 0.0f;
	if (PawnOwner)
	{
		UAnimMontage* UseAnim = PawnOwner->IsLocallyControlled() ? Animation.Pawn1P : Animation.Pawn3P;
		if (UseAnim)
		{
			Duration = PawnOwner->PlayAnimMontage(UseAnim);
		}
	}
	ThisAnimDuration = Duration;
	
	if(!HasAuthority())
	{
		Server_PlayWeaponAnimation(Animation);
	}
}

void ABow::StopWeaponAnimation(const FWeaponAnim& Animation)
{
	if(!HasAuthority())
	{
		Server_StopWeaponAnimation(Animation);
	}
	if (PawnOwner)
	{
		UAnimMontage* UseAnim = PawnOwner->IsLocallyControlled() ? Animation.Pawn1P : Animation.Pawn3P;
		if (UseAnim)
		{
			PawnOwner->StopAnimMontage(UseAnim);
		}
	}
}

void ABow::Server_PlayWeaponAnimation_Implementation(const FWeaponAnim& Animation)
{
	PlayWeaponAnimation(Animation);
}

 bool ABow::Server_PlayWeaponAnimation_Validate(const FWeaponAnim& Animation)
{
	 return  true;
}

void ABow::Server_StopWeaponAnimation_Implementation(const FWeaponAnim& Animation)
{
	StopWeaponAnimation(Animation);
}
bool ABow::Server_StopWeaponAnimation_Validate(const FWeaponAnim& Animation)
{
	return true;
}

FVector ABow::GetCameraAim() const
{
	AMyPlayerController* const PlayerController = GetInstigator() ? Cast<AMyPlayerController>(GetInstigator()->Controller) : NULL;
	FVector FinalAim = FVector::ZeroVector;

	if (PlayerController)
	{
		FVector CamLoc;
		FRotator CamRot;
		PlayerController->GetPlayerViewPoint(CamLoc, CamRot);
		FinalAim = CamRot.Vector();
	}
	else if (GetInstigator())
	{
		FinalAim = GetInstigator()->GetBaseAimRotation().Vector();
	}

	return FinalAim;
}


void ABow::SimulateInstantHit(const FVector& Origin)
{
	if (!GetInstigator())
	{
		UE_LOG(LogTemp, Warning, TEXT("Weapon did not have instigator"));
		return;
	}

	const FVector StartTrace = Origin;
	const FVector AimRot = GetInstigator()->GetBaseAimRotation().Vector();
	const FVector EndTrace = (AimRot * HitScanConfig.Distance) + StartTrace;

	FHitResult WeaponHit = WeaponTrace(StartTrace, EndTrace);

	SpawnImpactEffects(WeaponHit);
}

void ABow::SpawnImpactEffects(const FHitResult& Impact)
{
	if (ImpactParticles)
	{
		if (Impact.bBlockingHit)
		{
			//Dont play effects if our local player got hit
			if (Impact.GetActor() != nullptr && Impact.GetActor() == UGameplayStatics::GetPlayerPawn(this, 0))
			{
				return;
			}

			FHitResult UseImpact = Impact;

			// trace again to find component lost during replication
			if (!Impact.Component.IsValid())
			{
				const FVector StartTrace = Impact.ImpactPoint + Impact.ImpactNormal * 10.0f;
				const FVector EndTrace = Impact.ImpactPoint - Impact.ImpactNormal * 10.0f;
				FHitResult Hit = WeaponTrace(StartTrace, EndTrace);
				UseImpact = Hit;
			}

			FTransform const SpawnTransform(Impact.ImpactNormal.Rotation(), Impact.ImpactPoint);

			UParticleSystem* ImpactEffect = ImpactParticles;
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactEffect, SpawnTransform);
		}
	}

}

void ABow::DealDamage(const FHitResult& Impact, const FVector& ShootDir)
{
	if (APlayerCharacter* OwnerCharacter = Cast<APlayerCharacter>(GetOwner()))
	{

		if (AActor* HitActor = Impact.GetActor())
		{
			float DamageAmount = HitScanConfig.Damage;

			FPointDamageEvent PointDmg;
			PointDmg.DamageTypeClass = HitScanConfig.DamageType;
			PointDmg.HitInfo = Impact;
			PointDmg.ShotDirection = ShootDir;
			PointDmg.Damage = DamageAmount;

			HitActor->TakeDamage(PointDmg.Damage, PointDmg, OwnerCharacter->Controller, this);
		}
	}
}

FHitResult ABow::WeaponTrace(const FVector& StartTrace, const FVector& EndTrace) const
{
	// Perform trace to retrieve hit info
	FCollisionQueryParams TraceParams(SCENE_QUERY_STAT(WeaponTrace), true, GetInstigator());
	TraceParams.bReturnPhysicalMaterial = true;

	FHitResult Hit(ForceInit);
	GetWorld()->LineTraceSingleByChannel(Hit, StartTrace, EndTrace, COLISION_WEAPON, TraceParams);

	return Hit;
}

void ABow::FireShot()
{
	if (APlayerCharacter* OwnerCharacter = Cast<APlayerCharacter>(GetOwner()))
	{
		/**Firing logic: Local client does a weapon trace, sends trace to server, spawns FX.
		If hit actor is movable, server does a Bounding Box check, and then spawns FX for all clients. */
		if (GetNetMode() != NM_DedicatedServer && OwnerCharacter && OwnerCharacter->IsLocallyControlled())
		{
			if (APlayerController* OwnerController = Cast<APlayerController>(OwnerCharacter->GetController()))
			{
				FVector AimLoc;
				FRotator AimRot;

				OwnerController->GetPlayerViewPoint(AimLoc, AimRot);

				const FVector StartTrace = AimLoc;
				const FVector EndTrace = (AimRot.Vector() * HitScanConfig.Distance) + AimLoc;
				const FVector ShootDir = AimRot.Vector();

				FCollisionQueryParams TraceParams(SCENE_QUERY_STAT(WeaponTrace), true, GetInstigator());
				TraceParams.bReturnPhysicalMaterial = true;

				FHitResult WeaponHit = WeaponTrace(StartTrace, EndTrace);
				ProcessInstantHit(WeaponHit, StartTrace, ShootDir);
			}
		}
	}
}

void ABow::ProcessInstantHit(const FHitResult& Impact, const FVector& Origin, const FVector& ShootDir)
{
	if (APlayerCharacter* OwnerCharacter = Cast<APlayerCharacter>(GetOwner()))
	{
		if (OwnerCharacter->IsLocallyControlled() && GetNetMode() == NM_Client)
		{
			// if we're a client and we've hit something that is being controlled by the server
			if (Impact.GetActor() && Impact.GetActor()->GetRemoteRole() == ROLE_Authority)
			{
				// notify the server of the hit
				ServerNotifyHit(Impact, ShootDir);
			}
			else if (Impact.GetActor() == NULL)
			{
				if (Impact.bBlockingHit)
				{
					// notify the server of the hit
					ServerNotifyHit(Impact, ShootDir);
				}
				else
				{
					// notify server of the miss
					ServerNotifyMiss(ShootDir);
				}
			}
		}
	}

	// process a confirmed hit
	ProcessInstantHit_Confirmed(Impact, Origin, ShootDir);
}

void ABow::ProcessInstantHit_Confirmed(const FHitResult& Impact, const FVector& Origin, const FVector& ShootDir)
{
	if (HasAuthority())
	{
		DealDamage(Impact, ShootDir);

		//Replicate shot origin to remote clients, they can then spawn FX
		HitNotify = Origin;

		if (Impact.GetActor() && Impact.GetActor()->IsA<APlayerCharacter>())
		{
			if (APlayerCharacter* OwnerCharacter = Cast<APlayerCharacter>(GetOwner()))
			{
				if (AMyPlayerController* OwnerController = OwnerCharacter ? Cast<AMyPlayerController>(OwnerCharacter->GetController()) : nullptr)
				{
					OwnerController->ClientShotHitConfirmed();
				}
			}
		}
	}

	//Spawn local FX
	if (GetNetMode() != NM_DedicatedServer)
	{
		SpawnImpactEffects(Impact);
	}
}

void ABow::ServerNotifyMiss_Implementation(FVector_NetQuantizeNormal ShootDir)
{

}

void ABow::ServerNotifyHit_Implementation(const FHitResult& Impact, FVector_NetQuantizeNormal ShootDir)
{
	// if we have an instigator, calculate dot between the view and the shot
	if (GetInstigator() && (Impact.GetActor() || Impact.bBlockingHit))
	{
		const FVector Origin = WeaponMesh ? WeaponMesh->GetSocketLocation(NAME_MuzzleSocket) : FVector();
		const FVector ViewDir = (Impact.Location - Origin).GetSafeNormal();

		// is the angle between the hit and the view within allowed limits (limit + weapon max angle)
		const float ViewDotHitDir = FVector::DotProduct(GetInstigator()->GetViewRotation().Vector(), ViewDir);
		if (true) //TODO IMPLEMENT DOT CHECK (ViewDotHitDir > AllowedViewDotHitDir - WeaponAngleDot)
		{
			if (Impact.GetActor() == NULL)
			{
				if (Impact.bBlockingHit)
				{
					ProcessInstantHit_Confirmed(Impact, Origin, ShootDir);
				}
			}
			// assume it told the truth about static things because the don't move and the hit 
			// usually doesn't have significant gameplay implications
			else if (Impact.GetActor()->IsRootComponentStatic() || Impact.GetActor()->IsRootComponentStationary())
			{
				ProcessInstantHit_Confirmed(Impact, Origin, ShootDir);
			}
			else
			{
				// Get the component bounding box
				const FBox HitBox = Impact.GetActor()->GetComponentsBoundingBox();

				// calculate the box extent, and increase by a leeway
				FVector BoxExtent = 0.5 * (HitBox.Max - HitBox.Min);
				BoxExtent *= HitScanConfig.ClientSideHitLeeway;

				// avoid precision errors with really thin objects
				BoxExtent.X = FMath::Max(20.0f, BoxExtent.X);
				BoxExtent.Y = FMath::Max(20.0f, BoxExtent.Y);
				BoxExtent.Z = FMath::Max(20.0f, BoxExtent.Z);

				// Get the box center
				const FVector BoxCenter = (HitBox.Min + HitBox.Max) * 0.5;

				// if we are within client tolerance
				if (FMath::Abs(Impact.Location.Z - BoxCenter.Z) < BoxExtent.Z &&
					FMath::Abs(Impact.Location.X - BoxCenter.X) < BoxExtent.X &&
					FMath::Abs(Impact.Location.Y - BoxCenter.Y) < BoxExtent.Y)
				{
					ProcessInstantHit_Confirmed(Impact, Origin, ShootDir);
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("%s Rejected client side hit of %s (outside bounding box tolerance)"), *GetNameSafe(this), *GetNameSafe(Impact.GetActor()));
				}
			}
		}
		else if (false)//ViewDotHitDir <= InstantConfig.AllowedViewDotHitDir)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s Rejected client side hit of %s (facing too far from the hit direction)"), *GetNameSafe(this), *GetNameSafe(Impact.GetActor()));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("%s Rejected client side hit of %s"), *GetNameSafe(this), *GetNameSafe(Impact.GetActor()));
		}
	}
}

void ABow::ServerHandleFiring_Implementation()
{
	const bool bShouldUpdateAmmo = (CurrentAmmoInClip > 0 && CanFire());

	HandleFiring();
	if (NeedAmmo) {
		if (bShouldUpdateAmmo)
		{
			// update ammo
			UseClipAmmo();

			// update firing FX on remote clients
			BurstCounter++;
			OnRep_BurstCounter();
		}
	}
}

bool ABow::ServerHandleFiring_Validate()
{
	return true;
}
