// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "BlasterHUD.generated.h"


USTRUCT(BlueprintType)
struct FHUDPackage
{
	GENERATED_BODY()
public:
	class UTexture2D* CrosshairsCenter;  // ʮ����׼��ͼ  ��
	UTexture2D* CrosshairsLeft;			 // ʮ����׼��ͼ  ��
	UTexture2D* CrosshairsRight;	     // ʮ����׼��ͼ  ��
	UTexture2D* CrosshairsTop;			 // ʮ����׼��ͼ  ��
	UTexture2D* CrosshairsBottom;		 // ʮ����׼��ͼ  ��
	float CrosshairSpread;				 // ʮ����׼ɢ��ֵ �����Ϸ��ʮ��׼�߻���ݽ�ɫ��λ���ƶ���΢�ſ�
	FLinearColor CrosshairColor;		 // ʮ����׼color
};

/**
 *
 */
UCLASS()
class BLASTER_API ABlasterHUD : public AHUD
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
	//��д�����е�DrawHUD()��������ʵ���Զ�����û����棨UI����Ⱦ�߼�
	virtual void DrawHUD() override;

public:
	/// <summary>
	/// ��ӽ�ɫ��Ϣui
	/// </summary>
	void AddCharacterOverlay();
	/// <summary>
	/// ���ƥ��״̬����
	/// </summary>
	void AddStateAnnouncement();
	/// <summary>
	/// �����̭����
	/// </summary>
	/// <param name="AttackerName"></param>
	/// <param name="VictimName"></param>
	void AddElimAnnouncement(FString AttackerName, FString VictimName);


public:
	FORCEINLINE void SetHUDPackage(const FHUDPackage& Package) { HUDPackage = Package; }


private:
	/// <summary>
	/// ����׼�ģ���׼�����ĵ㡢����������ͼ���
	/// </summary>
	/// <param name="Texture">Ҫ���Ƶ�׼������</param>
	/// <param name="ViewportCenter">��Ļ���ĵ�</param>
	/// <param name="Spread">��������ĵ��׼���ſ�����</param>
	/// <param name="CrosshairColor">׼����ɫ</param>
	void DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread, FLinearColor CrosshairColor);


private:
	FHUDPackage HUDPackage;

	UPROPERTY()
		class APlayerController* OwningPlayerCtr;

	/// <summary>
	/// ׼������ſ��ٶ�
	/// </summary>
	UPROPERTY(EditAnywhere)
		float MaxCrosshairSpread = 16.f;

	/// <summary>
	/// ��̭������
	/// </summary>
	UPROPERTY(EditAnywhere)
		TSubclassOf<class UElimAnnouncement> ElimAnnouncementClass;

	/// <summary>
	/// ��̭����item�ĳ���չʾʱ��
	/// </summary>
	UPROPERTY(EditAnywhere)
		float ElimAnnouncementTime = 1.5f;


public:
	UPROPERTY(EditAnywhere, Category = "Player Stats")
		TSubclassOf<class UUserWidget> CharacterOverlayClass;
	UPROPERTY()
		class UCharacterOverlayWidget* CharacterOverlayWidget;

	UPROPERTY(EditAnywhere, Category = "Announcements")
		TSubclassOf<class UUserWidget> AnnouncementClass;

	UPROPERTY()
		class UAnnouncementWidget* AnnouncementWidget;

};
