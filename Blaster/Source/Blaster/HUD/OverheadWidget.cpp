// Fill out your copyright notice in the Description page of Project Settings.


#include "OverheadWidget.h"
#include "Components/TextBlock.h"

void UOverheadWidget::SetDisplayText(FString TextToDisplay)
{
	if (DisplayText)
	{
		DisplayText->SetText(FText::FromString(TextToDisplay));
	}
}

void UOverheadWidget::ShowPlayerNetRole(APawn* InPawn)
{
	FString Role;
	ENetRole RemotelRole = InPawn->GetRemoteRole();
	switch (RemotelRole)
	{
	case ROLE_None:	//未定义任何角色
		Role = FString("None");
		break;
	case ROLE_SimulatedProxy:	//代表远程网络中的 Actor，其所有变化都应该同步到本地。
		Role = FString("Simulated Proxy");
		break;
	case ROLE_AutonomousProxy:	//本地玩家控制的 Actor，本地玩家可以直接控制它的状态，并将其状态更新发送给其他远程玩家。
		Role = FString("Autonomous Proxy");
		break;
	case ROLE_Authority:	//在网络上作为可信源的 Actor。具有此角色的 Actor 可以发出命令并对游戏进行更改。
		Role = FString("Authority");
		break;
	default:
		Role = FString("None");
		break;
	}
	FString RemoteRoleString = FString::Printf(TEXT("Remote Role: %s"), *Role);
	SetDisplayText(RemoteRoleString);
}

void UOverheadWidget::NativeDestruct()
{
	RemoveFromParent();
}
