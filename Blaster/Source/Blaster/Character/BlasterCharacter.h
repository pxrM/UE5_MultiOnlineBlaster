// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Blaster/BlasterTypes/TurningInPlace.h"
#include "Blaster/Interface/InteractWithCrosshairsInterface.h"
#include "BlasterCharacter.generated.h"

UCLASS()
class BLASTER_API ABlasterCharacter : public ACharacter, public IInteractWithCrosshairsInterface
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

	UPROPERTY(EditAnyWhere, Category = Combat)
		class UAnimMontage* FireWeaponMontage;	//������̫�涯�������ڽ�ɫ��ͼָ��

	UPROPERTY(EditAnyWhere, Category = Combat)
		UAnimMontage* HitReactMontage; //�ܻ���̫�涯��

	UPROPERTY(EditAnyWhere)
		float CameraThreshold = 200.f; //����ͽ�ɫ������ֵ

	/*  �����ɫʹ��  */
	bool bRotateRootBone;	//�Ƿ���ת������
	float TurnThreshold = 0.5f; //ԭ��ת�����ֵ
	FRotator ProxyRotationLastFrame;  //������һ�ε���תֵ
	FRotator ProxyRotationCur;
	float ProxyYawOffset;
	float TimeSinceLastMovementReplication; //��һ�δ����ɫ�ƶ����������ͬ��ʱ��

	/*  player health  */
	UPROPERTY(EditAnywhere, Category = "Player Stats")
		float MaxHealth = 100.f; //��󽡿�ֵ
	UPROPERTY(ReplicatedUsing = OnRep_CurHealth, VisibleAnywhere, Category = "Player Stats")
		float CurHealth = MaxHealth;


protected:
	void MoveForward(float Value);
	void MoveRight(float Value);
	void Turn(float Value);
	void LookUp(float Value);
	void EquipBtnPressed();
	void CrouchBtnPressed();
	void AimBtnPressed();
	void AimBtnReleased();
	virtual void Jump() override;
	void FireBtnPressed();
	void FireBtnReleased();

	void AimOffset(float DeltaTime);
	void CalculateAO_Pitch();
	void SimProxiesTurn(); //ģ�����ʹ��


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
	FORCEINLINE bool ShouldRotateRootBone() const { return bRotateRootBone; }

	AWeapon* GetEquippedWeapon();

	FORCEINLINE ETurningInPlace GetTurningInPlace()const { return TurningInPlace; }

	void PlayFireMontage(bool bAiming);
	void PlayHitReactMontage();

	FVector GetHitTarget() const;

	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	UFUNCTION(NetMulticast, Unreliable)
		void MulticastHit();	//�����ܻ����� NetMulticast��ӷ����ͬ�������пͻ��� Unreliable��ʾͬ����Ϣ���ɿ�

	//���ö����ڷ������ϵ��˶�״̬�����仯ʱ���ͻ��˻�ͨ���ú����յ�֪ͨ�����¶�Ӧ���˶�״̬��
	virtual void OnRep_ReplicatedMovement() override;


private:
	/// <summary>
	/// �� OverlappingWeapon �����ڿͻ����ϸ���ʱ������֪ͨ�ͻ��˸��£������Զ����øûص��������д���
	/// </summary>
	UFUNCTION()
		void OnRep_OverlappingWeapon(AWeapon* LastWeapon);
	/// <summary>
	/// ͬ��CurHealth
	/// </summary>
	UFUNCTION()
		void OnRep_CurHealth();

	/// <summary>
	/// �ڿͻ��˵��øú���ʱʵ���ϻᷢ��һ�� RPC ���󵽷����������������ִ����ʵ�ְ汾=ServerEquipBtnPressed_Implementation��
	/// https://docs.unrealengine.com/5.1/zh-CN/rpcs-in-unreal-engine/
	/// </summary>
	UFUNCTION(Server, Reliable)	//����Ϊһ��Ҫ�ڿͻ����ϵ��á�����Ҫ�ڷ�������ִ�д���� RPC����ʹ�ÿɿ������紫�䷽ʽ����ͨ�š�
		void ServerEquipBtnPressed();

	void TurnInPlace(float DeltaTime);

	void HideCameraIfCharacterClose(); //�����ɫ��ǽʱ��������ɫ��Ӷ���ס��Ұ��̫��ʱ���ؽ�ɫ

	float CalculateSpeed();

};
