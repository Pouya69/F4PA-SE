#include "Workbench_Additions.h"
#include <string.h>
#include <Scaleform/G/GFx_Movie.h>
#include <Scaleform/G/GFx_Value.h>
#include <Scaleform/Scaleform.h>
#include "../Shared.h"
#include "../Scaleform_PArroyo.h"

namespace PArroyo_Menus {
	namespace Workbench_Additions {
		extern bool formulaApplied = false;
		std::uint32_t currentIndex;
		bool bIsScrappingAllJunk = false;
	}
}

bool PArroyo_Menus::Workbench_Additions::RegisterScaleform(Scaleform::GFx::Movie* a_view, Scaleform::GFx::Value* a_value)
{
	Scaleform::GFx::Value currentSWFPath;
	// Register native code handlers.
	if (a_view->asMovieRoot->GetVariable(&currentSWFPath, "root.loaderInfo.url"))
	{
		if (_stricmp(currentSWFPath.GetString(), "Interface/ExamineMenu.swf") == 0)
		{
			Scaleform::GFx::Value bgsCodeObj;
			a_view->asMovieRoot->GetVariable(&bgsCodeObj, "root.BaseInstance.BGSCodeObj");

			Shared::RegisterFunction<WorkbenchRepair_CanRepairSelectedCustom>(&bgsCodeObj, a_view->asMovieRoot, "CanRepairSelectedItemCustom");
			Shared::RegisterFunction<WorkbenchRepair_CanRepairCustom>(&bgsCodeObj, a_view->asMovieRoot, "CanRepairCustom");
			
			Shared::RegisterFunction<Debug_ActionScript>(&bgsCodeObj, a_view->asMovieRoot, "DebugPrint");
			Shared::RegisterFunction<WorkbenchRepair_RepairSelectedCustom>(&bgsCodeObj, a_view->asMovieRoot, "RepairSelectedItemCustom");
			Shared::RegisterFunction<WorkbenchRepair_RepairSelected_RepairKit>(&bgsCodeObj, a_view->asMovieRoot, "RepairSelectedItem_RepairKit");
			Shared::RegisterFunction<Workbench_AddCND_ForItemCard>(&bgsCodeObj, a_view->asMovieRoot, "GetCNDForItemCard");
			Shared::RegisterFunction<WorkbenchRepair_GetRepairKitCount>(&bgsCodeObj, a_view->asMovieRoot, "GetRepairKitCount");
			Shared::RegisterFunction<WorkbenchRepair_NoRepairNeeded>(&bgsCodeObj, a_view->asMovieRoot, "NoRepairNeeded");
			Shared::RegisterFunction<WorkbenchRepair_NoRepairKits>(&bgsCodeObj, a_view->asMovieRoot, "NoRepairKits");

			Shared::RegisterFunction<Workbench_IsWeaponOrArmor>(&bgsCodeObj, a_view->asMovieRoot, "IsWeaponOrArmor");

			Shared::RegisterFunction<Workbench_ScrapAllJunk>(&bgsCodeObj, a_view->asMovieRoot, "ScrapAllJunk");

			Shared::RegisterFunction<Workbench_IsInAllJunk>(&bgsCodeObj, a_view->asMovieRoot, "IsInAllJunk");

			Shared::RegisterFunction<Workbench_HasAnyJunk>(&bgsCodeObj, a_view->asMovieRoot, "HasAnyJunk");
			Shared::RegisterFunction<WorkbenchRepair_NoJunk>(&bgsCodeObj, a_view->asMovieRoot, "NoJunk");
			
			Shared::RegisterFunction<OnEscapePress>(&bgsCodeObj, a_view->asMovieRoot, "CancelBackPressed");
			
			REX::DEBUG("ExamineMenu.swf finished hooks.");

			// Shared::RegisterFunction<PlayUISound>(&bgsCodeObj, a_view->asMovieRoot, "PlayUISound");

			// a_view->asMovieRoot->Invoke("root.Menu_mc.onCodeObjCreate", nullptr, nullptr, 0);
		}

		if (_stricmp(currentSWFPath.GetString(), "Interface/ExamineConfirmMenu.swf") == 0)
		{
			Scaleform::GFx::Value bgsCodeObj;
			a_view->asMovieRoot->GetVariable(&bgsCodeObj, "root.Menu_mc.BGSCodeObj");

			Shared::RegisterFunction<Debug_ActionScript>(&bgsCodeObj, a_view->asMovieRoot, "DebugPrintExamine");
			Shared::RegisterFunction<OnEscapePress>(&bgsCodeObj, a_view->asMovieRoot, "OnEscapePress");
			Shared::RegisterFunction<Workbench_HasAnyJunkExamine>(&bgsCodeObj, a_view->asMovieRoot, "HasAnyJunkExamine");
			Shared::RegisterFunction<OnEscapePress>(&bgsCodeObj, a_view->asMovieRoot, "CancelConfirmMenu");
			Shared::RegisterFunction<Workbench_CompleteScrapAllJunk>(&bgsCodeObj, a_view->asMovieRoot, "CompleteScrapAllJunk");
		}
		return true;
	}
	return false;
}
