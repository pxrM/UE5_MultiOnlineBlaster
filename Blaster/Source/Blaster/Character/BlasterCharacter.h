// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Blaster/BlasterTypes/TurningInPlace.h"
#include "BlasterCharacter.generated.h"

UCLASS()
class BLASTER_API ABlasterCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ABlasterCharacter();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	//-----------------------------------------------------------------------------------------------------------

private:
	/// <summary>
	/// ������ɸ�
	/// </summary>
	UPROPERTY(VisibleAnywhere, Category = Camera)
		class USpringArmComponent* CameraBoom;

	/// <summary>
	/// ��ɫ�������
	/// </summary>
	UPROPERTY(VisibleAnywhere, Category = Camera)
		class UCameraComponent* FollowCamera;

	/// <summary>
	/// ͷ��ui
	/// </summary>
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		class UWidgetComponent* OverheadWidget;

	//UPROPERTY(Replicated)//���ڱ��ĳ����Ա������Ҫ����ͬ�����ƣ�Replicated��
	//UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon) ���ڱ��ĳ����Ա������Ҫ����ͬ�����ƣ���ָ�����ñ�������ʱ��Ҫ���õĻص�����
	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
		class AWeapon* OverlappingWeapon;

	UPROPERTY(VisibleAnywhere)
		class UCombatComponent* CombatCmp;

	float AO_Yaw;
	float InterpAO_Yaw;
	float AO_Pitch;
	FRotator StartingAimRotation; //���ܻ���Ծ������Ľ�ɫ��ʼĿ��ֵ

	ETurningInPlace TurningInPlace; //��ɫת��


protected:
	void MoveForward(float Value);
	void MoveRight(float Value);
	void Turn(float Value);
	void LookUp(float Value);
	void EquipBtnPressed();
	void CrouchBtnPressed();
	void AimBtnPressed();
	void AimBtnReleased();
	void AimOffset(float DeltaTime);
	virtual void Jump() override;
	void FireBtnPressed();
	void FireBtnReleased();


public:
	/// <summary>
	/// ���ڻ�ȡ��Ҫ��������ͬ���������б�
	/// </summary>
	/// <param name="OutLifetimeProps">��Ҫ��������ͬ���������б�</param>
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	// �� Actor �������ʼ������ C++ �����н��ж���ĳ�ʼ���������ú�������Ϸ����ʱ�����á�
	virtual void PostInitializeComponents() override;
	//������������������
	//FORCEINLINE void SetOverlappingWeapon(AWeapon* Weapon) { OverlappingWeapon = Weapon; }
	void SetOverlappingWeapon(AWeapon* Weapon);
	//�Ƿ�װ��������
	bool IsWeaponEquipped();
	//�Ƿ�������׼
	bool IsAiming();

	FORCEINLINE float GetAO_Yaw()const { return AO_Yaw; }
	FORCEINLINE float GetAO_Pitch()const { return AO_Pitch; }

	AWeapon* GetEquippedWeapon();

	FORCEINLINE ETurningInPlace GetTurningInPlace()const { return TurningInPlace; }

private:
	/// <summary>
	/// �� OverlappingWeapon �����ڿͻ����ϸ���ʱ������֪ͨ�ͻ��˸��£������Զ����øûص��������д���
	/// </summary>
	UFUNCTION()
		void OnRep_OverlappingWeapon(AWeapon* LastWeapon);
	/// <summary>
	/// �ڿͻ��˵��øú���ʱʵ���ϻᷢ��һ�� RPC ���󵽷����������������ִ����ʵ�ְ汾=ServerEquipBtnPressed_Implementation��
	/// https://docs.unrealengine.com/5.1/zh-CN/rpcs-in-unreal-engine/
	/// </summary>
	UFUNCTION(Server, Reliable)	//����Ϊһ��Ҫ�ڿͻ����ϵ��á�����Ҫ�ڷ�������ִ�д���� RPC����ʹ�ÿɿ������紫�䷽ʽ����ͨ�š�
		void ServerEquipBtnPressed();

	void TurnInPlace(float DeltaTime);

};
