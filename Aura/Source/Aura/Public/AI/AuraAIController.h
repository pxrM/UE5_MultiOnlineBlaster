// P

/*
 * 行为：受思想支配而表现出来的外表活动（如：做出动作，发出声音，作出反应）
 * 树：一种逻辑描述（树结构）
 * 行为树：遵循树结构特点（有根，有节点），遵循从上至下，从左至右的顺序执行行为
 *		行为可复用（如闲逛的行为可以使用于当前行为树，也适用于其他行为树）
 *		所有复杂行为可以拆分为单一行为（如闲逛，打招呼都是行为）
 *		行为树实现不唯一，顺序并不固定
 * UE中的行为树：
 *		以数据为驱动元，进行逻辑节点检查，寻找合理逻辑叶子节点，然后进行动作执行
 *		扩展方便，容易转接，被广泛使用在各种AI设计中，是一种优良的程序设计方案
 *		可以解决复杂AI需求，使用方便，支持横向扩展节点和向下延伸
 *
 *		Root 根节点：无法被附着，可以用来设置黑板数据
 *		Composite 节点：定义一个分支的根起点，及该分支如何被执行的基本规则
 *			Root下只能跟随composite
 *			包含Selector，Sequence，Simple Parallel
 *			节点内容简单，但是附加逻辑项多，逻辑点繁琐
 *			所有逻辑结构设计尽量遵循 Composite 规则
 *			Sequence：从左至右执行，只要有一个子项不成立，则停止执行剩余项（所有子项成立，则sequence成立）
 *			Selector：从左至右执行，只要有一个子项条件成立，则停止向下执行（任意子项成立，则selector成立）
 *			Simple Parallel 并行树：允许单个任务在任务树旁执行（Finish Mode 中的设置将确定节点是否立即完成、是否终止次要树，或是否延迟次要树的完成）
 *		Task 节点：是行为树的叶子，实际“执行”操作，不含输出连接，真正在干活的角色，可以重写，支持扩展
 *			Make Noise，制造噪音，需要Pawn带PawnNoiseEmitter组件，其他Pawn如果带有PawnSensing组件，则可以听到噪音
 *			Move To移动到目标点，通过黑板提供
 *			Play Sound 播放声音
 *			Run Behavior 执行其他行为树
 *			Wait，Wait BB Time，等待事件，无法被终止
 *			Play Anim，促使控制Pawn播放动画
 *			......
 *		Decorator 条件语句节点：附着于其他节点，用来约束节点或是分支是否可以执行
 *			装饰器，用来决定节点是否可以进行执行，是一个条件检查工具，可以附着到任务节点和复合节点
 *				Blackboard，借助一个黑板数据，进行逻辑操作判定
 *					Blackboard 黑板是行为树中的核心模块，用于支撑行为树中的数据存储和读取，相当于草稿纸，可以把需要参与逻辑的数据罗列在上面，使用方便，数据扩展性强，有较好的维护性
 *			Compare Blackboard Entries，对比黑板内两个数据
 *			Cone Check，锥形检测，可以检测目标是否在锥形范围内
 *			Cooldown，锁定节点或是分支，直到时间结束
 *			Does Path Exist，检查AB两点见路径是否可行
 *			Force Success decorator 将节点结果改为成功
 *			Is At Location，检查AI控制的角色是否在指定的位置
 *			Keep in Cone，持续检查锥形范围内目标
 *			Loop，以一定次数或是无限次数循环节点
 *			TimeLimit，节点逻辑时间定时器
 *		当父节点是Selector时则具备所有终结逻辑：
 *			NONE，不终止执行
 *			Self，中止 self，以及在此节点下运行的所有子树
 *			Lower Priority，中止此节点右方的所有节点
 *			Both，终止 self、此节点下运行的所有子树、以及此节点右方的所有节点
 *		Services 节点：附着在 Composite 或Task节点上，只要其分支节点被执行，便将按所定义的频率执行，常用于检查和更新黑板
 *			可以按照指定的频率执行，主要意图在于检查和更新黑板内容
 *			以行为树系统的形态取代了传统平行节点，一般会重写此节点
 */

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "AuraAIController.generated.h"

class UBehaviorTreeComponent;

/**
 *
 */
UCLASS()
class AURA_API AAuraAIController : public AAIController
{
	GENERATED_BODY()

public:
	AAuraAIController();

protected:
	// // 管理黑板数据组件
	// UPROPERTY()
	// TObjectPtr<UBlackboardComponent> BlackboardComponent;
	// 行为树组件
	UPROPERTY()
	TObjectPtr<UBehaviorTreeComponent> BehaviorTreeComponent;
};
