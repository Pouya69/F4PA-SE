#pragma once

namespace PArroyo_Menus {
	namespace Workbench_Additions {
		extern bool formulaApplied;
		extern std::uint32_t currentIndex;
		extern bool bIsScrappingAllJunk;

		bool RegisterScaleform(Scaleform::GFx::Movie* a_view, Scaleform::GFx::Value* a_value);
	}
}