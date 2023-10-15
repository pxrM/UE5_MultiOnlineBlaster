#pragma once

UENUM(BlueprintType)
enum class ETeam :uint8
{
	ET_RedTeam UENUM(DisplayName = "RedTeam"),
	ET_BlueTeam UENUM(DisplayName = "BlueTeam"),
	ET_NoTeam UENUM(DisplayName = "NoTeam"),

	ET_MAX UENUM(DisplayName = "DefaultMAX"),
};