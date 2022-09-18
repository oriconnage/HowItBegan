// Fill out your copyright notice in the Description page of Project Settings.


#include "Buildable.h"
#include "components/BoxComponent.h"
#include "engine/CollisionProfile.h"
#include <Runtime/Engine/Classes/Engine/SkeletalMesh.h>
#include <Animation/AnimationAsset.h>
#include "GameFramework/CharacterMovementComponent.h"
#include "../Player/PlayerCharacter.h"
#include "../Components/BuilderComponent.h"
#include <Engine/StaticMesh.h>

// Sets default values
ABuildable::ABuildable()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SetReplicates(true);

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

	// setting up preview mesh
	PreviewMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("PreviewMesh"));
	PreviewMesh->SetupAttachment(RootComponent);
	//PreviewMeshMeshcompo->SetVisibility(true);
	//build mesh setup
	BuildMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("BuildMesh"));
	BuildMesh->SetupAttachment(RootComponent);
	BuildMesh->SetVisibility(false);

	//collision volume setup
	CollisionVol = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionVolume"));
	CollisionVol->SetupAttachment(RootComponent);
	CollisionVol->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);

	BuildAnimation = CreateDefaultSubobject<UAnimationAsset>(TEXT("Build"));
	DestroyAnimation = CreateDefaultSubobject<UAnimationAsset>(TEXT("destroy"));
}


// Called when the game starts or when spawned
void ABuildable::BeginPlay()
{
	Super::BeginPlay();
	
}

void ABuildable::SetQuantity(const int32 NewQuantity)
{

}


// Called every frame
void ABuildable::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}
void ABuildable::build()
{
	if (!HasAuthority())
	{
		Severbuild();
	}
	
	BuildMesh->SetVisibility(true);
	BuildMesh->PlayAnimation(BuildAnimation, false);
	CollisionVol->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
	PreviewMesh->PlayAnimation(DestroyAnimation,false);
	PreviewMesh->DestroyComponent(true);
}

void ABuildable::Severbuild_Implementation()
{
	build();
}

bool ABuildable::Severbuild_Validate()
{
	return true;
}

void ABuildable::MarkDirtyForReplication()
{

}

