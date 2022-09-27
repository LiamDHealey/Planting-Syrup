// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Tiles/Effects/TileEffectTrigger.h"

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SyrupGameMode.generated.h"


/* \/ ============== \/ *\
|  \/ ASyrupGameMode \/  |
\* \/ ============== \/ */
/**
 * The base game mode for Syrup. Handles the turn system and triggering tile effects.
 */
UCLASS(Blueprintable)
class SYRUP_API ASyrupGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	/**
	 * Gets the delegate used to bind and trigger tile effects.
	 * 
	 * @param WorldContextObject - An object in the same world as the delegate.
	 * 
	 * @return The delegate used to bind and trigger tile effects.
	 */
	UFUNCTION()
	static FTileEffecTrigger& GetTileEffectTriggerDelegate(const UObject* WorldContextObject);

	UPROPERTY(BlueprintAssignable, BlueprintCallable, Meta = (AutoCreateRefTerm = "Locations"))
	FTileEffecTrigger TileEffectTriggerDelegate;
};
/* /\ ============== /\ *\
|  /\ ASyrupGameMode /\  |
\* /\ ============== /\ */