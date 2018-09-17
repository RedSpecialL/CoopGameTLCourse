// Fill out your copyright notice in the Description page of Project Settings.

#include "SPickupActor.h"

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
}

void ASPickupActor::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	// TODO: Grant a power up to player if available.
}

// Called when the game starts or when spawned
void ASPickupActor::BeginPlay()
{
	Super::BeginPlay();
	
}
