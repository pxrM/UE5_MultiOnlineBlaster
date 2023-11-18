// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "SessionMenu.generated.h"

/**
 *
 */
UCLASS()
class MULTIPLAYERSESSION_API USessionMenu : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
		void MenuSetup(int32 InNumPublicConnections = 4
			, FString InMatchType = FString(TEXT("FreeForAll"))
			, FString InLobbyPath = FString(TEXT("/Game/ThirdPerson/Maps/MLobbyMap")));

protected:
	virtual void NativeConstruct()override;
	virtual void NativeDestruct()override;

	//自定义委托的回调
	UFUNCTION()
		void OnCreateSession(bool bWasSuessionful);
	void OnFindSession(const TArray<FOnlineSessionSearchResult>& SessionSearch, bool bWasSuccessful);
	void OnJoinSession(EOnJoinSessionCompleteResult::Type Result);
	UFUNCTION()
		void OnDestroySession(bool bWasSuessionful);
	UFUNCTION()
		void OnStartSession(bool bWasSuessionful);

private:
	UPROPERTY(meta = (BindWidget))
		class UButton* Button_Host;
	UPROPERTY(meta = (BindWidget))
		UButton* Button_Join;

	UFUNCTION()
		void HostBtnClicked();
	UFUNCTION()
		void JoinBtnClicked();

	void MenuTearDown();

private:
	class UMultiPlayerSessionSubsystem* MultiPlayerSessionSubsystem;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		int32 NumPublicConnections{ 4 };
	UPROPERTY(BlueprintReadWrite, meta=(AllowPrivateAccess="true"))
		FString MatchTypeVal{ TEXT("FreeForAll") };
	FString PathToLobby{ TEXT("") };

};
