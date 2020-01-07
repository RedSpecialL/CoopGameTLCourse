// Fill out your copyright notice in the Description page of Project Settings.


#include "SExplosiveBarrel.h"

#include "Components/StaticMeshComponent.h"
#include "SHealthComponent.h"
#include "Particles/ParticleSystem.h"
#include "Materials/Material.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

// Sets default values
ASExplosiveBarrel::ASExplosiveBarrel()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(FName("MeshComp"));
	Mesh->SetCanEverAffectNavigation(false);
	Mesh->SetSimulatePhysics(true);
	Mesh->SetIsReplicated(true);

	RootComponent = Mesh;

	HealthComponent = CreateDefaultSubobject<USHealthComponent>(FName("HealthComponent"));
	HealthComponent->OnHealthChanged.AddDynamic(this, &ASExplosiveBarrel::HandleTakeDamage);

	bExploded = false;

	SetReplicates(true);
	SetReplicateMovement(true);
}

// Called when the game starts or when spawned
void ASExplosiveBarrel::BeginPlay()
{
	Super::BeginPlay();
	
}

void ASExplosiveBarrel::HandleTakeDamage(USHealthComponent* OwningHealthComponent, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
	if (bExploded)
	{
		return;
	}

	if (Health <= 0.0f)
	{
		bExploded = true;
		SelfDestract();
	}
}

void ASExplosiveBarrel::SelfDestract()
{
	FVector MyLocation = GetActorLocation();
	if (ExplodedMaterial != nullptr)
	{
		Mesh->SetMaterial(0, ExplodedMaterial);
	}

	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, MyLocation);
	if (Mesh->IsSimulatingPhysics())
	{
		Mesh->AddImpulse(FVector{ 0.0f, 0.0f, 50000.0f });
	}

	if (GetLocalRole() == ROLE_Authority)
	{
		TArray<AActor*> IgnoredActors;
		IgnoredActors.Add(this);

		// ApplyDamage.
		UGameplayStatics::ApplyRadialDamage(this, ExplosionDamage, MyLocation, ExplosionRadius,
			nullptr, IgnoredActors, this, GetInstigatorController(), true);

		TArray<AActor*> OverlapedActors;
		TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
		ObjectTypes.Add(EObjectTypeQuery::ObjectTypeQuery1);

		if (UKismetSystemLibrary::SphereOverlapActors(GetWorld(), MyLocation, PushingRadius, ObjectTypes, AActor::StaticClass(), IgnoredActors, OverlapedActors))
		{
			for (AActor* Actor : OverlapedActors)
			{
				UPrimitiveComponent* Component = Cast<UPrimitiveComponent>(Actor->GetRootComponent());

				if (Component != nullptr && Component->IsSimulatingPhysics())
				{
					Component->AddRadialForce(MyLocation, PushingRadius, PushingForce, ERadialImpulseFalloff::RIF_Linear, true);
				}
			}
		}
	}
}

