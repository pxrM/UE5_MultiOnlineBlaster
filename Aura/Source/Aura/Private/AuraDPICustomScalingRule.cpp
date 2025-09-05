// P


#include "AuraDPICustomScalingRule.h"

FVector2D UAuraDPICustomScalingRule::DesignedSize(1920, 1080);

float UAuraDPICustomScalingRule::GetDPIScaleBasedOnSize(FIntPoint Size) const
{
	float NominalAspectRatio = (4.0f / 3.0f);
	if (Size.X == 0 || Size.Y == 0)
	{
		return 1.0f;
	}
	else
	{
		const float ScaleX = Size.X / DesignedSize.X;
		const float ScaleY = Size.Y / DesignedSize.Y;
		return FMath::Min(ScaleX, ScaleY);
	}
	return 1.0f;
}

void UAuraDPICustomScalingRule::SetDesignedSize(float InX, float InY)
{
    DesignedSize.Set(InX, InY);
}

FVector2D UAuraDPICustomScalingRule::GetDesignedSize()
{
    return DesignedSize;
}
