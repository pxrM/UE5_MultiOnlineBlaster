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
	void SaveFramePackage(FFramePackage& Package);

public:
	void ShowFramePackage(const FFramePackage& Package, const FColor Color);

private:
	UPROPERTY()
		ABlasterCharacter* Character;
	UPROPERTY()
		class ABlasterPlayerController* Controller;


};
