#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "LFAnimNotifyStateBase.generated.h"

/**
 * 基础动画通知状态类
 */
UCLASS(Blueprintable, Abstract)
class ULFAnimNotifyStateBase : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	// 构造函数
	ULFAnimNotifyStateBase();
};