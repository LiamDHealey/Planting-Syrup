// Fill out your copyright notice in the Description page of Project Settings.

#include "Plant.h"

#include "Syrup/Systems/SyrupGameMode.h"
#include "Effects/TileEffect.h"
#include "Components/InstancedStaticMeshComponent.h"

DEFINE_LOG_CATEGORY(LogPlant);

/* \/ ====== \/ *\
|  \/ APlant \/  |
\* \/ ====== \/ */

/* -------------------- *\
\* \/ Initialization \/ */

/**
 * Binds effect triggers and initializes size.
 */
void APlant::BeginPlay()
{
	Super::BeginPlay();

	SubtileMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);

	bIsFinishedPlanting = true;
	TimeUntilGrown = GetTimeUntilGrown() + 1;
	Grow();
	bIsFinishedPlanting = ASyrupGameMode::IsPlayerTurn(this);

	ASyrupGameMode::GetTileEffectTriggerDelegate(this).AddDynamic(this, &APlant::ReceiveEffectTrigger);
	ASyrupGameMode::GetTileEffectTriggerDelegate(GetWorld()).Broadcast(ETileEffectTriggerType::PlantSpawned, this, GetSubTileLocations());
}

/**
 * Initializes Health, Range, and Shape; and sets the appropriate mesh.
 *
 * @param Transform - The new transform of the plant.
 */
void APlant::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	SubtileMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	Health = GetMaxHealth();
	Range = GetRange();
	Shape.Add(FIntPoint::ZeroValue);
}

/* /\ Initialization /\ *\
\* -------------------- */



/* ----------- *\
\* \/ Shape \/ */

/*
 * The relative locations of all of the sub-tiles of this plant.
 *
 * @return The relative locations of all of the sub-tiles of this plant.
 */
TSet<FIntPoint> APlant::GetRelativeSubTileLocations() const
{
	return GetShape();
}

/* /\ Shape /\ *\
\* ----------- */



/* ------------ *\
\* \/ Health \/ */

/**
 * Causes this plant to take damage.
 *
 * @param Amount - The number of damage points to damage this plant by.
 * @param Cause - The tile that caused this damage.
 *
 * @return Whether or not this plant was killed by the damage.
 */
bool APlant::ReceiveDamage(int Amount, ATile* Cause)
{
	if (!bIsFinishedPlanting)
	{
		return false;
	}

	int OldHealth = Health;
	Health -= FMath::Max(0, Amount);
	bool bDead = Health <= 0;
	if (bDead && OldHealth > 0)
	{
		SubtileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		ASyrupGameMode::GetTileEffectTriggerDelegate(GetWorld()).Broadcast(ETileEffectTriggerType::PlantKilled, this, GetSubTileLocations());
		ReceiveEffectTrigger(ETileEffectTriggerType::OnDeactivated, nullptr, TSet<FIntPoint>());
	}
	OnDamageRecived(Amount, Cause, bDead);
	return bDead;
}

/* /\ Health /\ *\
\* ------------ */



/* ------------ *\
\* \/ Growth \/ */

/**
 * Plants a plant with the given transform.
 * 
 * @param WoldContextObject - Any object in the would to spawn the plant in.
 * @param EnergyReserve - The variable attempt to subtract the planting cost from.
 * @param PlantClass - The type of plant to plant.
 * @param Transform - The location to spawn the plant at.
 * 
 * @return Whether there was enough energy and space to plant the plant.
 */
bool APlant::SowPlant(UObject* WorldContextObject, int& EnergyReserve, TSubclassOf<APlant> PlantClass, FTransform Transform)
{
	return SowPlant(WorldContextObject, EnergyReserve, PlantClass, UGridLibrary::WorldTransformToGridTransform(Transform));
}
bool APlant::SowPlant(UObject* WorldContextObject, int& EnergyReserve, TSubclassOf<APlant> PlantClass, FGridTransform Transform)
{
	APlant* DefaultPlant = PlantClass.GetDefaultObject();
	int NeededEnergy = DefaultPlant->GetPlantingCost();
	if (EnergyReserve >= NeededEnergy)
	{
		if (SowPlant(WorldContextObject, PlantClass, Transform))
		{
			EnergyReserve -= NeededEnergy;
			return true;
		}
	}
	return false;
}
bool APlant::SowPlant(UObject* WorldContextObject, TSubclassOf<APlant> PlantClass, FTransform Transform)
{
	return SowPlant(WorldContextObject, PlantClass, UGridLibrary::WorldTransformToGridTransform(Transform));
}
bool APlant::SowPlant(UObject* WorldContextObject, TSubclassOf<APlant> PlantClass, FGridTransform Transform)
{
	if (!IsValid(PlantClass) || PlantClass.Get()->HasAnyClassFlags(CLASS_Abstract))
	{
		UE_LOG(LogPlant, Error, TEXT("Tried to sow null or abstract plant."))
		return false;
	}
	
	TSet<ATile*> BlockingTiles;
	if (!UGridLibrary::OverlapShape(WorldContextObject, UGridLibrary::TransformShape(PlantClass.GetDefaultObject()->GetShape(), Transform), BlockingTiles, TArray<AActor*>(), ECollisionChannel::ECC_GameTraceChannel3))
	{
		WorldContextObject->GetWorld()->SpawnActor<APlant>(PlantClass, UGridLibrary::GridTransformToWorldTransform(Transform));
		return true;
	}

	return false;
}

/**
 * Updates the plants so that it is 1 turn closer to fully grown, and causes the effects of being fully grown if needed.
 */
void APlant::Grow_Implementation()
{
	if (bIsFinishedPlanting && !IsGrown())
	{
		TimeUntilGrown--;

		if (IsGrown())
		{
			ReceiveEffectTrigger(ETileEffectTriggerType::OnActivated, nullptr, TSet<FIntPoint>());
		}
	}
}

/* /\ Growth /\ *\
\* ------------ */



/* ------------ *\
\* \/ Effect \/ */

/**
 * Sets the range of this plant's effects.
 *
 * @param NewRange - The value to set the range to. Will be clamped >= 0.
 */
void APlant::SetRange(const int NewRange)
{
	TSet<FIntPoint> OldEffectLocations = GetEffectLocations();
	TSet<FIntPoint> NewEffectLocations = UGridLibrary::ScaleShapeUp(GetSubTileLocations(), FMath::Max(0, NewRange));

	TSet<FIntPoint> DeactivatedLocations = OldEffectLocations.Difference(NewEffectLocations);
	if (!DeactivatedLocations.IsEmpty())
	{
		ReceiveEffectTrigger(ETileEffectTriggerType::OnDeactivated, nullptr, DeactivatedLocations);
	}

	Range = FMath::Max(0, NewRange);
	TSet<FIntPoint> ActivatedLocations = NewEffectLocations.Difference(OldEffectLocations);
	if (!ActivatedLocations.IsEmpty())
	{
		ReceiveEffectTrigger(ETileEffectTriggerType::OnActivated, nullptr, ActivatedLocations);
	}
}

/**
 * Activates the appropriate effects given the trigger.
 *
 * @param TriggerType - The type of trigger that was activated.
 * @param Triggerer - The tile that triggered this effect.
 * @param LocationsToTrigger - The Locations where the trigger applies an effect. If this is empty all effect locations will be effected.
 */
void APlant::ReceiveEffectTrigger(const ETileEffectTriggerType TriggerType, const ATile* Triggerer, const TSet<FIntPoint>& LocationsToTrigger)
{
	if(TriggerType == ETileEffectTriggerType::PlantsGrow)
	{
		Grow();
	} 
	else if (!bIsFinishedPlanting && TriggerType == ETileEffectTriggerType::PlayerTurn)
	{
		bIsFinishedPlanting = true;
	}

	if ((IsGrown() || TriggerType == ETileEffectTriggerType::PlantsGrow) && Health > 0)
	{
		TSet<FIntPoint> EffectedLocations = GetEffectLocations();
		TSet<FIntPoint> TriggeredLocations = LocationsToTrigger.IsEmpty() ? EffectedLocations : LocationsToTrigger.Intersect(EffectedLocations);

		TInlineComponentArray<UActorComponent*> Components = TInlineComponentArray<UActorComponent*>();
		GetComponents(UTileEffect::StaticClass(), Components);
		for (UActorComponent* EachComponent : Components)
		{
			Cast<UTileEffect>(EachComponent)->ActivateEffect(TriggerType, Triggerer, TriggeredLocations);
		}
	}
}

/**
 * Gets the locations where the effects of this plant will apply.
 *
 * @return A set of all locations where the effects of this plant will apply.
 */
TSet<FIntPoint> APlant::GetEffectLocations() const
{
	return UGridLibrary::ScaleShapeUp(GetSubTileLocations(), Range);
}

/* /\ Effect /\ *\
\* ------------ */

/* /\ ====== /\ *\
|  /\ APlant /\  |
\* /\ ====== /\ */