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

	AddToViewport();	//��ӵ��ӿ�
	SetVisibility(ESlateVisibility::Visible);
	bIsFocusable = true;	//�����С�����ڵ����򵼺���ʱ���ܽ���

	UWorld* World = GetWorld();
	if (World)
	{
		APlayerController* PlayerController = World->GetFirstPlayerController();
		if (PlayerController)
		{
			FInputModeUIOnly InputModeData;
			InputModeData.SetWidgetToFocus(TakeWidget()); // ����Ҫ�۽���С����
			InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock); // ��������Ƿ��������ӿ�
			PlayerController->SetInputMode(InputModeData); // ��������ģʽֻ������ui
			PlayerController->SetShowMouseCursor(true);	// ��ʾ�����
		}
	}

	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance)
	{
		MultiPlayerSessionSubsystem = GameInstance->GetSubsystem<UMultiPlayerSessionSubsystem>();
	}

	if (MultiPlayerSessionSubsystem)
	{
		// �󶨻ص�����
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
			MultiPlayerSessionSubsystem->JoinSession(Result);	// ��UMultiPlayerSessionSubsystem::JoinSession
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
			//��ȡIP��ַ�������ͻ�����ת����IP��ַ�ļ�����������
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
	RemoveFromParent();	//�Ƴ��˵�

	UWorld* World = GetWorld();
	if (World)
	{
		APlayerController* PlayerController = World->GetFirstPlayerController();
		if (PlayerController)
		{
			FInputModeGameOnly InputModeData;
			PlayerController->SetInputMode(InputModeData); // ��������ģʽֻ������ui
			PlayerController->SetShowMouseCursor(false);	// ��ʾ�����
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
