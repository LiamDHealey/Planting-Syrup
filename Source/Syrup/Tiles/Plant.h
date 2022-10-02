// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "Effects/TileEffectTrigger.h"

#include "CoreMinimal.h"
#include "Tile.h"
#include "Plant.generated.h"

class UApplyField;
class UPreventTrashSpawn;

DECLARE_LOG_CATEGORY_EXTERN(LogPlant, Log, All);

/* \/ ====== \/ *\
|  \/ APlant \/  |
\* \/ ====== \/ */
/**
 * A plant on the grid that grows, can take damage, and creates a protection radius.
 */
UCLASS(HideCategories = ("ActorTick", "Tile", "Replication", "Rendering", "Collision", "Actor", "Input", "HLOD", "WorldPartition", "Cooking", "DataLayers"))
class SYRUP_API APlant : public ATile
{
	GENERATED_BODY()

	/* -------------------- *\
	\* \/ Initialization \/ */

public:

	/**
	 * Creates the plants components.
	 */
	APlant();

private:

	/**
	 * Binds effect triggers and initializes size.
	 */
	virtual void BeginPlay() override;

	/**
	 * Initializes Health, Range, and Shape; and sets the appropriate mesh.
	 *
	 * @param Transform - The new transform of the plant.
	 */
	virtual void OnConstruction(const FTransform& Transform) override;

	/* /\ Initialization /\ *\
	\* -------------------- */

	/* ----------- *\
	\* \/ Shape \/ */

public:
	/**
	 * Gets the mesh of this plant type.
	 * 
	 * @return The static mesh to use for this plant type.
	 */
	UFUNCTION(BlueprintPure, Category = "Shape")
	FORCEINLINE UStaticMesh* GetMesh() const { return Mesh; };

	/**
	 * Gets the shape of this plant type.
	 * 
	 * @return A set containing all of the relative locations of the sub-tiles making up the shape of this plant type.
	 */
	UFUNCTION(BlueprintPure, Category = "Shape")
	FORCEINLINE TSet<FIntPoint> GetShape() const { return Shape; };

protected:
	/**
	 * The relative locations of all of the sub-tiles of this plant.
	 *
	 * @return The relative locations of all of the sub-tiles of this plant.
	 */
	virtual TSet<FIntPoint> GetRelativeSubTileLocations() const override;

	//The mesh of this plant.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Shape")
	UStaticMeshComponent* MeshComponent;

	//The static mesh to use for this plant type.
	UPROPERTY(EditDefaultsOnly, Category = "Shape")
	UStaticMesh* Mesh = nullptr;

	//A set containing all of the relative locations of the sub-tiles making up the shape of this plant type.
	UPROPERTY(EditDefaultsOnly, Category = "Shape")
	TSet<FIntPoint> Shape = TSet<FIntPoint>();

	/* /\ Shape /\ *\
	\* ----------- */

	/* ------------ *\
	\* \/ Health \/ */

public:

	/**
	 * Causes this plant to take damage.
	 * 
	 * @param Amount - The number of damage points to damage this plant by.
	 * @param Cause - The tile that caused this damage.
	 * 
	 * @return Whether or not this plant was killed by the damage.
	 */
	UFUNCTION(BlueprintCallable, Category = "Health")
	bool ReceiveDamage(int Amount, ATile* Cause);

	/**
	 * Gets the current health of this plant.
	 * 
	 * @return The number of damage points this plant can take before dying.
	 */
	UFUNCTION(BlueprintPure, Category = "Health")
	FORCEINLINE int GetHealth() const { return Health; };

	/**
	 * Gets the max health of this plant type.
	 * 
	 * @return The number of damage points a plant of this type can take before dying.
	 */
	UFUNCTION(BlueprintPure, Category = "Health")
	FORCEINLINE int GetMaxHealth() const { return Cast<APlant>(GetClass()->GetDefaultObject())->Health; };

protected:

	//The health of this plant.
	UPROPERTY(EditDefaultsOnly, Category = "Health", Meta = (ClampMin = "1"))
	int Health = 1;

	/* /\ Health /\ *\
	\* ------------ */

	/* ------------ *\
	\* \/ Growth \/ */

public:

	/**
	 * Gets the turns taken for this plant type to grow.
	 * 
	 * @return The number of turns it takes for this plant type's effects to begin taking effect.
	 */
	UFUNCTION(BlueprintPure, Category = "Growth")
	FORCEINLINE int GetTimeUntilGrown() const { return TimeUntilGrown; };

	/**
	 * Gets the total number of turns that it takes for this plant type to grow.
	 * 
	 * @return The total number of turns that it takes for this plant type to grow.
	 */
	UFUNCTION(BlueprintPure, Category = "Growth")
	FORCEINLINE int GetInitialTimeUntilGrown() const { return Cast<APlant>(GetClass()->GetDefaultObject())->TimeUntilGrown; };

	/**
	 * Gets whether or not this plant is fully grown.
	 * 
	 * @return Whether or not this plant is fully grown.
	 */
	UFUNCTION(BlueprintPure, Category = "Growth")
	FORCEINLINE bool IsGrown() const { return TimeUntilGrown <= 0; };

	/**
	 * Gets cost to plant this plant type.
	 * 
	 * @return The amount of energy required to plant a plant of this type.
	 */
	UFUNCTION(BlueprintPure, Category = "Growth")
	FORCEINLINE int GetPlantingCost() const { return PlantingCost; };

protected:

	//The turns remaining until this plant is fully grown.
	UPROPERTY(EditDefaultsOnly, Category = "Growth", Meta = (ClampMin = "1"))
	int TimeUntilGrown = 1;
	
	//The amount of energy required to plant a plant of this type.
	UPROPERTY(EditDefaultsOnly, Category = "Growth", Meta = (ClampMin = "0"))
	int PlantingCost = 1;

	/**
	 * Updates the plants so that it is 1 turn closer to fully grown, and causes the effects of being fully grown if needed.
	 */
	UFUNCTION(BlueprintNativeEvent)
	void Grow();

	/**
	 * Updates the plants so that it is 1 turn closer to fully grown, and causes the effects of being fully grown if needed.
	 */
	UFUNCTION()
	void Grow_Implementation();

	/* /\ Growth /\ *\
	\* ------------ */

	/* ------------ *\
	\* \/ Effect \/ */

public:
	
	/**
	 * Gets the range of this plant type's effects.
	 * 
	 * @return The scale applied to the shape of this plant type to get all effected locations of this plant type's effects.
	 */
	UFUNCTION(BlueprintPure, Category = "Effect")
	FORCEINLINE int GetRange() const { return Range; };

protected:

	//Makes the plant grow grass in the effect area.
	UPROPERTY(VisibleAnywhere, Category = "Effect")
	UApplyField* GrassComponent;

	//Makes the plant prevent trash from spawning within the effected area.
	UPROPERTY(VisibleAnywhere, Category = "Effect")
	UPreventTrashSpawn* PreventTrashComponent;

	//The scale applied to the shape of this plant type to get all effected locations of this plant type's effects.
	UPROPERTY(EditDefaultsOnly, Category = "Effect", Meta = (ClampMin = "0"))
	int Range = 1;

private:

	/**
	 * Activates the appropriate effects given the trigger.
	 * 
	 * @param TriggerType - The type of trigger that was activated.
	 * @param LocationsToTrigger - The Locations where the trigger applies an effect. If this is empty all effect locations will be effected.
	 */
	UFUNCTION()
	void ReceiveEffectTrigger(const ETileEffectTriggerType TriggerType, const TSet<FIntPoint>& LocationsToTrigger);

	/**
	 * Gets the locations where the effects of this plant will apply.
	 * 
	 * @return A set of all locations where the effects of this plant will apply.
	 */
	UFUNCTION()
	TSet<FIntPoint> GetEffectLocations() const;

	/* /\ Effect /\ *\
	\* ------------ */

	/* -------- *\
	\* \/ UI \/ */

public:
	
	/**
	 * Gets the name of this plant type.
	 * 
	 * @return The text to use when referring to a plant of this type.
	 */
	UFUNCTION(BlueprintPure, Category = "UI")
	FORCEINLINE FText GetDisplayName() const { return DisplayName; };

protected:

	//The text to use when referring to a plant of this type.
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	FText DisplayName = FText();

	/* /\ UI /\ *\
	\* -------- */
};
/* /\ ====== /\ *\
|  /\ APlant /\  |
\* /\ ====== /\ */