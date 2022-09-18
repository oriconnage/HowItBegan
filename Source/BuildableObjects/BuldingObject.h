// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "BuldingObject.generated.h"

/**
 * Base Class for different Types of build able objects.
 */
UCLASS(Abstract, Blueprintable, EditInlineNew, DefaultToInstanced)
class HOWITBEGAN_API UBuldingObject : public UObject
{
	GENERATED_BODY()

protected:

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool IsSupportedForNetworking() const override;
	virtual class UWorld* GetWorld() const override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
public:

	UBuldingObject();

	//This is used as a template to get the material
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced)
	class UMaterialItems* ItemTemplate;

	UPROPERTY(Transient)
	class UWorld* World;

	/**The mesh to display for this items pickup*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Building")
	class UStaticMesh* BuildMesh;

	/**The mesh to display for this items pickup*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Building")
	class UStaticMesh* PreviewBuildMesh;

	/**The thumbnail for this item*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Building")
	class UTexture2D* Thumbnail;

	/**The display name for this item in the inventory*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Building")
	FText DisplayName;

	/**An optional description for the item*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Building", meta = (MultiLine = true))
	FText Description;

	
	/**The weight of the item*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item", meta = (ClampMin = 0.0))
	int32 materialAmount;


	/**The maximum size that a stack of items can be*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item", meta = (ClampMin = 2, EditCondition = bStackable))
		int32 MaxStackSize;

	/**The tooltip in the inventory for this item
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item")
	TSubclassOf<class UItemTooltip_Widget> ItemTooltip;

	/**The tooltip in the inventory for this item*
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item")
	TSubclassOf<class UItemsObject> MaterialItem;

	/**The inventory that owns this item*/
	UPROPERTY()
		class UInventoryComponent* OwningInventory;

	/**Used to efficiently replicate inventory items
	UPROPERTY()
		int32 RepKey;

	UFUNCTION()
		void OnRep_Quantity();

	UFUNCTION(BlueprintCallable, Category = "Item")
		void SetQuantity(const int32 NewQuantity);

	UFUNCTION(BlueprintPure, Category = "Item")
		FORCEINLINE int32 GetQuantity() const { return Quantity; }


	UFUNCTION(BlueprintCallable, Category = "Item")
		FORCEINLINE bool IsStackFull() const { return Quantity >= MaxStackSize; }

	UFUNCTION(BlueprintPure, Category = "Item")
		virtual bool ShouldShowInInventory() const;

	UFUNCTION(BlueprintImplementableEvent)
		void OnUse(class APlayerCharacter* Character);

	virtual void Use(class APlayerCharacter* Character);
	virtual void AddedToInventory(class UInventoryComponent* Inventory);

	/**Mark the object as needing replication. We must call this internally after modifying any replicated properties*/
	void MarkDirtyForReplication();

protected:

	/**The amount of the item*/
	//UPROPERTY(ReplicatedUsing = OnRep_Quantity, EditAnywhere, Category = "Item", meta = (UIMin = 1, EditCondition = bStackable))
		int32 Quantity;
	
};
