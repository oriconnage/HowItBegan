// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DeathWidget.generated.h"

/**
 * 
 */
UCLASS()
class HOWITBEGAN_API UDeathWidget : public UUserWidget
{
	GENERATED_BODY()
public:

	UPROPERTY(BlueprintReadOnly, Category = "HUD")
		class APlayerCharacter* killer;
};
