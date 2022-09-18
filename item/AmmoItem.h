// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EquippableItem.h"
#include "AmmoItem.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class HOWITBEGAN_API UAmmoItem : public UEquippableItem
{
	GENERATED_BODY()

public:
	UAmmoItem();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	TSubclassOf<class AArrow> ArrowClass;
	

};
