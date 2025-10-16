#pragma once

//射线长度
#define TRACE_LENGTH 8000.f
//深度值-紫色
#define CUSTOM_DEPTH_COLOR_PURPLE 250
//深度值-蓝色
#define CUSTOM_DEPTH_COLOR_BLUE 251
//深度值-棕色
#define CUSTOM_DEPTH_COLOR_TAN 252

/// <summary>
/// 武器类型
/// </summary>
UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	EWT_AssaultRifle UMETA(DisplayName = "突击步枪"),
	EWT_RocketLauncher UMETA(DisplayName= "火箭炮发射器"),
	EWT_Pistol UMETA(DisplayName = "手枪"),
	EWT_SubmachineGun UMETA(DisplayName = "冲锋枪"),
	EWT_Shotgun UMETA(DisplayName = "霰弹枪"),
	EWT_SniperRifle UMETA(DisplayName = "狙击步枪"),
	EWT_GrenadeLauncher UMETA(DisplayName = "榴弹发射器"),
	EWT_Flag UMETA(DisplayName = "旗帜"),

	EWT_MAX UMETA(DisplayName = "DefaultMax")
};