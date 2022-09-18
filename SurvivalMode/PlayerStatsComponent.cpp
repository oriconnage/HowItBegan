// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerStatsComponent.h"

#include "SurvivalPlayerCharacter.h"
#include "Net/UnrealNetwork.h"
#include "Engine/Engine.h"

// Sets default values for this component's properties
UPlayerStatsComponent::UPlayerStatsComponent()
{
	//Set up 
	MaxHealth = 100.0f;
	MaxHunger = 100.0f;
	MaxStamina = 100.0f;
	MaxThrist = 100.0f;

	
	Health = MaxHealth;
	Hunger = MaxHunger;
	Stamina = MaxStamina;
	Thirst = MaxThrist;

	ThirstDelta = -0.5;
	HungerDelta = -0.3;

	isThisSurvivalPlayer = false;
	
	SetIsReplicatedByDefault(true);
	PrimaryComponentTick.bCanEverTick = true;
}


float UPlayerStatsComponent::ModifyHealth(const float Delta)
{
	const float oldHealth = Health;

	Health = FMath::Clamp<float>(Health + Delta, 0.f, MaxHealth);
	return Health - oldHealth;
}

float UPlayerStatsComponent::ModifyStamina(const float Delta)
{
	const float oldStamina = Stamina;

	Stamina = FMath::Clamp<float>(Stamina + Delta, 0.f, MaxStamina);
	return Stamina - oldStamina;
}

float UPlayerStatsComponent::ModifyHunger( float Delta)
{
	const float oldHunger = Hunger;

	Hunger = FMath::Clamp<float>(Hunger + Delta, -100.f, MaxHunger);
	return Hunger - oldHunger;
}

float UPlayerStatsComponent::ModifyThirst( float Delta)
{
	const float oldThirst = Thirst;

	Thirst = FMath::Clamp<float>(Thirst + Delta, -100.f, MaxThrist);
	return Thirst - oldThirst;
}

void UPlayerStatsComponent::HandlePLayerStats()
{
	
	 ModifyHunger(HungerDelta);
	 ModifyThirst(ThirstDelta);
}

void UPlayerStatsComponent::OnRep_Health(float OldHealth)
{
	OnHealthModified(Health - OldHealth);
}

void UPlayerStatsComponent::OnRep_Stamina(float OldStamina)
{
	OnStaminaModified(Stamina - OldStamina);
}

void UPlayerStatsComponent::OnRep_Hunger(float OldHunger)
{
	OnHungerModified(Hunger - OldHunger);
}

void UPlayerStatsComponent::OnRep_Thirst(float OldThirst)
{
	OnThirsthModified(Thirst - OldThirst);
}

// Called when the game starts
void UPlayerStatsComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	if (isThisSurvivalPlayer) 
	{
		GetWorld()->GetTimerManager().SetTimer(TimeHandler_Status, this, &UPlayerStatsComponent::HandlePLayerStats, 3.0f, true);
	}
}

void UPlayerStatsComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UPlayerStatsComponent, Thirst);
	DOREPLIFETIME(UPlayerStatsComponent, Hunger);
	DOREPLIFETIME(UPlayerStatsComponent, Health);
	DOREPLIFETIME(UPlayerStatsComponent, Stamina);
}


// Called every frame
void UPlayerStatsComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	//Penalty for low health 
	(Health < 50) ? MaxStamina = 50 : MaxStamina = 100;
	
	if (isThisSurvivalPlayer) {
		if (APlayerCharacter* Character = Cast<APlayerCharacter>(GetOwner())) {
			
			if (Hunger <= 0)
			{
				Character->Damage((Hunger * -2), FDamageEvent(), Character->GetController(), Character);
			}
			if (Thirst <= 0)
			{
				Character->Damage((Thirst * -2), FDamageEvent(), Character->GetController(), Character);
			}
		}
	}
}

