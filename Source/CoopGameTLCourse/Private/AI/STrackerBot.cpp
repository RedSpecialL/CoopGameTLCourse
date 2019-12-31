// Fill out your copyright notice in the Description page of Project Settings.

#include "STrackerBot.h"
#include "SCharacter.h"
#include "ShealthComponent.h"

#include "DrawDebugHelpers.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"

#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"

#include "Kismet/GameplayStatics.h"

#include "Particles/ParticleSystem.h"

#include "Sound/SoundCue.h"

static int32 DebugTrackerBotDrawing = 0;
FAutoConsoleVariableRef CVARDebugTrackerBotDrawing(
	TEXT("COOP.DebugTrackerBot"),
	DebugTrackerBotDrawing,
	TEXT("Draw debug lines for tracker bot"),
	ECVF_Cheat);


// Sets default values
ASTrackerBot::ASTrackerBot()
{
 	// Set this pawn to call Tick() every frame. 
	// You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(FName("MeshComp"));
	MeshComp->SetCanEverAffectNavigation(false);
	MeshComp->SetSimulatePhysics(true);
	RootComponent = MeshComp;

	SphereComponent = CreateDefaultSubobject<USphereComponent>(FName("SphereComponent"));
	SphereComponent->SetSphereRadius(200);
	SphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	SphereComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	
	SphereComponent->SetupAttachment(RootComponent);

	HealthComponent = CreateDefaultSubobject<USHealthComponent>(FName("HealthComponent"));
	HealthComponent->OnHealthChanged.AddDynamic(this, &ASTrackerBot::HandleTakeDamage);

	bUseVelocityChange = false;
	bExploded = false;
	MovementForce = 1000.0f;
	RequiredDistanceToTarget = 100;
	
	ExplosionDamage = 50.0f;
	ExplosionRadius = 350.0f;

	SelfDamageInterval = 0.25f;
}

void ASTrackerBot::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	if (!bStartedSelfDestruction && !bExploded)
	{
		ASCharacter* PlayerPawn = Cast<ASCharacter>(OtherActor);
		if (PlayerPawn != nullptr && !USHealthComponent::IsFriendly(OtherActor, this))
		{
			// We overlapped with a player!
			if (Role == ROLE_Authority)
			{
				// Start self destruction sequence.
				GetWorldTimerManager().SetTimer(TimerHandle_SelfDamage, this, &ASTrackerBot::DamageSelf,
					SelfDamageInterval, true, 0.0f);
			}

			bStartedSelfDestruction = true;

			UGameplayStatics::SpawnSoundAttached(SelfDestractSound, RootComponent);
		}
	}
}

// Called when the game starts or when spawned
void ASTrackerBot::BeginPlay()
{
	Super::BeginPlay();
	
	if (Role == ROLE_Authority)
	{
		NextPathPoint = GetNextPathPoint();
	}
}

FVector ASTrackerBot::GetNextPathPoint()
{
	AActor* BestTarget = nullptr;
	float NearestTargetDistance = FLT_MAX;

	for (FConstPawnIterator It = GetWorld()->GetPawnIterator(); It; ++It)
	{
		APawn* TestPawn = It->Get();
		if (TestPawn == nullptr || USHealthComponent::IsFriendly(TestPawn, this))
		{
			continue;
		}

		USHealthComponent* TestPawnHealthComp = Cast<USHealthComponent>(TestPawn->GetComponentByClass(USHealthComponent::StaticClass()));
		if (TestPawnHealthComp && TestPawnHealthComp->GetHealth() > 0)
		{
			float Distance = (TestPawn->GetActorLocation() - GetActorLocation()).Size();

			if (Distance < NearestTargetDistance)
			{
				BestTarget = TestPawn;
				NearestTargetDistance = Distance;
			}
		}
	}

	if (BestTarget != nullptr)
	{
		UNavigationPath* NavPath = UNavigationSystemV1::FindPathToActorSynchronously(this, GetActorLocation(), BestTarget);

		GetWorldTimerManager().ClearTimer(TimerHandle_RefreshPath);
		GetWorldTimerManager().SetTimer(TimerHandle_RefreshPath, this, &ASTrackerBot::RefreshPath, 5.0f, false);

		if (NavPath != nullptr && NavPath->PathPoints.Num() > 1)
		{
			// Return next point in the path.
			return NavPath->PathPoints[1];
		}
	}

	// Failed to find path
	return GetActorLocation();
}

void ASTrackerBot::HandleTakeDamage(USHealthComponent* OwningHealthComponent, float Health,
	float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy,
	AActor* DamageCauser)
{
	// Explode on hitpoint == 0

	if (MatInst == nullptr) {
		MatInst = MeshComp->CreateDynamicMaterialInstance(0, MeshComp->GetMaterial(0));
	}

	if (MatInst != nullptr) {
		MatInst->SetScalarParameterValue("LastTimeDamageTaken", GetWorld()->TimeSeconds);
	}

	if (Health <= 0.0f)
	{
		SelfDestract();
	}
}

void ASTrackerBot::SelfDestract()
{
	if (bExploded) {
		return;
	}

	bExploded = true;

	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, GetActorLocation());
	UGameplayStatics::PlaySoundAtLocation(this, ExploadSound, GetActorLocation());

	MeshComp->SetVisibility(false, true);
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if (Role == ROLE_Authority)
	{
		TArray<AActor*> IgnoredActors;
		IgnoredActors.Add(this);

		// ApplyDamage.
		UGameplayStatics::ApplyRadialDamage(this, ExplosionDamage, GetActorLocation(), ExplosionRadius,
			nullptr, IgnoredActors, this, GetInstigatorController(), true);

		if (DebugTrackerBotDrawing)
		{
			DrawDebugSphere(GetWorld(), GetActorLocation(), ExplosionRadius, 12, FColor::Red, false, 2.0f, 0, 1);

			SetLifeSpan(2.0f);
		}

		
	}
}

void ASTrackerBot::DamageSelf()
{
	UGameplayStatics::ApplyDamage(this, 20.0f, GetInstigatorController(), this, nullptr);
}

void ASTrackerBot::RefreshPath()
{
	NextPathPoint = GetNextPathPoint();
}

// Called every frame
void ASTrackerBot::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if(Role == ROLE_Authority && !bExploded)
	{
		float DistatnceToTarget = (GetActorLocation() - NextPathPoint).Size();

		if (DistatnceToTarget <= RequiredDistanceToTarget)
		{
			NextPathPoint = GetNextPathPoint();

			if (DebugTrackerBotDrawing)
			{
				DrawDebugString(GetWorld(), GetActorLocation(), FString("TargetReached"));
			}
		}
		else
		{
			//Keep moving towards next target.
			FVector ForceDirection = NextPathPoint - GetActorLocation();
			ForceDirection.Normalize();

			ForceDirection *= MovementForce;

			MeshComp->AddForce(ForceDirection, NAME_None, bUseVelocityChange);
			
			if (DebugTrackerBotDrawing)
			{
				DrawDebugDirectionalArrow(GetWorld(), GetActorLocation(), GetActorLocation() + ForceDirection, 32, FColor::Yellow, false, 0.0f, 0, 1.0f);
			}
		}
		if (DebugTrackerBotDrawing)
		{
			DrawDebugSphere(GetWorld(), NextPathPoint, 20, 12, FColor::Yellow, false, 0.0f, 1.0f);
		}
	}
}
