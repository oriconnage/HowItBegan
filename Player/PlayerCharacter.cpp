// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacter.h"
#include "Engine/Engine.h"
#include "MyPlayerController.h"
#include "Net/UnrealNetwork.h"
#include "../Components/BuilderComponent.h"
#include "../Components/InventoryComponent.h"
#include "../Components/InteractionComponet.h"
#include"../item/ItemsObject.h"
#include "../item/WeaponItem.h"
#include"../item/GearItem.h"
#include "../item/EquippableItem.h"
#include"../item/equippableMeleeWeapon.h"
#include "../Weapons/Bow.h"
#include "../Weapons/MeleeDamage.h"
#include "../Weapons/meleeWeapon.h"
#include "../Worlds/PickUp.h"
#include "../Widget/PlayerHUD.h"
#include "HowItBegan/HowItBegan.h"
#include "../HelperFile.h"
#include"Materials/MaterialInstance.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/DamageType.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/CapsuleComponent.h"
#include "Camera/CameraComponent.h"
#include "HowItBegan/SurvivalMode/PlayerStatsComponent.h"
#include "HowItBegan/Weapons/MeleeWeaponClass.h"


#define LOCTEXT_NAMESPACE "PlayerCharacter"

static FName Name_AimSocket("ADSSocket");
static FName Name_CameraSocket("headSocket");
// Sets default values
APlayerCharacter::APlayerCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	//Spring arm 
	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>("SpringArmComponent");
	SpringArmComponent->SetupAttachment(GetMesh(), Name_CameraSocket);
	SpringArmComponent->TargetArmLength = 0.f;
	//Camera
	CameraComponent = CreateDefaultSubobject<UCameraComponent>("CameraComponent");
	CameraComponent->SetupAttachment(SpringArmComponent);
	CameraComponent->bUsePawnControlRotation = true;

	// setting up Equip-able slots 
	HelmetMesh = PlayerMeshes.Add(EEquippableSlot::EIS_Helmet, CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("HelmetMesh")));
	ChestMesh = PlayerMeshes.Add(EEquippableSlot::EIS_Chest, CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ChestMesh")));
	ArmMesh = PlayerMeshes.Add(EEquippableSlot::EIS_Arm,CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ArmMesh")));
	LegsMesh = PlayerMeshes.Add(EEquippableSlot::EIS_Legs, CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("LegsMesh")));
	SkirtMesh = PlayerMeshes.Add(EEquippableSlot::EIS_Skirt,CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkirtMesh")));
	CloakMesh = PlayerMeshes.Add(EEquippableSlot::EIS_Cloak, CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CloakMesh")));
	PauldronMesh = PlayerMeshes.Add(EEquippableSlot::EIS_Pauldron, CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("PauldronMesh")));
	HandsMesh = PlayerMeshes.Add(EEquippableSlot::EIS_Hands, CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("HandsMesh")));
	BackpackMesh = PlayerMeshes.Add(EEquippableSlot::EIS_Backpack, CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("BackpackMesh")));
	//ArrowMesh = PlayerMeshes.Add(EEquippableSlot::EIS_Arrows, CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ArrowMesh")));

	//Tell all the body meshes to use the Head mesh for animation
	for (auto& Kvp : PlayerMeshes)
	{
		USkeletalMeshComponent* MeshComponent = Kvp.Value;
		MeshComponent->SetupAttachment(GetMesh());
		MeshComponent->SetMasterPoseComponent(GetMesh());
	}
	PlayerMeshes.Add(EEquippableSlot::EIS_Head, GetMesh());
	GetMesh()->SetOwnerNoSee(true);

	//Give the player an inventory with 20 slots, and an 80kg capacity
	PlayerInventory = CreateDefaultSubobject<UInventoryComponent>("PlayerInventory");
	PlayerInventory->SetCapacity(20);
	PlayerInventory->SetWeightCapacity(80.f);
	PlayerInventory->OnItemAdded.AddDynamic(this, &APlayerCharacter::ItemAddedToInventory);
	PlayerInventory->OnItemRemoved.AddDynamic(this, &APlayerCharacter::ItemRemovedFromInventory);


	LootPlayerInteraction = CreateDefaultSubobject<UInteractionComponet>("PlayerInteraction");
	LootPlayerInteraction->InteractableActionText = LOCTEXT("LootPlayerText", "Loot");
	LootPlayerInteraction->InteractableNameText = LOCTEXT("LootPlayerName", "Player");
	LootPlayerInteraction->SetupAttachment(GetRootComponent());
	LootPlayerInteraction->SetActive(false, true);
	LootPlayerInteraction->bAutoActivate = false;

	//We create a scene component that the inventory camera roots to. This way we can rotate this root to allow players to rotate their character in the inventory menu.
	USceneComponent* InventoryCaptureRoot = CreateDefaultSubobject<USceneComponent>("InventoryCaptureRoot");
	InventoryCaptureRoot->SetupAttachment(GetCapsuleComponent());
	InventoryCaptureRoot->SetRelativeRotation(FRotator(0.f, 180.f, 0.f));

	InventoryPlayerCapture = CreateDefaultSubobject<USceneCaptureComponent2D>("InventoryPlayerCapture");
	InventoryPlayerCapture->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList;
	InventoryPlayerCapture->SetupAttachment(InventoryCaptureRoot);
	InventoryPlayerCapture->SetRelativeLocation(FVector(-1100.f, 0.f, 0.f));

	//player Stats Health , stamina , Thirsts , Hunger 
	PLayerStatsComp = CreateDefaultSubobject<UPlayerStatsComponent>("PlayerStats");

	// Build component
	BuildComponent = CreateDefaultSubobject<UBuilderComponent>(TEXT("BuildComponent"));

	CraftComponent = CreateDefaultSubobject<UCraftComponent>("CraftComponent");
	
	InteractionCheckFrequency = 0.f;
	InteractionCheckDistance = 1000.f;

	PlayerOverlay = EOverlayState::Default;

	SprintSpeed = GetCharacterMovement()->MaxWalkSpeed * 1.3f;
	WalkSpeed = 300;
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;

	//isSprinting = false;



	
	MeleeAttackDistance = 150.f;
	MeleeAttackDamage = 20.f;
	bIsAiming = false;
	SetReplicateMovement(true);
	SetReplicates(true);
	bAlwaysRelevant = true;
	PLayerStatsComp->isThisSurvivalPlayer = false;
	isRespawning = false;
	//PLayerStatsComp->isThisSurvivalPlayer ==true ? isRespawning = true: isRespawning = false;
}

// Called to bind functionality to input
void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);


	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &APlayerCharacter::BeginInteract);
	PlayerInputComponent->BindAction("Interact", IE_Released, this, &APlayerCharacter::EndInteract);

	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &APlayerCharacter::StartCrouching);
	PlayerInputComponent->BindAction("Crouch", IE_Released, this, &APlayerCharacter::StopCrouching);
	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &APlayerCharacter::StartSprinting);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &APlayerCharacter::StopSprinting);

	PlayerInputComponent->BindAxis("MoveForward", this, &APlayerCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &APlayerCharacter::MoveRight);
	// draw bow power it up
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &APlayerCharacter::StartFire);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &APlayerCharacter::StopFire);

	PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &APlayerCharacter::StartAiming);
	PlayerInputComponent->BindAction("Aim", IE_Released, this, &APlayerCharacter::StopAiming);

	PlayerInputComponent->BindAction("RequestBuild", IE_Pressed, BuildComponent, &UBuilderComponent::RequestBuild);
	PlayerInputComponent->BindAction("ToggleBuildMode", IE_Pressed, BuildComponent, &UBuilderComponent::ToggleBuildMode);
}

// Called when the game starts or when spawned
void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	/**/
}

void APlayerCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(APlayerCharacter, Killer);
	DOREPLIFETIME(APlayerCharacter, bSprinting);
	DOREPLIFETIME(APlayerCharacter, RangedWeapon);
	DOREPLIFETIME(APlayerCharacter, MeleeWeapon);
	DOREPLIFETIME_CONDITION(APlayerCharacter, bIsAiming, COND_SkipOwner);
}

// Called every frame
void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (GetWorld()->TimeSince(InteractionData.LastInteractionCheckTime) > InteractionCheckFrequency)
	{
		PerformInteractionCheck();
	}

	bSprinting ? PLayerStatsComp->ModifyStamina(-0.5) : PLayerStatsComp->ModifyStamina(+0.01);
	
	if (!IsLocallyControlled())
	{
		if (CameraComponent != nullptr) {
			CameraComponent->DestroyComponent();
			SpringArmComponent->DestroyComponent();
			InventoryPlayerCapture->DestroyComponent();
		}
	}

	if (InventoryPlayerCapture)
	{
		InventoryPlayerCapture->ShowOnlyActors.Add(this);
		if (RangedWeapon)
		{
			InventoryPlayerCapture->ShowOnlyActors.Add(RangedWeapon);
		}

	}

	//LootPlayerInteraction->OnInteract.AddDynamic(this, &APlayerCharacter::BeginLootingPlayer);
	//When the player spawns in with the defualt items
	for (auto& PlayerMesh : PlayerMeshes)
	{
		//UE_LOG(LogTemp, Warning, TEXT("key: %f, value: %d"), PlayerMesh.Key, PlayerMesh.Value->SkeletalMesh);
		NakedMeshes.Add(PlayerMesh.Key, PlayerMesh.Value->SkeletalMesh);
	}
	
	if (IsLocallyControlled())
	{
		const float DesiredFOV = IsAiming() ? 70.f : 100.f;
		CameraComponent->SetFieldOfView(FMath::FInterpTo(CameraComponent->FieldOfView, DesiredFOV, DeltaTime, 10.f));
		if (RangedWeapon->isBow())
		{
			const FVector ADSLocation = RangedWeapon->GetWeaponMesh()->GetSocketLocation(Name_AimSocket);
			const FVector DefaultCameraLocation = GetMesh()->GetSocketLocation(Name_CameraSocket);

			const FVector CameraLoc = bIsAiming ? ADSLocation : DefaultCameraLocation;

			const float InterpSpeed = FVector::Dist(ADSLocation, DefaultCameraLocation) / RangedWeapon->ADSTime;
			CameraComponent->SetWorldLocation(FMath::VInterpTo(CameraComponent->GetComponentLocation(), CameraLoc, DeltaTime, InterpSpeed));
		}
	}
}



void APlayerCharacter::SetActorHiddenInGame(bool bNewHidden)
{
	Super::SetActorHiddenInGame(bNewHidden);

	if (RangedWeapon)
	{
		RangedWeapon->SetActorHiddenInGame(bNewHidden);//SetActorHiddenInGame(bNewHidden);
	}
	if (MeleeWeapon) {
		MeleeWeapon->SetActorHiddenInGame(bNewHidden);
	}
}
void APlayerCharacter::Restart()
{
	Super::Restart();
}

float APlayerCharacter::TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);

	if (!IsAlive())
	{
		return 0.f;
	}

	const float DamageDealt = PLayerStatsComp->ModifyHealth(-Damage);

	UE_LOG(LogTemp, Warning, TEXT("Health: %f"), PLayerStatsComp->GetHealth());

	if (PLayerStatsComp->GetHealth() <= 0.f)
	{
		if (APlayerCharacter* KillerCharacter = Cast<APlayerCharacter>(DamageCauser->GetOwner()))
		{
			KilledByPlayer(DamageEvent, KillerCharacter, DamageCauser);
		}
		else
		{
			OnOtherDeath(DamageEvent, DamageCauser);
		}
	}
	return DamageDealt;
}



void APlayerCharacter::OnRep_Killer()
{
	SetLifeSpan(20.f);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	GetMesh()->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
	GetMesh()->SetOwnerNoSee(false);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECR_Ignore);
	SetReplicatingMovement(false);
	LootPlayerInteraction->Activate();

	// unequip item so they can be looted by another character 
	if (HasAuthority()) {
		TArray<UEquippableItem*> EquippedInvItems;
		EquippedItems.GenerateValueArray(EquippedInvItems);

		for (auto& Equippable : EquippedInvItems)
		{
			Equippable->SetEquipped(false);
		}
	}

	if (IsLocallyControlled())
	{
		SpringArmComponent->TargetArmLength = 500.f;
		SpringArmComponent->AttachToComponent(GetCapsuleComponent(), FAttachmentTransformRules::SnapToTargetIncludingScale);
		bUseControllerRotationPitch = true;

		if (AMyPlayerController* PC = Cast<AMyPlayerController>(GetController()))
		{
			PC->Died(Killer);
		}
	}
}

void APlayerCharacter::OnOtherDeath(FDamageEvent const& DamageEvent, const AActor* DamageCauser)
{
	Killer = this;
	OnRep_Killer();
}

void APlayerCharacter::KilledByPlayer(FDamageEvent const& DamageEvent,
	APlayerCharacter* Character, const AActor* DamageCauser)
{
	Killer = Character;
	OnRep_Killer();
}

void APlayerCharacter::BeginMeleeAttack()
{
	if (GetWorld()->TimeSince(LastMeleeAttackTime) > MeleeAttackMontage->GetPlayLength())
	{
		FHitResult Hit;
		FCollisionShape Shape = FCollisionShape::MakeSphere(15.f);

		FVector StartTrace = CameraComponent->GetComponentLocation();
		FVector EndTrace = (CameraComponent->GetComponentRotation().Vector() * MeleeAttackDistance) + StartTrace;

		FCollisionQueryParams QueryParams = FCollisionQueryParams("MeleeSweep", false, this);

		PlayAnimMontage(MeleeAttackMontage);

		if (GetWorld()->SweepSingleByChannel(Hit, StartTrace, EndTrace, FQuat(), ECC_GameTraceChannel1, Shape, QueryParams))
		{
			if (APlayerCharacter* HitPlayer = Cast<APlayerCharacter>(Hit.GetActor()))
			{
				if (AMyPlayerController* PC = Cast<AMyPlayerController>(GetController()))
				{
					PC->ClientShotHitConfirmed();
				}
			}
		}

		ServerProcessMeleeHit(Hit);

		LastMeleeAttackTime = GetWorld()->GetTimeSeconds();
	}
}

void APlayerCharacter::ServerProcessMeleeHit_Implementation(const FHitResult& MeleeHit)
{
	if (GetWorld()->TimeSince(LastMeleeAttackTime) > MeleeAttackMontage->GetPlayLength() && (GetActorLocation() - MeleeHit.ImpactPoint).Size() <= MeleeAttackDistance)
	{
		MulticastPlayMeleeFX();

		UGameplayStatics::ApplyPointDamage(MeleeHit.GetActor(), MeleeAttackDamage, (MeleeHit.TraceStart - MeleeHit.TraceEnd).GetSafeNormal(), MeleeHit, GetController(), this, UMeleeDamage::StaticClass());
	}
	LastMeleeAttackTime = GetWorld()->GetTimeSeconds();
}

void APlayerCharacter::MulticastPlayMeleeFX_Implementation()
{
	if (!IsLocallyControlled())
	{
		PlayAnimMontage(MeleeAttackMontage);
	}
}


void APlayerCharacter::ServerSetAiming_Implementation(const bool bNewAiming)
{
	SetAiming(bNewAiming);
}


void APlayerCharacter::OnRep_EquippedWeapon()
{
	if (RangedWeapon)
	{
		RangedWeapon->OnEquip();
	} else if (MeleeWeapon)
	{
		MeleeWeapon->OnEquip();
	}
}


bool APlayerCharacter::CanAim() const
{
	return RangedWeapon != nullptr;
}

void APlayerCharacter::StartAiming()
{
	if (CanAim())
	{
		SetAiming(true);
	}
}

void APlayerCharacter::StopAiming()
{
	SetAiming(false);
}

void APlayerCharacter::SetAiming(const bool bNewAiming)
{
	if ((bNewAiming && !CanAim()) || bNewAiming == bIsAiming)
	{
		return;
	}

	if (!HasAuthority())
	{
		ServerSetAiming(bNewAiming);
	}

	bIsAiming = bNewAiming;
}

void APlayerCharacter::StartFire()
{
	if (RangedWeapon)
	{
		RangedWeapon->StartFire();
	}
	else if (MeleeWeapon) {
		MeleeWeapon->StartAttack();
		//GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Green, TEXT("GotMeleeWeapon"));
	}
	else
	{
		BeginMeleeAttack();
	//	GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Green, TEXT("GotNoWeapon"));
	}
}

void APlayerCharacter::StopFire()
{
	if (RangedWeapon)
	{
		RangedWeapon->StopFire();
	}
}

void APlayerCharacter::EquipWeapon(class UWeaponItem* WeaponItem)
{
	if (WeaponItem && WeaponItem->BowClass && HasAuthority())
	{
		if (RangedWeapon)
		{
			UnEquipWeapon();
		}

		//Spawn the weapon in
		FActorSpawnParameters SpawnParams;
		SpawnParams.bNoFail = true;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		SpawnParams.Owner = SpawnParams.Instigator = this;

		if (ABow* Weapon = GetWorld()->SpawnActor<ABow>(WeaponItem->BowClass, SpawnParams))
		{
			Weapon->Item = WeaponItem;

			RangedWeapon = Weapon;
			OnRep_EquippedWeapon();

			Weapon->OnEquip();
		}
	}
}

void APlayerCharacter::UnEquipWeapon()
{
	
	if (HasAuthority() && RangedWeapon)
	{
		RangedWeapon->OnUnEquip();
		RangedWeapon->Destroy();
		RangedWeapon = nullptr;
		OnRep_EquippedWeapon();
	}
}

void APlayerCharacter::EquipMeleeWeapon(UEquippableMeleeWeapon* WeaponItem)
{
	if (WeaponItem && WeaponItem->WeaponClass && HasAuthority())
	{
		if (MeleeWeapon)
		{
			UnEquipMeleeWeapon();
		}

		//Spawn the weapon in
		FActorSpawnParameters SpawnParams;
		SpawnParams.bNoFail = true;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		SpawnParams.Owner = SpawnParams.Instigator = this;

		if (AMeleeWeaponClass* Weapon = GetWorld()->SpawnActor<AMeleeWeaponClass>(WeaponItem->WeaponClass, SpawnParams))
		{
			Weapon->Item = WeaponItem;

			MeleeWeapon = Weapon;
			OnRep_EquippedWeapon();

			Weapon->OnEquip();
		}
	}
}

void APlayerCharacter::UnEquipMeleeWeapon()
{
	if (HasAuthority() && MeleeWeapon)
	{
		MeleeWeapon->OnUnEquip();
		MeleeWeapon->Destroy();
		MeleeWeapon = nullptr;
		OnRep_EquippedWeapon();
	}
}


void APlayerCharacter::StartReload()
{
	if (RangedWeapon)
	{
		RangedWeapon->StartReload();
	}
}

void APlayerCharacter::StartSprinting()
{
	if (PLayerStatsComp->GetStamina() <= 10.f)
	{
		StopSprinting();
	}
	else
		SetSprinting(true);
}

void APlayerCharacter::StopSprinting()
{
	SetSprinting(false);
}

void APlayerCharacter::SetSprinting(const bool bNewSprinting)
{
	if (bNewSprinting == bSprinting)
		return;

	if (GetLocalRole() < ROLE_Authority)
	{
		ServerSetSprinting(bNewSprinting);
	}

	bSprinting = bNewSprinting;

	GetCharacterMovement()->MaxWalkSpeed = bSprinting ? SprintSpeed : WalkSpeed;
}

void APlayerCharacter::StartCrouching()
{
	Crouch();
}

void APlayerCharacter::StopCrouching()
{
	UnCrouch();
}

void APlayerCharacter::MoveForward(float Val)
{
	if (Val != 0.f)
	{
		AddMovementInput(GetActorForwardVector(), Val);
	}
}

void APlayerCharacter::MoveRight(float Val)
{
	if (Val != 0.f)
	{
		AddMovementInput(GetActorRightVector(), Val);
	}
}


void APlayerCharacter::ServerSetSprinting_Implementation(bool i)
{
	SetSprinting(i);
}

bool APlayerCharacter::ServerSetSprinting_Validate(bool)
{
	return true;
}

void APlayerCharacter::PerformInteractionCheck()
{
	if (GetController() == nullptr)
	{
		return;
	}
	InteractionData.LastInteractionCheckTime = GetWorld()->GetTimeSeconds();

	FVector EyesLoc;
	FRotator EyesRot;
	GetController()->GetPlayerViewPoint(EyesLoc, EyesRot);

	FVector TraceStart = EyesLoc;
	FVector TraceEnd = (EyesRot.Vector() * InteractionCheckDistance) + TraceStart;
	FHitResult TraceHit;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	if (GetWorld()->LineTraceSingleByChannel(TraceHit, TraceStart, TraceEnd, ECC_Visibility, QueryParams))
	{
		//Check if we hit an interactable object
		if (TraceHit.GetActor())
		{
			if (UInteractionComponet* InteractionComponent = Cast<UInteractionComponet>(TraceHit.GetActor()->GetComponentByClass(UInteractionComponet::StaticClass())))
			{
				float Distance = (TraceStart - TraceHit.ImpactPoint).Size();
				if (InteractionComponent != GetInteractable() && Distance <= InteractionComponent->InteractionDistance)
				{
					FoundNewInteractable(InteractionComponent);
				}
				else if (Distance > InteractionComponent->InteractionDistance && GetInteractable())
				{
					CouldntFindInteractable();
				}

				return;
			}
		}
	}
	CouldntFindInteractable();
}

void APlayerCharacter::CouldntFindInteractable()
{
	//We've lost focus on an interactable. Clear the timer.
	if (GetWorldTimerManager().IsTimerActive(TimerHandle_Interact))
	{
		GetWorldTimerManager().ClearTimer(TimerHandle_Interact);
	}
	//Tell the interactable we've stopped focusing on it, and clear the current interactable
	if (UInteractionComponet* Interactable = GetInteractable())
	{
		Interactable->EndFocus(this);

		if (InteractionData.bInteractHeld)
		{
			EndInteract();
		}
	}
	InteractionData.ViewedInteractionComponent = nullptr;
}

void APlayerCharacter::FoundNewInteractable(UInteractionComponet* Interactable)
{
	EndInteract();

	if (UInteractionComponet* OldInteractable = GetInteractable())
	{
		OldInteractable->EndFocus(this);
	}

	InteractionData.ViewedInteractionComponent = Interactable;
	Interactable->BeginFocus(this);
}

void APlayerCharacter::BeginInteract()
{
	if (!HasAuthority())
	{
		ServerBeginInteract();
	}

	if (HasAuthority())
	{
		PerformInteractionCheck();
	}

	InteractionData.bInteractHeld = true;

	if (UInteractionComponet* Interactable = GetInteractable())
	{
		Interactable->BeginInteract(this);

		if (FMath::IsNearlyZero(Interactable->InteractionTime))
		{
			Interact();
		}
		else
		{
			GetWorldTimerManager().SetTimer(TimerHandle_Interact, this, &APlayerCharacter::Interact, Interactable->InteractionTime, false);
		}
	}
}

void APlayerCharacter::EndInteract()
{
	if (!HasAuthority())
	{
		ServerEndInteract();
	}

	InteractionData.bInteractHeld = false;

	//clears the timer 
	GetWorldTimerManager().ClearTimer(TimerHandle_Interact);

	if (UInteractionComponet* Interactable = GetInteractable())
	{
		Interactable->EndInteract(this);
	}
}

void APlayerCharacter::Interact()
{
	//clears the timer
	GetWorldTimerManager().ClearTimer(TimerHandle_Interact);
	//interact with the object
	if (UInteractionComponet* Interactable = GetInteractable())
	{
		Interactable->Interact(this);
	}
}

bool APlayerCharacter::IsInteracting() const
{
	return GetWorldTimerManager().IsTimerActive(TimerHandle_Interact);

	

}
float APlayerCharacter::GetRemainingInteractTime() const
{
	return GetWorldTimerManager().GetTimerRemaining(TimerHandle_Interact);
}

void APlayerCharacter::SetLootSource(UInventoryComponent* NewLootSource)
{
	/**If the thing we're looting gets destroyed, we need to tell the client to remove their Loot screen*/
	if (NewLootSource && NewLootSource->GetOwner())
	{
		NewLootSource->GetOwner()->OnDestroyed.AddUniqueDynamic(this, &APlayerCharacter::OnLootSourceOwnerDestroyed);
	}

	if (HasAuthority())
	{
		if (NewLootSource)
		{
			//Looting a player keeps their body alive for an extra 2 minutes to provide enough time to loot their items
			if (APlayerCharacter* Character = Cast<APlayerCharacter>(NewLootSource->GetOwner()))
			{
				Character->SetLifeSpan(120.f);
			}
		}

		LootSource = NewLootSource;
		OnRep_LootSource();
	}
	else
	{
		ServerSetLootSource(NewLootSource);
	}
}

bool APlayerCharacter::IsLooting() const
{
	return LootSource != nullptr;
}

EOverlayState APlayerCharacter::SetOverlayState(EOverlayState i)
{
	EOverlayState PrevState = GetCurrentState();
	PlayerOverlay = i;

	return PlayerOverlay;
}

void APlayerCharacter::LootItem(UItemsObject* ItemToGive)
{
	if (HasAuthority())
	{
		if (PlayerInventory && LootSource && ItemToGive && LootSource->HasItem(ItemToGive->GetClass(), ItemToGive->GetQuantity()))
		{
			const FItemAddResult AddResult = PlayerInventory->TryAddItem(ItemToGive);

			if (AddResult.AmountGiven > 0)
			{
				LootSource->ConsumeItem(ItemToGive, AddResult.AmountGiven);
			}
			else
			{
				//Tell player why they couldn't loot the item
				if (AMyPlayerController* PC = Cast<AMyPlayerController>(GetController()))
				{
					PC->ClientShowNotification(AddResult.ErrorText);
				}
			}
		}
	}
	else
	{
		ServerLootItem(ItemToGive);
	}
}

USkeletalMeshComponent* APlayerCharacter::GetSlotSkeletalMeshComponent(const EEquippableSlot Slot)
{
	if (PlayerMeshes.Contains(Slot))
	{
		return *PlayerMeshes.Find(Slot);
	}
	return nullptr;
}

void APlayerCharacter::ItemAddedToInventory(UItemsObject* Item)
{

}

void APlayerCharacter::ItemRemovedFromInventory(UItemsObject* Item)
{
}

bool APlayerCharacter::EquipItem(UEquippableItem* Item)
{
	EquippedItems.Add(Item->Slot, Item);
	OnEquippedItemsChanged.Broadcast(Item->Slot, Item);
	return true;
}

bool APlayerCharacter::UnEquipItem(UEquippableItem* Item)
{
	if (Item)
	{
		if (EquippedItems.Contains(Item->Slot))
		{
			if (Item == *EquippedItems.Find(Item->Slot))
			{
				EquippedItems.Remove(Item->Slot);
				OnEquippedItemsChanged.Broadcast(Item->Slot, nullptr);
				return true;
			}
		}
	}
	return false;
}

void APlayerCharacter::EquipGear(UGearItem* Gear)
{
	if (USkeletalMeshComponent* GearMesh = GetSlotSkeletalMeshComponent(Gear->Slot))
	{
		GearMesh->SetSkeletalMesh(Gear->Mesh);
		GearMesh->SetMaterial(GearMesh->GetMaterials().Num() - 1, Gear->MaterialInstance);
	}
}

void APlayerCharacter::UnEquipGear(const EEquippableSlot Slot)
{
	if (USkeletalMeshComponent* EquippableMesh = GetSlotSkeletalMeshComponent(Slot))
	{
		if (USkeletalMesh* BodyMesh = *NakedMeshes.Find(Slot))
		{
			EquippableMesh->SetSkeletalMesh(BodyMesh);

			//Put the materials back on the body mesh (Since gear may have applied a different material)
			for (int32 i = 0; i < BodyMesh->Materials.Num(); ++i)
			{
				if (BodyMesh->Materials.IsValidIndex(i))
				{
					EquippableMesh->SetMaterial(i, BodyMesh->Materials[i].MaterialInterface);
				}
			}
		}
		else
		{
			//For some gear like backpacks, there is no naked mesh
			EquippableMesh->SetSkeletalMesh(nullptr);
		}
	}
}
void APlayerCharacter::UseItem(UItemsObject* Item)
{
	if (GetLocalRole() < ROLE_Authority && Item)
	{
		ServerUseItem(Item);
	}

	if (HasAuthority())
	{
		if (PlayerInventory && !PlayerInventory->FindItem(Item))
		{
			return;
		}
	}

	if (Item)
	{
		Item->OnUse(this);
		Item->Use(this);
	}
}

void APlayerCharacter::DropItem(UItemsObject* Item, const int32 Quantity)
{
	if (PlayerInventory && Item && PlayerInventory->FindItem(Item))
	{
		if (!HasAuthority())
		{
			ServerDropItem(Item, Quantity);
			return;
		}

		if (HasAuthority())
		{
			const int32 ItemQuantity = Item->GetQuantity();
			const int32 DroppedQuantity = PlayerInventory->ConsumeItem(Item, Quantity);

			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = this;
			SpawnParams.bNoFail = true;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

			FVector SpawnLocation = GetActorLocation();
			SpawnLocation.Z -= GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

			FTransform SpawnTransform(GetActorRotation(), SpawnLocation);

			ensure(PickupClass);

			if (APickUp* Pickup = GetWorld()->SpawnActor<APickUp>(PickupClass, SpawnTransform, SpawnParams))
			{
				Pickup->InitializePickup(Item->GetClass(), DroppedQuantity);
			}
		}
	}
}

void APlayerCharacter::CraftItem(UItemsObject* Item)
{
	if (PlayerInventory && Item)
	{
		if (GetLocalRole() < ROLE_Authority)
		{
			ServerCraftItem(Item);
			return;
		}
		if (Item)
		{
			Item->Craft(this);
		}
	}
}



void APlayerCharacter::BeginLootingPlayer(APlayerCharacter* Character) 
{
	if (Character)
	{
		Character->SetLootSource(PlayerInventory);
	}
}

void APlayerCharacter::OnLootSourceOwnerDestroyed(AActor* DestroyedActor)
{
	//Remove loot source 
	if (HasAuthority() && LootSource && DestroyedActor == LootSource->GetOwner())
	{
		ServerSetLootSource(nullptr);
	}
}

void APlayerCharacter::OnRep_LootSource()
{
	//Bring up or remove the looting menu 
	if (AMyPlayerController* PC = Cast<AMyPlayerController>(GetController()))
	{
		if (PC->IsLocalController())
		{
			if (APlayerHUD* HUD = Cast<APlayerHUD>(PC->GetHUD()))
			{
				if (LootSource)
				{
					HUD->OpenLootWidget();
				}
				else
				{
					HUD->CloseLootWidget();
				}
			}
		}
	}
}

void APlayerCharacter::ServerCraftItem_Implementation(UItemsObject* Item)
{
	CraftItem(Item);
}

bool APlayerCharacter::ServerCraftItem_Validate(UItemsObject* Item)
{
	return true;
}

void APlayerCharacter::ServerSetLootSource_Implementation(UInventoryComponent* NewLootSource)
{
	SetLootSource(NewLootSource);
}

bool APlayerCharacter::ServerSetLootSource_Validate(UInventoryComponent* NewLootSource)
{
	return true;
}

void APlayerCharacter::ServerDropItem_Implementation(UItemsObject* Item, int32 Quantity)
{
	DropItem(Item, Quantity);
}

bool APlayerCharacter::ServerDropItem_Validate(UItemsObject* Item, int32)
{
	return true;
}

void APlayerCharacter::ServerUseItem_Implementation(UItemsObject* Item)
{
	UseItem(Item);
}

bool APlayerCharacter::ServerUseItem_Validate(UItemsObject* Item)
{
	return true;
}

void APlayerCharacter::ServerLootItem_Implementation(UItemsObject* ItemToLoot)
{
	LootItem(ItemToLoot);
}

bool APlayerCharacter::ServerLootItem_Validate(UItemsObject* ItemToLoot)
{
	return true;
}

void APlayerCharacter::ServerEndInteract_Implementation()
{
	EndInteract();
}

bool APlayerCharacter::ServerEndInteract_Validate()
{

	return true;
}

void APlayerCharacter::ServerBeginInteract_Implementation()
{
	BeginInteract();
}

bool APlayerCharacter::ServerBeginInteract_Validate()
{
	return true;
}
#undef LOCTEXT_NAMESPACE


