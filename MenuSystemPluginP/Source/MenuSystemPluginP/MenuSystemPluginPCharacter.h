// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "MenuSystemPluginPCharacter.generated.h"


UCLASS(config=Game)
class AMenuSystemPluginPCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;
	
	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputMappingContext* DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* LookAction;

public:
	AMenuSystemPluginPCharacter();
	

protected:

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);
			

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	// To add mapping context
	virtual void BeginPlay();

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	/*------------------------------------------------------------------------------------------------------------------------------*/
public:
	/**
	���߻ػ�ָ��
		��һ������ʹ�ö���߳�ʱ���в��е�ִ����ͬʱ���У������ζ�ű������ܻ����ڶ���̳߳���������Ϊ�ķ�ʽִ��
		��������Ҫʹ�ù���ָ��
	**/
	//TSharedPtr<class IOnlineSession, ESPMode::ThreadSafe>OnlineSessionInterface;
	IOnlineSessionPtr OnlineSessionInterface;

protected:
	/* �����Ự ���¼��̡�1��ʱ���� */
	UFUNCTION(BlueprintCallable)
		void CreateGameSession();
	/* ����Ự ���¼��̡�2��ʱ���� */
	UFUNCTION(BlueprintCallable)
		void JoinGameSession();

	/// <summary>
	/// �����Ự���ί�� �ص�����
	/// </summary>
	/// <param name="SessionName">�Ự��</param>
	/// <param name="bWasSuccessful">�Ƿ�ɹ�</param>
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	/// <summary>
	/// ���һỰ���ί�� �ص�����
	/// </summary>
	/// <param name="bWasSuccessful"></param>
	void OnFindSessionComplete(bool bWasSuccessful);
	/// <summary>
	/// ����Ự���ί�� �ص�����
	/// </summary>
	/// <param name="SessionName"></param>
	/// <param name="Result"></param>
	void OnJionSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);

private:
	/// <summary>
	/// �����Ự���ί��
	/// </summary>
	FOnCreateSessionCompleteDelegate CreateSessionCompleteDelegate;
	/// <summary>
	/// ���һỰ���ί��
	/// </summary>
	FOnFindSessionsCompleteDelegate FindSessionCompleteDelegate;
	/// <summary>
	/// ��ϷѰ�ҽ��
	/// </summary>
	TSharedPtr<FOnlineSessionSearch> SessionSearch;
	/// <summary>
	/// ����Ự���ί��
	/// </summary>
	FOnJoinSessionCompleteDelegate JoinSessionCompleteDelegate;

	/*------------------------------------------------------------------------------------------------------------------------------*/
};

