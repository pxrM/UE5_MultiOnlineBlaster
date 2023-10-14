// Fill out your copyright notice in the Description page of Project Settings.
/*
	�����ӳٲ������
*/

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LagCompensationComponent.generated.h"


/// <summary>
/// ���ڲ���ֱ�Ӵ�box��ָ�룬��Ϊָ��ָ����ǵ�ַ��������Ҫ���������ݽṹ�洢���ӵ���Ϣ
/// </summary>
USTRUCT(BlueprintType)
struct FBoxInformation
{
	GENERATED_BODY()
public:
	/// <summary>
	/// λ��
	/// </summary>
	UPROPERTY()
		FVector Location;
	/// <summary>
	/// ��ת
	/// </summary>
	UPROPERTY()
		FRotator Rotation;
	/// <summary>
	/// ���ӷ�Χ
	/// </summary>
	UPROPERTY()
		FVector BoxExtent;
};

/// <summary>
/// ��ɫ������
/// </summary>
USTRUCT(BlueprintType)
struct FFramePackage
{
	GENERATED_BODY()
public:
	/// <summary>
	/// �洢ʱ��
	/// </summary>
	UPROPERTY()
		float Time;
	/// <summary>
	/// ��ɫ��ͬ��λ����Ӧ��box��Ϣ
	/// </summary>
	UPROPERTY()
		TMap<FName, FBoxInformation> HitBoxInfo;
	/// <summary>
	/// ��ɫָ��
	/// </summary>
	UPROPERTY()
		ABlasterCharacter* Character;
};

/// <summary>
/// �������������н��
/// </summary>
USTRUCT(BlueprintType)
struct FServerSideRewindResult
{
	GENERATED_BODY()
public:
	/// <summary>
	/// �Ƿ�����
	/// </summary>
	UPROPERTY()
		bool bHitConfirmed;
	/// <summary>
	/// �Ƿ�ͷ
	/// </summary>
	UPROPERTY()
		bool bHeadShot;
};

/// <summary>
/// ����ǹ���������������н��
/// </summary>
USTRUCT(BlueprintType)
struct FShotgunServerSideRewindResult
{
	GENERATED_BODY()
public:
	/// <summary>
	/// ÿ����ɫ��ͷ�����д���
	/// </summary>
	UPROPERTY()
		TMap<ABlasterCharacter*, uint32> HeadShots;
	/// <summary>
	/// ÿ����ɫ��������д���
	/// </summary>
	UPROPERTY()
		TMap<ABlasterCharacter*, uint32> BodyShots;
};



UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class BLASTER_API ULagCompensationComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	friend class ABlasterCharacter;
public:
	// Sets default values for this component's properties
	ULagCompensationComponent();
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
protected:
	// Called when the game starts
	virtual void BeginPlay() override;


private:
	/// <summary>
	/// ����ÿһ֡��box���ݰ�
	/// </summary>
	void SaveFramePackage();
	/// <summary>
	/// ����һ֡�����ݰ�
	/// </summary>
	/// <param name="Package"></param>
	void SaveFramePackage(FFramePackage& Package);
	/// <summary>
	/// ��������ʱ���ȡ��Ҫ����֡���ݰ�
	/// </summary>
	/// <param name="HitCharacter">���еĽ�ɫ</param>
	/// <param name="HitTime">����ʱ��</param>
	/// <returns></returns>
	FFramePackage GetFrameToCheck(ABlasterCharacter* HitCharacter, float HitTime);
	/// <summary>
	/// ������ʱ���ǰһ֡�ͺ�һ֮֡����в�ֵ����
	/// </summary>
	/// <param name="OlderFrmae"></param>
	/// <param name="YoungerFrame"></param>
	/// <param name="HitTime"></param>
	/// <returns></returns>
	FFramePackage InterpBetweenFrames(const FFramePackage& OlderFrmae, const FFramePackage& YoungerFrame, float HitTime);
	/// <summary>
	/// �����ɫ��ǰbox��λ��
	/// </summary>
	/// <param name="HitCharacter"></param>
	/// <param name="OutFrameackage"></param>
	void CacheBoxPositions(ABlasterCharacter* HitCharacter, FFramePackage& OutFrameackage);
	/// <summary>
	/// �ƶ����н�ɫ��box��ָ��ʱ���λ��
	/// </summary>
	/// <param name="HitCharacter"></param>
	/// <param name="Package"></param>
	void MoveBoxes(ABlasterCharacter* HitCharacter, const FFramePackage& Package);
	/// <summary>
	/// �ָ����н�ɫbox��λ�õ�����λ��
	/// </summary>
	/// <param name="HitCharacter"></param>
	/// <param name="Package"></param>
	void ResetHitBoxes(ABlasterCharacter* HitCharacter, const FFramePackage& Package);
	/// <summary>
	/// ���ý�ɫ�������ײ�Ƿ���
	/// </summary>
	/// <param name="HitCharacter"></param>
	/// <param name="CollisionEnabled"></param>
	void EnableCharacterMeshCollision(ABlasterCharacter* HitCharacter, ECollisionEnabled::Type CollisionEnabled);

	/*
		������������������
	*/
	/// <summary>
	/// �������������
	/// </summary>
	/// <param name="HitCharacter">���еĽ�ɫ</param>
	/// <param name="TraceStart">�����ʼλ��</param>
	/// <param name="HitLocation">����λ��</param>
	/// <param name="HitTime">����ʱ��</param>
	/// <returns>���н��</returns>
	FServerSideRewindResult ServerSideRewind(
		ABlasterCharacter* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize& HitLocation,
		float HitTime);
	/// <summary>
	/// �������н�������ߣ�
	/// </summary>
	/// <param name="Package">���������Ľ����</param>
	/// <param name="HitCharacter">���еĽ�ɫ</param>
	/// <param name="TraceStart">�����ʼλ��</param>
	/// <param name="HitLocaton">����λ��</param>
	/// <returns></returns>
	FServerSideRewindResult ConfirmHit(
		const FFramePackage& Package,
		ABlasterCharacter* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize& HitLocation);

	/*
		������������������
	*/
	/// <summary>
	/// ������������������
	/// </summary>
	/// <param name="HitCharacter"></param>
	/// <param name="TraceStart">��ʼ�������</param>
	/// <param name="InitialVelocity">��ʼ�ٶ�</param>
	/// <param name="HitTime"></param>
	/// <returns></returns>
	FServerSideRewindResult ProjectileServerSideRewind(
		ABlasterCharacter* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize100& InitialVelocity,
		float HitTime);
	/// <summary>
	/// �������н��������·����
	/// </summary>
	/// <param name="HitCharacter"></param>
	/// <param name="TraceStart"></param>
	/// <param name="InitialVelocity"></param>
	/// <param name="HitTime"></param>
	/// <returns></returns>
	FServerSideRewindResult ProjectileConfirmHit(
		const FFramePackage& Package,
		ABlasterCharacter* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize100& InitialVelocity,
		float HitTime);

	/*
		����ǹ��������
	*/
	/// <summary>
	/// �������������
	/// </summary>
	/// <param name="HitCharacters">������н�ɫ</param>
	/// <param name="TraceStart">�����ʼλ��</param>
	/// <param name="HitLocations">�������λ��</param>
	/// <param name="HitTime">����ʱ��</param>
	/// <returns></returns>
	FShotgunServerSideRewindResult ShotgunServerSideRewind(
		const TArray<ABlasterCharacter*>& HitCharacters,
		const FVector_NetQuantize& TraceStart,
		const TArray<FVector_NetQuantize>& HitLocations,
		float HitTime);
	/// <summary>
	/// �������н���������ߣ�
	/// </summary>
	/// <param name="HitCharacters"></param>
	/// <param name="TraceStart"></param>
	/// <param name="HitLocations"></param>
	/// <returns></returns>
	FShotgunServerSideRewindResult ShotgunConfirmHit(
		const TArray<FFramePackage>& FramePackages,
		const FVector_NetQuantize& TraceStart,
		const TArray<FVector_NetQuantize>& HitLocations);


public:
	/// <summary>
	/// debug ��ʾ���ݵ�box��
	/// </summary>
	/// <param name="Package"></param>
	/// <param name="Color"></param>
	void ShowFramePackage(const FFramePackage& Package, const FColor Color);

	/// <summary>
	/// ������������������������󹥻��������ȡ����
	/// </summary>
	/// <param name="HitCharacter">���еĽ�ɫ</param>
	/// <param name="TraceStart">�����ʼλ��</param>
	/// <param name="HitLocation">����λ��</param>
	/// <param name="HitTime">����ʱ��</param>
	/// <param name="DamageCauser">�˺�ԭ��</param>
	UFUNCTION(Server, Reliable)
		void ServerScoreRequest(
			ABlasterCharacter* HitCharacter,
			const FVector_NetQuantize& TraceStart,
			const FVector_NetQuantize& HitLocation,
			float HitTime
		);
	/// <summary>
	/// ���䵯������������������󹥻��������ȡ����
	/// </summary>
	/// <param name="HitCharacter"></param>
	/// <param name="TraceStart"></param>
	/// <param name="InitialVelocity"></param>
	/// <param name="HitTime"></param>
	UFUNCTION(Server, Reliable)
		void ProjectileServerScoreRequest(
			ABlasterCharacter* HitCharacter,
			const FVector_NetQuantize& TraceStart,
			const FVector_NetQuantize100& InitialVelocity,
			float HitTime
		);
	/// <summary>
	/// ������ǹ����������������󹥻��������ȡ����
	/// </summary>
	/// <param name="HitCharacters"></param>
	/// <param name="TraceStart"></param>
	/// <param name="HitLocations"></param>
	/// <param name="HitTime"></param>
	UFUNCTION(Server, Reliable)
		void ShotgunServerScoreRequest(
			const TArray<ABlasterCharacter*>& HitCharacters,
			const FVector_NetQuantize& TraceStart,
			const TArray<FVector_NetQuantize>& HitLocations,
			float HitTime
		);


private:
	UPROPERTY()
		ABlasterCharacter* Character;
	UPROPERTY()
		class ABlasterPlayerController* Controller;
	/// <summary>
	/// �洢4���֡���ݣ�ʱ��̫����̫�̶���Ӱ������
	/// </summary>
	UPROPERTY(EditAnywhere)
		float MaxRecordTime = 4.f;
	/// <summary>
	/// �洢��MaxRecordTime��ʱ���ڷ�����֡���ݣ�ʹ��˫�������������ͷβ�ڵ�ִ����Ӻ��Ƴ�����
	/// </summary>
	TDoubleLinkedList<FFramePackage> FrameHistory;

};
