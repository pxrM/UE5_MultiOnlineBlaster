// Fill out your copyright notice in the Description page of Project Settings.


#include "SessionMenu.h"
#include "Components/Button.h"
#include "MultiPlayerSessionSubsystem.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"

void USessionMenu::MenuSetup(int32 InNumPublicConnections, FString InMatchType, FString InLobbyPath)
{
	NumPublicConnections = InNumPublicConnections;
	MatchTypeVal = InMatchType;
	PathToLobby = FString::Printf(TEXT("%s?listen"), *InLobbyPath);

	AddToViewport();	//添加到视口
	SetVisibility(ESlateVisibility::Visible);
	bIsFocusable = true;	//允许此小部件在单击或导航到时接受焦点

	UWorld* World = GetWorld();
	if (World)
	{
		APlayerController* PlayerController = World->GetFirstPlayerController();
		if (PlayerController)
		{
			FInputModeUIOnly InputModeData;
			InputModeData.SetWidgetToFocus(TakeWidget()); // 设置要聚焦的小部件
			InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock); // 设置鼠标是否锁定到视口
			PlayerController->SetInputMode(InputModeData); // 设置输入模式只作用于ui
			PlayerController->SetShowMouseCursor(true);	// 显示鼠标光标
		}
	}

	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance)
	{
		MultiPlayerSessionSubsystem = GameInstance->GetSubsystem<UMultiPlayerSessionSubsystem>();
	}

	if (MultiPlayerSessionSubsystem)
	{
		// 绑定回调函数
		MultiPlayerSessionSubsystem->MultiplayerExternalOnCreateSessionComplete.AddDynamic(this, &ThisClass::OnCreateSession);
		MultiPlayerSessionSubsystem->MultiplayerExternalOnFindSessionsComplete.AddUObject(this, &ThisClass::OnFindSession);
		MultiPlayerSessionSubsystem->MultiplayerExternalOnJoinSessionComplete.AddUObject(this, &ThisClass::OnJoinSession);
		MultiPlayerSessionSubsystem->MultiplayerExternalOnDestroySessionComplete.AddDynamic(this, &ThisClass::OnDestroySession);
		MultiPlayerSessionSubsystem->MultiplayerExternalOnStartSessionComplete.AddDynamic(this, &ThisClass::OnStartSession);
	}
}

void USessionMenu::NativeConstruct()
{
	Super::NativeConstruct();

	if (Button_Host)
	{
		Button_Host->OnClicked.AddDynamic(this, &ThisClass::HostBtnClicked);
	}
	if (Button_Join)
	{
		Button_Join->OnClicked.AddDynamic(this, &ThisClass::JoinBtnClicked);
	}
}

void USessionMenu::NativeDestruct()
{
	MenuTearDown();
	Super::NativeDestruct();
}

void USessionMenu::OnCreateSession(bool bWasSuessionful)
{
	if (bWasSuessionful)
	{
		/*if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 6.f, FColor::Yellow, FString(TEXT("Session created successfully!")));
		}*/

		UWorld* World = GetWorld();
		if (World)
		{
			World->ServerTravel(PathToLobby);
		}
	}
	else
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 6.f, FColor::Red, FString(TEXT("Faild create to session!")));
		}
		Button_Host->SetIsEnabled(true);
	}
}

void USessionMenu::OnFindSession(const TArray<FOnlineSessionSearchResult>& SessionSearch, bool bWasSuccessful)
{
	if (MultiPlayerSessionSubsystem == nullptr)
		return;

	for (auto Result : SessionSearch)
	{
		FString SettingsValue;
		Result.Session.SessionSettings.Get(FName("MatchType"), SettingsValue);
		if (SettingsValue == MatchTypeVal)
		{
			MultiPlayerSessionSubsystem->JoinSession(Result);	// 调UMultiPlayerSessionSubsystem::JoinSession
			return;
		}
	}

	if (!bWasSuccessful || SessionSearch.Num() == 0)
	{
		Button_Join->SetIsEnabled(true);
	}
}

void USessionMenu::OnJoinSession(EOnJoinSessionCompleteResult::Type Result)
{
	if (MultiPlayerSessionSubsystem == nullptr)
		return;

	IOnlineSubsystem* SubSystem = IOnlineSubsystem::Get();
	if (SubSystem)
	{
		auto SessionInterface = SubSystem->GetSessionInterface();
		if (SessionInterface.IsValid())
		{
			//获取IP地址，并将客户端跳转至该IP地址的监听服务器上
			FString Address;
			SessionInterface->GetResolvedConnectString(NAME_GameSession, Address);

			APlayerController* PlayerController = GetGameInstance()->GetFirstLocalPlayerController();
			if (PlayerController)
			{
				PlayerController->ClientTravel(Address, ETravelType::TRAVEL_Absolute);
			}
		}
	}

	if (Result != EOnJoinSessionCompleteResult::Success)
	{
		Button_Join->SetIsEnabled(true);
	}
}

void USessionMenu::OnDestroySession(bool bWasSuessionful)
{
}

void USessionMenu::OnStartSession(bool bWasSuessionful)
{
}

void USessionMenu::MenuTearDown()
{
	RemoveFromParent();	//移除菜单

	UWorld* World = GetWorld();
	if (World)
	{
		APlayerController* PlayerController = World->GetFirstPlayerController();
		if (PlayerController)
		{
			FInputModeGameOnly InputModeData;
			PlayerController->SetInputMode(InputModeData); // 设置输入模式只作用于ui
			PlayerController->SetShowMouseCursor(false);	// 显示鼠标光标
		}
	}
}

void USessionMenu::HostBtnClicked()
{
	Button_Host->SetIsEnabled(false);
	if (MultiPlayerSessionSubsystem)
	{
		MultiPlayerSessionSubsystem->CreateSession(NumPublicConnections, MatchTypeVal);
	}
}

void USessionMenu::JoinBtnClicked()
{
	Button_Join->SetIsEnabled(false);
	if (MultiPlayerSessionSubsystem)
	{
		MultiPlayerSessionSubsystem->FindSession(10000);
	}
}
