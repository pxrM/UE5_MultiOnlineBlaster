// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AssetTypeActions_Base.h"

/**
 * FAssetTypeActions_Base ������ AssetTypeActions �Ļ��࣬�ṩ�й��ض��ʲ����͵Ĳ�����������Ϣ��
 * ���ǿ�ѡ�ģ���һ��Ĭ����Ҫ�ṩ�������޷��������˫�����ʲ��༭���Ȳ�����
 * 
 * ���༭���� ContentBrowser �д����ʲ������������ʲ���˫���򿪱༭��Դ�ȣ�ʱ������Ѱ�Ƿ��ж�Ӧ�� AssetTypeActionsClass ��ע�ᡣ
 * 
 * AssetTypeActions ���и������ʲ������Լ��ʲ���Ϣ��ʾ�йص��麯�������ǿ���ͨ����д�������Զ����ʲ���������Ϊ���ʲ���ʾ��ʽ��
 */
class CUSTOMDATATYPEEDITOR_API FCustomNormalDistributionActions :public FAssetTypeActions_Base
{
public:
	FCustomNormalDistributionActions(EAssetTypeCategories::Type InAssetCategory);
	~FCustomNormalDistributionActions();

	// ���ش˲���֧�ֵ���Դ�����
	virtual UClass* GetSupportedClass() const override;
	// ���ش˲��������ƣ�ͨ�����������û���������ʾ���ı���
	virtual FText GetName() const override;
	// ���ش˲�����������ɫ��ͨ���������û������б�ʶ��ͬ���͵���Դ��
	virtual FColor GetTypeColor() const override;
	// ���ش˲�����������Դ�������
	virtual uint32 GetCategories() override;

	// ���ݸ�������Դ�����б����Ҽ������˵���
	virtual void GetActions(const TArray<UObject*>& InObjects, class FMenuBuilder& MenuBuilder) override;
	// ����������Դ�����б����Ƿ��п��õĲ�����
	virtual bool HasActions(const TArray<UObject*>& InObjects) const override;

	
	/*
	* Ϊ�Զ����������͹����༭��
	*	˫���ʲ��򿪱༭���������������� UAssetEditorSubsystem::OpenEditorForAsset()
	*	���Ż���� FAssetTypeActions_Base::OpenAssetEditor -> FSimpleAssetEditor::CreateEditor -> FSimpleAssetEditor::InitEditor...
	* ���Լ��� AssetTypeActions ����д OpenAssetEditor()��Ȼ�����������Զ���� FAssetEditorToolkit �࣬�����Լ��ı༭����
	* 
	* InObjects��һ�� TArray ���͵Ķ������飬������Ҫ�༭���ʲ�����
	*/
	virtual void OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<class IToolkitHost> EditWithinLevelEditor /* = TSharedPtr<IToolkitHost>() */) override;

	// ���µ���
	void ExecuteReimport(const TArray<UObject*>& InObjects);

private:
	EAssetTypeCategories::Type AssetCategory;

};
