// Fill out your copyright notice in the Description page of Project Settings.


#include "MultiPlayerSessionSubsystem.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"

UMultiPlayerSessionSubsystem::UMultiPlayerSessionSubsystem()
	:CreateSessionCompleteDelegate(FOnCreateSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnCreateSessionComplete))
	, FindSessionsCompleteDelegate(FOnFindSessionsCompleteDelegate::CreateUObject(this, &ThisClass::OnFindSessionComplete))
	, JoinSessionCompleteDelegate(FOnJoinSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnJoinSessionComplete))
	, DestroySessionCompleteDelegate(FOnDestroySessionCompleteDelegate::CreateUObject(this, &ThisClass::OnDestroySessionComplete))
	, StartSessionCompleteDelegate(FOnStartSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnStartSessionComplete))
{
	IOnlineSubsystem* SubSystem = IOnlineSubsystem::Get();
	if (SubSystem)
	{
		SessionInterface = SubSystem->GetSessionInterface();
	}
}

void UMultiPlayerSessionSubsystem::CreateSession(int32 NumPublicConnections, FString MatchType)
{
	if (!SessionInterface.IsValid()) {
		return;
	}

	// ���Ự�Ƿ��Ѿ�����,�������������
	auto ExistingSession = SessionInterface->GetNamedSession(NAME_GameSession);
	if (ExistingSession)
	{
		bCreateSessionOnDestroy = true;
		LastNumPublicConnections = NumPublicConnections;
		LastMatchType = MatchType;

		DestroySession();
	}

	CreateSessionCompleteDelegateHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegate);

	LastSessionSettings = MakeShareable(new FOnlineSessionSettings());
	LastSessionSettings->bIsLANMatch = IOnlineSubsystem::Get()->GetSubsystemName() == "NULL" ? true : false; //�����������stream�Ͳ�ʹ�þ�����
	LastSessionSettings->NumPublicConnections = NumPublicConnections;	//�趨������������
	LastSessionSettings->bAllowJoinInProgress = true; //�Ƿ��������Ϸ;�м���
	LastSessionSettings->bAllowJoinViaPresence = true; //�Ƿ��������ͨ��״̬����
	LastSessionSettings->bShouldAdvertise = true; //����Steam�����Ự
	LastSessionSettings->bUsesPresence = true; //��ʾ�û�״̬��Ϣ
	LastSessionSettings->bUseLobbiesIfAvailable = true; //����api
	LastSessionSettings->Set(FName("MatchType"), MatchType, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);	//���ûỰ��ƥ��value
	LastSessionSettings->BuildUniqueId = 1;	//���ڷ�ֹ��ͬ�Ĺ��������������п����˴�

	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController(); //��ȡ���ص�һ�����
	// �����Ự
	bool bSuccessful = SessionInterface->CreateSession(
		*LocalPlayer->GetPreferredUniqueNetId(),//��ȡΨһ����id
		NAME_GameSession,//�Ự��
		*LastSessionSettings //�Ự������
	);
	if (!bSuccessful)
	{
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
		MultiplayerExternalOnCreateSessionComplete.Broadcast(false);
	}
}

void UMultiPlayerSessionSubsystem::FindSession(int32 MaxSearchResults)
{
	if (!SessionInterface.IsValid()) {
		return;
	}

	FindSessionsCompleteDelegateHandle = SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegate);

	LastSessionSearch = MakeShareable(new FOnlineSessionSearch());
	LastSessionSearch->MaxSearchResults = MaxSearchResults;
	LastSessionSearch->bIsLanQuery = IOnlineSubsystem::Get()->GetSubsystemName() == "NULL" ? true : false;;
	//SEARCH_PRESENCE��ֻ��Ѱ���е���Ϸ����ֻ������ͬID����ϷSession
	LastSessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);

	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	bool bSuccessful = SessionInterface->FindSessions(*LocalPlayer->GetPreferredUniqueNetId(), LastSessionSearch.ToSharedRef());
	if (!bSuccessful)
	{
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
		// ���ⲿ�㲥
		MultiplayerExternalOnFindSessionsComplete.Broadcast(TArray<FOnlineSessionSearchResult>(), bSuccessful);
	}
}

void UMultiPlayerSessionSubsystem::JoinSession(const FOnlineSessionSearchResult& SessionResult)
{
	if (!SessionInterface.IsValid())
	{
		// ���ⲿ�㲥
		MultiplayerExternalOnJoinSessionComplete.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
		return;
	}

	JoinSessionCompleteDelegateHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegate);

	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	bool bSuccessful = SessionInterface->JoinSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, SessionResult);
	if (!bSuccessful)
	{
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
		MultiplayerExternalOnJoinSessionComplete.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
	}
}

void UMultiPlayerSessionSubsystem::DestroySession()
{
	if (!SessionInterface.IsValid())
	{
		MultiplayerExternalOnDestroySessionComplete.Broadcast(false);
		return;
	}

	DestroySessionCompleteDelegateHandle = SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegate);

	if (!SessionInterface->DestroySession(NAME_GameSession))
	{
		SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
		MultiplayerExternalOnDestroySessionComplete.Broadcast(false);
	}
}

void UMultiPlayerSessionSubsystem::StartSession()
{
}


void UMultiPlayerSessionSubsystem::OnCreateSessionComplete(FName SesionName, bool bWasSuccessful)
{
	if (!SessionInterface.IsValid()) {
		return;
	}

	SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);

	MultiplayerExternalOnCreateSessionComplete.Broadcast(bWasSuccessful);
}

void UMultiPlayerSessionSubsystem::OnFindSessionComplete(bool bWasSuccessful)
{
	if (!SessionInterface.IsValid()) {
		return;
	}

	SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);

	auto Results = LastSessionSearch->SearchResults.Num() > 0 ? LastSessionSearch->SearchResults : TArray<FOnlineSessionSearchResult>();
	MultiplayerExternalOnFindSessionsComplete.Broadcast(Results, bWasSuccessful);
}

void UMultiPlayerSessionSubsystem::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if (!SessionInterface.IsValid()) {
		return;
	}

	SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
	MultiplayerExternalOnJoinSessionComplete.Broadcast(Result);
}

void UMultiPlayerSessionSubsystem::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (SessionInterface)
	{
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
	}

	if (bWasSuccessful && bCreateSessionOnDestroy)
	{
		bCreateSessionOnDestroy = false;
		CreateSession(LastNumPublicConnections, LastMatchType);
	}

	MultiplayerExternalOnDestroySessionComplete.Broadcast(bWasSuccessful);
}

void UMultiPlayerSessionSubsystem::OnStartSessionComplete(FName SessionName, bool bWasSuccessful)
{
}
