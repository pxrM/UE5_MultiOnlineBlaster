// Fill out your copyright notice in the Description page of Project Settings.
/*
	��ɫս����� ��������
*/

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Blaster/HUD/BlasterHUD.h"
#include "Blaster/Weapon/WeaponTypes.h"
#include "Blaster/BlasterTypes/CombatState.h"
#include "CombatComponent.generated.h"

#define TRACE_LENGTH 8000.f

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class BLASTER_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UCombatComponent();
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;


public:
	friend class ABlasterCharacter;	//�ǽ�ɫ����Է���������


private:
	UPROPERTY()
		class ABlasterCharacter* Character;
	UPROPERTY()
		class ABlasterPlayerController* Controller;
	UPROPERTY()
		class ABlasterHUD* HUD;

	//��ǰװ��������
	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
		class AWeapon* EquippedWeapon;

	UPROPERTY(Replicated)
		bool bAiming; //�Ƿ�������׼

	UPROPERTY(EditAnywhere)
		float BaseWalkSpeed; //ԭʼ�ٶ�

	UPROPERTY(EditAnywhere)
		float AimWalkSpeed; //��׼ʱ�ٶ�

	bool bFireBtnPressed; //�Ƿ��¿����

	FVector HitTarget;	//���Ŀ��λ��

	/*	 ʮ��׼��	*/
	float CrosshairVelocityFactor; //ʮ��׼�ߵ����ţ������Ϸ��ʮ��׼�߻���ݽ�ɫ��λ���ƶ���΢�ſ�
	float CrosshairInAirFactor;    //��ɫ�ڿ���ʱʮ��׼�ߵ�����
	float CrosshairInAimFactor;    //��ɫ����׼ʱʮ��׼�ߵ�����
	float CrosshairShootingFactor; //��ɫ�ڿ���ʱʮ��׼�ߵ�����
	FHUDPackage HUDPackage; //ʮ��׼����ͼ�ṹ��

	/*	 ��׼��Ұ	*/
	float DefultFOV;	//û��׼ʱ��Ĭ����Ұ
	float CurrentFOV;	//��ǰ��Ұ
	UPROPERTY(EditAnywhere, Category = Combat)
		float ZoomedFOV = 30.f;  //��׼ʱ�ķŴ���Ұ
	UPROPERTY(EditAnywhere, Category = Combat)
		float ZoomInterpSpeed = 20.f;  //��׼ʱ����Ұ�����ٶ�

	/*	 �Զ�����	*/
	FTimerHandle FireTimer;	//�����ʱ��
	bool bCanFire = true; //�Ƿ���Կ���

	TMap<EWeaponType, int32> CarriedAmmoMap;	//��ͬ�������͵ĵ�ҩ����
	UPROPERTY(ReplicatedUsing = OnRep_CurWeaponCarriedAmmo)
		int32 CurWeaponCarriedAmmo;  //Я����ҩ������ǰװ��������
	UPROPERTY(EditAnywhere, Category = Combat)
		int32 StartingARAmmo = 30;	//������ʼ��ÿ�������ĵ�ҩ����

	UPROPERTY(ReplicatedUsing = OnRep_CombatState)
		ECombatState CombatState = ECombatState::ECS_Unoccupied; //ս��״̬


public:
	/// <summary>
	/// ���ڻ�ȡ��Ҫ��������ͬ���������б�
	/// </summary>
	/// <param name="OutLifetimeProps">��Ҫ��������ͬ���������б�</param>
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/// <summary>
	/// ������װ������ǰ��ɫ���ϡ�ֻ�ڷ���������
	/// </summary>
	/// <param name="WeaponToEquip"></param>
	void EquipWeapon(AWeapon* WeaponToEquip);

	/// <summary>
	/// ���¼��ص��У��ͻ��˷���
	/// </summary>
	void ReloadMag();

	/// <summary>
	/// ���¼��ص��н���
	/// �ڶ�����̫������Ӷ����¼�����Game/Blueprints/Character/Animation/ReloadMag
	/// </summary>
	UFUNCTION(BlueprintCallable)
		void FinishReloadMag();


protected:
	void SetAiming(bool bIsAiming);

	UFUNCTION(Server, Reliable)
		void ServerSetAiming(bool bIsAiming);

	UFUNCTION()
		void OnRep_EquippedWeapon();

	UFUNCTION()
		void FireBtnPressed(bool bPressed);

	void Fire();

	/*  FVector_NetQuantize ��ue���������紫��Ľṹ�壬����ѹ�����Ż� FVector �����ݴ��䡣
		�ýṹ����Խ� FVector ��ֵ�����紫��ʱ����ѹ����ʹ���ݴ�С��С���������縺�غʹ����ӳ١�
		FVector_NetQuantize ֧��ÿ���������ʹ�� 20 λ�����������ھ��Ⱥ�ʵʱ��֮������һ��ƽ�⡣*/
	UFUNCTION(Server, Reliable)
		void ServerFire(const FVector_NetQuantize& TraceHitTarget);

	//���Ϊ NetMulticast �� Reliable������ζ�Ÿú������ڸ����ͻ����Ͻ��е��ã����Ҹú�����ִ�н�����ӿͻ���ͬ�����������������ͻ��ˡ�
	//�ڶ�����Ϸ�У��˺���ͨ�����������пͻ��˹㲥ĳЩ���������������пͻ��������ɱ�ըЧ����
	UFUNCTION(NetMulticast, Reliable)
		void MulticastFire(const FVector_NetQuantize& TraceHitTarget);

	UFUNCTION(Server, Reliable)
		void ServerReloadMag();	//����������
	void HandleReloadMag();	//�������Ϳͻ��˵���

	//���߼�⣬���ڼ�����׼��λ������Ӧ������ռ�λ�úͷ���
	void TraceUnderCroshairs(FHitResult& TraceHitResult);

	//����hudʮ��׼��
	void SetHUDCrosshairs(float DeltaTime);


private:
	//��׼ʱ������Ұ���ŵĺ���
	void InterpFOV(float DeltaTime);

	//�����Զ������ʱ��
	void StartFireTimer();
	//�Զ������ʱ���ص�
	void FireTimerFinished();

	bool CanFire(); //�Ƿ�ɿ���

	void InitializeCarriedAmmo(); //��ʼ������

	UFUNCTION()
		void OnRep_CurWeaponCarriedAmmo();

	UFUNCTION()
		void OnRep_CombatState();

};
