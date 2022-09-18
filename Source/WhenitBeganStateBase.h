// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "WhenitBeganStateBase.generated.h"

/**
 * 
 */
UCLASS()
class HOWITBEGAN_API AWhenitBeganStateBase : public AGameStateBase
{
	GENERATED_BODY()

public:
	AWhenitBeganStateBase();

	UPROPERTY(EditDefaultsOnly,Category = "Game Rules")
	float RespawnTimeSpan;
	
};
