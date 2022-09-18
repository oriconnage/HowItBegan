// Fill out your copyright notice in the Description page of Project Settings.


#include "SurvivalPlayerCharacter.h"



#include "PlayerStatsComponent.h"
#include "HowItBegan/Components/BuilderComponent.h"
#include "HowItBegan/item/EquippableItem.h"


#define LOCTEXT_NAMESPACE "SurvivalPlayerCharacter"

// Sets default values
ASurvivalPlayerCharacter::ASurvivalPlayerCharacter()
{



	//player Stats Health , stamina , Thirsts , Hunger 
	PLayerStatsComp = CreateDefaultSubobject<UPlayerStatsComponent>("PlayerStats");
	PLayerStatsComp->isThisSurvivalPlayer = true;

}

// Called when the game starts or when spawned
void ASurvivalPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ASurvivalPlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ASurvivalPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	

}

#undef LOCTEXT_NAMESPACE