// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../item/EquippableItem.h"
#include "WeaponItem.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class HOWITBEGAN_API UWeaponItem : public UEquippableItem
{
	GENERATED_BODY()
public:
	virtual bool Equip(class APlayerCharacter* Character) override;
	virtual bool UnEquip(class APlayerCharacter* Character) override;

	//The weapon class to give to the player upon equipping this weapon item
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	TSubclassOf<class ABow> BowClass;
};
