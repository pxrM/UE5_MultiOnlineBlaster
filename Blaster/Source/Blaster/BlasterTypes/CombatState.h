#pragma once

UENUM(BlueprintType)
enum class ECombatState : uint8
{
	ECS_Unoccupied UMETA(DisplayName = "¿ÕÏÐÖÐ"),
	ECS_Reloading UMETA(DisplayName = "¸ü»»µ¯¼ÐÖÐ"),

	ECS_MAX UMETA(DisplayName = "DefaultMax")
};