// 제작자 : 이건주
// 제작일 : 2026-03-04
// 수정일 : 2026-03-05

#include "MHGameplayTags.h"

namespace MHGameplayTags
{
	//Input Tags
	UE_DEFINE_GAMEPLAY_TAG(Input_Move, "Input.Move");
	UE_DEFINE_GAMEPLAY_TAG(Input_Look, "Input.Look");
	
	//Element Tags (속성 태그)
	UE_DEFINE_GAMEPLAY_TAG(Element,        "Element");
	UE_DEFINE_GAMEPLAY_TAG(Element_Fire,   "Element.Fire");
	UE_DEFINE_GAMEPLAY_TAG(Element_Water,  "Element.Water");
	UE_DEFINE_GAMEPLAY_TAG(Element_Thunder,"Element.Thunder");
	UE_DEFINE_GAMEPLAY_TAG(Element_Ice,    "Element.Ice");
	UE_DEFINE_GAMEPLAY_TAG(Element_Dragon, "Element.Dragon");
}