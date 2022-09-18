// Fill out your copyright notice in the Description page of Project Settings.


#include "Arrow.h"
#include <Components/SkeletalMeshComponent.h>
#include <GameFramework/ProjectileMovementComponent.h>
#include <Components/SphereComponent.h>

// Sets default values
AArrow::AArrow()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	ArrowMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ArrowMesh"));
	ArrowMesh->SetSimulatePhysics(true);
	SetRootComponent(ArrowMesh);


	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	ProjectileMovement->InitialSpeed = 1000.f;


	SetReplicateMovement(true);
	SetReplicates(true);
}

void AArrow::InitializeArrow(const TSubclassOf<class UAmmoItem> AmmoClass, const int32 Quantity)
{

}

// Called when the game starts or when spawned
void AArrow::BeginPlay()
{
	Super::BeginPlay();
	
}



