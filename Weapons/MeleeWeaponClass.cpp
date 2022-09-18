// Fill out your copyright notice in the Description page of Project Settings.


#include "MeleeWeaponClass.h"
#include <HowItBegan/Player/MyPlayerController.cpp>
#include "Net/UnrealNetwork.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/AudioComponent.h"
#include "../Components/InventoryComponent.h"
#include "HowItBegan/SurvivalMode/PlayerStatsComponent.h"
// Sets default values
AMeleeWeaponClass::AMeleeWeaponClass()
{
	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeleeWeapon"));
	WeaponMesh->bReceivesDecals = false;
	WeaponMesh->SetCollisionObjectType(ECC_WorldDynamic);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	RootComponent = WeaponMesh;

	AttachSocket1P = FName("GripPoint_");
	AttachSocket3P = FName("GripPoint_");

	bPlayingAttackAnim = false;
	bIsAttacking = false;
	bIsEquipped = false;
	bIsPendingEquip = false;

	CurrentState = EMeleeWeaponState::Idle;

 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PrePhysics;
	bReplicates = true;
	bNetUseOwnerRelevancy = true;
}

void AMeleeWeaponClass::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMeleeWeaponClass, PawnOwner);
	DOREPLIFETIME_CONDITION(AMeleeWeaponClass, Item, COND_InitialOnly);
}


void AMeleeWeaponClass::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}


// Called when the game starts or when spawned
void AMeleeWeaponClass::BeginPlay()
{
	Super::BeginPlay();
	
	if (HasAuthority())
		PawnOwner = Cast<APlayerCharacter>(GetOwner());
}

void AMeleeWeaponClass::Destroyed()
{
	Super::Destroyed();
}


void AMeleeWeaponClass::OnEquip()
{
	AttachMeshToPawn();

	bIsPendingEquip = true;

	DetermineWeaponState();

	OnEquipFinished();

	if (PawnOwner && PawnOwner->IsLocallyControlled())
	{
		PlayWeaponSound(EquipSound);
	}
}

void AMeleeWeaponClass::OnEquipFinished()
{
	AttachMeshToPawn();

	bIsEquipped = true;
	bIsPendingEquip = false;

	// Determine the state so that the can reload checks will work
	DetermineWeaponState();
}

void AMeleeWeaponClass::OnUnEquip()
{
	bIsEquipped = false;
}

bool AMeleeWeaponClass::IsEquipped() const
{
	return bIsEquipped;
}

bool AMeleeWeaponClass::IsAttachedToPawn() const
{
	return bIsEquipped || bIsPendingEquip;
}

void AMeleeWeaponClass::AttachMeshToPawn()
{
}

bool AMeleeWeaponClass::HasStaminaToAttack() const
{
	float Stamina =0;
	if (HasAuthority()) {
		if (PawnOwner) {
			 Stamina = PawnOwner->PLayerStatsComp->GetStamina();
		}
	}
	return (Stamina > 0);
}

void AMeleeWeaponClass::StartAttack()
{
	if (!HasAuthority()) {
		ServerStartAttack();
	}

	if (bIsAttacking) {
		bIsAttacking = false;
		DetermineWeaponState();
	}
}

void AMeleeWeaponClass::ServerStartAttack_Implementation()
{
	Attack();
}

bool AMeleeWeaponClass::ServerStartAttack_Validate()
{
	return true;
}

bool AMeleeWeaponClass::CanAttack() const
{
	bool bCanAttack = PawnOwner != nullptr;
	bool bHasStaminatoAttack = HasStaminaToAttack();
	bool bStateOkToAttack = (CurrentState == EMeleeWeaponState::Idle || CurrentState == EMeleeWeaponState::Attack);
	return ((bCanAttack == true) && (bHasStaminatoAttack == true) && (bStateOkToAttack == true));
}

void AMeleeWeaponClass::DetermineWeaponState()
{
}

USkeletalMeshComponent* AMeleeWeaponClass::GetWeaponMesh() const
{
	return WeaponMesh;
}

APlayerCharacter* AMeleeWeaponClass::GetPawnOwner() const
{
	return PawnOwner;
}

void AMeleeWeaponClass::SetPawnOwner(APlayerCharacter* Character)
{
	if (PawnOwner != Character)
	{
		PawnOwner = Character;
		SetOwner(Character);
		SetInstigator(Character);
	}
}

void AMeleeWeaponClass::Attack()
{
	if (APlayerCharacter* OwnerCharacter = Cast<APlayerCharacter>(GetOwner()))
	{
		if (GetNetMode() != NM_DedicatedServer && OwnerCharacter && OwnerCharacter->IsLocallyControlled())
		{
			if (AMyPlayerController* OwnerController = Cast<AMyPlayerController>(OwnerCharacter->GetController()))
			{
				if (GetWorld()->TimeSince(WeaponConfig.LastMeleeAttackTime) > AttackAnim.Pawn1P->GetPlayLength()
					|| GetWorld()->TimeSince(WeaponConfig.LastMeleeAttackTime) > AttackAnim.Pawn3P->GetPlayLength())
				{
					FVector EyeLoc;
					FRotator EyeRot;

					OwnerController->GetPlayerViewPoint(EyeLoc, EyeRot);

				//	FCollisionShape Shape = FCollisionShape::MakeSphere(15.f);

					const FVector StartTrace = EyeLoc;
					const FVector EndTrace = (EyeRot.Vector() * WeaponConfig.MeleeAttackDistance) + StartTrace;
					const FVector Dir = EyeRot.Vector();
					FCollisionQueryParams QueryParams = FCollisionQueryParams("MeleeSweep", false, GetInstigator());

					PlayWeaponAnimation(AttackAnim);
					FHitResult Hit = Trace(StartTrace, EndTrace);
					processHit(Hit, StartTrace, Dir);
				}
			}
		}
	}
}


void AMeleeWeaponClass::OnRep_HitNotify()
{
}

void AMeleeWeaponClass::OnRep_PawnOwner()
{
}

	UAudioComponent* AMeleeWeaponClass::PlayWeaponSound(USoundCue * Sound)
	{
		return nullptr;
	}

	float AMeleeWeaponClass::PlayWeaponAnimation(const FMeleeWeaponAnim & Animation)
	{
		return 0.0f;
	}

	void AMeleeWeaponClass::DealDamage(const FHitResult & Impact, const FVector & ShootDir)
	{
		if (APlayerCharacter* OwnerCharacter = Cast<APlayerCharacter>(GetOwner()))
		{
			if (AActor* hitTarget = Impact.GetActor())
			{
				float Dmg = WeaponConfig.MeleeAttackDamage;
				FPointDamageEvent PointDmg;
				PointDmg.DamageTypeClass = WeaponConfig.MeleeDamageType;
				PointDmg.HitInfo = Impact;
				PointDmg.ShotDirection = ShootDir;
				PointDmg.Damage = Dmg;

				hitTarget->TakeDamage(PointDmg.Damage, PointDmg, OwnerCharacter->Controller, this);

			}
		}
	}

	FHitResult AMeleeWeaponClass::Trace(const FVector & StartTrace, const FVector & EndTrace) const
	{
		FCollisionShape Shape = FCollisionShape::MakeSphere(15.f);
		FCollisionQueryParams QueryParams = FCollisionQueryParams("MeleeSweep", false, GetInstigator());
		FHitResult Hit(ForceInit);	
		GetWorld()->SweepSingleByChannel(Hit, StartTrace, EndTrace, FQuat(), ECC_GameTraceChannel1, Shape, QueryParams);
		return Hit;	
	}

	void AMeleeWeaponClass::processHit(const FHitResult& Impact, const FVector& start, const FVector& dir)
	{
	
	}

void AMeleeWeaponClass::ServerNotifyHit_Implementation(const FHitResult& Impact, FVector_NetQuantizeNormal ShootDir)
{
	
}

void AMeleeWeaponClass::ServerNotifyMiss_Implementation(FVector_NetQuantizeNormal dir)
{
	
}

	void AMeleeWeaponClass::processHit_Confirmed(const FHitResult& Impact, const FVector& Origin, const FVector& end)
	{
	
	}







	