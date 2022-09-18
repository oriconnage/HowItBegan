// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ItemsObject.generated.h"

class UItemsObject;
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnItemModified);


UENUM(BlueprintType)
enum class EItemRarity : uint8
{
	IR_Common UMETA(DisplayName = "Common"),
	IR_Uncommon UMETA(DisplayName = "Uncommon"),
	IR_Rare UMETA(DisplayName = "Rare"),
	IR_VeryRare UMETA(DisplayName = "Very Rare"),
	IR_Legendary UMETA(DisplayName = "Legendary")
};

USTRUCT(BlueprintType)
struct FMaterialOne
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditDefaultsOnly, Category = "Crafting")
	int32 Quantiy;

	UPROPERTY(EditDefaultsOnly, Category = "Crafting")
	TSubclassOf<class UItemsObject> MaterialClass;

	/** defaults */
	FMaterialOne()
	{
		Quantiy = 1;
	}
};

UCLASS(Abstract, Blueprintable, EditInlineNew, DefaultToInstanced)
class HOWITBEGAN_API UItemsObject : public UObject
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

	UItemsObject();

	UPROPERTY(Transient)
	class UWorld* World;

	/**The mesh to display for this items pickup*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item")
	class UStaticMesh* PickupMesh;

	/**The thumbnail for this item*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	class UTexture2D* Thumbnail;

	/**The display name for this item in the inventory*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item")
	FText DisplayName;

	/**An description for the item*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item", meta = (MultiLine = true))
	FText Description;

	/**The text for using the item. (Equip, Eat, etc)*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item")
	FText UseActionText;

	/**The rarity of the item*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item")
	EItemRarity Rarity;

	/**The weight of the item*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item", meta = (ClampMin = 0.0))
	float Weight;

	/**Whether or not this item can be stacked*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item")
	bool bStackable;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item")
	bool bIsCraftable;

	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item")
	 FMaterialOne MaterialItems;

	UItemsObject* Materialitem;
	
	/**The maximum size that a stack of items can be*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item", meta = (ClampMin = 2, EditCondition = bStackable))
	int32 MaxStackSize;

	/**The tooltip in the inventory for this item*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item")
	TSubclassOf<class UItemTooltip_Widget> ItemTooltip;

	/**The tooltip in the inventory for this item*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item")
	TSubclassOf<class UCraftToolTip_Widget> CraftTooltip;

	/**The inventory that owns this item*/
	UPROPERTY()
		class UInventoryComponent* OwningInventory;

	UPROPERTY()
		class UCraftComponent* OwningCraftComponent;


	/**Used to efficiently replicate inventory items*/
	UPROPERTY()
		int32 RepKey;

	UPROPERTY(BlueprintAssignable)
		FOnItemModified OnItemModified;

	UFUNCTION()
		void OnRep_Quantity();

	UFUNCTION(BlueprintCallable, Category = "Item")
		void SetQuantity(const int32 NewQuantity);

	UFUNCTION(BlueprintPure, Category = "Item")
		FORCEINLINE int32 GetQuantity() const { return Quantity; }

	UFUNCTION(BlueprintCallable, Category = "Item")
		FORCEINLINE float GetStackWeight() const { return Quantity * Weight; };

	UFUNCTION(BlueprintCallable, Category = "Item")
		FORCEINLINE bool IsStackFull() const { return Quantity >= MaxStackSize; }

	UFUNCTION(BlueprintPure, Category = "Item")
	virtual bool ShouldShowInInventory() const;



	UFUNCTION(BlueprintPure, Category = "Item")
	FORCEINLINE UItemsObject* GetMatItem() { return Materialitem; }

	UFUNCTION(BlueprintPure, Category = "Item")
	FORCEINLINE int32 GetMatQty() { return MaterialItems.Quantiy; }

	UFUNCTION(BlueprintImplementableEvent)
	void OnUse(class APlayerCharacter* Character);
	virtual void Use(class APlayerCharacter* Character);
	virtual void Craft(class APlayerCharacter* Character);
	virtual void AddedToInventory(class UInventoryComponent* Inventory);
	virtual void AddedToCraftInventory(class UCraftComponent* Inventory);
	
	//call after modifying any replicated properties
	void MarkDirtyForReplication();

protected:

	/**The amount of the item*/
	UPROPERTY(ReplicatedUsing = OnRep_Quantity, EditAnywhere, Category = "Item", meta = (UIMin = 1, EditCondition = bStackable))
		int32 Quantity;
};
