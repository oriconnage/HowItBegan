// Fill out your copyright notice in the Description page of Project Settings.


#include "BuldingObject.h"

void UBuldingObject::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

bool UBuldingObject::IsSupportedForNetworking() const
{
	return true;
}

class UWorld* UBuldingObject::GetWorld() const
{
	return World;
}

void UBuldingObject::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{

}

UBuldingObject::UBuldingObject()
{

}
