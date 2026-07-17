#include "Scaleform_PArroyo.h"
#include <cstdint>
#include <string>
#include <RE/B/BGSInventoryItem.h>
#include <RE/B/BSFixedString.h>
#include <RE/E/ExtraDataList.h>
#include <RE/P/PipboyArray.h>
#include <RE/P/PipboyPrimitiveValue.h>
#include <RE/T/TESForm.h>
#include <Scaleform/G/GFx_ASMovieRootBase.h>
#include <Scaleform/G/GFx_Value.h>
#include <chrono>
#include <format>
#include <thread>
#include <RE/B/BGSInventoryInterface.h>
#include <RE/E/ENUM_FORM_ID.h>
#include <RE/P/PlayerCharacter.h>
#include <RE/U/UIUtils.h>
#include <Scaleform/G/GFx_FunctionHandler.h>
#include <REX/LOG.h>
#include "debugLog.h"
#include "ItemDegradation.h"
#include "Menus/LevelUpMenu.h"
#include "Menus/PipboyTabs.h"
#include "Menus/RepairMenu.h"
#include "Shared.h"
#include <RE/A/Actor.h>
#include <RE/M/MenuControls.h>
#include <RE/P/PipboyManager.h>
#include <RE/T/TESObjectWEAP.h>
#include <RE/U/UI.h>
#include <string_view>
#include <RE/B/BGSComponent.h>
#include <RE/B/BGSConstructibleObject.h>
#include <RE/B/BGSTypedFormValuePair.h>
#include <RE/B/BSTArray.h>
#include <RE/B/BSTTuple.h>
#include <RE/E/ExamineMenu.h>
#include <RE/I/IMenu.h>
#include <RE/T/TESDataHandler.h>
#include <RE/T/TESBoundObject.h>
#include <RE/E/ExamineConfirmMenu.h>
#include <RE/I/InventoryUserUIInterfaceEntry.h>
#include <RE/P/PipboyDataManager.h>
#include <RE/S/SendHUDMessage.h>
#include <RE/S/Setting.h>
#include <RE/T/TESObjectMISC.h>
#include <RE/T/TESObjectREFR.h>
#include <Scaleform/P/Ptr.h>
#include "GameForms.h"
#include "Menus/Workbench_Additions.h"

void GFxUtilities::SetScaleformValue(Scaleform::GFx::Value* dst, const char* name, Scaleform::GFx::Value value)
{
	dst->SetMember(name, &value);
}

void GFxUtilities::SetScaleformValue(Scaleform::GFx::Value* dst, const char* name, Scaleform::GFx::Value* value)
{
	dst->SetMember(name, value);
}

void GFxUtilities::RegisterString(Scaleform::GFx::Value* destination, Scaleform::GFx::ASMovieRootBase* root, const char* name, const char* str)
{
	Scaleform::GFx::Value* fxValue;
	root->CreateString(fxValue, str);
	destination->SetMember(name, *fxValue);
}

namespace InventoryUtils
{
	using namespace RE;

	std::uint32_t GetPipboyInventoryObjectCount()
	{
		return PipboyInventoryObjects.size();

		// RE::PlayerCharacter* player = RE::PlayerCharacter::GetSingleton();
		// return player->inventoryList->data.size();
	}

	std::string GetInventoryDisplayName(std::uint32_t index)
	{
		PipboyPrimitiveValue<BSFixedString>* pipboyObject = static_cast<PipboyPrimitiveValue<BSFixedString>*>(PipboyInventoryObjects.at(index)->memberMap.find(BSFixedString("text"))->second);
		
		// RE::PlayerCharacter* player = RE::PlayerCharacter::GetSingleton();
		// auto a = player->inventoryList->data.at(index).GetDisplayFullName()
		// return player->inventoryList->data.at(index).GetDisplayFullName;

		return pipboyObject->value.c_str();
	}

	std::uint32_t GetHandleIDByIndex(std::uint32_t index)
	{
		PipboyPrimitiveValue<std::uint32_t>* pipboyObject = static_cast<PipboyPrimitiveValue<std::uint32_t>*>(PipboyInventoryObjects.at(index)->memberMap.find(BSFixedString("HandleID"))->second);

		// RE::PlayerCharacter* player = RE::PlayerCharacter::GetSingleton();
		// auto a = player->inventoryList->data.at(index).GetDisplayFullName()

		return pipboyObject->value;
	}

	std::uint32_t GetStackIDByIndex(std::uint32_t index)
	{

		PipboyArray* StackIDs = static_cast<PipboyArray*>(PipboyInventoryObjects.at(index)->memberMap.find(BSFixedString("StackID"))->second);
		std::uint32_t Result = 0;

		for (int i = 0; i < StackIDs->elements.size(); i++)
		{
			PipboyPrimitiveValue<std::uint32_t>* pipboyObject = static_cast<PipboyPrimitiveValue<std::uint32_t>*>(StackIDs->elements.at(i));
			Result = pipboyObject->value;
		}

		return Result;
	}

	
	TESForm* GetInventoryFormByHandleID(std::uint32_t HandleID)
	{
		return BGSInventoryInterface::GetSingleton()->RequestInventoryItem(HandleID)->object;
	}
	

	TESForm* GetInventoryFormByIndex(std::uint32_t index)
	{
		PipboyPrimitiveValue<std::uint32_t>* pipboyObject = static_cast<PipboyPrimitiveValue<std::uint32_t>*>(PipboyInventoryObjects.at(index)->memberMap.find(BSFixedString("HandleID"))->second);
		std::uint32_t HandleID = pipboyObject->value;

		// RE::PlayerCharacter* player = RE::PlayerCharacter::GetSingleton();

		// return player->inventoryList->data.at(index).object;

		return GetInventoryFormByHandleID(HandleID);
	}

	
	const BGSInventoryItem* GetInventoryItemByHandleID(std::uint32_t HandleID)
	{
		return RE::BGSInventoryInterface::GetSingleton()->RequestInventoryItem(HandleID);
	}
	

	const BGSInventoryItem* GetInventoryItemByIndex(std::uint32_t index)
	{
		// auto player = RE::PlayerCharacter::GetSingleton();
		if (PipboyInventoryObjects.size() <= index)
			return nullptr;

		PipboyPrimitiveValue<std::uint32_t>* pipboyObject = static_cast<PipboyPrimitiveValue<std::uint32_t>*>(PipboyInventoryObjects.at(index)->memberMap.find(BSFixedString("HandleID"))->second);
		std::uint32_t HandleID = pipboyObject->value;
		return GetInventoryItemByHandleID(HandleID);
		// return &player->inventoryList->data.at(index);
	}

	const RE::BGSInventoryItem* GetInventoryItemWorkbenchByIndex()
	{
		auto examineMenu = RE::UI::GetSingleton()->GetMenu<RE::ExamineMenu>();
		std::uint32_t selectedIndex = examineMenu->GetSelectedIndex();

		if (selectedIndex >= examineMenu->invInterface.stackedEntries.size())
			selectedIndex = 0;

		if (!examineMenu->invInterface.entriesInvalid && (selectedIndex & 0x80000000) == 0 && selectedIndex < examineMenu->invInterface.stackedEntries.size()) {
			InventoryUserUIInterfaceEntry* inventoryUUIEntry = (examineMenu->invInterface.stackedEntries.data() + selectedIndex);
			return BGSInventoryInterface::GetSingleton()->RequestInventoryItem(inventoryUUIEntry->invHandle.id);
		}

		return nullptr;
	}

	const RE::BGSInventoryItem* GetCurrentEquippedWeapon(RE::Actor* a_actor)
	{
		a_actor->inventoryList->rwLock.lock_read();
		for (RE::BGSInventoryItem& inventoryItem : a_actor->inventoryList->data) {
			if (!inventoryItem.IsEquipped(0))
				continue;

			RE::TESObjectWEAP* weapon = static_cast<RE::TESObjectWEAP*>(inventoryItem.object);
			if (inventoryItem.object && weapon->formType == RE::ENUM_FORM_ID::kWEAP) {
				a_actor->inventoryList->rwLock.unlock_read();
				return &inventoryItem;
			}
		}
		a_actor->inventoryList->rwLock.unlock_read();


		return nullptr;
	}

	std::uint32_t GetIndexByInventoryItem(const BGSInventoryItem* item)
	{
		std::uint32_t inventoryCount = GetPipboyInventoryObjectCount();

		for (std::uint32_t i = 0; i < inventoryCount; i++)
		{
			const BGSInventoryItem* iter = GetInventoryItemByIndex(i);

			if (item == iter)
			{
				return i;
			}
		}

		return -1;
	}

	const BGSInventoryItem::Stack* GetStackByStackID(const BGSInventoryItem* Item, int StackID)
	{
		BGSInventoryItem::Stack* traverse = Item->stackData.get();
		if (!traverse)
			return nullptr;

		while (StackID != 0) {
			traverse = traverse->nextStack.get();
			if (!traverse)
				return nullptr;
			StackID--;
		}

		return traverse;
	}

	const RE::BGSInventoryItem* GetInventoryItemByForm(RE::TESForm* a_form)
	{
		if (!a_form)
			return nullptr;

		auto a_actor = RE::PlayerCharacter::GetSingleton();
		a_actor->inventoryList->rwLock.lock_read();
		for (RE::BGSInventoryItem& inventoryItem : a_actor->inventoryList->data) {
			if ((std::uint32_t) inventoryItem.object == (std::uint32_t) a_form) {
				a_actor->inventoryList->rwLock.unlock_read();
				return &inventoryItem;
			}
		}
		a_actor->inventoryList->rwLock.unlock_read();
		return nullptr;
	}

	ExtraDataList* GetExtraDataListByStackID(const BGSInventoryItem* Item, int StackID)
	{
		if (!Item)
			return nullptr;

		const BGSInventoryItem::Stack* stack = GetStackByStackID(Item, StackID);
		return stack->extra.get() ? stack->extra.get() : nullptr;
	}

	ExtraDataList* GetExtraDataListByIndex(std::uint32_t index)
	{
		const BGSInventoryItem* Item = GetInventoryItemByIndex(index);
		std::uint32_t              StackID = GetStackIDByIndex(index);

		if (!Item)
			return nullptr;

		const BGSInventoryItem::Stack* stack = GetStackByStackID(Item, StackID);
		return stack->extra.get() ? stack->extra.get() : nullptr;
	}
}

void Pipboy_Repair::Call(const Params& a_params)
{
	std::uint32_t arraySize = a_params.args[0].GetArraySize();
	Scaleform::GFx::Value arrayElement;
	Scaleform::GFx::Value menuType;
	Scaleform::GFx::Value invObjectArray;

	std::uint32_t iMenuType;
	PArroyo_Menus::RepairMenu::InvObject currentInvObject;

	for (int i = 0; i < arraySize; i++)
	{
		a_params.args[0].GetElement(i, &arrayElement);
		arrayElement.GetMember("iCurrentTab", &menuType);
		arrayElement.GetMember("invItem", &invObjectArray);
		invObjectArray.GetElement(0, &invObjectArray);

		iMenuType = menuType.GetInt();
		currentInvObject = PArroyo_Menus::RepairMenu::RepairMenuFunctions::CreateInvObjectFromArray(invObjectArray);
	}

	REX::DEBUG(std::format("Pipboy Menu: Repair Button Pressed, type: {}", iMenuType).c_str());
	PArroyo_Menus::RepairMenu::RepairMenuFunctions::OpenRepairFromPipboy(a_params.movie->asMovieRoot, iMenuType, currentInvObject);
	// PArroyo_Menus::PipboyTabs::CloseMenu();
	// PArroyo_Menus::PipboyTabs::RegisterForInput(false);
}

void Pipboy_CheckInventoryItem::Call(const Params& a_params)
{
	const int index = a_params.args[0].GetInt();

	const bool result = PArroyo_Menus::RepairMenu::RepairMenuFunctions::CheckInventoryForEligibleRepair(index);

	*a_params.retVal = result;
}

void RepairMenu_CloseMenu::Call(const Params& a_params)
{
	PArroyo_Menus::RepairMenu::RepairMenuFunctions::RegisterForInput(false);
	
	//Scaleform::OpenMenu_Internal("HUDMenu");

	// Scaleform::CloseMenu_Internal("RepairMenu");
	PArroyo_Menus::RepairMenu::RepairMenuFunctions::HandleMenuClose();

	if (RE::UI::GetSingleton()->GetMenuOpen("PipboyMenu")) {
		PArroyo_Menus::PipboyTabs::RegisterForInput(true);
		RE::PipboyManager::GetSingleton()->inputEventHandlingEnabled = true;
		auto menuControls = RE::MenuControls::GetSingleton();
		//auto handler = reinterpret_cast<RE::BSInputEventUser*>(menuControls->pipboyHandler);
		//menuControls->RegisterHandler(handler);
		RE::IMenu* menu = RE::UI::GetSingleton()->GetMenu("PipboyMenu").get();
		Scaleform::GFx::ASMovieRootBase* movieRoot = menu->uiMovie->asMovieRoot.get();
		movieRoot->Invoke("root.Menu_mc.CWPipboy_loader.content.closePipboyRepairMenu", nullptr, nullptr, 0);
	}
	else {
		a_params.movie->asMovieRoot->Invoke("root.Menu_mc.onCodeObjDestruction", nullptr, nullptr, 0);
		PArroyo_Menus::RepairMenu::CloseRepairMenu();
	}
}

void Scaleform_PlayUISound::Call(const Params& a_params)
{
	if (!a_params.args[0].IsString())
	{
		return;
	}
	else
	{
		const std::string soundS = a_params.args[0].GetString();
		REX::DEBUG(std::format("Scaleform_PlayUISound: Playing UI sound: {}", soundS).c_str());

		RE::UIUtils::PlayMenuSound(soundS.c_str());
	}
}

void RepairMenu_RepairItemPipboy::Call(const Params& a_params)
{
	//	this will pass through the object we are using to repair the current object
	std::uint32_t repairCount = a_params.args[0].GetArraySize(); //	this should always be 1 in this case, but just to be sure
	Scaleform::GFx::Value arrayElement;
	Scaleform::GFx::Value handleID;
	Scaleform::GFx::Value stackID;
	Scaleform::GFx::Value newCondition;

	REX::DEBUG("RepairMenu Pipboy: Repairing Items");

	//PArroyo_Menus::PipboyTabs::CloseMenu();

	//std::this_thread::sleep_for(std::chrono::milliseconds(1000));

	for (int i = 0; i < repairCount; i++)
	{
		a_params.args[0].GetElement(i, &arrayElement);
		arrayElement.GetMember("iHandleID", &handleID);
		arrayElement.GetMember("iStackID", &stackID);
		arrayElement.GetMember("fCondition", &newCondition);

		const RE::BGSInventoryItem* myItem = InventoryUtils::GetInventoryItemByHandleID(handleID.GetUInt());

		PArroyo_Menus::RepairMenu::RepairMenuFunctions::RepairItemPipboy(myItem, newCondition.GetNumber());
		// PArroyo_Menus::RepairMenu::RepairMenuFunctions::HandleMenuOpen(a_params.movie->asMovieRoot.get(), -1);
	}
}

void RepairMenu_RepairItems::Call(const Params& a_params)
{
	auto player = RE::PlayerCharacter::GetSingleton();

	std::uint32_t repairCount = a_params.args[0].GetArraySize();
	Scaleform::GFx::Value arrayElement;
	Scaleform::GFx::Value handleID;
	Scaleform::GFx::Value stackID;
	Scaleform::GFx::Value newCondition;
	Scaleform::GFx::Value repairCost;

	int totalRepairCost = 0;

	REX::DEBUG("RepairMenu: Repairing Items");

	for (int i = 0; i < repairCount; i++)
	{
		a_params.args[0].GetElement(i, &arrayElement);
		arrayElement.GetMember("iHandleID", &handleID);
		arrayElement.GetMember("iStackID", &stackID);
		arrayElement.GetMember("fCondition", &newCondition);
		arrayElement.GetMember("fCost", &repairCost);

		const RE::BGSInventoryItem* myItem = InventoryUtils::GetInventoryItemByHandleID(handleID.GetUInt());

		if (myItem->object->GetFormType() == RE::ENUM_FORM_ID::kWEAP)
		{
			PArroyo::ItemDegradation::WeaponConditionData myWeapon(player, myItem->object, myItem->stackData->extra.get());
			PArroyo_Menus::RepairMenu::RepairMenuFunctions::RepairItem(myWeapon, newCondition.GetNumber());
		}
		else if (myItem->object->GetFormType() == RE::ENUM_FORM_ID::kARMO)
		{
			PArroyo::ItemDegradation::ArmorConditionData myArmor(player, myItem->object, myItem->stackData->extra.get());
			PArroyo_Menus::RepairMenu::RepairMenuFunctions::RepairItem(myArmor, newCondition.GetNumber());
		}

		totalRepairCost += repairCost.GetInt();
	}

	PArroyo_Menus::RepairMenu::RepairMenuFunctions::GiveCapsToVendor(totalRepairCost);
	
	RE::UIUtils::PlayMenuSound("OBJLunchboxKidsRobotBuild");
	// RE::SendHUDMessage::ShowHUDMessage(std::format("Removed {}.", repairCount).c_str(), "", false, true);
	// PArroyo_Menus::RepairMenu::RepairMenuFunctions::HandleMenuOpen(a_params.movie->asMovieRoot.get(), totalRepairCost);
}

void RepairMenu_OpenMenu::Call(const Params& a_params)
{
	REX::DEBUG("RepairMenu: OpenMenu called from AS3");
	
	if (RE::UI::GetSingleton()->GetMenuOpen("PipboyMenu")) {
		PArroyo_Menus::PipboyTabs::RegisterForInput(false);
		RE::PipboyManager::GetSingleton()->inputEventHandlingEnabled = false;
		auto menuControls = RE::MenuControls::GetSingleton();
		// auto handler = reinterpret_cast<RE::BSInputEventUser*>(menuControls->pipboyHandler);
		// menuControls->UnregisterHandler(handler);
		// static_cast<RE::BSInputEventUser*>();
	}


	

	PArroyo_Menus::RepairMenu::RepairMenuFunctions::HandleMenuOpen(a_params.movie->asMovieRoot.get());
	// Scaleform::CloseMenu_Internal("PipboyMenu");
}

void Pipboy_CheckWorkshopTab::Call(const Params& a_params)
{
	const bool result = PArroyo_Menus::RepairMenu::RepairMenuFunctions::CheckWorkshopTab();

	*a_params.retVal = result;
}

void Pipboy_Ready::Call(const Params& a_params)
{
	REX::DEBUG("Pipboy_Ready called");
	PArroyo_Menus::PipboyTabs::RegisterForInput(true);
	PArroyo_Menus::PipboyTabs::UpdateMenus(a_params.movie->asMovieRoot.get());
}

void Debug_ActionScript::Call(const Params& a_params)
{
	std::string s = std::format("Type is not registered for debug: {}", (std::int32_t) a_params.args[0].GetType());
	switch (a_params.args[0].GetType()) {
		case Scaleform::GFx::Value::ValueType::kUInt:
			s = std::to_string(a_params.args[0].GetUInt());
			break;
		case Scaleform::GFx::Value::ValueType::kInt:
			s = std::to_string(a_params.args[0].GetInt());
			break;
		case Scaleform::GFx::Value::ValueType::kBoolean:
			s = std::to_string(a_params.args[0].GetBoolean());
			break;
		case Scaleform::GFx::Value::ValueType::kString:
			s = a_params.args[0].GetString();
			break;
		default:
			break;
	}

	REX::DEBUG(std::format("Debug_ActionScript: {}", s).c_str());
}

void Pipboy_UpdateItemCardsOnSection::Call(const Params& a_params)
{
	const std::uint32_t section = a_params.args[0].GetUInt();
	// Or GetUInt() ?
	LOG_TO_CONSOLE(std::format("Pipboy_UpdateItemCardsOnSection type: {}", section).c_str());
	PArroyo_Menus::PipboyTabs::UpdateItemCardsOnSection(section);
}

void HUD_Ready::Call(const Params& a_params)
{
	auto movie = a_params.movie;
	if (movie)
		Shared::HUD::UpdateMenus(movie->asMovieRoot);
}

void HUD_GetCurrentCondition::Call(const Params& a_params)
{
	const PArroyo::ItemDegradation::WeaponConditionData condition = PArroyo::ItemDegradation::WeaponConditionData(RE::PlayerCharacter::GetSingleton());
	//const float result = condition.extraData ? condition.extraData->GetHealthPerc() : -1.0f;
	const float result = condition.extraData->GetHealthPerc();
	Scaleform::GFx::Value returnValue = result;
	a_params.retVal->SetElement(0, returnValue);
}

void Pipboy_CheckForcedLevelUp::Call(const Params& a_params)
{
	const bool result = PArroyo_Menus::LevelUpMenu::GetForcedLevelUp();

	*a_params.retVal = result;
}

void WaitForRepairMenu()
{}

void Pipboy_AddCND_ForItemCard::Call(const Params& a_params)
{
	Scaleform::GFx::Value handleIDValue;
	float returnValue = -1.0f;
	std::int32_t index = a_params.args[0].GetInt();
	const RE::BGSInventoryItem* currentHoveredItem = InventoryUtils::GetInventoryItemByIndex(index);
	if (!currentHoveredItem) {
		*a_params.retVal = returnValue;
		return;
	}

	switch (currentHoveredItem->object->GetFormType())
	{
	case RE::ENUM_FORM_ID::kWEAP:
		returnValue = PArroyo::ItemDegradation::WeaponConditionData(RE::PlayerCharacter::GetSingleton(), currentHoveredItem->object, currentHoveredItem->stackData->extra.get()).extraData->GetHealthPerc();
		break;
	case RE::ENUM_FORM_ID::kARMO:
		returnValue = PArroyo::ItemDegradation::ArmorConditionData(RE::PlayerCharacter::GetSingleton(), currentHoveredItem->object, currentHoveredItem->stackData->extra.get()).extraData->GetHealthPerc();
		break;
	default:
		break;
	}

	REX::DEBUG(std::format("AddCND_ForItemCard - Called. Return Value: {}", returnValue).c_str());
	*a_params.retVal = returnValue;

	// a_params.args.getu
	// const PArroyo::ItemDegradation::WeaponConditionData condition = PArroyo::ItemDegradation::WeaponConditionData(RE::PlayerCharacter::GetSingleton());
}

void Workbench_AddCND_ForItemCard::Call(const Params& a_params)
{
	auto examineMenu = RE::UI::GetSingleton()->GetMenu<RE::ExamineMenu>();
	
	if (!examineMenu) {
		*a_params.retVal = -1.0f;
		return;
	}

	const RE::BGSInventoryItem* currentHoveredItem;
	if (!examineMenu->workbenchRef || !examineMenu->workbenchRef.get()) {
		currentHoveredItem = InventoryUtils::GetInventoryItemByHandleID(examineMenu->modItem.id);
		
	}
	else {
		currentHoveredItem = InventoryUtils::GetInventoryItemWorkbenchByIndex();
	}



	Scaleform::GFx::Value handleIDValue;
	float returnValue = -1.0f;
	if (!currentHoveredItem) {
		*a_params.retVal = returnValue;
		return;
	}
	switch (currentHoveredItem->object->GetFormType())
	{
	case RE::ENUM_FORM_ID::kWEAP:
		returnValue = PArroyo::ItemDegradation::WeaponConditionData(RE::PlayerCharacter::GetSingleton(), currentHoveredItem->object, currentHoveredItem->stackData->extra.get()).extraData->GetHealthPerc();
		break;
	case RE::ENUM_FORM_ID::kARMO:
		returnValue = PArroyo::ItemDegradation::ArmorConditionData(RE::PlayerCharacter::GetSingleton(), currentHoveredItem->object, currentHoveredItem->stackData->extra.get()).extraData->GetHealthPerc();
		break;
	default:
		break;
	}

	REX::DEBUG(std::format("WorkbenchAddCND_ForItemCard - Called. Return Value: {}", returnValue).c_str());
	*a_params.retVal = returnValue;
}


void PipboyInventory_Ready::Call(const Params& a_params)
{
	REX::DEBUG("AS3, PipboyInventoryRead - Called.");
}

void Pipboy_IsRepairMenuOpen::Call(const Params& a_params)
{
	auto ui = RE::UI::GetSingleton();
	RE::BSFixedString repairMenuStr("RepairMenu");

	*a_params.retVal = ui ? RE::UI::GetSingleton()->GetMenuOpen(repairMenuStr) : false;
}

void WorkbenchRepair_CanRepairSelectedCustom::Call(const Params& a_params)
{
	// std::int32_t index = a_params.args[0].GetInt();
	bool returnValue = false;

	auto examineMenu = RE::UI::GetSingleton()->GetMenu<RE::ExamineMenu>();
	//Scaleform::GFx::Value selected;
	//examineMenu->itemSelectList.GetElement(examineMenu->GetSelectedIndex(), &selected);



	const RE::BGSInventoryItem* currentHoveredItem = InventoryUtils::GetInventoryItemWorkbenchByIndex();
	if (!currentHoveredItem) {
		*a_params.retVal = returnValue;
		return;
	}
	switch (currentHoveredItem->object->GetFormType())
	{
	case RE::ENUM_FORM_ID::kWEAP:
		returnValue = true;
		break;
	case RE::ENUM_FORM_ID::kARMO:
		returnValue = true;
		break;
	default:
		break;
	}

	RE::BSFixedString menuName = "PowerArmorModMenu"sv;

	if (RE::UI::GetSingleton()->GetMenuOpen(menuName)) {
		returnValue = false;
	}

	if (returnValue) {
		//auto choiceData = examineMenu->QCurrentModChoiceData();
		//if (!choiceData || !choiceData->recipe || !choiceData->recipe->requiredItems) {
		//	returnValue = false;
		//}
	}

	REX::DEBUG(std::format("WorkbenchRepair_CanRepairSelectedCustom - Called. Return Value: {}", returnValue ? 1 : 0).c_str());
	*a_params.retVal = returnValue;

}


void ScaleformVisitMembersFunction(const char* chars, const Scaleform::GFx::Value& valueVisiting)
{
	REX::DEBUG(std::format("ScaleformVisitMembersFunction Value - chars: '{}', valueType: {}", chars, (std::int32_t) valueVisiting.GetType()).c_str());
}

void WorkbenchRepair_RepairSelectedCustom::Call(const Params& a_params)
{
	auto examineMenu = RE::UI::GetSingleton()->GetMenu<RE::ExamineMenu>();
	PArroyo_Menus::Workbench_Additions::currentIndex = examineMenu->GetSelectedIndex();
	examineMenu->repairing = true;
	examineMenu->TryCreate();
	
	/*
	//std::int32_t index = a_params.args[0].GetInt();
	bool returnValue = false;
	const RE::BGSInventoryItem* currentHoveredItem = InventoryUtils::GetInventoryItemWorkbenchByIndex();


	REX::DEBUG("WorkbenchRepair_RepairSelectedCustom - Called.");

	if (!currentHoveredItem) {
		*a_params.retVal = returnValue;
		return;
	}

	auto dataHandler = RE::TESDataHandler::GetSingleton();
	*/
	// auto& cobjs = dataHandler->GetFormArray<RE::BGSConstructibleObject>();



	// RE::BSTArray<RE::BSTTuple<RE::TESForm*, RE::BGSTypedFormValuePair::SharedVal>>* itemsNeeded = nullptr;
	/*
	for (const RE::BGSConstructibleObject* craftableObject : cobjs) {
		if (!craftableObject || !craftableObject->requiredItems)
			continue;


		

		REX::DEBUG("WorkbenchRepair_RepairSelectedCustom - COBJS: CreatedItem FormID: '{}', Item: '{}'", craftableObject->GetCreatedItem()->GetFormID(), currentHoveredItem->object->GetFormID());

		if (craftableObject->GetCreatedItem()->GetFormID() != currentHoveredItem->object->GetFormID())
			continue;

		itemsNeeded = craftableObject->requiredItems;
		REX::DEBUG("WorkbenchRepair_RepairSelectedCustom - Found items needed from COBJ!");
		for (std::uint32_t i = 0; i < itemsNeeded->size(); i++)
		{
			auto& item = itemsNeeded->at(i);
			REX::DEBUG(std::format("WorkbenchRepair_RepairSelectedCustom - Item from COBJ: '{}', count: {}", item.first->GetFormEditorID(), item.second.i).c_str());
		}
		break;

	}
	*/
	/*
	auto player = RE::PlayerCharacter::GetSingleton();

	examineMenu->BuildWeaponScrappingArray();
	bool bPlayerHasEnough = true;

	RE::BSTArray<RE::BSTTuple<RE::TESForm*, RE::BGSTypedFormValuePair::SharedVal>> failureData;
	RE::BSTHashMap<RE::TESBoundObject*, std::uint32_t> availableComponents;

	for (std::uint32_t i = 0; i < examineMenu->scrappingArray.size(); i++)
	{
		auto& item = examineMenu->scrappingArray.at(i);

		std::uint32_t componentCount = item.second;
		RE::TESBoundObject* componentNeeded = item.first;

		std::uint32_t playerCountForComponent = player->inventoryList->GetItemCount(componentNeeded);
		if (playerCountForComponent < componentCount) {
			bPlayerHasEnough = false;
			// break;
		}

		auto a = RE::BGSTypedFormValuePair::SharedVal();
		a.i = componentCount;
		a.f = componentCount;

		RE::BSTTuple<RE::TESBoundObject*, std::uint32_t> insertObject = RE::BSTTuple<RE::TESBoundObject*, std::uint32_t>(componentNeeded, componentCount);
		availableComponents.emplace(insertObject);

		failureData.push_back(RE::BSTTuple<RE::TESForm*, RE::BGSTypedFormValuePair::SharedVal>(componentNeeded, a));
		REX::DEBUG(std::format("WorkbenchRepair_RepairSelectedCustom - Item from Scrapping Array: '{}', count: {}, player has {}", componentNeeded->GetFormEditorID(), componentCount, playerCountForComponent).c_str());
	}

	if (bPlayerHasEnough) {
		RE::ExamineConfirmMenu::InitDataRepairFailure* repairFailureMessageData = new RE::ExamineConfirmMenu::InitDataRepairFailure(&failureData);
		repairFailureMessageData->hasCancelButton = true;
		repairFailureMessageData->buttonLabel = "Repair";
		// repairFailureMessageData.confirmType = RE::ExamineConfirmMenu::CONFIRM_TYPE::kSimple;
		repairFailureMessageData->confirmQuestion = "Workbench repair selected?";
		repairFailureMessageData->availableComponents = availableComponents;
		// RE::ExamineConfirmMenu::InitData repairMessageData = RE::ExamineConfirmMenu::("Repair selected using workbench?", "Repair", RE::ExamineConfirmMenu::CONFIRM_TYPE::kSimple);
		examineMenu->ShowConfirmMenu(repairFailureMessageData, new CustomRepairAcceptCallback(examineMenu.get()));
	}
	else {
		RE::ExamineConfirmMenu::InitDataRepairFailure* repairFailureMessageData = new RE::ExamineConfirmMenu::InitDataRepairFailure(&failureData);
		repairFailureMessageData->availableComponents = availableComponents;
		//repairFailureMessageData->hasCancelButton = false;
		//repairFailureMessageData->buttonLabel = "Repair";
		// repairFailureMessageData.confirmType = RE::ExamineConfirmMenu::CONFIRM_TYPE::kSimple;
		//repairFailureMessageData->confirmQuestion = "Workbench repair selected?";
		examineMenu->ShowConfirmMenu(repairFailureMessageData, new RE::RepairFailureCallback(examineMenu.get()));
	}
	*/
	// examineMenu->RepairSelectedItem.GetElement(examineMenu->GetSelectedIndex());
	

	
	
	//examineMenu->requirementsList.VisitMembers(ScaleformVisitMembersFunction);

	
	// REX::DEBUG(std::format("WorkbenchRepair_RepairSelectedCustom - RequirementList type is: {}", (std::int32_t) examineMenu->requirementsList..GetType()).c_str());
	/*
	for (std::uint32_t i = 0; i < examineMenu->requirementsList.GetArraySize(); i++) {
		Scaleform::GFx::Value req;
		examineMenu->requirementsList.GetElement(i, &req);
		REX::DEBUG(std::format("WorkbenchRepair_RepairSelectedCustom - RequirementList: requirement type is: {}", (std::int32_t)req.GetType()).c_str());
	}
	*/
	/*
	REX::DEBUG(std::format("Size of soundArray: {}", examineMenu->queuedSoundArray.size()).c_str());
	for (std::uint32_t i = 0; i < examineMenu->queuedSoundArray.size(); i++) {
		RE::BGSComponent* item = examineMenu->queuedSoundArray.at(i);
		REX::DEBUG(std::format("WorkbenchRepair_RepairSelectedCustom - SOUNDARRAY: requirement '{}' scalar is: {}", item->scrapItem->GetFormEditorID(), item->modScrapScalar->GetValue()).c_str());
	}
	*/
	/*

	switch (currentHoveredItem->object->GetFormType())
	{
	case RE::ENUM_FORM_ID::kWEAP: {
		auto weaponInfo = PArroyo::ItemDegradation::WeaponConditionData(RE::PlayerCharacter::GetSingleton(), currentHoveredItem->object, currentHoveredItem->stackData->extra.get());
		auto weaponObject = static_cast<RE::TESObjectWEAP*>(weaponInfo.Form);
		//weaponInfo.extraData->GetByType(RE::EXTRA_DATA_TYPE::r)
		break;
	}
	case RE::ENUM_FORM_ID::kARMO:
		returnValue = true;
		break;
	default:
		break;
	}

	RE::BSFixedString menuName = "PowerArmorModMenu"sv;

	if (RE::UI::GetSingleton()->GetMenuOpen(menuName)) {
		returnValue = false;
	}


	if (!returnValue) {
		REX::DEBUG(std::format("WorkbenchRepair_RepairSelectedCustom - Power Armor. Not supposed to work.").c_str());
		return;
	}

	*/
	


	// @TODO
	// a_params.movie->asMovieRoot->Invoke("root.BaseInstance.vanilaRepair", nullptr, nullptr, 0);

	// RE::BSFixedString menuNameConfirm = "ExamineConfirmMenu"sv;
	//auto confirmMenu = RE::UI::GetSingleton()->GetMenu(menuNameConfirm);
	/*
	RE::UIMessageQueue* uiMessageQueue = RE::UIMessageQueue::GetSingleton();
	if (RE::UI* ui = RE::UI::GetSingleton())
	{
		if (ui->menuMap.contains("ExamineConfirmMenu"))
		{
			uiMessageQueue->AddMessage("ExamineConfirmMenu", RE::UI_MESSAGE_TYPE::kShow);
		}
	}
	*/
	// RE::SendHUDMessage::ShowHUDMessage()
}

bool CanRepairCustom(const RE::BGSInventoryItem* currentHoveredItem)
{
	float returnValue = -1.0f;

	switch (currentHoveredItem->object->GetFormType())
	{
	case RE::ENUM_FORM_ID::kWEAP:
		returnValue = PArroyo::ItemDegradation::WeaponConditionData(RE::PlayerCharacter::GetSingleton(), currentHoveredItem->object, currentHoveredItem->stackData->extra.get()).extraData->GetHealthPerc();
		break;
	case RE::ENUM_FORM_ID::kARMO:
		returnValue = PArroyo::ItemDegradation::ArmorConditionData(RE::PlayerCharacter::GetSingleton(), currentHoveredItem->object, currentHoveredItem->stackData->extra.get()).extraData->GetHealthPerc();
		break;
	default:
		break;
	}

	return returnValue >= 0 && returnValue < 1.0f;
}

void WorkbenchRepair_CanRepairCustom::Call(const Params& a_params) {
	const RE::BGSInventoryItem* currentHoveredItem = InventoryUtils::GetInventoryItemWorkbenchByIndex();
	if (!currentHoveredItem) {
		*a_params.retVal = false;
		return;
	}
	bool returnValue = CanRepairCustom(currentHoveredItem);

	REX::DEBUG(std::format("WorkbenchRepair_CanRepairCustom - Called.").c_str());
	*a_params.retVal = returnValue;
}

void CustomRepairFailureCallback::OnAccept()
{
	REX::DEBUG("CustomRepairFailureCallback Accept - Called.");
}

void CustomRepairAcceptCallback::OnAccept()
{
	REX::DEBUG("CustomRepairAcceptCallback Accept - Called.");
}

void WorkbenchRepair_RepairSelected_RepairKit::Call(const Params& a_params)
{
	using namespace RE;
	if (a_params.retVal)
	{
		Scaleform::Ptr<RE::ExamineMenu> examineMenu = UI::GetSingleton()->GetMenu<RE::ExamineMenu>();

		std::uint32_t selectedIndex = examineMenu->GetSelectedIndex();
		if (!examineMenu->invInterface.entriesInvalid && (selectedIndex & 0x80000000) == 0 && selectedIndex < examineMenu->invInterface.stackedEntries.size())
		{
			InventoryUserUIInterfaceEntry* inventoryUUIEntry = (examineMenu->invInterface.stackedEntries.data() + selectedIndex);
			const BGSInventoryItem* inventoryItem = BGSInventoryInterface::GetSingleton()->RequestInventoryItem(inventoryUUIEntry->invHandle.id);

			if (inventoryItem)
			{
				PlayerCharacter* playerCharacter = PlayerCharacter::GetSingleton();

				if (!playerCharacter->IsGodMode())
				{
					// TESObjectMISC* repairKit = TESDataHandler::GetSingleton()->LookupForm<TESObjectMISC>(0x1D59F7, "FalloutCascadia.esm");

					auto repairKit = RE::TESDataHandler::GetSingleton()->LookupForm<RE::TESObjectMISC>(0x002E4F, CURRENT_ESP);
					TESObjectREFR::RemoveItemData removeItemData(repairKit, 1);
					playerCharacter->RemoveItem(removeItemData);

					PipboyDataManager* pipboyDataManager = PipboyDataManager::GetSingleton();
					pipboyDataManager->inventoryData.RepopulateItemCardOnSection(RE::ENUM_FORM_ID::kMISC);
				}

				switch (examineMenu->GetCurrentObj()->formType.get())
				{
				case ENUM_FORM_ID::kARMO:
				case ENUM_FORM_ID::kWEAP:
					BGSInventoryItem::Stack* stack = inventoryItem->GetStackByID(inventoryUUIEntry->stackIndex.at(0));
					if (stack)
					{
						// TODO - Check for 'Master Mechanic', if player has the perk, repair amount is set to 100% instead of scaling with repair skill.
						float repairSkill = playerCharacter->GetActorValue(*PA_Skills.Repair);
						float repairAmount = Shared::repairKitBaseRepair->GetValue() + Shared::repairKitSkillMutliplier->GetValue() * (repairSkill / 100.0f); // Repair amount scales from 20% at 0 repair skill to 50% at 100 repair skill.

						if (stack->extra->GetHealthPerc() + repairAmount > 1.0f)
						{
							stack->extra->SetHealthPerc(1.0f);
						}
						else
						{
							stack->extra->SetHealthPerc(stack->extra->GetHealthPerc() + repairAmount);
						}

						if (inventoryItem) {
							std::string msg = std::format("{}'s new condition: {}%", inventoryItem->GetDisplayFullName(inventoryUUIEntry->stackIndex.at(0)), (int) (stack->extra->GetHealthPerc() * 100));
							RE::SendHUDMessage::ShowHUDMessage(msg.c_str(), nullptr, true, true);
						}

						BGSInventoryItem::CheckStackIDFunctor compareFunction(inventoryUUIEntry->stackIndex.at(0));
						BGSInventoryItem::SetHealthFunctor writeFunction(stack->extra->GetHealthPerc());
						writeFunction.shouldSplitStacks = 0x101;

						playerCharacter->FindAndWriteStackDataForInventoryItem(examineMenu->GetCurrentObj(), compareFunction, writeFunction);

						examineMenu->UpdateOptimizedAutoBuildInv();
						selectedIndex = examineMenu->GetSelectedIndex();
						examineMenu->UpdateItemList(selectedIndex);
						examineMenu->uiMovie->asMovieRoot->Invoke("root.BaseInstance.UpdateButtons", nullptr, nullptr, 0);

						// BGSSoundDescriptorForm* craftingSound = TESDataHandler::GetSingleton()->LookupForm<BGSSoundDescriptorForm>(0x1F252D, "Fallout4.esm");
						
						
						RE::UIUtils::PlayMenuSound("OBJLunchboxKidsRobotBuild");

						PipboyDataManager* pipboyDataManager = PipboyDataManager::GetSingleton();
						pipboyDataManager->inventoryData.RepopulateItemCardOnSection(examineMenu->GetCurrentObj()->formType.get());
					}
					break;
				}
			}
		}
	}
}

void WorkbenchRepair_GetRepairKitCount::Call(const Params& a_params)
{
	//RE::TESObjectMISC* repairKit = RE::TESDataHandler::GetSingleton()->LookupForm<RE::TESObjectMISC>(0x1D59F7, "FalloutCascadia.esm");
	// playerCharacter->RemoveItem(removeItemData);
	std::uint32_t c;
	//c = RE::PlayerCharacter::GetSingleton()->GetItemCount();
	auto repairKit = RE::TESDataHandler::GetSingleton()->LookupForm<RE::TESObjectMISC>(0x002E4F, CURRENT_ESP);
	if (!repairKit) {
		*a_params.retVal = 0;
		return;
	}

	const bool bSuccess = RE::PlayerCharacter::GetSingleton()->GetItemCount(c, repairKit, false);
	*a_params.retVal = bSuccess ? c : 0;
}

void WorkbenchRepair_NoRepairNeeded::Call(const Params& a_params)
{
	RE::GameSettingCollection* gameSettingCollection = RE::GameSettingCollection::GetSingleton();
	RE::SendHUDMessage::ShowHUDMessage(gameSettingCollection->GetSetting("sNoNeedToRepairMessage")->GetString().data(), nullptr, true, true);
}

void WorkbenchRepair_NoRepairKits::Call(const Params& a_params)
{
	RE::GameSettingCollection* gameSettingCollection = RE::GameSettingCollection::GetSingleton();
	RE::SendHUDMessage::ShowHUDMessage("You don't have any repair kits!", nullptr, true, true);
}

void Workbench_IsWeaponOrArmor::Call(const Params& a_params)
{
	bool returnValue = false;

	const RE::BGSInventoryItem* currentHoveredItem = InventoryUtils::GetInventoryItemWorkbenchByIndex();
	if (!currentHoveredItem) {
		*a_params.retVal = returnValue;
		return;
	}
	switch (currentHoveredItem->object->GetFormType())
	{
	case RE::ENUM_FORM_ID::kWEAP:
		returnValue = true;
		break;
	case RE::ENUM_FORM_ID::kARMO:
		returnValue = false;
		break;
	default:
		break;
	}

	*a_params.retVal = returnValue;
}

void Workbench_ScrapAllJunk::Call(const Params& a_params)
{
	Scaleform::Ptr<RE::ExamineMenu> examineMenu = RE::UI::GetSingleton()->GetMenu<RE::ExamineMenu>();
	PArroyo_Menus::Workbench_Additions::bIsScrappingAllJunk = true;
	//examineMenu->BuildWeaponScrappingArray();
	examineMenu->uiMovie->asMovieRoot->Invoke("root.BaseInstance.scrapAllJunkCallbackFromCPP", nullptr, nullptr, 0);
	

}

void Workbench_HasAnyJunk::Call(const Params& a_params)
{
	auto player = RE::PlayerCharacter::GetSingleton();

	bool bHasJunk = false;

	for (std::uint32_t i = 0; i < player->inventoryList->data.size(); i++)
	{
		RE::BGSInventoryItem inventoryItem = player->inventoryList->data.at(i);

		if (!inventoryItem.object || !Shared::IsJunkItem(inventoryItem.object) || inventoryItem.IsQuestObject(0))
			continue;

		auto baseComp = Shared::GetBaseComponentFromForm(inventoryItem.object);
		if (!baseComp || !baseComp->scrapItem || baseComp->scrapItem->GetFormID() == inventoryItem.object->GetFormID())
			continue;
		

		switch (inventoryItem.object->GetFormType())
		{
		case RE::ENUM_FORM_ID::kWEAP: {
			if (static_cast<RE::TESObjectWEAP*>(inventoryItem.object)->HasKeyword(Shared::notScrappableKeyword)) {
				continue;
			}
			break;
		}
		case RE::ENUM_FORM_ID::kARMO: {
			if (static_cast<RE::TESObjectARMO*>(inventoryItem.object)->HasKeyword(Shared::notScrappableKeyword)) {
				continue;
			}
			break;
		}
		default:
			break;
		}

		bHasJunk = true;
		break;
	}

	*a_params.retVal = bHasJunk;
}

void WorkbenchRepair_NoJunk::Call(const Params& a_params)
{
	RE::GameSettingCollection* gameSettingCollection = RE::GameSettingCollection::GetSingleton();
	RE::SendHUDMessage::ShowHUDMessage("You don't have any junk to scrap!", nullptr, true, true);
}

void OnEscapePress::Call(const Params& a_params)
{
	REX::DEBUG("this.BGSCodeObj.OnEscapePress");
	Scaleform::Ptr<RE::ExamineMenu> examineMenu = RE::UI::GetSingleton()->GetMenu<RE::ExamineMenu>();
	if (examineMenu)
	{
		REX::DEBUG("Examine menu!");
		examineMenu->repairing = false;
		
	}
	PArroyo_Menus::Workbench_Additions::bIsScrappingAllJunk = false;
}

void Workbench_HasAnyJunkExamine::Call(const Params& a_params)
{
	if (!PArroyo_Menus::Workbench_Additions::bIsScrappingAllJunk) {
		*a_params.retVal = false;
		return;
	}

	auto player = RE::PlayerCharacter::GetSingleton();

	bool bHasJunk = false;

	for (std::uint32_t i = 0; i < player->inventoryList->data.size(); i++)
	{
		RE::BGSInventoryItem inventoryItem = player->inventoryList->data.at(i);

		if (!inventoryItem.object || !Shared::IsJunkItem(inventoryItem.object) || inventoryItem.IsQuestObject(0))
			continue;

		auto baseComp = Shared::GetBaseComponentFromForm(inventoryItem.object);
		if (!baseComp || !baseComp->scrapItem || baseComp->scrapItem->GetFormID() == inventoryItem.object->GetFormID())
			continue;

		switch (inventoryItem.object->GetFormType())
		{
		case RE::ENUM_FORM_ID::kWEAP: {
			if (static_cast<RE::TESObjectWEAP*>(inventoryItem.object)->HasKeyword(Shared::notScrappableKeyword)) {
				continue;
			}
			break;
		}
		case RE::ENUM_FORM_ID::kARMO: {
			if (static_cast<RE::TESObjectARMO*>(inventoryItem.object)->HasKeyword(Shared::notScrappableKeyword)) {
				continue;
			}
			break;
		}
		default:
			break;
		}


		bHasJunk = true;
		break;
	}

	*a_params.retVal = bHasJunk;
}

void Workbench_CompleteScrapAllJunk::Call(const Params& a_params)
{
	/*
	auto player = RE::PlayerCharacter::GetSingleton();
	RE::UIMessageQueue* uiMessageQueue = RE::UIMessageQueue::GetSingleton();
	uiMessageQueue->AddMessage("ExamineConfirmMenu", RE::UI_MESSAGE_TYPE::kHide);

	Scaleform::Ptr<RE::ExamineMenu> a_this = RE::UI::GetSingleton()->GetMenu<RE::ExamineMenu>();


	for (auto it = a_this->scrappingArray.begin(); it != a_this->scrappingArray.end(); ++it) {
		player->inventoryList->AddItem2(it->first, it->second, );
	}

	RE::SendHUDMessage::ShowHUDMessage("All junk items were scrapped!", "OBJLunchboxKidsRobotBuild", false, true);

	PArroyo_Menus::Workbench_Additions::bIsScrappingAllJunk = false;
	a_this->uiMovie->asMovieRoot->Invoke("root.BaseInstance.UpdateButtons", nullptr, nullptr, 0);
	*/
}

void Workbench_IsInAllJunk::Call(const Params& a_params)
{
	*a_params.retVal = PArroyo_Menus::Workbench_Additions::bIsScrappingAllJunk;
}
