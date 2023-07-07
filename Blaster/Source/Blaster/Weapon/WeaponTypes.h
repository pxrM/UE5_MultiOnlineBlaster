#pragma once

#define TRACE_LENGTH 8000.f

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
	EWT_GrenadeLacuncher UMETA(DisplayName = "榴弹发射器"),

	EWT_MAX UMETA(DisplayName = "DefaultMax")
};