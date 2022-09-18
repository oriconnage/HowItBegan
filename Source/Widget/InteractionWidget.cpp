// Fill out your copyright notice in the Description page of Project Settings.


#include "InteractionWidget.h"

#include "../Components/InteractionComponet.h"

void UInteractionWidget::UpdateInteractionWidget(class UInteractionComponet* InteractionComponent)
{
	OwningInteractionComponent = InteractionComponent;
	OnUpdateInteractionWidget();
}