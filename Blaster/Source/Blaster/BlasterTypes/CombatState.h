#pragma once

UENUM(BlueprintType)
enum class ECombatState : uint8
{
	ECS_Unoccupied UMETA(DisplayName = "������"),
	ECS_Reloading UMETA(DisplayName = "����������"),
	ECS_ThrowingGrenade UMETA(DisplayName = "Ͷ��������"),

	ECS_MAX UMETA(DisplayName = "DefaultMax")
};