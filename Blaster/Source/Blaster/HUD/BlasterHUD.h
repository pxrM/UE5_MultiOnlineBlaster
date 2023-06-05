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
	FLinearColor CrosshairColor;		 // ʮ�ֱ�׼color
};

/**
 *
 */
UCLASS()
class BLASTER_API ABlasterHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void DrawHUD() override;	//��д�����е�DrawHUD()��������ʵ���Զ�����û����棨UI����Ⱦ�߼�
	FORCEINLINE void SetHUDPackage(const FHUDPackage& Package) { HUDPackage = Package; }

protected:
	virtual void BeginPlay() override;
	void AddCharacterOverlay();

private:
	void DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread, FLinearColor CrosshairColor);  //����׼��

private:
	FHUDPackage HUDPackage;

	UPROPERTY(EditAnywhere)
		float MaxCrosshairSpread = 16.f;

public:
	UPROPERTY(EditAnywhere, Category = "Player Stats")
		TSubclassOf<class UUserWidget> CharacterOverlayClass;
	class UCharacterOverlayWidget* CharacterOverlayWidget;
};
