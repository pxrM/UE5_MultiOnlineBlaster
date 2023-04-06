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

	// 检查会话是否已经创建,如果创建先销毁
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
	LastSessionSettings->bIsLANMatch = IOnlineSubsystem::Get()->GetSubsystemName() == "NULL" ? true : false; //如果连接上了stream就不使用局域网
	LastSessionSettings->NumPublicConnections = NumPublicConnections;	//设定最大的联机人数
	LastSessionSettings->bAllowJoinInProgress = true; //是否可以在游戏途中加入
	LastSessionSettings->bAllowJoinViaPresence = true; //是否允许玩家通过状态加入
	LastSessionSettings->bShouldAdvertise = true; //允许Steam发布会话
	LastSessionSettings->bUsesPresence = true; //显示用户状态信息
	LastSessionSettings->bUseLobbiesIfAvailable = true; //兼容api
	LastSessionSettings->Set(FName("MatchType"), MatchType, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);	//设置会话的匹配value
	LastSessionSettings->BuildUniqueId = 1;	//用于防止不同的构建在搜索过程中看到彼此

	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController(); //获取本地第一个玩家
	// 创建会话
	bool bSuccessful = SessionInterface->CreateSession(
		*LocalPlayer->GetPreferredUniqueNetId(),//获取唯一网络id
		NAME_GameSession,//会话名
		*LastSessionSettings //会话设置类
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
	//SEARCH_PRESENCE是只搜寻现有的游戏，并只搜索相同ID的游戏Session
	LastSessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);

	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	bool bSuccessful = SessionInterface->FindSessions(*LocalPlayer->GetPreferredUniqueNetId(), LastSessionSearch.ToSharedRef());
	if (!bSuccessful)
	{
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
		// 给外部广播
		MultiplayerExternalOnFindSessionsComplete.Broadcast(TArray<FOnlineSessionSearchResult>(), bSuccessful);
	}
}

void UMultiPlayerSessionSubsystem::JoinSession(const FOnlineSessionSearchResult& SessionResult)
{
	if (!SessionInterface.IsValid())
	{
		// 给外部广播
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
