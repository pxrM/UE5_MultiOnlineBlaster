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
	class UTexture2D* CrosshairsCenter;  // 十字瞄准贴图  中
	UTexture2D* CrosshairsLeft;			 // 十字瞄准贴图  左
	UTexture2D* CrosshairsRight;	     // 十字瞄准贴图  右
	UTexture2D* CrosshairsTop;			 // 十字瞄准贴图  上
	UTexture2D* CrosshairsBottom;		 // 十字瞄准贴图  下
	float CrosshairSpread;				 // 十字瞄准散开值 射击游戏中十字准线会根据角色的位置移动稍微张开
	FLinearColor CrosshairColor;		 // 十字标准color
};

/**
 *
 */
UCLASS()
class BLASTER_API ABlasterHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void DrawHUD() override;	//重写父类中的DrawHUD()函数，以实现自定义的用户界面（UI）渲染逻辑
	FORCEINLINE void SetHUDPackage(const FHUDPackage& Package) { HUDPackage = Package; }

protected:
	virtual void BeginPlay() override;
	void AddCharacterOverlay();

private:
	void DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread, FLinearColor CrosshairColor);  //绘制准心

private:
	FHUDPackage HUDPackage;

	UPROPERTY(EditAnywhere)
		float MaxCrosshairSpread = 16.f;

public:
	UPROPERTY(EditAnywhere, Category = "Player Stats")
		TSubclassOf<class UUserWidget> CharacterOverlayClass;
	class UCharacterOverlayWidget* CharacterOverlayWidget;
};
