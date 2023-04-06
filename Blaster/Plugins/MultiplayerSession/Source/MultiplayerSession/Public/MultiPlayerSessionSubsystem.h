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
	//	�Ự�˵�
	//
	/// <summary>
	/// �����Ự
	/// </summary>
	/// <param name="NumPublicConnections">�趨������������</param>
	/// <param name="MacthType">�Ự�Ĺؼ���</param>
	void CreateSession(int32 NumPublicConnections, FString MatchType);
	/// <summary>
	/// ���һỰ
	/// </summary>
	/// <param name="MaxSearchResults">���������Ѱ����</param>
	void FindSession(int32 MaxSearchResults);
	/// <summary>
	/// ����Ự
	/// </summary>
	/// <param name="SessionResult">�������</param>
	void JoinSession(const FOnlineSessionSearchResult& SessionResult);
	/// <summary>
	/// ɾ���Ự
	/// </summary>
	void DestroySession();
	/// <summary>
	/// ��ʼ�Ự
	/// </summary>
	void StartSession();

public:
	/// <summary>
	/// �����ص� �ⲿʹ��
	/// </summary>
	FMultiplayerOnCreateSessionComplete MultiplayerExternalOnCreateSessionComplete;
	/// <summary>
	/// ���һص� �ⲿʹ��
	/// </summary>
	FMultiplayerOnFindSessionsComplete MultiplayerExternalOnFindSessionsComplete;
	/// <summary>
	/// ����ص� �ⲿʹ��
	/// </summary>
	FMultiplayerOnJoinSessionComplete MultiplayerExternalOnJoinSessionComplete;
	/// <summary>
	/// ɾ���ص� �ⲿʹ��
	/// </summary>
	FMultiplayerOnDestroySessionComplete MultiplayerExternalOnDestroySessionComplete;
	/// <summary>
	/// ��ʼ�ص� �ⲿʹ��
	/// </summary>
	FMultiplayerOnStartSessionComplete MultiplayerExternalOnStartSessionComplete;

protected:
	//
	//	ί�лص�����
	//

	void OnCreateSessionComplete(FName SesionName, bool bWasSuccessful);
	void OnFindSessionComplete(bool bWasSuccessful);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);
	void OnStartSessionComplete(FName SessionName, bool bWasSuccessful);


private:
	/// <summary>
	/// ��װ���߻Ự�������͵�����ָ��
	/// </summary>
	IOnlineSessionPtr SessionInterface;
	/// <summary>
	/// �ϴεĻỰ����
	/// </summary>
	TSharedPtr<FOnlineSessionSettings> LastSessionSettings;
	/// <summary>
	/// �ϴε��������
	/// </summary>
	TSharedPtr<FOnlineSessionSearch> LastSessionSearch;


	//���پɻỰʱ�����µĻỰ
	bool bCreateSessionOnDestroy{ false };
	int32 LastNumPublicConnections;
	FString LastMatchType;


	//
	//	����ί��
	//

	FOnCreateSessionCompleteDelegate CreateSessionCompleteDelegate;
	FOnFindSessionsCompleteDelegate FindSessionsCompleteDelegate;
	FOnJoinSessionCompleteDelegate JoinSessionCompleteDelegate;
	FOnDestroySessionCompleteDelegate DestroySessionCompleteDelegate;
	FOnStartSessionCompleteDelegate StartSessionCompleteDelegate;

	//
	//	ί�о��
	//

	FDelegateHandle CreateSessionCompleteDelegateHandle;
	FDelegateHandle FindSessionsCompleteDelegateHandle;
	FDelegateHandle JoinSessionCompleteDelegateHandle;
	FDelegateHandle DestroySessionCompleteDelegateHandle;
	FDelegateHandle StartSessionCompleteDelegateHandle;

};
