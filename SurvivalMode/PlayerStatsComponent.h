// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerStatsComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HOWITBEGAN_API UPlayerStatsComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UPlayerStatsComponent();

	//Define Health  Variables 
	UPROPERTY(ReplicatedUsing = OnRep_Health, BlueprintReadOnly, Category = "Status")
	float Health;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Status")
	float MaxHealth;

	// Stamina
	UPROPERTY(ReplicatedUsing = OnRep_Stamina, BlueprintReadOnly, Category = "Status")
	float Stamina;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Status")
	float MaxStamina;

	UPROPERTY(ReplicatedUsing = OnRep_Hunger, BlueprintReadOnly, Category = "Status")
	float Hunger;
	UPROPERTY(EditAnywhere,Category ="Status")
	 float HungerDelta;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Status")
	float MaxHunger;

	UPROPERTY(ReplicatedUsing = OnRep_Thirst, BlueprintReadOnly, Category = "Status")
	float Thirst;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Status")
	float MaxThrist;
	
	UPROPERTY(EditAnywhere, Category = "Status")
	 float ThirstDelta;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Status")
	bool isThisSurvivalPlayer;
	
	FTimerHandle TimeHandler_Status;
	
protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
public:
	//Modify the players health by either a negative or positive amount. Return the amount of health actually removed
	float ModifyHealth(const float Delta);
	float ModifyStamina(const float Delta);
	float ModifyHunger( float Delta);
	float ModifyThirst( float Delta);

	void HandlePLayerStats();
	
	UFUNCTION()
	void OnRep_Health(float OldHealth);

	UFUNCTION()
	void OnRep_Stamina(float OldStamina);

	UFUNCTION()
	void OnRep_Hunger(float OldHealth);

	UFUNCTION()
	void OnRep_Thirst(float OldStamina);

	UFUNCTION(BlueprintImplementableEvent)
	void OnHealthModified(const float HealthDelta);

	UFUNCTION(BlueprintImplementableEvent)
	void OnStaminaModified(const float StaminaDelta);

	UFUNCTION(BlueprintImplementableEvent)
	void OnHungerModified(const float Delta);

	UFUNCTION(BlueprintImplementableEvent)
	void OnThirsthModified(const float Delta);

	
	float GetStamina() const {return Stamina;}
	float GetHealth() const	 {return Health;}
	float GetHunger() const  {return Hunger;}
	float GetThirst() const  {return Thirst;}
	
public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};



