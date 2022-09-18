// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ItemTooltip_Widget.generated.h"

/**
 * 
 */
UCLASS()
class HOWITBEGAN_API UItemTooltip_Widget : public UUserWidget
{
	GENERATED_BODY()
public:

	/**The item this tooltip should display*/
	UPROPERTY(BlueprintReadOnly, Category = "Tooltip", meta = (ExposeOnSpawn = true))
	class UItemsObject* TooltipItem;
};
