// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

// 自定义深度/模板值
#define CUSTOM_DEPTH_RED 250
// 子弹的碰撞通道
#define ECC_Projectile ECollisionChannel::ECC_GameTraceChannel1
// 技能目标碰撞通道
#define ECC_Target ECollisionChannel::ECC_GameTraceChannel2

#define DEFINE_GETTER_SETTER(type, var_name)\
	type Get##var_name() const { return var_name; }\
	void Set##var_name(const type In##var_name) { var_name = In##var_name; }