// Fill out your copyright notice in the Description page of Project Settings.


#include "SGrenadeLauncher.h"

void ASGrenadeLauncher::Fire()
{
	if (Role < ROLE_Authority)
	{
		ServerFire();
	}

	FVector SpawnLocation = SkeletalMeshComponent->GetSocketLocation(FName("MuzzleFlashSocket"));
	FRotator SpawnRotation = SkeletalMeshComponent->GetSocketRotation(FName("MuzzleFlashSocket"));

	FActorSpawnParameters ActorSpawnParams;
	ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

	// Spawn projectile.
	GetWorld()->SpawnActor<AActor>(ProjectileToSwawn, SpawnLocation, SpawnRotation, ActorSpawnParams);
}

void ASGrenadeLauncher::ServerFire_Implementation()
{
	Fire();
}

