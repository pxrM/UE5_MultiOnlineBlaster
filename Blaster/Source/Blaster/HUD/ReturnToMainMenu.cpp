// Fill out your copyright notice in the Description page of Project Settings.


#include "ReturnToMainMenu.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/GameModeBase.h"
#include "Components/Button.h"
#include "MultiPlayerSessionSubsystem.h"
#include "Blaster/Character/BlasterCharacter.h"


bool UReturnToMainMenu::Initialize()
{
	if (!Super::Initialize())
	{
		return false;
	}
	
	return true;
}

void UReturnToMainMenu::MenuSetup()
{
	AddToViewport();
	SetVisibility(ESlateVisibility::Visible);
	bIsFocusable = true;

	UWorld* World = GetWorld();
	if (World)
	{
		PlayerCtl = PlayerCtl == nullptr ? World->GetFirstPlayerController() : PlayerCtl;
		if (PlayerCtl)
		{
			FInputModeGameAndUI InputModeData;
			InputModeData.SetWidgetToFocus(TakeWidget());
			PlayerCtl->SetInputMode(InputModeData);
			PlayerCtl->SetShowMouseCursor(true);
		}
	}

	if (ReturnBtn)
	{
		ReturnBtn->OnClicked.AddDynamic(this, &UReturnToMainMenu::ReturnBtnClicked);
	}

	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance)
	{
		MultiPlayerSessionSubsystem = GameInstance->GetSubsystem<UMultiPlayerSessionSubsystem>();
		if (MultiPlayerSessionSubsystem)
		{
			MultiPlayerSessionSubsystem->MultiplayerExternalOnDestroySessionComplete.AddDynamic(this, &UReturnToMainMenu::OnDestroySession);
		}
	}
}

void UReturnToMainMenu::MenuTearDown()
{
	RemoveFromParent();

	UWorld* World = GetWorld();
	if (World)
	{
		PlayerCtl = PlayerCtl == nullptr ? World->GetFirstPlayerController() : PlayerCtl;
		if (PlayerCtl)
		{
			FInputModeGameOnly InputModeData;
			PlayerCtl->SetInputMode(InputModeData);
			PlayerCtl->SetShowMouseCursor(false);
		}
	}

	if (ReturnBtn && ReturnBtn->OnClicked.IsBound())
	{
		ReturnBtn->OnClicked.RemoveDynamic(this, &UReturnToMainMenu::ReturnBtnClicked);
	}
	if (MultiPlayerSessionSubsystem && MultiPlayerSessionSubsystem->MultiplayerExternalOnDestroySessionComplete.IsBound())
	{
		MultiPlayerSessionSubsystem->MultiplayerExternalOnDestroySessionComplete.RemoveDynamic(this, &UReturnToMainMenu::OnDestroySession);
	}
}

void UReturnToMainMenu::ReturnBtnClicked()
{
	ReturnBtn->SetIsEnabled(false);

	UWorld* World = GetWorld();
	if (World)
	{
		APlayerController* FirstPlayerController = World->GetFirstPlayerController();
		if (FirstPlayerController)
		{
			ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(FirstPlayerController->GetPawn());
			if (BlasterCharacter)
			{
				BlasterCharacter->ServerLeavaGame();
				BlasterCharacter->OnLeftGame.AddDynamic(this, &UReturnToMainMenu::OnPlayerLeftGame);
			}
			else
			{
				ReturnBtn->SetIsEnabled(true);
			}
		}
	}
}

void UReturnToMainMenu::OnDestroySession(bool bWasSuccessful)
{
	if (!bWasSuccessful)
	{
		ReturnBtn->SetIsEnabled(true);
		return;
	}

	UWorld* World = GetWorld();
	if (World)
	{
		AGameModeBase* GameMode = World->GetAuthGameMode<AGameModeBase>();
		if (GameMode)
		{
			GameMode->ReturnToMainMenuHost(); // �������������Ͽ�������ҵ�����
		}
		else
		{
			PlayerCtl = PlayerCtl == nullptr ? World->GetFirstPlayerController() : PlayerCtl;
			if (PlayerCtl)
			{
				PlayerCtl->ClientReturnToMainMenuWithTextReason(FText()); // ���ڽ���ҿ������л������˵����棬����ʾָ�����ı�ԭ��
			}
		}
	}
}

void UReturnToMainMenu::OnPlayerLeftGame()
{
	if (MultiPlayerSessionSubsystem)
	{
		MultiPlayerSessionSubsystem->DestroySession();
	}
}