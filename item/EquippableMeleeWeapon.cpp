// Fill out your copyright notice in the Description page of Project Settings.


#include "EquippableMeleeWeapon.h"
#include "../Player/PlayerCharacter.h"
#include"../Player/MyPlayerController.h"

UEquippableMeleeWeapon::UEquippableMeleeWeapon()
{

}

bool UEquippableMeleeWeapon::Equip(class APlayerCharacter* Character)
{
	bool bEquipSuccessful = Super::Equip(Character);

	if (bEquipSuccessful && Character)
	{
		Character->EquipMeleeWeapon(this);
	}

	return bEquipSuccessful;
}

bool UEquippableMeleeWeapon::UnEquip(class APlayerCharacter* Character)
{
	bool bUnEquipSuccessful = Super::UnEquip(Character);

	if (bUnEquipSuccessful && Character)
	{
		Character->UnEquipMeleeWeapon();
	}

	return bUnEquipSuccessful;
}