// Fill out your copyright notice in the Description page of Project Settings.
/*
	�����ӳٲ������
*/

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LagCompensationComponent.generated.h"


/// <summary>
/// ��ɫ������
/// </summary>
USTRUCT(BlueprintType)
struct FFramePackage
{
	GENERATED_BODY()

		/// <summary>
		/// �洢ʱ��
		/// </summary>
		UPROPERTY()
		float Time;
	UPROPERTY()
		TMap<FName, FBoxInformation> HitBoxInfo;
};

/// <summary>
/// ���ڲ���ֱ�Ӵ�box��ָ�룬��Ϊָ��ָ����ǵ�ַ��������Ҫ���������ݽṹ�洢���ӵ���Ϣ
/// </summary>
USTRUCT(BlueprintType)
struct FBoxInformation
{
	GENERATED_BODY()

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

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class BLASTER_API ULagCompensationComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	ULagCompensationComponent();
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;


public:



};
