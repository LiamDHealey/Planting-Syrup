// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VolumetricEffectActor.generated.h"


/* \/ ====================== \/ *\
|  \/ AVolumetricEffectActor \/  |
\* \/ ====================== \/ */
UCLASS()
class SYRUP_API AVolumetricEffectActor : public AActor
{
	GENERATED_BODY()
	
public:	
	/**
	 * Creates the instanced static mesh used for collision
	 */
	AVolumetricEffectActor();
	
	/**
	 * Sets the channels that this volume will overlap.
	 * 
	 * @param Channels - A bitwise int specifying all the channels to enable.
	 */
	UFUNCTION()
	void SetCollisionResponses(const TSet<TEnumAsByte<ECollisionChannel>>& OverlapedChannels, const TSet<TEnumAsByte<ECollisionChannel>>& BlockedChannels);

	/**
	 * Adds tiles to this effect volume.
	 * 
	 * @param TileLocations - The locations of the tiles to add to the volume.
	 */
	UFUNCTION(BlueprintCallable)
	void AddTiles(const TSet<FIntPoint>& TileLocations);

	/**
	 * Removes tiles from this effect volume.
	 * 
	 * @param TileLocations - The locations of the tiles to remove from the volume.
	 */
	UFUNCTION(BlueprintCallable)
	void RemoveTiles(const TSet<FIntPoint> TileLocations);

//private:
	//The mesh used to generate overlap events.
	UPROPERTY(VisibleAnywhere)
	UInstancedStaticMeshComponent* CollisionMesh;

	//The index where the location is stored is the index of the index storing it.
	UPROPERTY()
	TArray<FIntPoint> InstanceLocationsToIndices = TArray<FIntPoint>();
};
/* /\ ====================== /\ *\
|  /\ AVolumetricEffectActor /\  |
\* /\ ====================== /\ */
