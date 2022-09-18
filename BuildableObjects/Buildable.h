// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Buildable.generated.h"





UCLASS(Abstract)
class HOWITBEGAN_API ABuildable : public AActor
{
	GENERATED_BODY()
	
	friend class UBuilderComponent;
public:	
	// Sets default values for this actor's properties
	ABuildable();
 
	UBuilderComponent* buliderCompo;
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	/*
	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category ="Building")
	class UStaticMesh* BuildMesh;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Building")
	class UStaticMesh* PreviewMesh;
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
	class UBoxComponent* CollisionVol;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components")
	class USkeletalMeshComponent* BuildMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components")
	class USkeletalMeshComponent* PreviewMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	class UAnimationAsset* BuildAnimation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	class UAnimationAsset* DestroyAnimation;

public:	

	UFUNCTION(BlueprintCallable, Category = "Building")
	void SetQuantity(const int32 NewQuantity);
	void Material(class UCraftingItems* item);
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	UFUNCTION(BlueprintCallable, Category = "Building")
	virtual void build();
	UFUNCTION(Server, Reliable, WithValidation)
	virtual void Severbuild();

	/**Mark the object as needing replication. We must call this internally after modifying any replicated properties*/
	void MarkDirtyForReplication();
};
