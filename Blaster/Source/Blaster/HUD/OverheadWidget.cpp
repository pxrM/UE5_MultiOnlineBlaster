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
	case ROLE_None:	//δ�����κν�ɫ
		Role = FString("None");
		break;
	case ROLE_SimulatedProxy:	//����Զ�������е� Actor�������б仯��Ӧ��ͬ�������ء�
		Role = FString("Simulated Proxy");
		break;
	case ROLE_AutonomousProxy:	//������ҿ��Ƶ� Actor��������ҿ���ֱ�ӿ�������״̬��������״̬���·��͸�����Զ����ҡ�
		Role = FString("Autonomous Proxy");
		break;
	case ROLE_Authority:	//����������Ϊ����Դ�� Actor�����д˽�ɫ�� Actor ���Է����������Ϸ���и��ġ�
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
