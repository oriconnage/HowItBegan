// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterBaseClass.h"



#include "GeneratedCodeHelpers.h"
#include "MyPlayerController.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SceneCaptureComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/SpringArmComponent.h"
#include "HowItBegan/Components/InteractionComponet.h"
#include "HowItBegan/Components/InventoryComponent.h"
#include "Net/UnrealNetwork.h"
#include "../item/EquippableItem.h"
#include "HowItBegan/Worlds/PickUp.h"
#include "HowItBegan/item/GearItem.h"
#include"HowItBegan/item/ItemsObject.h"
#include "HowItBegan/item/WeaponItem.h"
#include "HowItBegan/SurvivalMode/PlayerStatsComponent.h"
#include "Engine/Engine.h"

#define LOCTEXT_NAMESPACE "PlayerCharacterBase"

static FName Name_CameraSocket_1("headSocket");
ACharacterBaseClass::ACharacterBaseClass()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	SetReplicateMovement(true);
	SetReplicates(true);
	bAlwaysRelevant = true;
}

#undef LOCTEXT_NAMESPACE


