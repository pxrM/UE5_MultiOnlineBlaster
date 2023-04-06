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
	在线回话指针
		当一个程序使用多个线程时，有并行的执行线同时进行，这就意味着变量可能会在在多个线程出现意外行为的方式执行
		所以这里要使用共享指针
	**/
	//TSharedPtr<class IOnlineSession, ESPMode::ThreadSafe>OnlineSessionInterface;
	IOnlineSessionPtr OnlineSessionInterface;

protected:
	/* 创建会话 按下键盘“1”时调用 */
	UFUNCTION(BlueprintCallable)
		void CreateGameSession();
	/* 加入会话 按下键盘“2”时调用 */
	UFUNCTION(BlueprintCallable)
		void JoinGameSession();

	/// <summary>
	/// 创建会话完成委托 回调函数
	/// </summary>
	/// <param name="SessionName">会话名</param>
	/// <param name="bWasSuccessful">是否成功</param>
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	/// <summary>
	/// 查找会话完成委托 回调函数
	/// </summary>
	/// <param name="bWasSuccessful"></param>
	void OnFindSessionComplete(bool bWasSuccessful);
	/// <summary>
	/// 加入会话完成委托 回调函数
	/// </summary>
	/// <param name="SessionName"></param>
	/// <param name="Result"></param>
	void OnJionSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);

private:
	/// <summary>
	/// 创建会话完成委托
	/// </summary>
	FOnCreateSessionCompleteDelegate CreateSessionCompleteDelegate;
	/// <summary>
	/// 查找会话完成委托
	/// </summary>
	FOnFindSessionsCompleteDelegate FindSessionCompleteDelegate;
	/// <summary>
	/// 游戏寻找结果
	/// </summary>
	TSharedPtr<FOnlineSessionSearch> SessionSearch;
	/// <summary>
	/// 加入会话完成委托
	/// </summary>
	FOnJoinSessionCompleteDelegate JoinSessionCompleteDelegate;

	/*------------------------------------------------------------------------------------------------------------------------------*/
};

