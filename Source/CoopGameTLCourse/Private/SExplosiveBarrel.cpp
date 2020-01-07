// Fill out your copyright notice in the Description page of Project Settings.


#include "SExplosiveBarrel.h"

#include "Components/StaticMeshComponent.h"
#include "SHealthComponent.h"
#include "Particles/ParticleSystem.h"
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
	RootComponent = Mesh;

	HealthComponent = CreateDefaultSubobject<USHealthComponent>(FName("HealthComponent"));
	HealthComponent->OnHealthChanged.AddDynamic(this, &ASExplosiveBarrel::HandleTakeDamage);

	bExploded = false;
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
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, GetActorLocation());


	if (Mesh->IsSimulatingPhysics())
	{
		Mesh->AddImpulse(FVector{ 0.0f, 0.0f, 50000.0f });
	}

	//UGameplayStatics::PlaySoundAtLocation(this, ExploadSound, GetActorLocation());

	if (GetLocalRole() == ROLE_Authority)
	{

		TArray<AActor*> IgnoredActors;
		IgnoredActors.Add(this);

		// ApplyDamage.
		UGameplayStatics::ApplyRadialDamage(this, ExplosionDamage, GetActorLocation(), ExplosionRadius,
			nullptr, IgnoredActors, this, GetInstigatorController(), true);

		TArray<AActor*> OverlapedActors;
		TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
		ObjectTypes.Add(EObjectTypeQuery::ObjectTypeQuery1);

		if (UKismetSystemLibrary::SphereOverlapActors(GetWorld(), GetActorLocation(), PushingRadius, ObjectTypes, AActor::StaticClass(), IgnoredActors, OverlapedActors))
		{
			UE_LOG(LogTemp, Warning, TEXT("Overlaped"));

			for (AActor* Actor : OverlapedActors)
			{
				UE_LOG(LogTemp, Warning, TEXT("MyActor, %s"), *Actor->GetActorLocation().ToString());


				if (Actor == nullptr)
				{
					UE_LOG(LogTemp, Warning, TEXT("WrongActor o_O"));

					continue;
				}

				UPrimitiveComponent* Component = Cast<UPrimitiveComponent>(Actor->GetRootComponent());
				UE_LOG(LogTemp, Warning, TEXT("Component, %s, %s"), *Component->GetComponentLocation().ToString(), (Component->IsSimulatingPhysics() ? TEXT("True") : TEXT("False")));
				

				if (Component != nullptr && Component->IsSimulatingPhysics())
				{
					UE_LOG(LogTemp, Warning, TEXT("AddingForce"));

					Component->AddRadialForce(GetActorLocation(), PushingRadius, 3000000.0f, ERadialImpulseFalloff(), true);
				}
			}
		}

	}


}

