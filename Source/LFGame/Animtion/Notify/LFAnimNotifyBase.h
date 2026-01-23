#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "LFAnimNotifyBase.generated.h"

/**
 * 基础动画通知类
 */
UCLASS(Blueprintable, Abstract)
class ULFAnimNotifyBase : public UAnimNotify
{
	GENERATED_BODY()

public:
	// 构造函数
	ULFAnimNotifyBase();
	
};