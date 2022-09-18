// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CraftMenuWidget.generated.h"

/**
 * 
 */
UCLASS()
class HOWITBEGAN_API UCraftMenuWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadOnly, Category = "Craft Item Widget", meta = (ExposeOnSpawn = true))
	TSubclassOf<class UItemsObject>ItemClass;

	
	
};
