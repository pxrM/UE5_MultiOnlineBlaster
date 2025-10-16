// Fill out your copyright notice in the Description page of Project Settings.


#include "HealthPickup.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/BlasterComponent/BuffComponent.h"

AHealthPickup::AHealthPickup()
{
	bReplicates = true;

}

void AHealthPickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	if (const ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor))
	{
		if (UBuffComponent* Buff = BlasterCharacter->GetBuffComp())
		{
			Buff->Heal(HealAmount, HealingTime);
		}
	}
	Destroy();
}