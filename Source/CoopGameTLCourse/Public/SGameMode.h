// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SGameMode.generated.h"

/**
 * 
 */
UCLASS()
class COOPGAMETLCOURSE_API ASGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
public:

	ASGameMode();

	virtual void StartPlay() override;

protected:

	FTimerHandle TimerHandle_BotSpawner;

	/** Bots to spawn in current wave. */
	int32 NumberOfBotsToSpawn;

	int32 WaveCount;

	UPROPERTY(EditDefaultsOnly, Category = "GameMode")
	float TimeBetweenWaves;

protected:

	/** Hook for BP to spawn a single bot. */
	UFUNCTION(BlueprintImplementableEvent, Category = "GameMode")
	void SpawnNewBot();
	
	void SpawnBotTimerElapsed();

	/** Starts spawning bots.*/
	void StartWave();
	/** Stops spawning bots.*/
	void EndWave();
	/** Sets timer for next startwave.*/
	void PrepareForNextWave();

};
