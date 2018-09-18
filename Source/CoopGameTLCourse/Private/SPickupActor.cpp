// Fill out your copyright notice in the Description page of Project Settings.

#include "SPickupActor.h"
#include "SPowerupActor.h"

#include "TimerManager.h"

#include "Components/DecalComponent.h"
#include "Components/SphereComponent.h"


// Sets default values
ASPickupActor::ASPickupActor()
{
	SphereComp = CreateDefaultSubobject<USphereComponent>(FName("SphereComp"));
	SphereComp->SetSphereRadius(75.0f);
	RootComponent = SphereComp;

	DecalComp = CreateDefaultSubobject<UDecalComponent>(FName("DecalComp"));
	DecalComp->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f));
	DecalComp->DecalSize = FVector(64.0f, 75.0f, 75.0f);
	DecalComp->SetupAttachment(RootComponent);

	CooldownDuration = 10.0f;
}

void ASPickupActor::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	if (PowerupInstance != nullptr)
	{
		PowerupInstance->ActivatePowerup();
		PowerupInstance = nullptr;

		// Set timer to respawn
		GetWorldTimerManager().SetTimer(TimerHandle_RespawnTimer, this,
			&ASPickupActor::Respawn, CooldownDuration);
	}
}

void ASPickupActor::Respawn()
{
	if (PowerupClass == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("PowerupClass is nullptri in %s. Please update your blueprint"), *GetName());

		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	PowerupInstance = GetWorld()->SpawnActor<ASPowerupActor>(PowerupClass, GetTransform(), SpawnParams);
}

// Called when the game starts or when spawned
void ASPickupActor::BeginPlay()
{
	Super::BeginPlay();

	Respawn();
}
