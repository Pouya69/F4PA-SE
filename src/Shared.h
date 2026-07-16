#pragma once
#include <RE/B/BGSKeyword.h>
#include <RE/T/TESDataHandler.h>
#include <Scaleform/Scaleform.h>
#include <cstdint>
#include <string>
#include <RE/A/Actor.h>
#include <RE/T/TESGlobal.h>
#include <RE/T/TESNPC.h>
#include <RE/T/TESObjectARMO.h>
#include <RE/T/TESObjectREFR.h>
#include <RE/T/TESObjectWEAP.h>
#include <Scaleform/G/GFx_ASMovieRootBase.h>
#include <Scaleform/G/GFx_FunctionHandler.h>
#include <Scaleform/G/GFx_Value.h>
#include <Scaleform/P/Ptr.h>
#include <RE/B/BGSInventoryItem.h>
#include <RE/E/ExtraDataList.h>
#include <RE/T/TESBoundObject.h>
#include <RE/T/TESObjectMISC.h>
#include <RE/B/BGSInventoryList.h>
#include <RE/T/TESForm.h>
#include "Prisma/PrismaUI_F4_API.h"
#include <unordered_map>
#include <RE/B/BGSLocation.h>
#include <RE/B/BSTTuple.h>
#include <RE/T/TESWorldSpace.h>
#include <RE/B/BGSConstructibleObject.h>
#include <RE/B/BGSListForm.h>
#include <RE/B/BGSTypedFormValuePair.h>
#include <RE/B/BSTArray.h>
#include "ItemDegradation.h"

namespace Shared {
	extern PRISMA_UI_API::IVPrismaUI4* prisma_api;

	extern RE::BGSKeyword* noDegradation;

	extern bool noArmorDegradation;
	extern bool noWeaponDegradation;

	extern RE::TESGlobal* fAutomaticWeaponConditionReduction;
	extern RE::TESGlobal* fBoltWeaponConditionReduction;

	extern RE::TESGlobal* fArmourConditionReductionPerPercentage;

	extern RE::BGSKeyword* crWeaponRanged;

	extern RE::TESObjectMISC* repairKit;

	extern RE::BGSKeyword* notScrappableKeyword;

	extern RE::TESGlobal* repairKitBaseRepair;
	extern RE::TESGlobal* repairKitSkillMutliplier;

	extern RE::TESGlobal* workbenchRepairSkillEffect;

	extern std::unordered_map<std::string, RE::TESWorldSpace*> HighwaymanWorldspacesMap;

	extern std::unordered_map<std::string, RE::BGSLocation*> HighwaymanLocationsMap;

	extern RE::BGSListForm* LocationsMapHighwaymanList;

	extern RE::BGSListForm* WeaponRecipesRepairList;
	extern RE::BGSListForm* ArmorRecipesRepairList;

	extern std::unordered_map<const RE::TESObjectWEAP*, const RE::BGSConstructibleObject*> weaponToCOBJ_Map;
	extern std::unordered_map<const RE::TESObjectARMO*, const RE::BGSConstructibleObject*> armorToCOBJ_Map;

	const RE::BGSConstructibleObject* GetCOBJ_FromWeapon(const RE::TESObjectWEAP* weaponObj);
	const RE::BGSConstructibleObject* GetCOBJ_FromArmor(const RE::TESObjectARMO* armorObj);

	void ApplyFormulaForRepairRequirements(const RE::BSTArray<RE::ExamineMenu::ModChoiceData>& modArray, const RE::ExtraDataList* extraData, RE::BSTArray<RE::BSTTuple<RE::TESForm*, RE::BGSTypedFormValuePair::SharedVal>>& recipeReqItems, RE::BSTArray<RE::BSTTuple<RE::TESForm*, RE::BGSTypedFormValuePair::SharedVal>>& reqItems, const float currentCondition, const float CurrentRepairSkill);

	void InitializeArmorAndWeapon_COBJs();

	void InitializeSharedForms(RE::TESDataHandler* dataHandler);

	bool IsJunkItem(RE::TESBoundObject* obj);
	
	template<typename T>
	bool RegisterFunction(Scaleform::GFx::Value* a_dest, Scaleform::Ptr<Scaleform::GFx::ASMovieRootBase> a_movieRoot, const char* a_func_name) {
		Scaleform::GFx::FunctionHandler* func = new T;
		Scaleform::GFx::Value funcValue;

		a_movieRoot->CreateFunction(&funcValue, func);
		return a_dest->SetMember(a_func_name, funcValue);
	}

	namespace HUD {
		void UpdateMenus(Scaleform::Ptr<Scaleform::GFx::ASMovieRootBase> movieRoot);
	}
	
	bool IsXPMetervisible();

	bool InMenuMode();

	float ConvertPercentageToFloat(std::uint8_t percentage);

	const char* GetItemDisplayName(RE::ExtraDataList* myExtraData, RE::TESBoundObject* baseForm);

	void RemovePipboyInventoryItem(const RE::BGSInventoryItem* item, bool bSilent);

	std::uint32_t GetAvailableComponentCount(RE::BGSInventoryList* a_list, RE::TESForm* a_form);

}

bool WeaponHasKeyword(RE::TESObjectWEAP* weapon, RE::BGSKeyword* keyword);

bool ArmorHasKeyword(RE::TESObjectARMO* armor, RE::BGSKeyword* keyword);

bool ReferenceHasKeyword(RE::TESObjectREFR* ref, RE::BGSKeyword* keyword);

bool ActorHasKeyword(RE::Actor* actor, RE::BGSKeyword* keyword);

bool NPCHasKeyword(RE::TESNPC* npc, RE::BGSKeyword* keyword);

bool IsFormIDStringBaseGame(std::string formIDString);

//	Returns FormID as hex string
std::string GetFormIDAsString(std::uint32_t formID);