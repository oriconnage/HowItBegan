// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include <Runtime/CoreUObject/Public/UObject/ObjectMacros.h>
#include "BuilderComponent.generated.h"

/* 
*Going to create a Separate Inventory for Crafting Materials 
* that will show in in the crafting materials 
* Crafting Items 
* 
* 
*/


USTRUCT()
struct FCheckingData
{
	GENERATED_BODY()

		FCheckingData()
	{
		//HitTarget = nullptr;
		LastCheckTime = 0.f;
		bIsBuilded = false;
	}

	//The current interactable component we're viewing, if there is one
	UPROPERTY()
	class ABuildable* HitTarget;

	//The time when we last checked for an interactable
	UPROPERTY()
		float LastCheckTime;

	//Whether the local player is holding the interact key
	UPROPERTY()
		bool bIsBuilded;

};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HOWITBEGAN_API UBuilderComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UBuilderComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "Building")
	void ToggleBuildMode();
	
	UFUNCTION(BlueprintCallable, Category = "Building")
	void RequestBuild();

	bool  PerformFloorCheck();

	void onBuild(class ACharacterBaseClass* Thischaracter);
private:
	// Getting the location 
	FVector GetNextNodePos() const;
	FRotator GetNextRotation() const;



	//Ref to the player Camera
	class UCameraComponent* PlayerCamera;

	class ABuildable* currrentBuild;


public:
	// checking if building
	bool isBuilding;
	//Variables
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
	float GridSize = 100.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
	float BuildDist = 500.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
	float FloorPosition = 130.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
	TSubclassOf<class ABuildable> BuildableItems;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player")
	TSubclassOf<class ACharacterBaseClass> player;

};
