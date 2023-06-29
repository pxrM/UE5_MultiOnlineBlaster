#pragma once

/// <summary>
/// 武器类型
/// </summary>
UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	EWT_AssaultRifle UMETA(DisplayName = "突击步枪"),
	EWT_RocketLauncher UMETA(DisplayName= "火箭炮发射器"),

	EWT_MAX UMETA(DisplayName = "DefaultMax")
};