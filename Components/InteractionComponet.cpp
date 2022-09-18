// Fill out your copyright notice in the Description page of Project Settings.


#include "InteractionComponet.h"
#include"../Player/PlayerCharacter.h"
#include "../Widget/InteractionWidget.h"


UInteractionComponet::UInteractionComponet()
{
	SetComponentTickEnabled(false);

	InteractionTime = 0.f;
	InteractionDistance = 200.f;
	InteractableNameText = FText::FromString("Interactable Object");
	InteractableActionText = FText::FromString("Interact");
	bAllowMultipleInteractors = true;

	Space = EWidgetSpace::Screen;
	DrawSize = FIntPoint(600, 100);
	bDrawAtDesiredSize = true;

	SetActive(true);
	SetHiddenInGame(true);

}

void UInteractionComponet:: SetInteractableNameText(const FText& NewNameText)
{
	InteractableNameText = NewNameText;
	RefreshWidget();
}

void UInteractionComponet:: SetInteractableActionText(const FText& NewActionText)
{
	InteractableActionText = NewActionText;
	RefreshWidget();
}

void UInteractionComponet::Deactivate()
{
	Super::Deactivate();

	for (int32 i = Interactors.Num() - 1; i >= 0; --i)
	{
		if (APlayerCharacter* Interactor = Interactors[i])
		{
			EndFocus(Interactor);
			EndInteract(Interactor);
		}
	}

	Interactors.Empty();
}

bool UInteractionComponet::CanInteract(class APlayerCharacter* Character) const
{
	//multi people can't interact at the same time
	const bool bPlayerAlreadyInteracting = !bAllowMultipleInteractors && Interactors.Num() >= 1;
	return !bPlayerAlreadyInteracting && IsActive() && GetOwner() != nullptr && Character != nullptr;
}

void UInteractionComponet::RefreshWidget()
{
	//Make sure the widget is initialized, and that we are displaying the right values (these may have changed)
	if (class UInteractionWidget* InteractionWidget = Cast<UInteractionWidget>(GetUserWidgetObject()))
	{
		InteractionWidget->UpdateInteractionWidget(this);
	}
}

// when you begin looking at the object 
void UInteractionComponet::BeginFocus(class APlayerCharacter* Character)
{
	if (!IsActive() || !GetOwner() || !Character)
	{
		return;
	}

	OnBeginFocus.Broadcast(Character);

	if (GetNetMode() != NM_DedicatedServer)
	{
		SetHiddenInGame(false);

		for (auto& VisualComp : GetOwner()->GetComponents())
		{
			if (UPrimitiveComponent* Prim = Cast<UPrimitiveComponent>(VisualComp))
			{
				Prim->SetRenderCustomDepth(true);
			}
		}
	}

	RefreshWidget();
}

void UInteractionComponet::EndFocus(class APlayerCharacter* Character)
{
	OnEndFocus.Broadcast(Character);

	if (GetNetMode() != NM_DedicatedServer)
	{
		SetHiddenInGame(true);

		for (auto& VisualComp : GetOwner()->GetComponents())
		{
			if (UPrimitiveComponent* Prim = Cast<UPrimitiveComponent>(VisualComp))
			{
				Prim->SetRenderCustomDepth(false);
			}
		}
	}
}

void UInteractionComponet::BeginInteract(class APlayerCharacter* Character)
{
	if (CanInteract(Character))
	{
		Interactors.AddUnique(Character);
		OnBeginInteract.Broadcast(Character);
	}
}

void UInteractionComponet::EndInteract(class APlayerCharacter* Character)
{
	Interactors.RemoveSingle(Character);
	OnEndInteract.Broadcast(Character);
}

void UInteractionComponet::Interact(class APlayerCharacter* Character)
{
	if (CanInteract(Character))
	{
		OnInteract.Broadcast(Character);
	}
}

float UInteractionComponet::GetInteractPercentage()
{
	if (Interactors.IsValidIndex(0))
	{
		printf("Not Valic");
		if (APlayerCharacter* Interactor = Interactors[0])
		{
			if (Interactor && Interactor->IsInteracting())
			{
				return 1.f - FMath::Abs(Interactor->GetRemainingInteractTime() / InteractionTime);
			}
		}
	}
	return 0.f;
}

