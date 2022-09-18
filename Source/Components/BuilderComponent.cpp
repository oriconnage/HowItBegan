// Fill out your copyright notice in the Description page of Project Settings.


#include "BuilderComponent.h"
#include "Engine/Engine.h"
#include "Camera/CameraComponent.h"
#include"../BuildableObjects/Buildable.h"
#include "DrawDebugHelpers.h"
#include "../Player/PlayerCharacter.h"
#include "../player/MyPlayerController.h"
#include "GameFramework/PlayerController.h"
//#include <Runtime/Engine/Classes/Kismet/KismetSystemLibrary.h>

// Sets default values for this component's properties
UBuilderComponent::UBuilderComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	
	isBuilding = false;
	
	// ...
}

// Called when the game starts
void UBuilderComponent::BeginPlay()
{
	Super::BeginPlay();
	PlayerCamera = GetOwner()->FindComponentByClass<UCameraComponent>();
	if (PlayerCamera == nullptr) {
		UE_LOG(LogTemp,Error,TEXT("PLayer camera not found."))
	}
}


// Called every frame
void UBuilderComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// checking if we're in build mode 
	// if yes get the rotation and node pos and draw it 
	if (isBuilding)
	{
			FVector Location = GetNextNodePos();
			FRotator Rotation = GetNextRotation();

			if (currrentBuild == nullptr)
			{
				//create current build
				currrentBuild = GetWorld()->SpawnActor<ABuildable>(BuildableItems, Location, Rotation, FActorSpawnParameters());
			}
			else {
				currrentBuild->SetActorLocationAndRotation(Location, Rotation);
				//currrentBuild->
			}
		//DrawDebugBox(GetWorld(), Location, FVector(100, 100, 100), Rotation.Quaternion(), FColor::Blue,false,0,0,10);
	}else if(currrentBuild != nullptr)
	{
		currrentBuild->Destroy();
		currrentBuild = nullptr;
	}
}

void UBuilderComponent::ToggleBuildMode()
{
	isBuilding = !isBuilding;

/*	if(APlayerCharacter* OwnerCharacter = Cast<APlayerCharacter>(GetOwner()))
	{
		if(OwnerCharacter && OwnerCharacter->IsLocallyControlled())
		{
			if(AMyPlayerController* OwnerController = Cast<AMyPlayerController>(OwnerCharacter->GetController()))
			{
				//OwnerController->ToggleBuildMode();
			}
		}
	}*/
	GEngine->AddOnScreenDebugMessage(-1, 15, FColor::Green, 
	FString::Printf( TEXT("ToggleBuildMode -> %d"),isBuilding));
}

void UBuilderComponent::RequestBuild()
{
	if (isBuilding == !isBuilding || currrentBuild ==nullptr)
	{
		return;
	}
	GEngine->AddOnScreenDebugMessage(-1, 15, FColor::Green, TEXT("RequestBuild"));
	currrentBuild->build();
	currrentBuild = nullptr;
}

bool UBuilderComponent::PerformFloorCheck()
{
	return false;
}

void UBuilderComponent::onBuild(class ACharacterBaseClass* Thischaracter)
{
	if (Thischaracter->GetController() == nullptr) {
		return;
	}

}

FVector UBuilderComponent::GetNextNodePos() const
{


	FHitResult OutHit;
	FVector EyesLoc = PlayerCamera->GetComponentLocation();
	FRotator EyeRot;
	//Thischaracter->GetController()->GetPlayerViewPoint(EyesLoc, EyeRot);

	FVector Start = EyesLoc;
	FVector directionVector = PlayerCamera->GetForwardVector();
	FVector End = ((directionVector * BuildDist) + Start);

	FCollisionQueryParams Queryparams;
	// ignore the player 
	//Queryparams.AddIgnoredActor();

	bool Hit = GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECC_Visibility, Queryparams);

	//checking if we hit an Object
	if (Hit) {
		if (OutHit.GetActor())
		{
			return OutHit.Location.GridSnap(GridSize);
		}
		else
		{
			return End;
		}
	}
	else {
		return End;
	}

	/**
	directionVector += GetOwner()->GetActorLocation();
	return FVector(
		FMath::GridSnap(directionVector.X, GridSize),
		FMath::GridSnap(directionVector.Y, GridSize),
		FloorPosition
	);
	**/
}

FRotator UBuilderComponent::GetNextRotation() const
{
	FRotator rotation = PlayerCamera->GetComponentRotation();
	return FRotator(0.f, FMath::GridSnap(rotation.Yaw, 90.f),0);
}

/*

*/