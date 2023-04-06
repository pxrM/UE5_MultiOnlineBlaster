// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "MultiPlayerSessionSubsystem.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayerOnCreateSessionComplete, bool, bWasSuccessful);
DECLARE_MULTICAST_DELEGATE_TwoParams(FMultiplayerOnFindSessionsComplete, const TArray<FOnlineSessionSearchResult>& SessionSearch, bool bWasSuccessful);
DECLARE_MULTICAST_DELEGATE_OneParam(FMultiplayerOnJoinSessionComplete, EOnJoinSessionCompleteResult::Type Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayerOnDestroySessionComplete, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayerOnStartSessionComplete, bool, bWasSuccessful);

/**
 *
 */
UCLASS()
class MULTIPLAYERSESSION_API UMultiPlayerSessionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UMultiPlayerSessionSubsystem();

	//
	//	会话菜单
	//
	/// <summary>
	/// 创建会话
	/// </summary>
	/// <param name="NumPublicConnections">设定最大的联机人数</param>
	/// <param name="MacthType">会话的关键字</param>
	void CreateSession(int32 NumPublicConnections, FString MatchType);
	/// <summary>
	/// 查找会话
	/// </summary>
	/// <param name="MaxSearchResults">设置最大搜寻数量</param>
	void FindSession(int32 MaxSearchResults);
	/// <summary>
	/// 加入会话
	/// </summary>
	/// <param name="SessionResult">搜索结果</param>
	void JoinSession(const FOnlineSessionSearchResult& SessionResult);
	/// <summary>
	/// 删除会话
	/// </summary>
	void DestroySession();
	/// <summary>
	/// 开始会话
	/// </summary>
	void StartSession();

public:
	/// <summary>
	/// 创建回调 外部使用
	/// </summary>
	FMultiplayerOnCreateSessionComplete MultiplayerExternalOnCreateSessionComplete;
	/// <summary>
	/// 查找回调 外部使用
	/// </summary>
	FMultiplayerOnFindSessionsComplete MultiplayerExternalOnFindSessionsComplete;
	/// <summary>
	/// 加入回调 外部使用
	/// </summary>
	FMultiplayerOnJoinSessionComplete MultiplayerExternalOnJoinSessionComplete;
	/// <summary>
	/// 删除回调 外部使用
	/// </summary>
	FMultiplayerOnDestroySessionComplete MultiplayerExternalOnDestroySessionComplete;
	/// <summary>
	/// 开始回调 外部使用
	/// </summary>
	FMultiplayerOnStartSessionComplete MultiplayerExternalOnStartSessionComplete;

protected:
	//
	//	委托回调函数
	//

	void OnCreateSessionComplete(FName SesionName, bool bWasSuccessful);
	void OnFindSessionComplete(bool bWasSuccessful);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);
	void OnStartSessionComplete(FName SessionName, bool bWasSuccessful);


private:
	/// <summary>
	/// 包装在线会话数据类型的智能指针
	/// </summary>
	IOnlineSessionPtr SessionInterface;
	/// <summary>
	/// 上次的会话设置
	/// </summary>
	TSharedPtr<FOnlineSessionSettings> LastSessionSettings;
	/// <summary>
	/// 上次的搜索结果
	/// </summary>
	TSharedPtr<FOnlineSessionSearch> LastSessionSearch;


	//销毁旧会话时创建新的会话
	bool bCreateSessionOnDestroy{ false };
	int32 LastNumPublicConnections;
	FString LastMatchType;


	//
	//	声明委托
	//

	FOnCreateSessionCompleteDelegate CreateSessionCompleteDelegate;
	FOnFindSessionsCompleteDelegate FindSessionsCompleteDelegate;
	FOnJoinSessionCompleteDelegate JoinSessionCompleteDelegate;
	FOnDestroySessionCompleteDelegate DestroySessionCompleteDelegate;
	FOnStartSessionCompleteDelegate StartSessionCompleteDelegate;

	//
	//	委托句柄
	//

	FDelegateHandle CreateSessionCompleteDelegateHandle;
	FDelegateHandle FindSessionsCompleteDelegateHandle;
	FDelegateHandle JoinSessionCompleteDelegateHandle;
	FDelegateHandle DestroySessionCompleteDelegateHandle;
	FDelegateHandle StartSessionCompleteDelegateHandle;

};
