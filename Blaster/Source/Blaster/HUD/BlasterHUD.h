// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "BlasterHUD.generated.h"

class UTexture2D;
class UUserWidget;
class APlayerController;
class UElimAnnouncement;
class UCharacterOverlayWidget;
class UAnnouncementWidget;

/// <summary>
/// 准心结构体
/// </summary>
USTRUCT(BlueprintType)
struct FHUDPackage
{
	GENERATED_BODY()
public:
	UTexture2D* CrosshairsCenter;		 // 十字瞄准贴图  中
	UTexture2D* CrosshairsLeft;			 // 十字瞄准贴图  左
	UTexture2D* CrosshairsRight;	     // 十字瞄准贴图  右
	UTexture2D* CrosshairsTop;			 // 十字瞄准贴图  上
	UTexture2D* CrosshairsBottom;		 // 十字瞄准贴图  下
	float CrosshairSpread;				 // 十字瞄准散开值 射击游戏中十字准线会根据角色的位置移动稍微张开
	FLinearColor CrosshairColor;		 // 十字瞄准color
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
	// 重写父类中的DrawHUD()函数，以实现自定义的用户界面（UI）渲染逻辑
	virtual void DrawHUD() override;

public:
	/// <summary>
	/// 添加角色信息ui
	/// </summary>
	void AddCharacterOverlay();
	/// <summary>
	/// 添加匹配状态公告
	/// </summary>
	void AddStateAnnouncement();
	/// <summary>
	/// 添加淘汰公告
	/// </summary>
	/// <param name="AttackerName"></param>
	/// <param name="VictimName"></param>
	void AddElimAnnouncement(const FString& AttackerName, const FString& VictimName);

	/// <summary>
	/// 淘汰公告item的计时器结束回调
	/// </summary>
	/// <param name="MsgToRemove">要删除的umg</param>
	UFUNCTION()
	void ElimAnnouncementTimerFinish(UElimAnnouncement* MsgToRemove);

public:
	FORCEINLINE void SetHUDPackage(const FHUDPackage& Package) { HUDPackage = Package; }


private:
	/// <summary>
	/// 绘制准心，由准心中心点、上下左右贴图组成
	/// </summary>
	/// <param name="Texture">要绘制的准心纹理</param>
	/// <param name="ViewportCenter">屏幕中心点</param>
	/// <param name="Spread">相对于中心点的准心张开距离</param>
	/// <param name="CrosshairColor">准心颜色</param>
	void DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread, FLinearColor CrosshairColor);


private:
	FHUDPackage HUDPackage;

	UPROPERTY()
	APlayerController* OwningPlayerCtr;

	/// <summary>
	/// 准心最大张开速度
	/// </summary>
	UPROPERTY(EditAnywhere)
	float MaxCrosshairSpread = 16.f;

	/// <summary>
	/// 淘汰公告类
	/// </summary>
	UPROPERTY(EditAnywhere)
	TSubclassOf<UElimAnnouncement> ElimAnnouncementClass;

	/// <summary>
	/// 淘汰公告item的持续展示时间
	/// </summary>
	UPROPERTY(EditAnywhere)
	float ElimAnnouncementTime = 2.5f;


public:
	UPROPERTY(EditAnywhere, Category = "Player Stats")
	TSubclassOf<UUserWidget> CharacterOverlayClass;
	UPROPERTY()
	UCharacterOverlayWidget* CharacterOverlayWidget;

	UPROPERTY(EditAnywhere, Category = "Announcements")
	TSubclassOf<UUserWidget> AnnouncementClass;

	UPROPERTY()
	UAnnouncementWidget* AnnouncementWidget;

	UPROPERTY()
	TArray<UElimAnnouncement*> ElimMessages;

};
