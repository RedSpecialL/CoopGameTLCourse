// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SExplosiveBarrel.generated.h"

class USHealthComponent;
class UParticleSystem;

UCLASS()
class COOPGAMETLCOURSE_API ASExplosiveBarrel : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASExplosiveBarrel();



protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	UFUNCTION()
	void HandleTakeDamage(USHealthComponent* OwningHealthComponent, float Health, float HealthDelta,
		const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	UPROPERTY(EditDefaultsOnly, Category = "Explosion")
	float ExplosionDamage;
	UPROPERTY(EditDefaultsOnly, Category = "Explosion")
	float ExplosionRadius;
	
	UPROPERTY(EditDefaultsOnly, Category = "Explosion")
	float PushingRadius;
	UPROPERTY(EditDefaultsOnly, Category = "Explosion")
	float PushingForce;

	UPROPERTY(ReplicatedUsing=OnRep_Exploded)
	bool bExploded = false;

	UFUNCTION()
	void SelfDestract();

	UFUNCTION()
	void OnRep_Exploded();

protected:
	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* Mesh;
	UPROPERTY(VisibleDefaultsOnly, Category = "Components")
	USHealthComponent* HealthComponent;
	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	UParticleSystem* ExplosionEffect;
	UPROPERTY(EditAnywhere)
	class UMaterial* ExplodedMaterial;
};
