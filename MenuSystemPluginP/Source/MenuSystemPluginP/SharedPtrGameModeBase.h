// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SharedPtrGameModeBase.generated.h"

/**
 *
 */
UCLASS()
class MENUSYSTEMPLUGINP_API ASharedPtrGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	virtual void StartPlay() override;

};

class FTestA {
public:
	FTestA()
	{
		UE_LOG(LogTemp, Warning, TEXT("Hello FTestA  START"));
	}
	void TestAFun()
	{
		UE_LOG(LogTemp, Warning, TEXT("Hello FTestA  TestAFun"));
	}
	~FTestA()
	{
		UE_LOG(LogTemp, Warning, TEXT("Hello FTestA  END"));
	}
};


class FTestB :public FTestA {
public:
	FTestB()
	{
		UE_LOG(LogTemp, Warning, TEXT("Hello FTestBBB  START"));
	}
	void TestBBBFun()
	{
		UE_LOG(LogTemp, Warning, TEXT("Hello TestBBBFun  派生方法"));
	}
	~FTestB()
	{
		UE_LOG(LogTemp, Warning, TEXT("Hello FTestBBB  END"));
	}
};



class IAnim {
public:
	virtual void SetRestraint(const TSharedPtr<IAnim>& InWhatAnim) = 0;
	virtual ~IAnim() {}
};
/* 大象 -》 狮子 -》老鼠 -》大象 */
class FLion;
class FMouse;
class FElephant :public IAnim {
public:
	FElephant()
	{
		UE_LOG(LogTemp, Warning, TEXT("大象 克制狮子"));
	}
	virtual void SetRestraint(const TSharedPtr<IAnim>& InWhatAnim)
	{
		Lion = StaticCastSharedPtr<FLion>(InWhatAnim);
	}
	~FElephant()
	{
		UE_LOG(LogTemp, Warning, TEXT("大象 GG"));
	}
public:
	TSharedPtr<FLion> Lion;
};
class FLion :public IAnim {
public:
	FLion()
	{
		UE_LOG(LogTemp, Warning, TEXT("狮子 克制老鼠"));
	}
	virtual void SetRestraint(const TSharedPtr<IAnim>& InWhatAnim)
	{
		Mouse = StaticCastSharedPtr<FMouse>(InWhatAnim);
	}
	~FLion()
	{
		UE_LOG(LogTemp, Warning, TEXT("狮子 GG"));
	}
public:
	TSharedPtr<FMouse> Mouse;
};
class FMouse :public IAnim {
public:
	FMouse()
	{
		UE_LOG(LogTemp, Warning, TEXT("老鼠 克制大象"));
	}
	virtual void SetRestraint(const TSharedPtr<IAnim>& InWhatAnim)
	{
		Elephant = StaticCastSharedPtr<FElephant>(InWhatAnim);
	}
	~FMouse()
	{
		UE_LOG(LogTemp, Warning, TEXT("老鼠 GG"));
	}
public:
	TSharedPtr<FElephant> Elephant;
};



class FTestC {
public:
	void TestCFunc(const TSharedPtr<FTestD>& Test) {}
};
class FTestD :public TSharedFromThis<FTestD>
{
public:
	FTestD()
	{
		FTestC test;
		test.TestCFunc(this->AsShared()); //可以将自己转为智能指针传递给其他地方
	}
};