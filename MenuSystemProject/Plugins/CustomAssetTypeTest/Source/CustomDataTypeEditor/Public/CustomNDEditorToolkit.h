// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Toolkits/AssetEditorToolkit.h"

/**
 * Ϊ�Զ����������͹����༭�� 
 */
class CUSTOMDATATYPEEDITOR_API FCustomNDEditorToolkit :public FAssetEditorToolkit
{
public:
	FCustomNDEditorToolkit();
	~FCustomNDEditorToolkit();


	// ��������������ע���ע���༭���еı�ǩ��������Tab Spawner����
	// ��ǩ�������������ɺ͹���༭���еı�ǩҳ��Tabs�������Ƕ����˱༭���Ĳ��ֽṹ�Ϳɼ��ԡ�
	// ͨ��������������������ڱ༭����ʼ��ʱע�����Լ��ı�ǩ�����������ڱ༭���ر�ʱע�����ǣ�ȷ���༭���ı�ǩҳ������ȷ����ʾ�͹���
	virtual void RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager) override;
	virtual void UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager) override;

	// ���ر༭���ı�ʶ���ơ��������ͨ����һ��Ψһ�ı�ʶ���������� Unreal Engine �ڲ�ʶ��Ͳ��ұ༭����
	virtual FName GetToolkitFName() const override { return "CustomNormalDistributionEditor"; }
	// ���ر༭���Ļ������ơ�ͨ��������ʾ�ڱ༭�������������� UI Ԫ���У����û�չʾ�༭�������ơ�
	virtual FText GetBaseToolkitName() const override { return INVTEXT("Custom Normal Distribution Editor"); }
	// ���ر༭�����������ģ�World-Centric���ı�ǩǰ׺������������ UE �е�һ�������ʾ�༭���ڸ�ģʽ�µĹ�����ʽ������������ص�ǰ׺��Ӱ��༭������������ģʽ�µı�ǩ��ʾ��
	virtual FString GetWorldCentricTabPrefix() const override { return "Custom Normal Distribution Editor"; }
	// ���ر༭������������ģʽ�µı�ǩ��ɫ���ű���������������ص���ɫ������Ӱ��༭����ǩ����ɫ
	virtual FLinearColor GetWorldCentricTabColorScale() const override { return {}; }


	// �ⲿ���õ���ڣ����������������֣����Ծ������������
	// ����滮���ڲ��ֲ����� FAssetEditorToolkit::InitAssetEditor()��
	void InitEditor(const TArray<UObject*>& InObjects);

	float GetMean() const;
	float GetStandarDeviation() const;
	void SetMean(float InMean);
	void SetStandarDeviation(float InStandardDeviation);


private:
	class UCustomNormalDistribution* NormalDistribution = nullptr;
};
