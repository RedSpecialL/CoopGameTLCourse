// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SWeapon.h"
#include "SGrenadeLauncher.generated.h"

/**
 * 
 */
UCLASS()
class COOPGAMETLCOURSE_API ASGrenadeLauncher : public ASWeapon
{
	GENERATED_BODY()

protected:
	virtual void Fire() override;

	virtual void ServerFire_Implementation() override;

	UPROPERTY(EditAnywhere)
	TSubclassOf<AActor> ProjectileToSwawn;
};
