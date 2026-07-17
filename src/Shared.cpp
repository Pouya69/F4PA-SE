#include "Shared.h"
#include <RE/B/BGSKeyword.h>
#include "debugLog.h"
#include "IGlobalConfig.h"
#include <RE/T/TESDataHandler.h>
#include <RE/A/Actor.h>
#include <RE/B/BSExtraData.h>
#include <RE/E/ExtraInstanceData.h>
#include <RE/E/EXTRA_DATA_TYPE.h>
#include <RE/T/TBO_InstanceData.h>
#include <RE/T/TESNPC.h>
#include <RE/T/TESObjectARMO.h>
#include <RE/T/TESObjectREFR.h>
#include <RE/T/TESObjectWEAP.h>
#include <RE/T/TESRace.h>
#include <cstdint>
#include <ios>
#include <sstream>
#include <string>
#include <string_view>
#include <RE/B/BGSInventoryItem.h>
#include <RE/B/BSFixedString.h>
#include <RE/E/ExtraDataList.h>
#include <RE/E/ExtraTextDisplayData.h>
#include <RE/I/IMenu.h>
#include <RE/P/PlayerCharacter.h>
#include <RE/S/SendHUDMessage.h>
#include <RE/S/Setting.h>
#include <RE/T/TESBoundObject.h>
#include <RE/T/TESGlobal.h>
#include <RE/U/UI.h>
#include <Scaleform/G/GFx_ASMovieRootBase.h>
#include <Scaleform/G/GFx_Value.h>
#include <Scaleform/P/Ptr.h>
#include <REX/LOG.h>
#include <RE/T/TESObjectMISC.h>
#include <RE/U/UIUtils.h>
#include <algorithm>
#include <RE/B/BGSInventoryList.h>
#include <RE/B/BSTSmartPointer.h>
#include <RE/E/ENUM_FORM_ID.h>
#include <RE/T/TESForm.h>
#include "Prisma/PrismaUI_F4_API.h"
#include "Prisma/Highwayman_Map.h"
#include "ItemDegradation.h"

namespace Shared {
	PRISMA_UI_API::IVPrismaUI4* prisma_api = nullptr;

	RE::BGSKeyword* noDegradation;
	bool noArmorDegradation;
	bool noWeaponDegradation;

	RE::TESGlobal* fAutomaticWeaponConditionReduction;
	RE::TESGlobal* fBoltWeaponConditionReduction;

	RE::TESGlobal* fArmourConditionReductionPerPercentage;


	RE::BGSKeyword* crWeaponRanged;

	RE::TESObjectMISC* repairKit;

	RE::BGSKeyword* notScrappableKeyword;

	RE::TESGlobal* repairKitBaseRepair;
	RE::TESGlobal* repairKitSkillMutliplier;

	RE::TESGlobal* workbenchRepairSkillEffect;

	std::unordered_map<std::string, RE::TESWorldSpace*> HighwaymanWorldspacesMap;

	std::unordered_map<std::string, RE::BGSLocation*> HighwaymanLocationsMap;

	RE::BGSListForm* LocationsMapHighwaymanList;

	RE::BGSListForm* WeaponRecipesRepairList;
	RE::BGSListForm* ArmorRecipesRepairList;

	std::unordered_map<const RE::TESObjectWEAP*, const RE::BGSConstructibleObject*> weaponToCOBJ_Map;
	std::unordered_map<const RE::TESObjectARMO*, const RE::BGSConstructibleObject*> armorToCOBJ_Map;

	void InitializeSharedForms(RE::TESDataHandler* dataHandler)
	{
		crWeaponRanged = dataHandler->LookupForm<RE::BGSKeyword>(0x189348, "Fallout4.esm");
		noDegradation = dataHandler->LookupForm<RE::BGSKeyword>(0x0D1BFE, MOD_ESM);

		fAutomaticWeaponConditionReduction = dataHandler->LookupForm<RE::TESGlobal>(0x0D1BFA, MOD_ESM);
		fBoltWeaponConditionReduction = dataHandler->LookupForm<RE::TESGlobal>(0x0D1BF9, MOD_ESM);

		fArmourConditionReductionPerPercentage = dataHandler->LookupForm<RE::TESGlobal>(0x0D1BF8, MOD_ESM);

		repairKit = dataHandler->LookupForm<RE::TESObjectMISC>(0x002E4F, CURRENT_ESP);

		notScrappableKeyword = dataHandler->LookupForm<RE::BGSKeyword>(0x005477, CURRENT_ESP);

		repairKitBaseRepair = dataHandler->LookupForm<RE::TESGlobal>(0x005E02, CURRENT_ESP);
		repairKitSkillMutliplier = dataHandler->LookupForm<RE::TESGlobal>(0x005E03, CURRENT_ESP);
		workbenchRepairSkillEffect = dataHandler->LookupForm<RE::TESGlobal>(0x005E04, CURRENT_ESP);

		WeaponRecipesRepairList = dataHandler->LookupForm<RE::BGSListForm>(0x00D035, CURRENT_ESP);
		ArmorRecipesRepairList = dataHandler->LookupForm< RE::BGSListForm>(0x00D036, CURRENT_ESP);

		InitializeArmorAndWeapon_COBJs();

		PArroyo::Highwayman::InitializeHighwaymanMapLocations();


	}

	RE::BGSComponent* GetBaseComponentFromForm(RE::TESForm* a_form)
	{
		if (!a_form) return nullptr;

		auto miscObj = static_cast<RE::TESObjectMISC*>(a_form);

		if (!miscObj || a_form->GetFormType() != RE::ENUM_FORM_ID::kMISC) {
			auto compObj = static_cast<RE::BGSComponent*>(a_form);
			return compObj ? compObj : nullptr;
		}

		if (!miscObj->componentData || miscObj->componentData->empty() || !miscObj->componentData->at(0).first)
			return nullptr;

		const auto first = miscObj->componentData->at(0).first;

		RE::BGSComponent* compObj = static_cast<RE::BGSComponent*>(first);
		return compObj;

		return nullptr;
	}

	const RE::BGSConstructibleObject* GetCOBJ_FromWeapon(const RE::TESObjectWEAP* weaponObj)
	{
		return weaponToCOBJ_Map.contains(weaponObj) ? weaponToCOBJ_Map.at(weaponObj) : nullptr;
	}

	const RE::BGSConstructibleObject* GetCOBJ_FromArmor(const RE::TESObjectARMO* armorObj)
	{
		return armorToCOBJ_Map.contains(armorObj) ? armorToCOBJ_Map.at(armorObj) : nullptr;
	}

	void ApplyFormulaForRepairRequirements(const RE::BSTArray<RE::ExamineMenu::ModChoiceData>& modArray, const RE::ExtraDataList* extraData,
		RE::BSTArray<RE::BSTTuple<RE::TESForm*, RE::BGSTypedFormValuePair::SharedVal>>& recipeReqItems, RE::BSTArray<RE::BSTTuple<RE::TESForm*,
		RE::BGSTypedFormValuePair::SharedVal>>& reqItems, const float currentCondition, const float CurrentRepairSkill)
	{
		const std::uint32_t repairSkillReduction = floor(CurrentRepairSkill / workbenchRepairSkillEffect->GetValue());
		std::uint32_t i = 0;

		// auto vm = RE::GameVM::GetSingleton()->GetVM();
		
		std::vector<RE::TESObjectMISC*> mods;

		
		

		auto* player = RE::PlayerCharacter::GetSingleton();

		// We first add the mods if any
		RE::BGSObjectInstanceExtra* instExtra = extraData->GetByType<RE::BGSObjectInstanceExtra>();
		if (instExtra && player && player->inventoryList) {
			for (auto& idx : instExtra->GetIndexData()) {
				// idx.objectID is the OMOD's raw FormID — resolve it to the real form
				auto mod = RE::TESForm::GetFormByID<RE::BGSMod::Attachment::Mod>(idx.objectID);
				for (const auto& modInModArray : modArray) {
					if (modInModArray.mod->GetFormID() == mod->GetFormID()) {

						for (auto needed = modInModArray.recipe->requiredItems->begin(); needed != modInModArray.recipe->requiredItems->end(); ++needed) {
							auto finalTuple = RE::BSTTuple<RE::TESForm*, RE::BGSTypedFormValuePair::SharedVal>(needed->first, needed->second);
							reqItems.push_back(finalTuple);
						}
					}
				}
			}
		}

		// Shared::GetAvailableComponentCount(inventoryList, form);
		
		

		for (auto needed = recipeReqItems.begin(); needed != recipeReqItems.end(); ++needed) {

			RE::BSTTuple<RE::TESForm*, RE::BGSTypedFormValuePair::SharedVal>* neededCopy = new RE::BSTTuple<RE::TESForm*, RE::BGSTypedFormValuePair::SharedVal>(needed->first, needed->second.i);

			RE::BSTTuple<RE::TESForm*, RE::BGSTypedFormValuePair::SharedVal>* foundInMod = nullptr;
			int ii = 0;
			for (auto reqInList = reqItems.begin(); reqInList != reqItems.end(); ++reqInList) {
				if ((std::uint32_t)reqInList->first->GetFormID() == (std::uint32_t)needed->first->GetFormID()) {
					foundInMod = reqInList;
					neededCopy->second.i += (reqInList->second.i * (1.0f - currentCondition));
					break;
				}
				ii++;
			}


			const std::uint32_t oldCount = neededCopy->second.i;

			if (repairSkillReduction > 1) {
				int newCount = neededCopy->second.i;
				newCount -= repairSkillReduction;
				neededCopy->second.i = std::max(newCount, 1);
			}

			// Condition Reduction
			neededCopy->second.i = (std::uint32_t) std::max(neededCopy->second.i * (1.0f - currentCondition), 1.0f);

			auto finalTuple = RE::BSTTuple<RE::TESForm*, RE::BGSTypedFormValuePair::SharedVal>(neededCopy->first, neededCopy->second.i);
			if (foundInMod) {
				reqItems[ii] = finalTuple;
			}
			else {
				reqItems.push_back(finalTuple);
			}



			

			


			//(*currentModChoiceData->recipe->requiredItems)[i] = *neededCopy;
			// needed.second.f = max((int)needed.second.f / repairSkillReduction, 1);
			// (*currentModChoiceData->recipe->requiredItems)[i] = needed;

			REX::DEBUG(std::format("RepairReduction - Comp from {} to {}. Skill Reduction: {}. With mods? {}", oldCount, neededCopy->second.i, repairSkillReduction, foundInMod ? 1 : 0).c_str());
			i++;
		}


		
	}

	
	void InitializeArmorAndWeapon_COBJs()
	{
		for (const auto objInList : WeaponRecipesRepairList->arrayOfForms) {
			const RE::BGSConstructibleObject* constructible = static_cast<RE::BGSConstructibleObject*>(objInList);
			weaponToCOBJ_Map.emplace(static_cast<RE::TESObjectWEAP*>(constructible->GetCreatedItem()), constructible);
		}

		for (const auto objInList : ArmorRecipesRepairList->arrayOfForms) {
			const RE::BGSConstructibleObject* constructible = static_cast<RE::BGSConstructibleObject*>(objInList);
			armorToCOBJ_Map.emplace(static_cast<RE::TESObjectARMO*>(constructible->GetCreatedItem()), constructible);
		}
	}

	bool IsJunkItem(RE::TESBoundObject* obj)
	{
		// "Take my junk" must take ONLY scrappable junk. caps (0xF), bobby pins
		// (0xA), keys, quest items and collectibles are all kMISC too, so a bare
		// kMISC test wrongly swept them up (reported by a VR user). The engine's
		// signal for real junk is a non-empty crafting-component list
		// (TESObjectMISC::componentData); the non-junk MISC above have none.
		// (Books are kBOOK and were never matched here.)
		if (!obj || obj->formType.get() != RE::ENUM_FORM_ID::kMISC) {
			return false;
		}
		auto* misc = static_cast<RE::TESObjectMISC*>(obj);
		if ((misc->formID & 0x00FFFFFFu) == 0x0000000Fu || (misc->formID & 0x00FFFFFFu) == 0x0000000Au || misc->HasKeyword(Shared::notScrappableKeyword)) return false;  // caps or bobby pins, never

		if (misc->componentData && !misc->componentData->empty()) {
			//if (misc->componentData->at(0).first->GetFormType() != RE::ENUM_FORM_ID::kMISC) {
			//	return false;
			//}

			// auto compObj = static_cast<RE::TESObjectMISC*>(misc->componentData->at(0).first);

			//if (compObj == nullptr && compObj->IsBoundObject())
				//return false;

			//return compObj && compObj->componentData->at(0);
			return true;
			
			//return compObj;
		}

		return false;
	}

	std::uint32_t GetAvailableComponentCount(RE::BGSInventoryList* a_list, RE::TESForm* a_form)
	{
		using namespace RE;
		auto reverseInv = a_list->data;
		std::reverse(reverseInv.begin(), reverseInv.end());

		//std::uint32_t amount = a_list->GetItemCount(reinterpret_cast<RE::TESBoundObject*>(a_form));
		std::uint32_t amount = 0;
		//for (BGSInventoryItem& item : reverseInv) {
		//	
		//}
		

		for (BGSInventoryItem& item : reverseInv)
		{
			if (item.object->formType == ENUM_FORM_ID::kMISC)
			{
				TESObjectMISC* miscItem = (TESObjectMISC*)(item.object);

				if (miscItem->componentData)
				{
					for (auto& componentData : *miscItem->componentData)
					{
						if (componentData.first == a_form)
						{
							std::uint32_t count = [item]
								{
									std::uint32_t _count = 0;
									BSTSmartPointer<BGSInventoryItem::Stack> pointer = item.stackData;

									while (pointer)
									{
										_count += pointer->GetCount();
										pointer = pointer->nextStack;
									}

									return _count;
								}();

							amount += componentData.second.i * count;
							break;
						}
					}
				}
			}
		}

		return amount;
	}

	bool IsXPMetervisible()
	{
		RE::BSFixedString menuString("HUDMenu");
		RE::IMenu* menu = RE::UI::GetSingleton()->GetMenu(menuString).get();
		Scaleform::GFx::Value openValue;

		// menu->uiMovie->asMovieRoot->GetVariable(&openValue, "root.XPMeter_mc.visible");
		if (!menu->uiMovie->asMovieRoot->GetVariable(&openValue, "root.HUDNotificationsGroup_mc.XPMeter_mc.visible")) {
			REX::DEBUG("root.HUDNotificationsGroup_mc.XPMeter_mc.visible did not return a value."sv);
		}
		
		return openValue.GetBoolean();
	}

	bool InMenuMode()
	{
		RE::UI* ui = RE::UI::GetSingleton();
		return (
			(ui->menuMode >= 1)
			|| ui->GetMenuOpen("CookingMenu")
			|| ui->GetMenuOpen("FaderMenu")
			|| ui->GetMenuOpen("FavoritesMenu")
			|| ui->GetMenuOpen("PowerArmorModMenu")
			|| ui->GetMenuOpen("RobotModMenu")
			|| ui->GetMenuOpen("VATSMenu")
			|| ui->GetMenuOpen("WorkshopMenu")
			|| ui->GetMenuOpen("DialogueMenu")
			);
	}

	float ConvertPercentageToFloat(std::uint8_t percentage)
	{
		return (percentage / static_cast<float>(100));
	}

	const char* GetItemDisplayName(RE::ExtraDataList* myExtraData, RE::TESBoundObject* baseForm)
	{
		RE::BSExtraData* extraData = myExtraData->GetByType(RE::EXTRA_DATA_TYPE::kTextDisplayData);
		RE::ExtraTextDisplayData* displayText = static_cast<RE::ExtraTextDisplayData*>(extraData);
		if (displayText)
		{
			return displayText->GetDisplayName(baseForm).c_str();
		}
		else
		{
			return "";
		}
	}

	void RemovePipboyInventoryItem(const RE::BGSInventoryItem* item, bool bSilent)
	{
		if (!bSilent)
		{
			std::string itemName = GetItemDisplayName(item->stackData->extra.get(), item->object);
			itemName.append(" ");
			itemName.append(RE::GameSettingCollection::GetSingleton()->GetSetting("sRemoveItemfromInventory")->GetString());
			RE::UIUtils::PlayPipboySound("OBJLunchboxKidsRobotBuild");
			RE::SendHUDMessage::ShowHUDMessage(itemName.c_str(), "", false, true);
		}
		RE::PlayerCharacter::GetSingleton()->inventoryList->RemoveItem1(item->object, item->GetCount(), false);
		//RE::PipboyDataManager::GetSingleton()->
		//RemovePipboyItem(&(*g_PipboyDataManager)->inventoryData, item);
	}

}

bool WeaponHasKeyword(RE::TESObjectWEAP* weapon, RE::BGSKeyword* keyword)
{
	if (weapon)
	{
		return weapon->HasKeyword(keyword);
	}

	return false;
}

bool ArmorHasKeyword(RE::TESObjectARMO* armor, RE::BGSKeyword* keyword)
{
	if (armor)
	{
		return armor->HasKeyword(keyword);
	}

	return false;
}

bool ReferenceHasKeyword(RE::TESObjectREFR* ref, RE::BGSKeyword* keyword)
{
	if (ref)
	{
		if (ref->HasKeyword(keyword))
			return true;

		RE::TBO_InstanceData* myInstanceData = nullptr;
		RE::BSExtraData* myExtraData = ref->extraList->GetByType(RE::EXTRA_DATA_TYPE::kInstanceData);

		if (myExtraData)
		{
			RE::ExtraInstanceData* myExtraInstanceData = dynamic_cast<RE::ExtraInstanceData*>(myExtraData);
			if (myExtraInstanceData)
			{
				myInstanceData = myExtraInstanceData->data.get();
			}
		}

		if (myInstanceData)
		{
			return myInstanceData->GetKeywordData()->HasKeyword(keyword);
		}
	}

	return false;
}

bool ActorHasKeyword(RE::Actor* actor, RE::BGSKeyword* keyword)
{
	if (actor)
	{
		if (actor->HasKeyword(keyword))
			return true;

		RE::TBO_InstanceData* myInstanceData = nullptr;
		RE::BSExtraData* myExtraData = actor->extraList->GetByType(RE::EXTRA_DATA_TYPE::kInstanceData);

		if (myExtraData)
		{
			RE::ExtraInstanceData* myExtraInstanceData = dynamic_cast<RE::ExtraInstanceData*>(myExtraData);
			if (myExtraInstanceData)
			{
				myInstanceData = myExtraInstanceData->data.get();
			}
		}

		if (myInstanceData)
		{
			return myInstanceData->GetKeywordData()->HasKeyword(keyword);
		}
	}

	return false;
}

bool NPCHasKeyword(RE::TESNPC* npc, RE::BGSKeyword* keyword)
{
	if (npc)
	{
		if (npc->HasKeyword(keyword))
			return true;
	}

	//	if still false, check if race has keyword (doesnt seem to be found on the NPC check if exists on race?)
	RE::TESRace* npcRace = npc->formRace;

	if (npcRace)
	{
		return npcRace->HasKeyword(keyword);
	}

	return false;
}


//	Returns True if FormID string is base game OR dynamically placed (FF index)
//	Assumes all DLC is installed
bool IsFormIDStringBaseGame(std::string formIDString)
{
	std::string formIndex = formIDString.substr(0, 2);

	if (formIndex == "00" || formIndex == "01" || formIndex == "02" || formIndex == "03" || formIndex == "04" || formIndex == "05" || formIndex == "06" || formIndex == "ff")
	{
		return true;
	}

	return false;
}

//	Returns FormID as hex string
std::string GetFormIDAsString(std::uint32_t formID)
{
	std::string result{};
	std::stringstream formIDStream;
	formIDStream << std::hex << formID;
	result = formIDStream.str();
	if (result.length() < 8)
	{
		std::string temp;
		for (int i = 0; i < (8 - result.length()); i++)
		{
			temp.append("0");
		}
		temp.append(result);
		result = temp;
	}
	return result;
}



void Shared::HUD::UpdateMenus(Scaleform::Ptr<Scaleform::GFx::ASMovieRootBase> movieRoot)
{
	REX::WARN("Menu Update requested event!");
	// movieRoot->Invoke("");
}
