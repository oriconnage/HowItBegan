// Fill out your copyright notice in the Description page of Project Settings.


#include "MyPlayerController.h"
#include "PlayerCharacter.h"
#include "../Widget/PlayerHUD.h"
#include "Kismet/GameplayStatics.h"
#include "../WhenitBeganStateBase.h"
#define LOCTEXT_NAMESPACE "PlayerController"
/*
void AMyPlayerController::SetupInputComponent(){
	Super::SetupInputComponent();
	InputComponent->BindAxis("Turn", this, &AMyPlayerController::Turn);
	InputComponent->BindAxis("LookUp", this, &AMyPlayerController::LookUp);
	InputComponent->BindAction("ToggleInventory", IE_Pressed, this, &AMyPlayerController::OpenInventory);
	InputComponent->BindAction("Pause", IE_Pressed, this, &AMyPlayerController::PauseGame);
	InputComponent->BindAction("Reload", IE_Pressed, this, &AMyPlayerController::StartReload);
}

void AMyPlayerController::BeginPlay()
{
	Super::BeginPlay();
}

void AMyPlayerController::ClientShotHitConfirmed_Implementation()
{
	if (APlayerHUD* HUD = Cast<APlayerHUD>(GetHUD()))
	{
		HUD->ShowHitmarker();
	}
}

void AMyPlayerController::Respawn()
{
	UnPossess();
	ChangeState(NAME_Inactive);

	if (GetLocalRole()< ROLE_Authority)
	{
		ServerRespawn();
	}
	else
	{
		ServerRestartPlayer();
	}
}



bool AMyPlayerController::IsInventoryOpen() const
{
	if (APlayerHUD* HUD = Cast<APlayerHUD>(GetHUD()))
	{
		return HUD->IsInventoryOpen();
	}

	return false;
}

void AMyPlayerController::ServerRespawn_Implementation()
{
	Respawn();
}

bool AMyPlayerController::ServerRespawn_Validate()
{
	return true;
}

void AMyPlayerController::OpenInventory()
{
	if (APlayerHUD* HUD = Cast<APlayerHUD>(GetHUD()))
	{
		HUD->OpenInventoryWidget();
	}
}



void AMyPlayerController::PauseGame()
{
	//if (ASurvivalHUD* HUD = Cast<ASurvivalHUD>(GetHUD()))
	//{
	//	HUD->OpenPauseWidget();
	//}
}

void AMyPlayerController::ResumeGame()
{
	//if (ASurvivalHUD* HUD = Cast<ASurvivalHUD>(GetHUD()))
	//{
	//	HUD->ClosePauseWidget();
	//}
}

void AMyPlayerController::ApplyRecoil(const FVector2D& RecoilAmount, const float RecoilSpeed, const float RecoilResetSpeed)
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

AMyPlayerController::AMyPlayerController()
{
	
}

void AMyPlayerController::Turn(float Rate)
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

void AMyPlayerController::LookUp(float Rate)
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

void AMyPlayerController::StartReload()
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


void AMyPlayerController::ClientShowNotification_Implementation(const FText& Message)
{
	if (APlayerHUD* HUD = Cast<APlayerHUD>(GetHUD()))
	{
		HUD->ShowNotification(Message);
	}
}

void AMyPlayerController::Died(APlayerCharacter* killer)
{
	if (AWhenitBeganStateBase* GS = Cast<AWhenitBeganStateBase>(UGameplayStatics::GetGameState(GetWorld())))
	{
		//Force the player to respawn
		FTimerHandle DummyHandle;
		GetWorldTimerManager().SetTimer(DummyHandle, this, &AMyPlayerController::Respawn, GS->RespawnTimeSpan, false);

		if (APlayerHUD* HUD = Cast<APlayerHUD>(GetHUD()))
		{
			//HUD->ShowDeathWidget(killer);
		}
	}
}

/**


AMyPlayerController::AMyPlayerController()
{
}

void AMyPlayerController::Turn(float Rate)
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

void AMyPlayerController::LookUp(float Rate)
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

void AMyPlayerController::Died(APlayerCharacter* killer)
{
	if (AWhenitBeganStateBase* GS = Cast<AWhenitBeganStateBase>(UGameplayStatics::GetGameState(GetWorld())))
	{
		//Force the player to respawn
		FTimerHandle DummyHandle;
		GetWorldTimerManager().SetTimer(DummyHandle, this, &AMyPlayerController::Respawn, GS->RespawnTimeSpan, false);

		if (APlayerHUD* HUD = Cast<APlayerHUD>(GetHUD()))
		{
			//HUD->ShowDeathWidget(killer);
		}
	}
}

void AMyPlayerController::Respawn()
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

bool AMyPlayerController::IsInventoryOpen() const
{
	if (APlayerHUD* HUD = Cast<APlayerHUD>(GetHUD()))
	{
		return HUD->IsInventoryOpen();
	}

	return false;
}

void AMyPlayerController::BeginPlay()
{
}

void AMyPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	InputComponent->BindAxis("Turn", this, &AMyPlayerController::Turn);
	InputComponent->BindAxis("LookUp", this, &AMyPlayerController::LookUp);
	InputComponent->BindAction("ToggleInventory", IE_Pressed, this, &AMyPlayerController::OpenInventory);
	InputComponent->BindAction("Pause", IE_Pressed, this, &AMyPlayerController::PauseGame);
	InputComponent->BindAction("Reload", IE_Pressed, this, &AMyPlayerController::StartReload);
	
}

void AMyPlayerController::StartReload()
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

void AMyPlayerController::OpenInventory()
{
	if (APlayerHUD* HUD = Cast<APlayerHUD>(GetHUD()))
	{
		HUD->OpenInventoryWidget();
	}
}

void AMyPlayerController::PauseGame()
{
}

void AMyPlayerController::ResumeGame()
{
}

void AMyPlayerController::ApplyRecoil(const FVector2D& RecoilAmount, const float RecoilSpeed, const float RecoilResetSpeed)
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

void AMyPlayerController::ServerRespawn_Implementation()
{
	Respawn();
}

bool AMyPlayerController::ServerRespawn_Validate()
{
	return true;
}

void AMyPlayerController::ClientShotHitConfirmed_Implementation()
{
	if (APlayerHUD* HUD = Cast<APlayerHUD>(GetHUD()))
	{
		HUD->ShowHitmarker();
	}
}

void AMyPlayerController::ClientShowNotification_Implementation(const FText& Message)
{
	if (APlayerHUD* HUD = Cast<APlayerHUD>(GetHUD()))
	{
		HUD->ShowNotification(Message);
	}
}
**/



#undef LOCTEXT_NAMESPACE

