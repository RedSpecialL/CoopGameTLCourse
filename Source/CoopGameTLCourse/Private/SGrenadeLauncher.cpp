// Fill out your copyright notice in the Description page of Project Settings.


#include "SGrenadeLauncher.h"

void ASGrenadeLauncher::Fire()
{
	if (Role < ROLE_Authority)
	{
		ServerFire();
	}

	AActor* MyOwner = GetOwner();
	if (MyOwner != nullptr && ProjectileToSwawn != nullptr)
	{
		FVector EyeLocation;
		FRotator EyeRotation;
		MyOwner->GetActorEyesViewPoint(EyeLocation, EyeRotation);
	
		FVector SpawnLocation = SkeletalMeshComponent->GetSocketLocation(FName("MuzzleFlashSocket"));
		FActorSpawnParameters ActorSpawnParams;
		ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

		// Spawn projectile.
		GetWorld()->SpawnActor<AActor>(ProjectileToSwawn, SpawnLocation, EyeRotation, ActorSpawnParams);
	}


}

void ASGrenadeLauncher::ServerFire_Implementation()
{
	Fire();
}

