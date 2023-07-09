// Fill out your copyright notice in the Description page of Project Settings.


#include "Projectile.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Sound/SoundCue.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/Blaster.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"

// Sets default values
AProjectile::AProjectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	SetRootComponent(CollisionBox);
	//设置碰撞类型为 ECC_WorldDynamic，表示该碰撞体与 WorldDynamic 类型的物体产生碰撞。
	CollisionBox->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	//设置碰撞可否，QueryAndPhysics 表示该碰撞体既能发生物理碰撞，又可以被其他物体查询是否有碰撞。
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	//设置该碰撞体与所有 Channel 的碰撞响应方式为 ECR_Ignore，即忽略所有 Channel 的碰撞。
	CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	//分别针对 ECC_Visibility 和 ECC_WorldStatic 两个 Channel 重新设置碰撞响应方式为 ECR_Block，
	//表示该碰撞体和 Visibility、WorldStatic 这两种类型的物体发生碰撞后，将被阻挡。
	CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);
	/*
	* 问题：这里碰撞通道并不准确，因为是通过角色身上的胶囊体碰撞的，胶囊体的大小有时候并不完全准确，所以有时候会和角色模型外的区域进行碰撞
	*			应该设法击中角色mesh而不是角色身上的胶囊体
	*			可以为角色网格专门创建一个自定义的碰撞通道（项目设置-Engine-Collision中设置）
	*/
	//CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Block);
	CollisionBox->SetCollisionResponseToChannel(ECC_SkeletalMesh, ECollisionResponse::ECR_Block);

	/*改到子类添加，以支持不同类型的移动控件
	//"UProjectileMovementComponent" 是UE中用于控制投射物（Projectile）的运动轨迹的组件类。
	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	//表示该组件将根据投射物的移动方向来旋转投射物。这使得投射物在飞行过程中，能够始终保持朝向移动的方向，看起来更加自然，也更易于控制。
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	*/
}

// Called when the game starts or when spawned
void AProjectile::BeginPlay()
{
	Super::BeginPlay();
	
	//在武器发射时，在武器前方创建一条轨迹特效的功能
	if (Tracer)
	{
		//则使用 "UGameplayStatics::SpawnEmitterAttached" 方法在 "CollisionBox" 网格上生成 "Tracer" 特效。
		//最后一个参数 "EAttachLocation::KeepWorldPosition" 表示该特效将保持在世界空间中不变。这意味着，该特效将不会受到源对象的任何转换或平移操作的影响。
		TracerComponent = UGameplayStatics::SpawnEmitterAttached(
			Tracer, CollisionBox, FName(),
			GetActorLocation(), GetActorRotation(),
			EAttachLocation::KeepWorldPosition
		);
	}

	if (HasAuthority())
	{
		CollisionBox->OnComponentHit.AddDynamic(this, &AProjectile::OnHit); //将OnHit函数绑定到CollisionBox组件的碰撞事件上，在碰撞事件触发时自动执行该函数。
	}
}

// Called every frame
void AProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	//ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor);
	//if (BlasterCharacter)
	//{
	//	BlasterCharacter->MulticastHit();
	//} //这里改为由更新角色健康值时触发，健康值会被同步到所有客服端，放到这那里会减少一次网络广播消耗

	Destroy(); //会在服务器和所有客户端进行广播
}

void AProjectile::SpawnTrailSystem()
{
	if (TrailSystem)
	{
		//在场景中附加并生成一个 Niagara 系统
		TrailSystemComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
			TrailSystem,//要生成的 Niagara 系统的指针。
			GetRootComponent(),//用于确定要附加生成的 Niagara 系统的位置的根组件。
			FName(),//附加点名称，可以指定骨骼名或者插槽名
			GetActorLocation(),
			GetActorRotation(),
			EAttachLocation::KeepWorldPosition,//保持其在世界空间中的位置。
			false //表示生成的 Niagara 系统不具有自动销毁功能。
		);
	}
}

void AProjectile::CollideManifestation()
{
	if (ImpactParticles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, GetActorTransform());
	}
	if (ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
	}
}

void AProjectile::StartDestroyTimer()
{
	//添加计时器延迟烟雾消失
	GetWorldTimerManager().SetTimer(
		TrailDestroyTimer,
		this,
		&AProjectile::TrailDestroyTimerFinished,
		TrailDestroyTime
	);
}

void AProjectile::TrailDestroyTimerFinished()
{
	Destroy();
}

void AProjectile::ExplodeDamage()
{
	//获取发射玩家的控制器
	APawn* FiringPawn = GetInstigator(); //SpawnParams.Instigator = InstigatorPawn;
	if (FiringPawn && HasAuthority()) //服务器执行的代码块
	{
		AController* FiringController = FiringPawn->GetController();
		if (FiringController)
		{
			// 带有衰减效果的径向伤害
			UGameplayStatics::ApplyRadialDamageWithFalloff(
				this, //世界上下文
				DamageVal,	//基础伤害
				10.f, //最小伤害
				GetActorLocation(),	//伤害范围的圆心点
				DamageInnerRadius, //伤害内半径
				DamageOuterRadius, //伤害外半径
				1.f, //衰减因子，控制伤害随距离增加而衰减的速度
				UDamageType::StaticClass(), //要应用的伤害类型
				TArray<AActor*>(), //忽略伤害的actor
				this, //伤害的来源对象
				FiringController //伤害的来源对象的控制器
			);
		}
	}
}

void AProjectile::Destroyed()
{
	Super::Destroyed();

	CollideManifestation();
}