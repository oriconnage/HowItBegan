// Fill out your copyright notice in the Description page of Project Settings.


#include "meleeWeapon.h"
#include "Net/UnrealNetwork.h"
#include "Components/SkeletalMeshComponent.h"
#include "../Components/InventoryComponent.h"
#include "../Player/MyPlayerController.h"
#include "../Player/PlayerCharacter.h"
#include <GameFramework/Actor.h>
#include "../item/AmmoItem.h"
#include <Engine/EngineTypes.h>
#include "../HelperFile.h"
#include "Sound/SoundCue.h"
#include "Curves/CurveVector.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "../HowitBegan.h"
#include "Engine/Engine.h"
#include "HowItBegan/SurvivalMode/PlayerStatsComponent.h"


// Sets default values
AmeleeWeapon::AmeleeWeapon()
{
	//WeaponMesh
	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeleeWeapon"));
	WeaponMesh->bReceivesDecals = false;
	WeaponMesh->SetCollisionObjectType(ECC_WorldDynamic);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	RootComponent = WeaponMesh;
	
	//Bool 
	bPlayingslashAnim = false;
	bPlayingBlockAnim = false;
	bIsEquipped = false;
	bPendingEquip = false;
	bPendingBlock = false;
	bWantToAttack = false;
	
	CurrentState = EMeleeState::Idle;
	AttachSocket1P = FName("GripPoint");
	AttachSocket3P = FName("GripPoint");

	LastAttackTime = 0.0f;
	BurstCounter = 0;
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PrePhysics;
	bReplicates = true;
	bNetUseOwnerRelevancy = true;
}

void AmeleeWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AmeleeWeapon, PawnOwner);

	DOREPLIFETIME_CONDITION(AmeleeWeapon, BurstCounter, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(AmeleeWeapon, Item, COND_InitialOnly);
	DOREPLIFETIME_CONDITION(AmeleeWeapon, bPendingBlock, COND_SkipOwner);
}

// Called when the game starts or when spawned
void AmeleeWeapon::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		PawnOwner = Cast<APlayerCharacter>(GetOwner());
	}
	
}
void AmeleeWeapon::Destroyed()
{
	Super::Destroyed();

	StopSimulatingMeleeAttack();
}
void AmeleeWeapon::OnEquip()
{
	AttachMeshToPawn();

	bPendingEquip = true;
	DetermineWeaponState();

	OnEquipFinished();


	if (PawnOwner && PawnOwner->IsLocallyControlled())
	{
		PlayWeaponAnimation(EquipAnim);
		PlayWeaponSound(EquipSound);
	}
	UE_LOG(LogTemp, Warning, TEXT("MeleeWeapon"));
}

void AmeleeWeapon::OnEquipFinished()
{
	AttachMeshToPawn();

	bIsEquipped = true;
	bPendingEquip = false;

	// Determine the state so that the can reload checks will work
	DetermineWeaponState();
}

void AmeleeWeapon::OnUnEquip()
{
	bIsEquipped = false;
	StopAttack();
	if (bPendingEquip) {
		StopWeaponAnimation(EquipAnim);
		bPendingEquip = false;

		GetWorldTimerManager().ClearTimer(TimerHandle_OnEquipFinished);
	}
	DetermineWeaponState();
}

bool AmeleeWeapon::IsEquipped() const
{
	return bIsEquipped;
}

bool AmeleeWeapon::IsAttachedToPawn() const
{
	return bIsEquipped || bPendingEquip;;
}

void AmeleeWeapon::StartAttack()
{
	if (GetLocalRole() < ROLE_Authority)
	{
		ServerStartAttack();
	//	GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Green, FString::Printf(TEXT("Player has authority %d"), bWantToAttack));
	}
	//	GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Green, FString::Printf(TEXT("!WantToAttack %d"), bWantToAttack));
	if (!bWantToAttack)
	{
		bWantToAttack = true;
		GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Green, FString::Printf(TEXT("WantToAttack---->  %d"), bWantToAttack));
		DetermineWeaponState();
	}
}

void AmeleeWeapon::ServerStartAttack_Implementation()
{
	StartAttack();
}

bool AmeleeWeapon::ServerStartAttack_Validate()
{
	return true;
}
void AmeleeWeapon::StopAttack()
{
	if ((GetLocalRole() < ROLE_Authority) && PawnOwner && PawnOwner->IsLocallyControlled())
	{
		
		ServerStopAttack();
	}

	if (bWantToAttack)
	{
		bWantToAttack = false;
		GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Green, FString::Printf(TEXT("Stop attack %d"), bWantToAttack));
		DetermineWeaponState();
	}
}

void AmeleeWeapon::ServerStopAttack_Implementation()
{
	StopAttack();
}

bool AmeleeWeapon::ServerStopAttack_Validate()
{
	return true;
}

void AmeleeWeapon::StartBlock()
{
	if (GetLocalRole() < ROLE_Authority)
	{
		ServerStartBlock();
		
	}

	if (!bPendingBlock)
	{
		bPendingBlock = true;
		DetermineWeaponState();
	}
}

void AmeleeWeapon::ServerStartBlock_Implementation()
{
	StartBlock();
}

bool AmeleeWeapon::ServerStartBlock_Validate()
{
	return true;
}
void AmeleeWeapon::StopBlock()
{
	if (GetLocalRole() < ROLE_Authority)
	{
		ServerStopBlock();
		PawnOwner->PLayerStatsComp->ModifyStamina(-HitConfig.StaminaCost);
	}

	if (!bWantToAttack)
	{
		bPendingBlock =false;
		DetermineWeaponState();
	}
}
void AmeleeWeapon::ServerStopBlock_Implementation()
{
	StopBlock();
}

bool AmeleeWeapon::ServerStopBlock_Validate()
{
	return true;
}

bool AmeleeWeapon::CanAttack() const
{
//	bool bCanFire = PawnOwner != nullptr;
//	bool bHasStamina= (PawnOwner->GetStamina() > 0);
	//bool bStateOKToFire = ((CurrentState == EMeleeState::Idle) || (CurrentState == EMeleeState::Attack));
	return true; // ((bCanFire == true) && (bStateOKToFire == true)&&(bPendingBlock ==false)/*&&(bHasStamina == true)*/);

}

bool AmeleeWeapon::CanBlock() const
{
	bool bCanBlock = PawnOwner != nullptr;
	bool bHasStamina = (PawnOwner->PLayerStatsComp->GetStamina() > 0);
	bool bStateOkToBlock = ((CurrentState == EMeleeState::Idle) || (CurrentState == EMeleeState::Block));
	return ((bCanBlock == true) && (bStateOkToBlock == true) && (bWantToAttack == false)&&(bHasStamina ==true));

}
USkeletalMeshComponent* AmeleeWeapon::GetWeaponMesh() const
{
	return WeaponMesh;
}

APlayerCharacter* AmeleeWeapon::GetPawnOwner() const
{
	return PawnOwner;
}

void AmeleeWeapon::SetPawnOwner(APlayerCharacter* Character)
{
	if (PawnOwner != Character)
	{
		SetInstigator(Character);
		PawnOwner = Character;
		// net owner for RPC calls
		SetOwner(Character);
	}
}

// Called every frame
void AmeleeWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
}

float AmeleeWeapon::GetEquipStartedTime() const
{
	return EquipStartedTime;
}

float AmeleeWeapon::GetEquipDuration() const
{
	return EquipDuration;
}

UAudioComponent* AmeleeWeapon::PlayWeaponSound(USoundCue* Sound)
{
	UAudioComponent* AC = NULL;
	if (Sound && PawnOwner)
	{
		AC = UGameplayStatics::SpawnSoundAttached(Sound, PawnOwner->GetRootComponent());
	}

	return AC;
}

float AmeleeWeapon::PlayWeaponAnimation(const FMeleeAnim& Animation)
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

	return Duration;
}

void AmeleeWeapon::StopWeaponAnimation(const FMeleeAnim& Animation)
{
	if (PawnOwner)
	{
		UAnimMontage* Anim = PawnOwner->IsLocallyControlled() ? Animation.Pawn1P : Animation.Pawn3P;
		if (Anim)
			PawnOwner->StopAnimMontage(Anim);
	}
}


void AmeleeWeapon::SpawnImpactEffects(const FHitResult& Impact)
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

void AmeleeWeapon::SetWeaponState(EMeleeState state)
{
	const EMeleeState PrevState = CurrentState;

	CurrentState = state;

	if (PrevState != EMeleeState::Attack && state == EMeleeState::Attack)
	{
		HandleAttack();
		GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Green, TEXT("Attack"));
	}
	else if (PrevState != EMeleeState::Block && state == EMeleeState::Block) {
		//Calls the block function
		//TODO Create a block function 
	}
	
}

void AmeleeWeapon::DealDamage(const FHitResult& Impact, const FVector& HitDir)
{
	if (APlayerCharacter* OwnerCharacter = Cast<APlayerCharacter>(GetOwner()))
	{

		if (AActor* HitActor = Impact.GetActor())
		{
			float DamageAmount = HitConfig.Damage;

			FPointDamageEvent PointDmg;
			PointDmg.DamageTypeClass = HitConfig.DamageType;
			PointDmg.HitInfo = Impact;
			PointDmg.ShotDirection = HitDir;
			PointDmg.Damage = DamageAmount;

			HitActor->TakeDamage(PointDmg.Damage, PointDmg, OwnerCharacter->Controller, this);
		}
	}
}

FHitResult AmeleeWeapon::WeaponTrace(const FVector& StartTrace, const FVector& EndTrace) const
{
	FCollisionShape Shape = FCollisionShape::MakeSphere(15.f);
	// Perform trace to retrieve hit info
	FCollisionQueryParams TraceParams(SCENE_QUERY_STAT(WeaponTrace), false, GetInstigator());
	TraceParams.bReturnPhysicalMaterial = true;

	FHitResult Hit(ForceInit);
	GetWorld()->SweepSingleByChannel(Hit, StartTrace, EndTrace,FQuat(), ECC_GameTraceChannel1, Shape, TraceParams);
//	GetWorld()->LineTraceSingleByChannel(Hit, StartTrace, EndTrace, ECC_GameTraceChannel1, TraceParams);

	return Hit;
}

void AmeleeWeapon::ProcessHit(const FHitResult& Impact, const FVector& Origin, const FVector& HitDir)
{

	if (APlayerCharacter* OwnerCharacter = Cast<APlayerCharacter>(GetOwner()))
	{
		if (OwnerCharacter->IsLocallyControlled() && GetNetMode() == NM_Client)
		{
			// if we're a client and we've hit something that is being controlled by the server
			if (Impact.GetActor() && Impact.GetActor()->GetRemoteRole() == ROLE_Authority)
			{
				ServerNotifyHit(Impact, HitDir);
			}
			else if (Impact.GetActor() == NULL)
			{
				if (Impact.bBlockingHit)
				{
					// notify the server of the hit
					ServerNotifyHit(Impact, HitDir);
				}
				else
				{
					// notify server of the miss
					ServerNotifyMiss(HitDir);
				}
			}
		}
		// process a confirmed hit
		ProcessInstantHit_Confirmed(Impact, Origin, HitDir);
	}
}

void AmeleeWeapon::SimulateHit(const FVector& Origin)
{
	if (!GetInstigator())
	{
		UE_LOG(LogTemp, Warning, TEXT("Weapon did not have instigator"));
		return;
	}
	const FVector StartTrace = Origin;
	const FVector AimRot = GetInstigator()->GetBaseAimRotation().Vector();
	const FVector EndTrace = (AimRot * HitConfig.Distance) + StartTrace;

	FHitResult WeaponHit = WeaponTrace(StartTrace, EndTrace);

	SpawnImpactEffects(WeaponHit);
}

void AmeleeWeapon::SpawnImpactEffect(const FHitResult& Impact)
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

void AmeleeWeapon::SimulateMeleeAttack()
{
	if (CurrentState != EMeleeState::Attack)
		return;

	if (!bPlayingslashAnim)
	{
		PlayWeaponAnimation(slashAnim);
		bPlayingslashAnim = true;
	}
	if (AttackAC == NULL)
		AttackAC = PlayWeaponSound(SlashSound);
	else
		PlayWeaponSound(SlashSound);

	AMyPlayerController* PC = (PawnOwner != NULL) ? Cast<AMyPlayerController>(PawnOwner->Controller) : NULL;
	if (PC != NULL && PC->IsLocalController())
	{
		//PC->ClientPlayCameraShake(CameraShake);
	}
}

void AmeleeWeapon::StopSimulatingMeleeAttack()
{
	if (bPlayingslashAnim)
	{
		StopWeaponAnimation(slashAnim);
		bPlayingslashAnim = false;
	}
	if (AttackAC)
	{
		AttackAC->FadeOut(0.1f, 0.0f);
		AttackAC = NULL;
		PlayWeaponSound(FinshedSound);

	}
}

void AmeleeWeapon::ServerNotifyHit_Implementation(const FHitResult& Impact, FVector_NetQuantizeNormal ShootDir)
{
	// if we have an instigator, calculate dot between the view and the shot
	if (GetInstigator() && (Impact.GetActor() || Impact.bBlockingHit))
	{
		const FVector Origin = WeaponMesh ? WeaponMesh->GetSocketLocation("Socket") : FVector();
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
				BoxExtent *= HitConfig.ClientSideHitLeeway;

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

void AmeleeWeapon::ServerNotifyMiss_Implementation(FVector_NetQuantizeNormal ShootDir)
{
}

void AmeleeWeapon::AttachMeshToPawn()
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

void AmeleeWeapon::DetermineWeaponState()
{
	EMeleeState Newstate = EMeleeState::Idle;
	GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Green, TEXT("Idle"));
	Newstate = CurrentState;
	if (bIsEquipped) {
		if (CanAttack()==true)
		{
			Newstate = EMeleeState::Attack;
			GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Green, TEXT("Attack"));
		}

	 if (bWantToAttack == false && bPendingBlock == true && CanBlock()==true)
		{
			Newstate = EMeleeState::Block;
			GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Green, TEXT("Block"));
		}
	}
	else if (bPendingEquip)
	{
		Newstate = EMeleeState::Equipping;
		GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Green, TEXT("Equip"));
	}

	GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Green, FString::Printf(TEXT("state = %d"),Newstate));
	SetWeaponState(Newstate);
}

void AmeleeWeapon::ProcessInstantHit_Confirmed(const FHitResult& Impact, const FVector& Origin, const FVector& HitDir)
{
	if (HasAuthority()) {
		//Deal damage 
		DealDamage(Impact, HitDir);
		
		// Replicate Hit Origin to the other clients - Spawn FX 
		HitNotify = Origin;

		if (Impact.GetActor() && Impact.GetActor()->IsA<APlayerCharacter>()) {
			if (APlayerCharacter* HitTarget = Cast<APlayerCharacter>(GetOwner()))
			{
				if (AMyPlayerController* TargetController = HitTarget ? Cast<AMyPlayerController>(HitTarget->GetController()) : nullptr)
				{
					TargetController->ClientShotHitConfirmed();
				}
			}
		}
	}
	//Spawn FX
	if (GetNetMode() != NM_DedicatedServer)
	{
		SpawnImpactEffect(Impact);
	}
}

void AmeleeWeapon::HandleAttack()
{
	
	if (CanAttack() && PawnOwner->PLayerStatsComp->GetStamina() > 0)
	{
		if (GetNetMode() != NM_DedicatedServer)
		{
				//SimulateMeleeAttack();
			PlayWeaponAnimation(slashAnim);
			PlayWeaponSound(SlashSound);
		}

		if (PawnOwner && PawnOwner->IsLocallyControlled())
		{
			meleeAttack();

		}

	}
	if (PawnOwner && PawnOwner->IsLocallyControlled())
	{
		// local client will notify server
		if (!HasAuthority())
		{
			ServerHandleAttack();
		}
		else
		{
			const bool bShouldUpdateStamina = (PawnOwner->PLayerStatsComp->GetStamina() > 0 && CanAttack());
			if (bShouldUpdateStamina)
			{
				OnRep_BurstCounter();
				PawnOwner->PLayerStatsComp->ModifyStamina(-(HitConfig.StaminaCost));
			}
			bReAttacking = (CurrentState == EMeleeState::Attack && HitConfig.TimeBetweenAttack > 0.0f);
			if (bReAttacking)
			{
				GetWorldTimerManager().SetTimer(TimerHandle_HandleAttack, this, &AmeleeWeapon::HandleAttack, FMath::Max<float>(HitConfig.TimeBetweenAttack + TimerIntervalAdjustment, SMALL_NUMBER), false);
				TimerIntervalAdjustment = 0.f;
			}

		}
	}
	LastAttackTime = GetWorld()->GetTimeSeconds();
}

void AmeleeWeapon::OnRep_HitNotify()
{
	SimulateHit(HitNotify);
}

void AmeleeWeapon::OnBurstStarted()
{
	// start firing, can be delayed to satisfy TimeBetweenShots
	const float GameTime = GetWorld()->GetTimeSeconds();
	if (LastAttackTime > 0 && HitConfig.TimeBetweenAttack > 0.0f &&
		LastAttackTime + HitConfig.TimeBetweenAttack > GameTime)
	{
		GetWorldTimerManager().SetTimer(TimerHandle_HandleAttack, this, &AmeleeWeapon::HandleAttack, LastAttackTime + HitConfig.TimeBetweenAttack - GameTime, false);
	}
	else
	{
		HandleAttack();
	}
}

void AmeleeWeapon::OnBurstFinished()
{
	// stop firing FX on remote clients
	BurstCounter = 0;
	OnRep_BurstCounter();

	// stop firing FX locally, unless it's a dedicated server
	if (GetNetMode() != NM_DedicatedServer)
	{
		StopSimulatingMeleeAttack();
	}

	GetWorldTimerManager().ClearTimer(TimerHandle_HandleAttack);
	bReAttacking = false;

	// reset firing interval adjustment
	TimerIntervalAdjustment = 0.0f;
}

void AmeleeWeapon::OnRep_BurstCounter()
{
	if (GetNetMode() != NM_DedicatedServer)
	{
		if (BurstCounter > 0)
		{
			SimulateMeleeAttack();
		}
		else
		{
			StopSimulatingMeleeAttack();
		}
	}
}

void AmeleeWeapon::OnRep_Block()
{
	if(bPendingBlock)
	{
		StartBlock();
	}else
	{
		StopBlock();
	}
	
}

void AmeleeWeapon::OnRep_PawnOwner()
{
}

void AmeleeWeapon::meleeAttack()
{
	if (APlayerCharacter* ownerCharacter = Cast<APlayerCharacter>(GetOwner()))
	{
		if (ownerCharacter && ownerCharacter->IsLocallyControlled())
		{
			if (APlayerController* OwnerController = Cast<APlayerController>(ownerCharacter->GetController()))
			{
				FVector AimLoc;
				FRotator AimRot;

				OwnerController->GetPlayerViewPoint(AimLoc, AimRot);

				const FVector StartTrace = AimLoc;
				const FVector EndTrace = (AimRot.Vector() * HitConfig.Distance) + AimLoc;
				const FVector HitDir = AimRot.Vector();
				FCollisionQueryParams TraceParams(SCENE_QUERY_STAT(WeaponTrace), true, GetInstigator());
				TraceParams.bReturnPhysicalMaterial = true;

				FHitResult WeaponHit = WeaponTrace(StartTrace, EndTrace);
				ProcessHit(WeaponHit, StartTrace, HitDir);
			}
		}
	}

}


void AmeleeWeapon::ServerHandleAttack_Implementation()
{
	const bool bShouldUpdateStamina = (PawnOwner->PLayerStatsComp->GetStamina() > 0 && CanAttack());

	HandleAttack();

	if (bShouldUpdateStamina)
	{
		// update stamina
		PawnOwner->PLayerStatsComp->ModifyStamina(-(HitConfig.StaminaCost));

		// update firing FX on remote clients
		BurstCounter++;
		OnRep_BurstCounter();
	}
}

bool AmeleeWeapon::ServerHandleAttack_Validate()
{
	return true;
}




