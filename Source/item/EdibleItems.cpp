// Fill out your copyright notice in the Description page of Project Settings.


#include "EdibleItems.h"

#include "../Player/PlayerCharacter.h"
#include "../Player/MyPlayerController.h"
#include "../Components/InventoryComponent.h"
#include "HowItBegan/SurvivalMode/PlayerStatsComponent.h"


#define LOCTEXT_NAMESPACE "FoodItem"
void UEdibleItems::Use(APlayerCharacter* Character)
{
	
	UE_LOG(LogTemp, Warning, TEXT("NUM Food Eating"));
	if (Character)
	{
		const float ActualHealedAmount = Character->PLayerStatsComp->ModifyHealth(HealAmount);
		const float ActualStaminaAmount = Character->PLayerStatsComp->ModifyStamina(StaminaAmount);
		const float ActualHungerAmount = Character->PLayerStatsComp->ModifyHunger(Hungeramount);
		const float ActualThirstAmount = Character->PLayerStatsComp->ModifyThirst(ThristAmount);
		
		/*
		 *	Check how much the food actual modify the player stats and if it is close to zero
		 *	that means that player doesn't need to eat 
		 * 
		 */
		const bool bNeedHealth = !FMath::IsNearlyZero(ActualHealedAmount);
		const bool bNeedStamina = !FMath::IsNearlyZero(ActualStaminaAmount);
		const bool bNeedFood = !FMath::IsNearlyZero(ActualHungerAmount);
		const bool bNeedWater = !FMath::IsNearlyZero(ActualThirstAmount);
		if (!Character->HasAuthority())
		{
			if (AMyPlayerController* PC = Cast<AMyPlayerController>(Character->GetController()))
			{
				if (bNeedHealth||bNeedFood||bNeedStamina||bNeedWater)
				{
					PC->ClientShowNotification(FText::Format(LOCTEXT("AteFoodText", "Ate {FoodName}, healed {HealAmount} health."), DisplayName, ActualHealedAmount));
				}
				else
				{
					PC->ClientShowNotification(FText::Format(LOCTEXT("FullHealthText", "No need to eat {FoodName}, health is already full."), DisplayName, HealAmount));
				}
			}
		}

		if (bNeedHealth || bNeedFood || bNeedStamina || bNeedWater)
		{
			if (UInventoryComponent* Inventory = Character->PlayerInventory)
			{
				Inventory->ConsumeItem(this, 1);
			}
		}
	}
	
}


UEdibleItems::UEdibleItems()
{
	HealAmount = 10.f;
	StaminaAmount = 20.f;
	Hungeramount = 10.f;
	ThristAmount = 10.f;
	
	UseActionText = LOCTEXT("ItemUseActionText", "Consume");
}

#undef LOCTEXT_NAMESPACE