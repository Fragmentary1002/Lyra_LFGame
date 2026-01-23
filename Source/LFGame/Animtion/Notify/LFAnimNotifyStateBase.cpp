#include "LFAnimNotifyStateBase.h"

ULFAnimNotifyStateBase::ULFAnimNotifyStateBase()
{
	// 设置默认值

#if WITH_EDITORONLY_DATA
	// Ninja Bear Green! :D
	NotifyColor = FColor(211, 221, 197);
#endif	
}
