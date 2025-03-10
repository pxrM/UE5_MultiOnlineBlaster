#pragma once

UENUM(BlueprintType)
enum class ECombatState : uint8
{
	ECS_Unoccupied UMETA(DisplayName = "空闲中"),
	ECS_Reloading UMETA(DisplayName = "更换弹夹中"),
	ECS_ThrowingGrenade UMETA(DisplayName = "投掷手榴弹中"),
	ECS_SwappingWeapons UMETA(DisplayName = "交换武器中"),

	ECS_MAX UMETA(DisplayName = "DefaultMax")
};