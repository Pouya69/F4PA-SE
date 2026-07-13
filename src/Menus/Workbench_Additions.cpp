#include "Workbench_Additions.h"
#include <string.h>
#include <Scaleform/G/GFx_Movie.h>
#include <Scaleform/G/GFx_Value.h>
#include <Scaleform/Scaleform.h>
#include "../Shared.h"
#include "../Scaleform_PArroyo.h"

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
			
			
			
			REX::DEBUG("ExamineMenu.swf finished hooks.");

			// Shared::RegisterFunction<PlayUISound>(&bgsCodeObj, a_view->asMovieRoot, "PlayUISound");

			// a_view->asMovieRoot->Invoke("root.Menu_mc.onCodeObjCreate", nullptr, nullptr, 0);
		}
		return true;
	}
	return false;
}
