// Fill out your copyright notice in the Description page of Project Settings.


#include "Casing.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

// Sets default values
ACasing::ACasing()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	CasingMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CasingMesh"));
	SetRootComponent(CasingMesh);

	CasingMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore); //ȡ�����������ײ
	CasingMesh->SetSimulatePhysics(true); //����ģ������
	CasingMesh->SetEnableGravity(true); //��������
	CasingMesh->SetNotifyRigidBodyCollision(true); //������ײģ����������ײ�巢����ײʱ����RigidBody�� OnCollisionEnter() �� OnCollisionExit() ������
	ShellEjectImpulse = 2.f;
}

// Called when the game starts or when spawned
void ACasing::BeginPlay()
{
	Super::BeginPlay();
	
	CasingMesh->OnComponentHit.AddDynamic(this, &ACasing::OnHit);
	//AddImpulse�������ɫʩ��һ��˲ʱ������GetActorForwardVector�ǻ�ȡ��ɫ����������ϵ�е���ǰ����һ���������
	CasingMesh->AddImpulse(GetActorForwardVector() * ShellEjectImpulse);
}

void ACasing::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (ShellSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ShellSound, GetActorLocation());
	}
	Destroy();
}

// Called every frame
void ACasing::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

