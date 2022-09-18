// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "HowItBegan/WhenitBeganStateBase.h"
#include "HowItBegan/Widget/PlayerHUD.h"
#include "Kismet/GameplayStatics.h"
#include "PlayerCharacter.h"
#include "MyPlayerController.generated.h"


UCLASS()
class HOWITBEGAN_API AMyPlayerController : public APlayerController
{
	GENERATED_BODY()
public:

	AMyPlayerController() {};
	// looking function
	void Turn(float Rate)
	{
		//If the player has moved their camera to compensate for recoil we need this to cancel out the recoil reset effect
		if (!FMath::IsNearlyZero(RecoilResetAmount.X, 0.01f))
		{
			if (RecoilResetAmount.X > 0.f && Rate > 0.f)
			{
				RecoilResetAmount.X = FMath::Max(0.f, RecoilResetAmount.X - Rate);
			}
			else if (RecoilResetAmount.X < 0.f && Rate < 0.f)
			{
				RecoilResetAmount.X = FMath::Min(0.f, RecoilResetAmount.X - Rate);
			}
		}

		//Apply the recoil over several frames
		if (!FMath::IsNearlyZero(RecoilBumpAmount.X, 0.1f))
		{
			FVector2D LastCurrentRecoil = RecoilBumpAmount;
			RecoilBumpAmount.X = FMath::FInterpTo(RecoilBumpAmount.X, 0.f, GetWorld()->DeltaTimeSeconds, CurrentRecoilSpeed);

			AddYawInput(LastCurrentRecoil.X - RecoilBumpAmount.X);
		}

		//Slowly reset back to center after recoil is processed
		FVector2D LastRecoilResetAmount = RecoilResetAmount;
		RecoilResetAmount.X = FMath::FInterpTo(RecoilResetAmount.X, 0.f, GetWorld()->DeltaTimeSeconds, CurrentRecoilResetSpeed);
		AddYawInput(LastRecoilResetAmount.X - RecoilResetAmount.X);

		AddYawInput(Rate);
	}
	void LookUp(float Rate)
	{
		if (!FMath::IsNearlyZero(RecoilResetAmount.Y, 0.01f))
		{
			if (RecoilResetAmount.Y > 0.f && Rate > 0.f)
			{
				RecoilResetAmount.Y = FMath::Max(0.f, RecoilResetAmount.Y - Rate);
			}
			else if (RecoilResetAmount.Y < 0.f && Rate < 0.f)
			{
				RecoilResetAmount.Y = FMath::Min(0.f, RecoilResetAmount.Y - Rate);
			}
		}

		//Apply the recoil over several frames
		if (!FMath::IsNearlyZero(RecoilBumpAmount.Y, 0.01f))
		{
			FVector2D LastCurrentRecoil = RecoilBumpAmount;
			RecoilBumpAmount.Y = FMath::FInterpTo(RecoilBumpAmount.Y, 0.f, GetWorld()->DeltaTimeSeconds, CurrentRecoilSpeed);

			AddPitchInput(LastCurrentRecoil.Y - RecoilBumpAmount.Y);
		}

		//Slowly reset back to center after recoil is processed
		FVector2D LastRecoilResetAmount = RecoilResetAmount;
		RecoilResetAmount.Y = FMath::FInterpTo(RecoilResetAmount.Y, 0.f, GetWorld()->DeltaTimeSeconds, CurrentRecoilResetSpeed);
		AddPitchInput(LastRecoilResetAmount.Y - RecoilResetAmount.Y);

		AddPitchInput(Rate);
	}

	UFUNCTION(Client, Reliable, BlueprintCallable)
	void ClientShowNotification(const FText& Message);
	
	void ClientShowNotification_Implementation(const FText& Message)
	{
		if (APlayerHUD* HUD = Cast<APlayerHUD>(GetHUD()))
		{
			HUD->ShowNotification(Message);
		}
	}

	void ClientShotHitConfirmed_Implementation()
	{
		if (APlayerHUD* HUD = Cast<APlayerHUD>(GetHUD()))
		{
			HUD->ShowHitmarker();
		}
	}

	void Died(class APlayerCharacter* killer)
	{
		if ( AWhenitBeganStateBase* GS = Cast<AWhenitBeganStateBase>(UGameplayStatics::GetGameState(GetWorld())))
		{
			//Force the player to respawn
			FTimerHandle DummyHandle;
			GetWorldTimerManager().SetTimer(DummyHandle, this, &AMyPlayerController::Respawn, GS->RespawnTimeSpan, false);

			if (APlayerHUD* HUD = Cast<APlayerHUD>(GetHUD()))
			{
				HUD->ShowDeathWidget(killer);
			}
		}
	}
	
	UFUNCTION(Client, Unreliable)
	void ClientShotHitConfirmed();




	UFUNCTION(BlueprintCallable)
	void Respawn()
	{
		if (APlayerCharacter* ThisCharacter = Cast<APlayerCharacter>(GetPawn()))
		{
			if (ThisCharacter->isRespawning)
			{
				UnPossess();
				ChangeState(NAME_Inactive);

				if (GetLocalRole() < ROLE_Authority)
				{
					ServerRespawn();
				}
				else
				{
					ServerRestartPlayer();
				}
			}
		}
	}

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerRespawn();
	
	void ServerRespawn_Implementation()
	{
		Respawn();
	}

	bool ServerRespawn_Validate()
	{
		return true;
	}
	
	//Only works for the local player.
	UFUNCTION(BlueprintPure)
	bool IsInventoryOpen() const
	{
		if ( APlayerHUD* HUD = Cast<APlayerHUD>(GetHUD()))
		{
			return HUD->IsInventoryOpen();
		}

		return false;
	}
protected:

	virtual void BeginPlay() override
	{
		Super::BeginPlay();
	}
	virtual void SetupInputComponent() override
	{
		Super::SetupInputComponent();
		InputComponent->BindAxis("Turn", this, &AMyPlayerController::Turn);
		InputComponent->BindAxis("LookUp", this, &AMyPlayerController::LookUp);
		InputComponent->BindAction("ToggleInventory", IE_Pressed, this, &AMyPlayerController::OpenInventory);
		InputComponent->BindAction("Pause", IE_Pressed, this, &AMyPlayerController::PauseGame);
		InputComponent->BindAction("Reload", IE_Pressed, this, &AMyPlayerController::StartReload);
	}
	
	void StartReload()
	{
		if (APlayerCharacter* ThisCharacter = Cast<APlayerCharacter>(GetPawn()))
		{
			if (ThisCharacter->IsAlive())
			{
				ThisCharacter->StartReload();
			}
			else // R key should respawn the player if dead
			{
				Respawn();
			}
		}
	}
	void OpenInventory()
	{
		if (APlayerHUD* HUD = Cast<APlayerHUD>(GetHUD()))
		{
			HUD->OpenInventoryWidget();
		}
	}
	
	void PauseGame()
	{
		
	}

	UFUNCTION(BlueprintCallable)
	void ResumeGame()
	{
		
	}
public:

	void ToggleBuildMode()
	{
		if (APlayerHUD* HUD = Cast<APlayerHUD>(GetHUD()))
		{
			HUD->OpenBuildingWidget();
		}
	}

	// Apply offest to the camera so it wiggles
	void ApplyRecoil(const FVector2D& RecoilAmount, const float RecoilSpeed, const float RecoilResetSpeed)
	{
		if (IsLocalPlayerController())
		{
			RecoilBumpAmount += RecoilAmount;
			RecoilResetAmount += -RecoilAmount;

			CurrentRecoilSpeed = RecoilSpeed;
			CurrentRecoilResetSpeed = RecoilResetSpeed;

			LastRecoilTime = GetWorld()->GetTimeSeconds();
		}
	}
	
	//The amount of recoil to apply. We store this in a variable as we smoothly apply the recoil over several frames
	UPROPERTY(VisibleAnywhere, Category = "Recoil")
	FVector2D RecoilBumpAmount;

	//The amount of recoil the gun has had, that we need to reset (After shooting we slowly want the recoil to return to normal.)
	UPROPERTY(VisibleAnywhere, Category = "Recoil")
	FVector2D RecoilResetAmount;

	//The speed at which the recoil bumps up per second
	UPROPERTY(EditDefaultsOnly, Category = "Recoil")
	float CurrentRecoilSpeed;

	//The speed at which the recoil resets per second
	UPROPERTY(EditDefaultsOnly, Category = "Recoil")
	float CurrentRecoilResetSpeed;

	//The last time that we applied recoil
	UPROPERTY(VisibleAnywhere, Category = "Recoil")
	float LastRecoilTime;
};
 