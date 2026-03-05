#include "MHGameplayTags.h"

namespace MHGameplayTags
{
	//Input Tags
	UE_DEFINE_GAMEPLAY_TAG(Input_Move, "Input.Move");
	
	//Element Tags (속성 태그)
	UE_DEFINE_GAMEPLAY_TAG(Element,        "Element");
	UE_DEFINE_GAMEPLAY_TAG(Element_Fire,   "Element.Fire");
	UE_DEFINE_GAMEPLAY_TAG(Element_Water,  "Element.Water");
	UE_DEFINE_GAMEPLAY_TAG(Element_Thunder,"Element.Thunder");
	UE_DEFINE_GAMEPLAY_TAG(Element_Ice,    "Element.Ice");
	UE_DEFINE_GAMEPLAY_TAG(Element_Dragon, "Element.Dragon");
}