// Fill out your copyright notice in the Description page of Project Settings.


#include "Trash.h"

#include "Syrup/Systems/SyrupGameMode.h"
#include "Effects/TileEffect.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Resources/Resource.h"

/* \/ ====== \/ *\
|  \/ ATrash \/  |
\* \/ ====== \/ */

/* -------------------- *\
\* \/ Initialization \/ */

/**
 * Initializes sink components.
 */
ATrash::ATrash()
{
	FSinkAmountUpdateDelegate AmountSetter;
	AmountSetter.BindUFunction(this, FName("SetDamage"));
	FSinkLocationsDelegate LocationGetter;
	LocationGetter.BindUFunction(this, FName("GetSubTileLocations"));
	FSinkAmountDelegate AmountGetter;
	AmountGetter.BindUFunction(this, FName("GetDamage"));

	DamageResourceSink = UResourceSink::CreateDefaultResourceSinkComponent(this, AmountSetter, LocationGetter, AmountGetter);
	AmountSetter.BindUFunction(this, FName("SetRange"));
	AmountGetter.BindUFunction(this, FName("GetRange"));
	RangeResourceSink = UResourceSink::CreateDefaultResourceSinkComponent(this, AmountSetter, LocationGetter, AmountGetter);
	AmountSetter.BindUFunction(this, FName("SetPickUpCost"));
	AmountGetter.BindUFunction(this, FName("GetPickUpCost"));
	PickUpCostResourceSink = UResourceSink::CreateDefaultResourceSinkComponent(this, AmountSetter, LocationGetter, AmountGetter);
}

/**
 * Sets up this trash after it has fallen.
 */
void ATrash::OnFinishedFalling()
{ 
	bActive = true;
	ReceiveEffectTrigger(ETileEffectTriggerType::OnActivated, nullptr, TSet<FIntPoint>());
}

/**
 * Binds effect triggers.
 */
void ATrash::BeginPlay()
{
	Super::BeginPlay();

	ASyrupGameMode::GetTileEffectTriggerDelegate(GetWorld()).Broadcast(ETileEffectTriggerType::TrashSpawned, this, GetSubTileLocations());
	ASyrupGameMode::GetTileEffectTriggerDelegate(this).AddDynamic(this, &ATrash::ReceiveEffectTrigger);
}

/**
 * Handles undoing of tile effects.
 */
void ATrash::Destroyed()
{
	ReceiveEffectTrigger(ETileEffectTriggerType::OnDeactivated, nullptr, TSet<FIntPoint>());
	Super::Destroyed();
}

/**
 * Initializes Damage, Range, and sets the appropriate mesh.
 *
 * @param Transform - The new transform of the trash.
 */
void ATrash::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	SubtileMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);

	SetDamage(Damage);
	RelativeSubTileLocations.Add(FIntPoint::ZeroValue);
}

/* /\ Initialization /\ *\
\* -------------------- */



/* ------------- *\
\* \/ Pick Up \/ */

/**
 * Gets cost to pickup this piece of trash. Will fail if energy reserve does not have enough energy to pick this up.
 *
 * @param EnergyReserve - The energy reserve of the thing trying to pick this up. Will have PickupCost subtracted from it.
 * @return Whether or not this was picked up,
 */
bool ATrash::PickUp(int& EnergyReserve)
{
	if (EnergyReserve >= PickUpCost)
	{
		EnergyReserve -= PickUpCost;
		ASyrupGameMode::GetTileEffectTriggerDelegate(GetWorld()).Broadcast(ETileEffectTriggerType::TrashPickedUp, this, GetSubTileLocations());
		Destroy();
		return true;
	}

	return false;
}

/* /\ Pick Up /\ *\
\* ------------- */



/* ------------ *\
\* \/ Effect \/ */

/**
 * Sets the range of this trash's effects.
 *
 * @param NewRange - The value to set the range to. Will be clamped >= 0.
 */
void ATrash::SetRange_Implementation(const int NewRange)
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
void ATrash::ReceiveEffectTrigger(const ETileEffectTriggerType TriggerType, const ATile* Triggerer, const TSet<FIntPoint>& LocationsToTrigger)
{
	if (bActive)
	{
		TSet<FIntPoint> EffectedLocations = GetEffectLocations();
		TSet<FIntPoint> TriggeredLocations = LocationsToTrigger.IsEmpty() ? EffectedLocations : LocationsToTrigger.Intersect(EffectedLocations);

		if (!TriggeredLocations.IsEmpty())
		{
			TInlineComponentArray<UActorComponent*> Components = TInlineComponentArray<UActorComponent*>();
			GetComponents(UTileEffect::StaticClass(), Components);
			for (UActorComponent* EachComponent : Components)
			{
				Cast<UTileEffect>(EachComponent)->ActivateEffect(TriggerType, Triggerer, TriggeredLocations);
			}
		}
	}
}

/**
 * Gets the locations where the effects of this trash will apply.
 *
 * @return A set of all locations where the effects of this trash will apply.
 */
TSet<FIntPoint> ATrash::GetEffectLocations() const
{
	return UGridLibrary::ScaleShapeUp(GetSubTileLocations(), Range);
}

/* /\ Effect /\ *\
\* ------------ */


/* /\ ====== /\ *\
|  /\ ATrash /\  |
\* /\ ====== /\ */