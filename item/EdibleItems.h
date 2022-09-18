// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../item/ItemsObject.h"
#include "EdibleItems.generated.h"

/**
 * 
 */
UCLASS()
class HOWITBEGAN_API UEdibleItems : public UItemsObject
{
	GENERATED_BODY()
public:
	UEdibleItems();

	/**The amount for the food to heal*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Healing")
	float HealAmount;

	/**The amount of stamina to restore*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Restoring")
	float StaminaAmount;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Restoring")
	float Hungeramount;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Restoring")
	float ThristAmount;
	
	virtual void Use(class APlayerCharacter* Character) override;


	class UMaterialItems* MatItems;
};
