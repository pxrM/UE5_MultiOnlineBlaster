// Fill out your copyright notice in the Description page of Project Settings.


#include "SharedPtrGameModeBase.h"

void ASharedPtrGameModeBase::StartPlay()
{
	Super::StartPlay();

	//TSharedPtr
	{
		//1.创建
		TSharedPtr<FTestA> TestPtr = MakeShareable(new FTestA); //引用计数+1
		//2.是否有效
		if (TestPtr.IsValid())
		{
			TestPtr->TestAFun(); //重载 -> 运算符 返回FTestA指针，.是TSharedPtr自己的方法，->是指向对象的方法
		}
		//3.获取引用计数
		int RefCount = TestPtr.GetSharedReferenceCount();
		UE_LOG(LogTemp, Warning, TEXT("RefCount=%d"), RefCount);
		//4.解引用
		//FTestA* TestA = TestPtr.Get();
		//5.手动释放(两种方式)
		//TestPtr = nullptr;
		//TestPtr.Reset();
	}
	//作用域结束 引用计数-1=0，调用析构

	//TSharedRef
	{
		TSharedRef<FTestA> TestRef = MakeShareable(new FTestA); //引用计数=1
		//TSharedRef<FTestA> TestPtr;  错误：引用不能为null
		int RefCount = TestRef.GetSharedReferenceCount();
		if(TestRef.IsUnique()){}	//唯一拥有这个对象的引用
	}

	//TSharedPtr 和 TSharedRef相互转换
	{
		//共享引用转共享指针
		{
			TSharedRef<FTestA> TestRef = MakeShareable(new FTestA);
			TSharedPtr<FTestA> TestPtr = TestRef;
		}
		//共享指针转共享引用
		{
			TSharedPtr<FTestA> TestPtr = MakeShareable(new FTestA);
			TSharedRef<FTestA> TestRef = TestPtr.ToSharedRef();
		}
	}

	//共享指针父子级之间的转化
	{
		TSharedPtr<FTestA> TAPtr = MakeShareable(new FTestB); //多态父类指针接收子类构造
		if (TAPtr.IsValid())
		{
			TAPtr->TestAFun();
			TSharedPtr<FTestB> TBPtr = StaticCastSharedPtr<FTestB>(TAPtr);
			if (TBPtr.IsValid())
			{
				TBPtr->TestBBBFun();
			}
		}
	}

	//共享指针去const
	{
		const TSharedPtr<FTestA> TACCPtr = MakeShareable(new FTestA);
		if (TACCPtr.IsValid())
		{
			TSharedPtr<FTestA> TAPtr = ConstCastSharedPtr<FTestA>(TACCPtr);
		}
	}

	{
		TSharedPtr<FElephant> Elephant = MakeShareable(new FElephant); //FElephant 引用计数=1
		TSharedPtr<FLion> Lion = MakeShareable(new FLion); //FLion 引用计数=1
		TSharedPtr<FMouse> Mouse = MakeShareable(new FMouse); //FMouse 引用计数=1
		Elephant->SetRestraint(Lion); //FLion 引用计数=2
		Lion->SetRestraint(Mouse);	//Mouse 引用计数=2
		Mouse->SetRestraint(Elephant); //Elephant 引用计数=2
		int ElephantRefCount = Elephant.GetSharedReferenceCount();
		int LionRefCount = Lion.GetSharedReferenceCount();
		int MouseRefCount = Mouse.GetSharedReferenceCount();
		UE_LOG(LogTemp, Warning, TEXT("ElephantRefCount=%d, LionRefCount=%d, MouseRefCount=%d"), ElephantRefCount, LionRefCount, MouseRefCount);
	}//此时出了作用域引用计数-1，没有被释放，因为这里有相互引用，解决：将类里的引用改为弱引用 TWeakPtr
	/*
	* 1.弱指针不能影响强指针的引用计数
	* 2.在使用弱指针是需要进行检测
	* 3.弱指针最大的用途就是解决环形引用问题
	* 4.弱指针不对资源进行竞争。只能使用资源，没有权限控制资源的生命周期
	*/
}
