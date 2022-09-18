// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/DamageType.h"
#include "MeleeDamage.generated.h"

/**
 * 
 */

UCLASS()
class HOWITBEGAN_API UGameBaseDamage : public UDamageType {

	GENERATED_BODY()
};


UCLASS()
class HOWITBEGAN_API UMeleeDamage : public UGameBaseDamage
{
	GENERATED_BODY()
	
};

UCLASS()
class HOWITBEGAN_API UHungerDamage : public UGameBaseDamage
{
	GENERATED_BODY()

};

UCLASS()
class HOWITBEGAN_API UEnviromentDamage : public UGameBaseDamage
{
	GENERATED_BODY()

};

UCLASS()
class HOWITBEGAN_API UWeaponDamage : public UGameBaseDamage {

	GENERATED_BODY()
};