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

	CasingMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore); //取消对相机的碰撞
	CasingMesh->SetSimulatePhysics(true); //设置模拟物理
	CasingMesh->SetEnableGravity(true); //开启重力
	CasingMesh->SetNotifyRigidBodyCollision(true); //设置碰撞模型与其他碰撞体发生碰撞时触发RigidBody的 OnCollisionEnter() 和 OnCollisionExit() 方法。
	ShellEjectImpulse = 2.f;
}

// Called when the game starts or when spawned
void ACasing::BeginPlay()
{
	Super::BeginPlay();
	
	CasingMesh->OnComponentHit.AddDynamic(this, &ACasing::OnHit);
	//AddImpulse用于向角色施加一个瞬时的力，GetActorForwardVector是获取角色在世界坐标系中的正前方归一化后的向量
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

