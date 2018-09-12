// Fill out your copyright notice in the Description page of Project Settings.

#include "SWeapon.h"

#include "CoopGameTLCourse.h"

#include "DrawDebugHelpers.h"

#include "Components/SkeletalMeshComponent.h"

#include "Kismet/GameplayStatics.h"

#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"

#include "PhysicalMaterials/PhysicalMaterial.h"

static int32 DebugWeaponDrawing = 0;
FAutoConsoleVariableRef CVARDebugWeaponDrawing(
	TEXT("COOP.DebugWeapons"),
	DebugWeaponDrawing,
	TEXT("Draw debug lines for weapons"),
	ECVF_Cheat);

// Sets default values
ASWeapon::ASWeapon()
{
	SkeletalMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(FName("SkeletalMeshComponent"));
	RootComponent = SkeletalMeshComponent;

	MuzzleSocketName = FName("MuzzleSocket");
	TracerTargetName = FName("Target");
}

void ASWeapon::Fire()
{
	// Trace the world from pawn eye to the crosshair location.

	AActor* MyOwner = GetOwner();
	if(MyOwner != nullptr)
	{
		FVector EyeLocation;
		FRotator EyeRotation;
		MyOwner->GetActorEyesViewPoint(EyeLocation, EyeRotation);
		FVector ShotDirection = EyeRotation.Vector();
		FVector TraceEnd = EyeLocation + (ShotDirection * 1000);
		FHitResult HitResult;
		
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(MyOwner);
		QueryParams.AddIgnoredActor(this);
		QueryParams.bTraceComplex = true;
		QueryParams.bReturnPhysicalMaterial = true;

		// Target particle paramether.
		FVector TraceEndPoint = TraceEnd;

		bool bLineTraceResult = GetWorld()->LineTraceSingleByChannel(
			HitResult, EyeLocation, TraceEnd, COLLISION_WEAPON, QueryParams);
		if (bLineTraceResult)
		{
			// Blocking hit process damage.
			AActor* HitActor = HitResult.GetActor();

			UGameplayStatics::ApplyPointDamage(HitActor, 20.0f, ShotDirection, HitResult,
				MyOwner->GetInstigatorController(), this, DamageType);

			EPhysicalSurface SurfaceType = UPhysicalMaterial::DetermineSurfaceType(HitResult.PhysMaterial.Get());
			UParticleSystem* SelectedEffect = nullptr;
			switch (SurfaceType)
			{
			case SURFACE_FLESHDEFAULT:
			case SURFACE_FLESHVULNERABLE:
				SelectedEffect = FleshImpactEffect;
				break;
			default:
				SelectedEffect = DefaultImpactEffect;
				break;
			}

			if (SelectedEffect != nullptr)
			{
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), SelectedEffect,
					HitResult.ImpactPoint, HitResult.ImpactNormal.Rotation());
			}

			TraceEndPoint = HitResult.ImpactPoint;
		}

		if(DebugWeaponDrawing > 0)
		{
			DrawDebugLine(GetWorld(), EyeLocation, TraceEnd, FColor::White, false, 1.0f, 0, 1.0f);
		}

		PlayFireEffects(TraceEndPoint);
	}
}

void ASWeapon::PlayFireEffects(FVector TraceEnd)
{
	if (MuzzleEffect != nullptr)
	{
		UGameplayStatics::SpawnEmitterAttached(
			MuzzleEffect, SkeletalMeshComponent, MuzzleSocketName);
	}

	if (TracerEffect != nullptr)
	{
		FVector MuzzleLocation = SkeletalMeshComponent->GetSocketLocation(MuzzleSocketName);
		UParticleSystemComponent* TracerComponent =
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), TracerEffect, MuzzleLocation);

		if (TracerComponent != nullptr)
		{
			TracerComponent->SetVectorParameter(TracerTargetName, TraceEnd);
		}
	}

	APawn* MyOwner = Cast<APawn>(GetOwner());
	if (MyOwner != nullptr)
	{
		APlayerController* PC = Cast<APlayerController>(MyOwner->GetController());
		if (PC != nullptr)
		{
			PC->ClientPlayCameraShake(FireCamShake);
		}
	}
}
