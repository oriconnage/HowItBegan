// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponItem.h"
#include "../Player/PlayerCharacter.h"
#include"../Player/MyPlayerController.h"

bool UWeaponItem::Equip(class APlayerCharacter* Character)
{
	bool bEquipSuccessful = Super::Equip(Character);

	if (bEquipSuccessful && Character)
	{
		Character->EquipWeapon(this);
	}

	return bEquipSuccessful;
}

bool UWeaponItem::UnEquip(class APlayerCharacter* Character)
{
	bool bUnEquipSuccessful = Super::UnEquip(Character);

	if (bUnEquipSuccessful && Character)
	{
		Character->UnEquipWeapon();
	}

	return bUnEquipSuccessful;
}