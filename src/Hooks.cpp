#include "Hooks.h"
#include <REL/Trampoline.h>
#include "detourXS/detourxs.h"
#include <REL/Relocation.h>
#include <RE/IDs.h>
#include <cstdint>
#include <cstdarg>
#include <RE/B/BGSInventoryItem.h>
#include <RE/B/BGSInventoryList.h>
#include <RE/T/TESBoundObject.h>
#include <RE/B/BSRandom.h>
#include <RE/E/ENUM_FORM_ID.h>
#include "Shared.h"
#include <RE/T/TESAmmo.h>
#include <RE/T/TESForm.h>
#include <RE/T/TESObjectARMO.h>
#include <RE/T/TESObjectWEAP.h>
#include <RE/W/WEAPON_TYPE.h>
#include <REX/LOG.h>
#include <RE/A/Actor.h>
#include <RE/A/ActorValueInfo.h>
#include <RE/E/ExtraDataList.h>
#include <RE/E/EXTRA_DATA_TYPE.h>
#include <RE/P/PlayerCharacter.h>
#include <RE/A/AlchemyItem.h>
#include <RE/B/BGSEquipIndex.h>
#include <RE/B/BGSObjectInstance.h>
#include <RE/B/BSCoreTypes.h>
#include <RE/T/TESObjectREFR.h>
#include "ItemDegradation.h"
#include <RE/E/EquippedItem.h>
#include <RE/P/PipboyDataManager.h>
#include <RE/W/WEAPON_FLAGS.h>
#include <RE/A/ActorEquipManager.h>
#include <RE/S/SendHUDMessage.h>
#include <RE/S/Setting.h>
#include <RE/B/BGSBodyPart.h>
#include <RE/B/BGSTypedFormValuePair.h>
#include <RE/B/BSTArray.h>
#include <RE/B/BSTTuple.h>
#include <RE/A/ActorUtils.h>
#include <RE/E/ExtraHealth.h>
#include <Windows.h>
#include <cmath>
#include <cstdio>
#include <format>
#include <RE/B/BGSEquipSlot.h>
#include <RE/B/BGSInventoryInterface.h>
#include <RE/B/BSFixedString.h>
#include <RE/E/ExamineMenu.h>
#include <RE/I/InventoryUserUIInterfaceEntry.h>
#include <RE/P/PipboyManager.h>
#include <RE/T/TESFullName.h>
#include <RE/U/UI.h>
#include <RE/W/WorkbenchMenuBase.h>
#include "debugLog.h"
#include "GameForms.h"
#include <RE/B/BaseFormComponent.h>
#include <RE/B/BGSSoundDescriptorForm.h>
#include <RE/B/BSTHashMap.h>
#include <RE/E/ExamineConfirmMenu.h>
#include <RE/P/PowerArmorModMenu.h>
#include <RE/R/RepairFailureCallback.h>
#include <RE/T/TESDataHandler.h>
#include <Scaleform/P/Ptr.h>
#include "Menus/Workbench_Additions.h"

namespace PArroyo {
	namespace Hooks {
		
		DetourXS hook_AddItem;
		typedef void(AddItemSig)(RE::BGSInventoryList* a_this, RE::TESBoundObject* a_boundObject, const RE::BGSInventoryItem::Stack* a_stack, std::uint32_t* a_oldCount, std::uint32_t* a_newCount);
		REL::Relocation<AddItemSig> AddItem_Original;
		void Hook_BGSInventoryListAddItem(RE::BGSInventoryList* a_this, RE::TESBoundObject* a_boundObject, const RE::BGSInventoryItem::Stack* a_stack, std::uint32_t* a_oldCount, std::uint32_t* a_newCount) {
			if (!a_boundObject || (a_boundObject->GetFormType() != RE::ENUM_FORM_ID::kWEAP && a_boundObject->GetFormType() != RE::ENUM_FORM_ID::kARMO)) {
				AddItem_Original(a_this, a_boundObject, a_stack, a_oldCount, a_newCount);
				return;
			}

			std::uint32_t iterCount = 0;
			for (const RE::BGSInventoryItem::Stack* traverse = a_stack; traverse; traverse->nextStack)
			{
				if (!traverse || !traverse->extra)
					break;
				bool willHaveHealth = false;
				if (a_boundObject->GetFormType() == RE::ENUM_FORM_ID::kWEAP)
				{
					RE::TESObjectWEAP* tempREFR = static_cast<RE::TESObjectWEAP*>(a_boundObject);
					if (tempREFR->weaponData.type == RE::WEAPON_TYPE::kGrenade || tempREFR->weaponData.type == RE::WEAPON_TYPE::kMine)
					{
						REX::DEBUG("BGSInventoryList::AddItem: REFR grenade/mine weapon type.");
						break;
					}
					willHaveHealth = true;

					if (traverse->extra->GetHealthPerc() < 0.0f) {
						if (tempREFR->IsMeleeWeapon())
							LOG_TO_CONSOLE("Added melee first time.\n");
						LOG_TO_CONSOLE(std::format("Added weapon first time. type: {}\n", (std::int32_t)tempREFR->weaponData.type.get()).c_str());
						const float cond = RE::BSRandom::Float(0.45f, 0.85f);
						traverse->extra->SetHealthPerc(cond);

						WPNUtilities::CalculateUpdatedRateOfFireValue(tempREFR, &tempREFR->weaponData, cond);
					}


					// Set to '1.0' when initializing if the 'noDegradation' keyword is on the object.
					if (tempREFR->HasKeyword(Shared::noDegradation))
					{
						REX::DEBUG("'DC_NoDegradation' keyword found on weapon: {}.", tempREFR->GetFormEditorID());
						break;
					}
				}

				if (a_boundObject->GetFormType() == RE::ENUM_FORM_ID::kARMO)
				{
					RE::TESObjectARMO* tempREFR = static_cast<RE::TESObjectARMO*>(a_boundObject);
					// Set to '1.0' when initializing if the 'noDegradation' keyword is on the object.

					if (tempREFR->armorData.rating == 0 && !tempREFR->armorData.damageTypes)
					{
						break;
					}
					willHaveHealth = true;

					if (tempREFR->HasKeyword(Shared::noDegradation))
					{
						REX::DEBUG("'CAS_NoDegradation' keyword found on armor: {}.", tempREFR->GetFormEditorID());
						break;
					}
				}


				if (a_boundObject->GetFormType() == RE::ENUM_FORM_ID::kCMPO) {
					auto player = RE::PlayerCharacter::GetSingleton();

					auto baseComp = Shared::GetBaseComponentFromForm(a_boundObject);
					std::uint32_t oldCountNew = 0;
					auto removeData = RE::TESObjectREFR::RemoveItemData(a_boundObject, *a_newCount);
					player->RemoveItem(removeData);

					player->inventoryList->AddItem2(baseComp->scrapItem, *a_newCount);
					//AddItem_Original(a_this, baseComp->scrapItem, a_stack, &oldCountNew, a_newCount);
					
				}

				// GetHealthPerc returns -1.0 if it can't find the 'kHealth' type.
				if (willHaveHealth && traverse->extra->GetHealthPerc() < 0.0f) {
					// Set health randomly
					LOG_TO_CONSOLE(std::format("Added health to item '{}' added.\n", a_boundObject->GetFormID()).c_str());
					traverse->extra->SetHealthPerc(RE::BSRandom::Float(0.45f, 0.85f));
					break;
				}


				iterCount++;
				if (iterCount > traverse->count - 1)
					break;

			}

			AddItem_Original(a_this, a_boundObject, a_stack, a_oldCount, a_newCount);
		}

		DetourXS hook_GetEquippedDamageResistance;
		typedef float(GetEquippedDamageResistanceSig)(RE::Actor*, const RE::ActorValueInfo*);
		REL::Relocation<GetEquippedDamageResistanceSig> GetEquippedDamageResistanceOriginal;
		float Hook_GetEquippedDamageResistance(RE::Actor* a_actor, const RE::ActorValueInfo* a_info)
		{
			float retailResistance = GetEquippedDamageResistanceOriginal(a_actor, a_info);
			// Only calculated for the player.
			if (a_actor != RE::PlayerCharacter::GetSingleton())
				return retailResistance;

			RE::BGSInventoryList* inventoryList = a_actor->inventoryList;
			inventoryList->rwLock.lock_read();

			for (RE::BGSInventoryItem& inventoryItem : inventoryList->data) {
				RE::TESObjectARMO* armor = static_cast<RE::TESObjectARMO*>(inventoryItem.object);
				if (inventoryItem.object && armor->formType == RE::ENUM_FORM_ID::kARMO && armor->Protects(a_info, false)) {
					if (!inventoryItem.IsEquipped(0))
						continue;

					RE::ExtraDataList* dataList = inventoryItem.stackData->extra.get();
					if (!dataList->HasType(RE::EXTRA_DATA_TYPE::kHealth))
						continue;

					float currentHealth = dataList->GetHealthPerc();
					if (currentHealth <= 0.0f) {
						// Fully broken. No damage resistance.
						return 0.0f;
					}

					REX::DEBUG("Damage resistance prior to condition modifier: {}", retailResistance);
					const float conditionModifier = 0.66f + min(0.34 * currentHealth / 0.5f, 0.34f);
					retailResistance *= conditionModifier;
					REX::DEBUG("Damage resistance after condition modifier: {}", retailResistance);
				}
			}

			inventoryList->rwLock.unlock_read();
			return retailResistance;
		}

		DetourXS hook_CombatFormulasCalcWeaponDamage;
		typedef float(CombatFormulasCalcWeaponDamageSig)(const RE::TESForm*, const RE::TESObjectWEAP::InstanceData*, const RE::TESAmmo*, float, float);
		REL::Relocation<CombatFormulasCalcWeaponDamageSig> CombatFormulasCalcWeaponDamageOriginal;
		float Hook_CombatFormulasCalcWeaponDamage(const RE::TESForm* a_actorForm, const RE::TESObjectWEAP::InstanceData* a_weapon, const RE::TESAmmo* a_ammo, float a_condition, float a_damageMultiplier)
		{
			float retailDamage = CombatFormulasCalcWeaponDamageOriginal(a_actorForm, a_weapon, a_ammo, a_condition, a_damageMultiplier);

			if (a_condition != -1.0f || a_condition < 0.75f)
				retailDamage = retailDamage * (0.5f + min((0.5f * a_condition) / 0.75f, 0.5f));

			return retailDamage;
		}

		DetourXS hook_TESObjectWeaponFire;
		typedef void(TESObjectWeaponFireSig)(const RE::BGSObjectInstanceT<RE::TESObjectWEAP>*, RE::TESObjectREFR*, RE::BGSEquipIndex, RE::TESAmmo*, RE::AlchemyItem*);
		REL::Relocation<TESObjectWeaponFireSig> TESObjectWeaponFireOriginal;
		void Hook_TESObjectWeaponFire(const RE::BGSObjectInstanceT<RE::TESObjectWEAP>* a_weapon, RE::TESObjectREFR* a_source, RE::BGSEquipIndex a_equipIndex, RE::TESAmmo* a_ammo, RE::AlchemyItem* a_poiso) {

			if (!a_source) {
				TESObjectWeaponFireOriginal(a_weapon, a_source, a_equipIndex, a_ammo, a_poiso);
				return;
			}


			// Only apply Weapon Condition Change on Player.
			RE::PlayerCharacter* playerCharacter = RE::PlayerCharacter::GetSingleton();
			if (a_source != playerCharacter) {
				TESObjectWeaponFireOriginal(a_weapon, a_source, a_equipIndex, a_ammo, a_poiso);
				return;
			}

			if (playerCharacter->IsGodMode() || Shared::noWeaponDegradation) {
				TESObjectWeaponFireOriginal(a_weapon, a_source, a_equipIndex, a_ammo, a_poiso);
				return;
			}



			// LOG_TO_CONSOLE("Shot weapon.\n");

			//if (weaponObject)
				//weaponObject->weaponData.attackDelaySec = WPNUtilities::CalculateUpdatedRateOfFireValue(weaponObject, a_weapon->instanceData);

			RE::TESObjectWEAP* weaponObject = static_cast<RE::TESObjectWEAP*>(a_weapon->object);


			if (weaponObject->weaponData.type == RE::WEAPON_TYPE::kGrenade || weaponObject->weaponData.type == RE::WEAPON_TYPE::kMine
				|| weaponObject->HasKeyword(Shared::noDegradation))
				return;

			// Getting the inventory form of the a_weapon
			RE::BGSInventoryItem* inventoryItem = nullptr;
			int index = 0;
			RE::TESFormID weaponFormID = a_weapon->object->GetFormID();
			for (RE::BGSInventoryItem& item : playerCharacter->inventoryList->data) {
				if (item.object->GetFormID() == weaponFormID) {
					inventoryItem = &item;
					break;
				}
				index++;
			}
			if (!inventoryItem) {
				TESObjectWeaponFireOriginal(a_weapon, a_source, a_equipIndex, a_ammo, a_poiso);
				return;
			}

			RE::TESObjectWEAP::InstanceData* weaponInstanceData = static_cast<RE::TESObjectWEAP::InstanceData*>(a_weapon->instanceData.get());

			RE::EquippedItem& equippedItem = playerCharacter->currentProcess->middleHigh->equippedItems[0];

			RE::TESAmmo* weaponAmmo = weaponInstanceData->ammo;

			double degradationAmount = ItemDegradation::GetDegradationMapping(weaponAmmo);
			if (degradationAmount == 0.01f)
				REX::WARN("TESObjectWEAP::Fire - degradation ammo mapping for ammo type '{}' is not declared in 'AmmoDegradationMap' defaulting to 0.01f", weaponAmmo->GetFormEditorID());

			const std::uint32_t flags = weaponInstanceData->flags.underlying();

			if (flags & (std::uint32_t)RE::WEAPON_FLAGS::kAutomatic)
				degradationAmount *= Shared::fAutomaticWeaponConditionReduction->GetValue();
			else if (flags & (std::uint32_t)RE::WEAPON_FLAGS::kBoltAction)
				degradationAmount *= Shared::fBoltWeaponConditionReduction->GetValue();

			// Linear reduction from 0 - 100, 100 resulting in 20% less damage to weapon.
			// Mapped out. from 0 to 20%
			RE::ActorValueInfo* gunSkill = weaponInstanceData->skill;
			float playerGunSkill = 0.0f;
			if (gunSkill) {
				REX::DEBUG("Weapon '{}' set to skill '{}'", a_weapon->object->GetFormEditorID(), gunSkill->GetFormEditorID());
				playerGunSkill = playerCharacter->GetActorValue(*gunSkill);
			}
			else {
				REX::DEBUG("Weapon '{}' is missing skill value. Change this value in xEdit.", a_weapon->object->GetFormEditorID());
			}
			degradationAmount *= 1.0f - (playerGunSkill / 100.0f * 0.2f);

			RE::ExtraDataList* extraDataList = inventoryItem->stackData->extra.get();

			double currentWeaponHealth = max(extraDataList->GetHealthPerc() - degradationAmount, 0.0f);


			extraDataList->SetHealthPerc(currentWeaponHealth);

			WPNUtilities::CalculateUpdatedRateOfFireValue(weaponObject, weaponInstanceData, currentWeaponHealth);
			TESObjectWeaponFireOriginal(a_weapon, a_source, a_equipIndex, a_ammo, a_poiso);
			const bool didUpdate = WPNUtilities::UpdateHUDCondition(ItemDegradation::WeaponConditionData(weaponObject, extraDataList));


			if (currentWeaponHealth == 0.0f) {
				// Weapon breaks.
				RE::ActorEquipManager::GetSingleton()->UnequipItem(playerCharacter, &equippedItem, false);
				RE::GameSettingCollection* gameSettingCollection = RE::GameSettingCollection::GetSingleton();
				RE::SendHUDMessage::ShowHUDMessage(gameSettingCollection->GetSetting("sWeaponBreak")->GetString().data(), "00DCUIWeaponBreak", true, true);
			}
			RE::PipboyDataManager::GetSingleton()->inventoryData.RepopulateItemCardOnSection(RE::ENUM_FORM_ID::kWEAP);
			if (weaponObject->weaponData.flags.underlying() & (std::uint32_t)RE::WEAPON_FLAGS::kAutomatic) {
				LOG_TO_CONSOLE(std::format("Shot weapon {}. Cond: {}, New FireRate: {}, {}\n", weaponObject->GetFormEditorID(), currentWeaponHealth, weaponInstanceData->attackSeconds, weaponInstanceData->attackDelaySec).c_str());
			}
			else {
				LOG_TO_CONSOLE(std::format("Shot weapon {}. Cond: {}, New FireRate: {}, {}\n", weaponObject->GetFormEditorID(), currentWeaponHealth, weaponInstanceData->attackDelaySec, weaponInstanceData->attackSeconds).c_str());
			}

		}

		DetourXS hook_CombatFormulasCalcTargetedLimbDamage;
		typedef float(CombatFormulasCalcTargetedLimbDamageSig)(RE::Actor*, const RE::BGSBodyPart*, float, RE::BSTArray<RE::BSTTuple<RE::TESForm*, RE::BGSTypedFormValuePair::SharedVal>, RE::BSTArrayHeapAllocator>*);
		REL::Relocation<CombatFormulasCalcTargetedLimbDamageSig> CombatFormulasCalcTargetedLimbDamageOriginal;
		float Hook_CombatFormulasCalcTargetedLimbDamage(RE::Actor* a_actor, const RE::BGSBodyPart* a_bodyPart, float a_physicalDamage, RE::BSTArray<RE::BSTTuple<RE::TESForm*, RE::BGSTypedFormValuePair::SharedVal>, RE::BSTArrayHeapAllocator>* a_damageTypes) {
			float retailDamage = CombatFormulasCalcTargetedLimbDamageOriginal(a_actor, a_bodyPart, a_physicalDamage, a_damageTypes);

			// Only player.
			RE::PlayerCharacter* playerCharacter = RE::PlayerCharacter::GetSingleton();
			if (a_actor != playerCharacter || playerCharacter->IsGodMode() || Shared::noArmorDegradation)
				return retailDamage;

			RE::ActorValueInfo* targetedLimbAV = a_bodyPart->data.actorValue;
			const float DR_ForHitLimb = RE::ActorUtils::GetEquippedArmorDamageResistance(a_actor, targetedLimbAV);

			if (DR_ForHitLimb >= a_physicalDamage) {
				LOG_TO_CONSOLE(std::format("Armor not dmged. DR_ForHitLimb is more than damage. DR_ForHitLimb: {}, dmg: {}\n", DR_ForHitLimb, a_physicalDamage).c_str());
				return retailDamage;
			}

			RE::BGSInventoryList* inventoryList = a_actor->inventoryList;
			inventoryList->rwLock.lock_read();
			for (RE::BGSInventoryItem& inventoryItem : inventoryList->data) {
				if (!inventoryItem.IsEquipped(0))
					continue;

				RE::TESObjectARMO* armor = static_cast<RE::TESObjectARMO*>(inventoryItem.object);
				if (inventoryItem.object && armor->formType == RE::ENUM_FORM_ID::kARMO && armor->Protects(targetedLimbAV, false)) {
					RE::ExtraDataList* armorExtraData = inventoryItem.stackData->extra.get();
					// If armor does not have kHealth, it would return -1.
					double armourHealth = armorExtraData->GetHealthPerc();

					if (armourHealth > 0.0f) {
						REX::DEBUG("Armor condition of '{}' before degradation: {}", armor->GetFormEditorID(), armourHealth);
						const double degradationAmount = (a_physicalDamage - DR_ForHitLimb) / DR_ForHitLimb * Shared::fArmourConditionReductionPerPercentage->GetValue();
						armourHealth = max(armourHealth - degradationAmount, 0.0f);

						if (armourHealth == 0.0f) {
							// Armor breaks.
							RE::BGSObjectInstance* armorInstance = new RE::BGSObjectInstance(armor, &armor->armorData);
							inventoryList->rwLock.unlock_read();
							RE::ActorEquipManager::GetSingleton()->UnequipObject(a_actor, armorInstance, 1, armor->equipSlot, 0, false, false, true, true, nullptr);
							inventoryList->rwLock.lock_read();
							if (a_actor == playerCharacter) {
								RE::SendHUDMessage::ShowHUDMessage("$F4CW_ArmorBroken", "00DCUIWeaponBreak", true, true);
								RE::PipboyDataManager::GetSingleton()->inventoryData.RepopulateItemCardOnSection(RE::ENUM_FORM_ID::kARMO);
							}
							else {
								RE::SendHUDMessage::ShowHUDMessage(std::format("You broke {}'s armor!", a_actor->GetDisplayFullName()).c_str(), "00DCUIWeaponBreak", true, true);
							}
						}

						armorExtraData->SetHealthPerc(armourHealth);
						// PArroyo::ARMOUtilities::UpdateArmorStats(ItemDegradation::ArmorConditionData(a_actor, armor, armorExtraData));

						LOG_TO_CONSOLE(std::format("Armor condition of '{}' after degradation: {}. degradationAmount: {}\n", armor->GetFormEditorID(), armourHealth, degradationAmount).c_str());
						REX::DEBUG("Armor condition of '{}' after degradation: {}. degradationAmount: {}", armor->GetFormEditorID(), armorExtraData->GetHealthPerc(), degradationAmount);
					}
					break;
				}
			}
			inventoryList->rwLock.unlock_read();
			return retailDamage;
		}


		DetourXS hook_SetHealthPerc;
		typedef void(SetHealthPercSig)(RE::ExtraDataList*, float);
		REL::Relocation<SetHealthPercSig> SetHealthPercOriginal;
		// Required to allow the game to store a value of 1.0, by default it would set it to '-1.0'
		// This causes issues in the way we handle initialization of the health data.
		// Here we simply, run the original function, then if 'a_health' was '1.0', we manually set it 
		// again to '1.0', reverting the original functions result of '-1.0'.
		void Hook_ExtraDataListSetHealthPerc(RE::ExtraDataList* a_this, float a_health)
		{
			SetHealthPercOriginal(a_this, a_health);
			if (a_health == 1.0f && a_this->GetHealthPerc() != 1.0f)
			{
				a_this->AddExtra(new RE::ExtraHealth(1.0f));
			}
		}

		DetourXS hook_GetWeaponDisplayRateOfFire;
		typedef float(GetWeaponDisplayRateOfFireSig)(const RE::TESObjectWEAP&, const RE::TESObjectWEAP::InstanceData*);
		REL::Relocation<GetWeaponDisplayRateOfFireSig> GetWeaponDisplayRateOfFireOriginal;
		float Hook_GetWeaponDisplayRateOfFire(const RE::TESObjectWEAP& a_weapon, const RE::TESObjectWEAP::InstanceData* a_data)
		{
			// float original = GetWeaponDisplayRateOfFireOriginal(a_weapon, a_data);

			// LOG_TO_CONSOLE(std::format("Original FireRate: {}, Attack Delay: {}", original, a_data->attackDelaySec).c_str());

			return a_data->attackDelaySec;
		}

		DetourXS hook_GetWeaponDisplayDamage;
		typedef float(GetWeaponDisplayDamageSig)(const RE::BGSObjectInstanceT<RE::TESObjectWEAP>&, const RE::TESAmmo*, float);
		REL::Relocation<GetWeaponDisplayDamageSig> GetWeaponDisplayDamageOriginal;
		float Hook_GetWeaponDisplayDamage(const RE::BGSObjectInstanceT<RE::TESObjectWEAP>& a_weapon, const RE::TESAmmo* a_ammo, float a_condition)
		{
			// RE::TESObjectWEAP::InstanceData* weaponInstanceData = static_cast<RE::TESObjectWEAP::InstanceData*>(a_weapon.instanceData.get());

			float retailDamage = GetWeaponDisplayDamageOriginal(a_weapon, a_ammo, a_condition);

			if (a_condition != -1.0f || a_condition < 0.75f)
				retailDamage = retailDamage * (0.5f + min((0.5f * a_condition) / 0.75f, 0.5f));

			return retailDamage;
		}


		DetourXS hook_ClosedownPipboy;
		typedef void(ClosedownPipboySig)(RE::PipboyManager*);
		REL::Relocation<ClosedownPipboySig> ClosedownPipboyOriginal;
		void Hook_ClosedownPipboy(RE::PipboyManager* a_manager)
		{
			auto ui = RE::UI::GetSingleton();
			RE::BSFixedString s = "CWRepairMenu";
			const bool bRepairMenuOpen = ui->GetMenuOpen(s);
			if (!bRepairMenuOpen)
				ClosedownPipboyOriginal(a_manager);

			REX::DEBUG(std::format("Closedownpipboy called. RepairMenu was open: '{}'", bRepairMenuOpen ? 1 : 0).c_str());

		}

		DetourXS hook_LowerPipboy;
		typedef void(LowerPipboySig)(RE::PipboyManager*, RE::PipboyManager::LOWER_REASON);
		REL::Relocation<LowerPipboySig> LowerPipboyOriginal;
		void Hook_LowerPipboy(RE::PipboyManager* a_manager, RE::PipboyManager::LOWER_REASON a_reason)
		{
			auto ui = RE::UI::GetSingleton();
			RE::BSFixedString s = "CWRepairMenu";
			const bool bRepairMenuOpen = ui->GetMenuOpen(s);
			if (!bRepairMenuOpen)
				LowerPipboyOriginal(a_manager, a_reason);

			REX::DEBUG(std::format("LowerPipboy called. RepairMenu was open: '{}'", bRepairMenuOpen ? 1 : 0).c_str());
		}

		DetourXS hook_GetBuildConfirmQuestion;
		typedef void(GetBuildConfirmQuestionSig)(RE::ExamineMenu*, char*, std::uint32_t);
		REL::Relocation<GetBuildConfirmQuestionSig> ExamineMenuGetBuildConfirmQuestionOriginal;
		void HookExamineMenuGetBuildConfirmQuestion(RE::ExamineMenu* a_this, char* a_buffer, std::uint32_t a_bufferLength)
		{
			RE::WorkbenchMenuBase::ModChoiceData* modChoiceData;
			const char* fullName;
			const char* type;

			if (PArroyo_Menus::Workbench_Additions::bIsScrappingAllJunk) {
				snprintf(a_buffer, a_bufferLength, "Scrap all junk items?");
				return;
			}

			auto choiceData = a_this->QCurrentModChoiceData();

			if (choiceData->recipe)
			{
				modChoiceData = new RE::WorkbenchMenuBase::ModChoiceData(*choiceData);
				if (a_this->repairing)
				{
					type = "$$Repair";
					std::uint32_t selectedIndex = a_this->GetSelectedIndex();
					if (!a_this->invInterface.entriesInvalid && (selectedIndex & 0x80000000) == 0 && selectedIndex < a_this->invInterface.stackedEntries.size())
					{
						RE::InventoryUserUIInterfaceEntry * inventoryUUIEntry = (a_this->invInterface.stackedEntries.data() + selectedIndex);
						const RE::BGSInventoryItem* inventoryItem = RE::BGSInventoryInterface::GetSingleton()->RequestInventoryItem(inventoryUUIEntry->invHandle.id);
						fullName = inventoryItem->GetDisplayFullName(inventoryUUIEntry->stackIndex.at(0));
					}
				}
				else
				{
					type = "$$Make";
					fullName = RE::TESFullName::GetFullName(*modChoiceData->recipe->createdItem).data();
				}
				snprintf(a_buffer, a_bufferLength, "%s %s?", type, fullName);
			}
		}

		DetourXS hook_ExamineMenuBuildConfirmed;
		typedef void(ExamineMenuBuildConfirmedSig)(RE::ExamineMenu*, bool);
		REL::Relocation<ExamineMenuBuildConfirmedSig> ExamineMenuBuildConfirmedOriginal;

		void HookExamineMenuBuildConfirmed(RE::ExamineMenu* a_this, bool a_ownerIsWorkbench)
		{
			using namespace RE;

			REX::DEBUG("ExamineMenuBuildConfirmed - Called");

			if (a_this->repairing)
			{
				std::uint32_t selectedIndex = a_this->GetSelectedIndex();
				if (!a_this->invInterface.entriesInvalid && (selectedIndex & 0x80000000) == 0 && selectedIndex < a_this->invInterface.stackedEntries.size())
				{
					InventoryUserUIInterfaceEntry* inventoryUUIEntry = (a_this->invInterface.stackedEntries.data() + selectedIndex);
					const BGSInventoryItem* inventoryItem = BGSInventoryInterface::GetSingleton()->RequestInventoryItem(inventoryUUIEntry->invHandle.id);

					if (inventoryItem)
					{
						switch (a_this->GetCurrentObj()->formType.get())
						{
						case ENUM_FORM_ID::kARMO:
						case ENUM_FORM_ID::kWEAP:
							BGSInventoryItem::Stack* stack = inventoryItem->GetStackByID(inventoryUUIEntry->stackIndex.at(0));

							if (stack)
							{
								stack->extra->SetHealthPerc(1.0f);

								std::string msg = std::format("{}'s new condition: {}%", inventoryItem->GetDisplayFullName(inventoryUUIEntry->stackIndex.at(0)), (int)(stack->extra->GetHealthPerc() * 100));
								RE::SendHUDMessage::ShowHUDMessage(msg.c_str(), nullptr, true, true);

								BGSInventoryItem::CheckStackIDFunctor compareFunction(inventoryUUIEntry->stackIndex.at(0));
								BGSInventoryItem::SetHealthFunctor writeFunction(stack->extra->GetHealthPerc());
								writeFunction.shouldSplitStacks = 0x101;

								PlayerCharacter::GetSingleton()->FindAndWriteStackDataForInventoryItem(a_this->GetCurrentObj(), compareFunction, writeFunction);

								BGSSoundDescriptorForm* craftingSound = TESDataHandler::GetSingleton()->LookupForm<BGSSoundDescriptorForm>(0x1F252D, "Fallout4.esm");

								a_this->ConsumeSelectedItems(true, craftingSound);
								a_this->UpdateOptimizedAutoBuildInv();
								selectedIndex = a_this->GetSelectedIndex();
								a_this->UpdateItemList(selectedIndex);
								//a_this->uiMovie->asMovieRoot->Invoke("root.BaseInstance.RefreshList", nullptr, nullptr, 0);
								a_this->CreateModdedInventoryItem();

								a_this->UpdateItemCard(false);
								a_this->repairing = false;

								

								if (!a_this->uiMovie->asMovieRoot->Invoke("root.BaseInstance.UpdateButtons", nullptr, nullptr, 0)) {
									REX::DEBUG("Repair Confirmed - Could not update buttons.");
								} // TODO - update custom buttons for this functionality.
							}
						}
					}
				}
			}
			else
			{
				ExamineMenuBuildConfirmedOriginal(a_this, a_ownerIsWorkbench);
			}
		}

		DetourXS hook_BuildWeaponScrappingArray;
		typedef void(BuildWeaponScrappingArraySig)(RE::ExamineMenu*);
		REL::Relocation<BuildWeaponScrappingArraySig> BuildWeaponScrappingArrayOriginal;
		void HookBuildWeaponScrappingArray(RE::ExamineMenu* a_this)
		{
			if (!PArroyo_Menus::Workbench_Additions::bIsScrappingAllJunk) {
				// Vanilla scrapping.
				BuildWeaponScrappingArrayOriginal(a_this);

				std::uint32_t selectedIndex = a_this->GetSelectedIndex();
				if (!a_this->invInterface.entriesInvalid && (selectedIndex & 0x80000000) == 0 && selectedIndex < a_this->invInterface.stackedEntries.size())
				{
					RE::InventoryUserUIInterfaceEntry* inventoryUUIEntry = (a_this->invInterface.stackedEntries.data() + selectedIndex);
					const RE::BGSInventoryItem* inventoryItem = RE::BGSInventoryInterface::GetSingleton()->RequestInventoryItem(inventoryUUIEntry->invHandle.id);
					if (inventoryItem) {
						
						if (inventoryItem->stackData->extra->GetHealthPerc() >= 0) {
							const float oneMinusCND = inventoryItem->stackData->extra->GetHealthPerc();
							for (std::uint32_t i = 0; i < a_this->scrappingArray.size(); i++) {
								a_this->scrappingArray[i].second = a_this->scrappingArray[i].second * oneMinusCND;
							}
						}
					}
				}

				return;
			}

			a_this->scrappingArray.clear();

			// Scrap all junk logic.
			auto player = RE::PlayerCharacter::GetSingleton();

			for (std::uint32_t i = 0; i < player->inventoryList->data.size(); i++)
			{
				RE::BGSInventoryItem inventoryItem = player->inventoryList->data.at(i);

				if (!inventoryItem.object || !Shared::IsJunkItem(inventoryItem.object) || inventoryItem.IsQuestObject(0))
					continue;

				auto baseComp = Shared::GetBaseComponentFromForm(inventoryItem.object);
				if (!baseComp || !baseComp->scrapItem || baseComp->scrapItem->GetFormID() == inventoryItem.object->GetFormID())
					continue;


				RE::TESObjectMISC* miscObject = static_cast<RE::TESObjectMISC*>(inventoryItem.object);
				if (!miscObject)
					continue;

				if (!miscObject->componentData || miscObject->componentData->empty())
					continue;

				for (auto it = miscObject->componentData->begin(); it != miscObject->componentData->end(); ++it) {
					//auto compObj = static_cast<RE::BGSComponent*>(it->first);
					//if (!it->first->IsBoundObject() && (compObj->scrapItem != nullptr && compObj->scrapItem->GetFormID() == miscObject->GetFormID()))
						//continue;

					auto boundObj = reinterpret_cast<RE::TESBoundObject*>(it->first);
					// REX::DEBUG("BuildWeaponScrappingArray - scrapItem found with count: {}", it->second.i);

					const auto count = it->second.i * inventoryItem.GetCount();
					// REX::DEBUG("BuildWeaponScrappingArray - scrapItem found with count: {}", count);
					a_this->scrappingArray.push_back(RE::BSTTuple<RE::TESBoundObject*, std::uint32_t>(boundObj, count));
					
				}
			}
			
		}

		DetourXS hook_RemoveItem1;
		typedef void(RemoveItem1Sig)(RE::BGSInventoryList*, RE::TESBoundObject*, std::uint32_t, std::uint32_t, bool);
		REL::Relocation<RemoveItem1Sig> RemoveItem1Original;
		void HookRemoveItem1(RE::BGSInventoryList* a_this, RE::TESBoundObject* a_object, std::uint32_t a_stackID, std::uint32_t a_count, bool a_manualMerge)
		{
			REX::DEBUG(std::format("RemoveItem1 called. stackID: {}, count: {}, manualMerge: {}", a_stackID, a_count, a_manualMerge ? 1 : 0).c_str());
			//RE::TESObjectREFR::RemoveItem
			//RemoveItem1Original(a_this, a_object, a_stackID, a_count, a_manualMerge);
			
			auto player = RE::PlayerCharacter::GetSingleton();
			if (!a_this || !player || !player->inventoryList || a_this->owner != player->inventoryList->owner) {
				RemoveItem1Original(a_this, a_object, a_stackID, a_count, a_manualMerge);
				return;
			}

			if (PArroyo_Menus::Workbench_Additions::bIsScrappingAllJunk) {
				if (!RE::UI::GetSingleton()->GetMenuOpen<RE::ExamineMenu>()) {
					RemoveItem1Original(a_this, a_object, a_stackID, a_count, a_manualMerge);
					PArroyo_Menus::Workbench_Additions::bIsScrappingAllJunk = false;
					return;
				}

				if (!Shared::IsJunkItem(a_object)) {

					for (std::uint32_t i = 0; i < player->inventoryList->data.size(); i++)
					{
						RE::BGSInventoryItem inventoryItem = player->inventoryList->data.at(i);

						if (!inventoryItem.object || !Shared::IsJunkItem(inventoryItem.object) || inventoryItem.IsQuestObject(0))
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

						auto removeData = RE::TESObjectREFR::RemoveItemData(inventoryItem.object, inventoryItem.GetCount());

						player->RemoveItem(removeData);

					}
					RE::SendHUDMessage::ShowHUDMessage("All junk items were scrapped!", "OBJLunchboxKidsRobotBuild", false, true);
					RE::UI::GetSingleton()->GetMenu<RE::ExamineMenu>()->uiMovie->asMovieRoot->Invoke("root.BaseInstance.UpdateButtons", nullptr, nullptr, 0);
					PArroyo_Menus::Workbench_Additions::bIsScrappingAllJunk = false;
				}
				else {
					RemoveItem1Original(a_this, a_object, a_stackID, a_count, a_manualMerge);
				}

				return;
			}
			
			RemoveItem1Original(a_this, a_object, a_stackID, a_count, a_manualMerge);
			
		}

		DetourXS hook_ShowBuildFailureMessage;
		typedef void(ShowBuildFailureMessageSig)(RE::WorkbenchMenuBase*);
		REL::Relocation<ShowBuildFailureMessageSig> WorkbenchMenuBaseShowBuildFailureMessageOriginal;
		void HookWorkbenchMenuBaseShowBuildFailureMessage(RE::WorkbenchMenuBase* a_this)
		{
			using namespace RE;

			if (a_this->repairing)
			{
				Scaleform::Ptr<RE::ExamineMenu> examineMenu = UI::GetSingleton()->GetMenu<RE::ExamineMenu>();

				std::uint32_t selectedIndex = examineMenu->GetSelectedIndex();
				if (!examineMenu->invInterface.entriesInvalid && (selectedIndex & 0x80000000) == 0 && selectedIndex < examineMenu->invInterface.stackedEntries.size())
				{
					InventoryUserUIInterfaceEntry* inventoryUUIEntry = (examineMenu->invInterface.stackedEntries.data() + selectedIndex);
					const BGSInventoryItem* inventoryItem = BGSInventoryInterface::GetSingleton()->RequestInventoryItem(inventoryUUIEntry->invHandle.id);

					if (inventoryItem)
					{
						auto modChoice = a_this->QCurrentModChoiceData();


						const float currentCondition = inventoryItem->GetStackByID(inventoryUUIEntry->stackIndex.at(0))->extra->GetHealthPerc();
						auto player = RE::PlayerCharacter::GetSingleton();
						const float repairSkill = player->GetActorValue(*PA_Skills.Repair);

						modChoice->requiredItems->clear();
						Shared::ApplyFormulaForRepairRequirements(examineMenu->modChoiceArray, inventoryItem->stackData->extra.get(), *modChoice->recipe->requiredItems, *modChoice->requiredItems, currentCondition, repairSkill);

						BSTArray<BSTTuple<TESForm*, BGSTypedFormValuePair::SharedVal>>* requiredItems = modChoice->requiredItems;
						if (requiredItems)
						{
							RepairFailureCallback* repairFailureCallback = new RepairFailureCallback(examineMenu.get());
							RE::ExamineConfirmMenu::InitDataRepairFailure* initDataRepair = new RE::ExamineConfirmMenu::InitDataRepairFailure(requiredItems);

							BSTHashMap<TESBoundObject*, std::uint32_t> availableComponents;

							BSTTuple<TESBoundObject*, std::uint32_t> insertObject;

							std::uint32_t size = requiredItems->size();
							if (size)
							{
								for (std::uint32_t i = 0; i < size; ++i) {
									const auto& tuple = requiredItems->at(i);
									TESForm* form = tuple.first;
									const auto& value = tuple.second;

									if (!form) continue;

									if (!form->IsBoundObject()) continue;

									TESObjectREFR* sharedContainerREF = examineMenu.get()->sharedContainerRef.get();
									BGSInventoryList* inventoryList = sharedContainerREF->inventoryList;

									if (inventoryList)
									{
										TESBoundObject* object = reinterpret_cast<TESBoundObject*>(form);
										std::uint32_t availableCount = Shared::GetAvailableComponentCount(inventoryList, form);
										insertObject = { object, availableCount };
									}
									else
									{
										BaseFormComponent* container = sharedContainerREF->HasContainer();
										if (container)
										{
											// TODO - handling needed here.
											REX::DEBUG("'sharedContainerREF' has container - MISSING HANDLING");
										}
									}

									availableComponents.insert(insertObject);
								}
								initDataRepair->availableComponents = availableComponents;
							}

							examineMenu->ShowConfirmMenu(initDataRepair, repairFailureCallback);
						}
						a_this->repairing = false;
					}
				}
			}
			else
			{
				GameSettingCollection* gameSettingCollection = GameSettingCollection::GetSingleton();
				SendHUDMessage::ShowHUDMessage(gameSettingCollection->GetSetting("sCannotBuildMessage")->GetString().data(), nullptr, true, true);
			}
		}

		DetourXS hook_QCurrentModChoiceData;
		typedef const RE::WorkbenchMenuBase::ModChoiceData* (QCurrentModChoiceDataSig)(RE::WorkbenchMenuBase*);
		REL::Relocation<QCurrentModChoiceDataSig> WorkbenchMenuBaseQCurrentModChoiceDataOriginal;
		const RE::WorkbenchMenuBase::ModChoiceData* HookWorkbenchMenuBaseQCurrentModChoiceData(RE::WorkbenchMenuBase* a_this)
		{
			if (a_this->repairing)
			{
				// TODO: Merge this into singular function returning dynamic repair cost.
				if (typeid(*a_this) == typeid(RE::PowerArmorModMenu))
				{
					RE::PowerArmorModMenu* powerArmorModMenu = dynamic_cast<RE::PowerArmorModMenu*>(a_this);
					return &powerArmorModMenu->repairData;
				}
				else
				{
					std::uint32_t modChoiceIndex = a_this->modChoiceIndex;
					if (modChoiceIndex >= a_this->modChoiceArray.size())
					{
						return 0;
					}
					else
					{


						RE::WorkbenchMenuBase::ModChoiceData* currentModChoiceDatasss = (a_this->modChoiceArray.data() + modChoiceIndex);

						RE::WorkbenchMenuBase::ModChoiceData* currentModChoiceData = new RE::WorkbenchMenuBase::ModChoiceData();
						currentModChoiceData->index = currentModChoiceDatasss->index;
						currentModChoiceData->mod = currentModChoiceDatasss->mod;
						currentModChoiceData->object = currentModChoiceDatasss->object;
						currentModChoiceData->rank = currentModChoiceDatasss->rank;
						currentModChoiceData->recipe = currentModChoiceDatasss->recipe;
						currentModChoiceData->requiredItems = new RE::BSTArray<RE::BSTTuple<RE::TESForm*, RE::BGSTypedFormValuePair::SharedVal>>();
						currentModChoiceData->requiredPerks = RE::BSTArray<RE::BSTTuple<RE::BGSPerk*, std::uint32_t>>();



						



						Scaleform::Ptr<RE::ExamineMenu> examineMenu = RE::UI::GetSingleton()->GetMenu<RE::ExamineMenu>();
						std::uint32_t selectedIndex = PArroyo_Menus::Workbench_Additions::currentIndex;
						
						if (!examineMenu->invInterface.entriesInvalid && (selectedIndex & 0x80000000) == 0 && selectedIndex < examineMenu->invInterface.stackedEntries.size())
						{

							RE::InventoryUserUIInterfaceEntry* inventoryUUIEntry = (examineMenu->invInterface.stackedEntries.data() + selectedIndex);
							const RE::BGSInventoryItem* inventoryItem = RE::BGSInventoryInterface::GetSingleton()->RequestInventoryItem(inventoryUUIEntry->invHandle.id);
							
							const RE::BGSConstructibleObject* COBJGrabbed = nullptr;
							if (inventoryItem->object->GetFormType() == RE::ENUM_FORM_ID::kWEAP) {
								COBJGrabbed = Shared::GetCOBJ_FromWeapon(static_cast<RE::TESObjectWEAP*>(inventoryItem->object));
							}
							else if (inventoryItem->object->GetFormType() == RE::ENUM_FORM_ID::kARMO) {
								COBJGrabbed = Shared::GetCOBJ_FromArmor(static_cast<RE::TESObjectARMO*>(inventoryItem->object));
							}

							if (!COBJGrabbed) {
								REX::WARN("No COBJ found from object '{}'", inventoryItem->object->GetFormEditorID());
								return 0;
							}
							
							currentModChoiceData->recipe = COBJGrabbed;

							if (!currentModChoiceData->recipe->requiredItems) {

								return 0;
							}

							// Remove any possible required perks, as we don't take that into account when repairing.
							if (!currentModChoiceData->requiredPerks.empty())
							{
								currentModChoiceData->requiredPerks.clear();
							}

							// Remove any possible conditions on the recipe, as we don't take that into account when repairing.
							currentModChoiceData->recipe->conditions.ClearAllConditionItems();

							const float currentCondition = inventoryItem->GetStackByID(inventoryUUIEntry->stackIndex.at(0))->extra->GetHealthPerc();
							const float repairSkill = RE::PlayerCharacter::GetSingleton()->GetActorValue(*PA_Skills.Repair);

							Shared::ApplyFormulaForRepairRequirements(examineMenu->modChoiceArray, inventoryItem->stackData->extra.get(), *currentModChoiceData->recipe->requiredItems, *currentModChoiceData->requiredItems, currentCondition, repairSkill);
							
						}

						

						
						

						return (currentModChoiceData);
					}
				}
			}
			else // Retail logic come into play here.
			{
				std::uint32_t modChoiceIndex = a_this->modChoiceIndex;
				if (modChoiceIndex >= a_this->modChoiceArray.size())
				{
					return 0;
				}
				else
				{
					return (a_this->modChoiceArray.data() + modChoiceIndex);
				}
			}
		}

		DetourXS hook_GetInventoryValue;
		typedef std::int64_t(GetInventoryValueSig)(RE::TESBoundObject*, const RE::ExtraDataList*);
		REL::Relocation<GetInventoryValueSig> GetInventoryValueOriginal;

		std::int64_t HookBGSInventoryItemUtilsGetInventoryValue(RE::TESBoundObject* a_baseObj, const RE::ExtraDataList* a_extra)
		{
			if (!a_extra->HasType(RE::EXTRA_DATA_TYPE::kHealth))
			{
				return GetInventoryValueOriginal(a_baseObj, a_extra);
			}

			RE::ExtraDataList* non_const_a_extra = const_cast<RE::ExtraDataList*>(a_extra);

			std::int64_t newValue;

			// Clamp to 5% value at minimum.
			if (non_const_a_extra->GetHealthPerc() < 0.05f)
			{
				newValue = std::int64_t(GetInventoryValueOriginal(a_baseObj, a_extra) * 0.05f * std::sqrt(0.05f));
			}
			else
			{
				newValue = std::int64_t(GetInventoryValueOriginal(a_baseObj, a_extra) * non_const_a_extra->GetHealthPerc() * std::sqrt(non_const_a_extra->GetHealthPerc()));
			}
			return newValue;
		}

		DetourXS hook_EquipObject;
		typedef bool(EquipObjectSig)(RE::ActorEquipManager*, RE::Actor*, const RE::BGSObjectInstance&, std::uint32_t, std::uint32_t, const RE::BGSEquipSlot*, bool, bool, bool, bool, bool);
		REL::Relocation<EquipObjectSig> EquipObjectOriginal;
		bool Hook_EquipObject(RE::ActorEquipManager* a_actorEquipManager, RE::Actor* a_actor, const RE::BGSObjectInstance& a_object, std::uint32_t a_stackID, std::uint32_t a_number, const RE::BGSEquipSlot* a_slot, bool a_queueEquip, bool a_forceEquip, bool a_playSounds, bool a_applyNow, bool a_locked)
		{
			const bool retailEquipObject = EquipObjectOriginal(a_actorEquipManager, a_actor, a_object, a_stackID, a_number, a_slot, a_queueEquip, a_forceEquip, a_playSounds, a_applyNow, a_locked);

			auto player = RE::PlayerCharacter::GetSingleton();
			if (!player && a_actor != player || !a_object.object)
				return retailEquipObject;

			RE::TESObjectWEAP* weaponObject = static_cast<RE::TESObjectWEAP*>(a_object.object);
			if (!weaponObject)
				return retailEquipObject;

			REX::DEBUG("EQUIPPING HOOKING...");

			auto weaponConditionData = ItemDegradation::WeaponConditionData(a_actor);
			if (weaponConditionData.extraData)
				PArroyo::WPNUtilities::UpdateHUDCondition(weaponConditionData);


			return retailEquipObject;
		}


		DetourXS hook_HandleItemEquip;
		typedef void(HandleItemEquipSig)(RE::Actor*, bool);
		REL::Relocation<HandleItemEquipSig> HandleItemEquipOriginal;
		void Hook_HandleItemEquip(RE::Actor* a_this, bool bCullBone)
		{
			HandleItemEquipOriginal(a_this, bCullBone);

			auto player = RE::PlayerCharacter::GetSingleton();
			if (a_this != player)
				return;

			REX::DEBUG("EQUIPPING HOOKING...");

			auto weaponConditionData = ItemDegradation::WeaponConditionData(a_this);
			if (weaponConditionData.extraData)
				PArroyo::WPNUtilities::UpdateHUDCondition(weaponConditionData);
		}

		DetourXS hook_OnPipboyClosed;
		typedef void(OnPipboyClosedSig)(RE::PipboyManager*);
		REL::Relocation<OnPipboyClosedSig> OnPipboyClosedOriginal;
		void Hook_OnPipboyClosed(RE::PipboyManager* a_pipboyManager)
		{
			auto ui = RE::UI::GetSingleton();
			RE::BSFixedString s = "CWRepairMenu";
			const bool bRepairMenuOpen = ui->GetMenuOpen(s);
			if (!bRepairMenuOpen) {
				OnPipboyClosedOriginal(a_pipboyManager);
				/*
				auto weaponConditionData = ItemDegradation::WeaponConditionData(RE::PlayerCharacter::GetSingleton());
				if (weaponConditionData.extraData && weaponConditionData.Form)
					PArroyo::WPNUtilities::UpdateHUDCondition(weaponConditionData);
					*/
			}

			REX::DEBUG(std::format("Closedownpipboy called. RepairMenu was open: '{}'", bRepairMenuOpen ? 1 : 0).c_str());



		}

		DetourXS hook_SetCurrentAmmoCount;
		typedef void(SetCurrentAmmoCountSig)(RE::Actor*, RE::BGSEquipIndex, std::uint32_t);
		REL::Relocation<SetCurrentAmmoCountSig> SetCurrentAmmoCountOriginal;
		void Hook_SetCurrentAmmoCount(RE::Actor* a_this, RE::BGSEquipIndex a_equipIndex, std::uint32_t a_count)
		{
			SetCurrentAmmoCountOriginal(a_this, a_equipIndex, a_count);
			auto weaponConditionData = ItemDegradation::WeaponConditionData(RE::PlayerCharacter::GetSingleton());
			PArroyo::WPNUtilities::UpdateHUDCondition(weaponConditionData);

		}

		DetourXS hook_ConsumeSelectedItems;
		typedef void(ConsumeSelectedItemsSig)(RE::ExamineMenu*, bool, const RE::BGSSoundDescriptorForm*);
		REL::Relocation<ConsumeSelectedItemsSig> ConsumeSelectedItemsOriginal;
		void HookConsumeSelectedItems(RE::ExamineMenu* a_this, bool a_autoBuild, const RE::BGSSoundDescriptorForm* a_consumeSound) {
			if (!PArroyo_Menus::Workbench_Additions::bIsScrappingAllJunk) {
				ConsumeSelectedItemsOriginal(a_this, a_autoBuild, a_consumeSound);
				return;
			}
		}

		using RemoveItem_t = RE::ObjectRefHandle(*)(RE::TESObjectREFR*, RE::TESObjectREFR::RemoveItemData&);
		inline REL::Relocation<RemoveItem_t> _OriginalRemoveItem;

		RE::ObjectRefHandle Hooked_RemoveItem( RE::TESObjectREFR* a_this, RE::TESObjectREFR::RemoveItemData& a_data)
		{
			if (PArroyo_Menus::Workbench_Additions::bIsScrappingAllJunk) {
				if (!RE::UI::GetSingleton()->GetMenuOpen<RE::ExamineMenu>()) {
					PArroyo_Menus::Workbench_Additions::bIsScrappingAllJunk = false;
					return _OriginalRemoveItem(a_this, a_data);
				}

				if (!Shared::IsJunkItem(a_data.object)) {
					PArroyo_Menus::Workbench_Additions::bIsScrappingAllJunk = false;
					auto player = RE::PlayerCharacter::GetSingleton();
					for (std::uint32_t i = 0; i < player->inventoryList->data.size(); i++)
					{
						RE::BGSInventoryItem inventoryItem = player->inventoryList->data.at(i);
						if (!inventoryItem.object || !Shared::IsJunkItem(inventoryItem.object) || inventoryItem.IsQuestObject(0))
							continue;

						
						//RE::TESObjectMISC* miscObj = static_cast<RE::TESObjectMISC*>(inventoryItem.object);

						//if (!miscObj->componentData)
							//continue;

						//const auto first = miscObj->componentData->at(0).first;

						//RE::BGSComponent* compObj = static_cast<RE::BGSComponent*>(first);

						//if ((compObj && compObj->scrapItem != nullptr && compObj->scrapItem->GetFormID() == miscObj->GetFormID()))
							//continue;

						//auto miscCompObj = static_cast<RE::TESObjectMISC*>(first);

						//if (miscCompObj && miscCompObj->componentData && !miscCompObj->componentData->empty() && miscCompObj->componentData->at(0).first && miscCompObj->componentData->at(0).first->GetFormID() == miscObj->GetFormID())
							//continue;
						

						auto baseComp = Shared::GetBaseComponentFromForm(inventoryItem.object);
						if (!baseComp || !baseComp->scrapItem || baseComp->scrapItem->GetFormID() == inventoryItem.object->GetFormID())
							continue;

						//if (inventoryItem.GetCount() == 0) {

						//}
						
						

						auto removeData = RE::TESObjectREFR::RemoveItemData(inventoryItem.object, inventoryItem.GetCount());

						player->RemoveItem(removeData);
					}
					RE::SendHUDMessage::ShowHUDMessage("All junk items were scrapped!", "OBJLunchboxKidsRobotBuild", false, true);
					RE::UI::GetSingleton()->GetMenu<RE::ExamineMenu>()->uiMovie->asMovieRoot->Invoke("root.BaseInstance.UpdateButtons", nullptr, nullptr, 0);

					RE::TESObjectREFR::RemoveItemData a = RE::TESObjectREFR::RemoveItemData(a_this, 0);
					return _OriginalRemoveItem(a_this, a);
				}
				else {
					return _OriginalRemoveItem(a_this, a_data);
				}
			}


			return _OriginalRemoveItem(a_this, a_data);
		}

		static void InstallRemoveItemHook()
		{
			REL::Relocation<std::uintptr_t> vtbl{ RE::PlayerCharacter::VTABLE[0] };
			_OriginalRemoveItem = vtbl.write_vfunc(0x6D, &Hooked_RemoveItem);
		}
		
		
		namespace Registers {
			void RegisterAllHooks() {
				RegisterDetourFunction(hook_CombatFormulasCalcWeaponDamage, RE::ID::CombatFormulas::CalcWeaponDamage, &Hook_CombatFormulasCalcWeaponDamage, CombatFormulasCalcWeaponDamageOriginal, "CalcWeaponDamage");
				RegisterDetourFunction(hook_AddItem, RE::ID::BGSInventoryList::AddItem1, &Hook_BGSInventoryListAddItem, AddItem_Original, "AddItem");
				RegisterDetourFunction(hook_GetEquippedDamageResistance, RE::ID::ActorUtils::GetEquippedArmorDamageResistance, &Hook_GetEquippedDamageResistance, GetEquippedDamageResistanceOriginal, "GetEquippedArmorDamageResistance");
				RegisterDetourFunction(hook_TESObjectWeaponFire, RE::ID::TESObjectWEAP::Fire, &Hook_TESObjectWeaponFire, TESObjectWeaponFireOriginal, "TESObjectWeaponFire");
				RegisterDetourFunction(hook_CombatFormulasCalcTargetedLimbDamage, RE::ID::CombatFormulas::CalcTargetedLimbDamage, &Hook_CombatFormulasCalcTargetedLimbDamage, CombatFormulasCalcTargetedLimbDamageOriginal, "CalcTargetedLimbDamage");
				RegisterDetourFunction(hook_SetHealthPerc, RE::ID::ExtraDataList::SetHealthPerc, &Hook_ExtraDataListSetHealthPerc, SetHealthPercOriginal, "SetHealthPerc");
				RegisterDetourFunction(hook_GetWeaponDisplayDamage, RE::ID::CombatFormulas::GetWeaponDisplayDamage, &Hook_GetWeaponDisplayDamage, GetWeaponDisplayDamageOriginal, "GetWeaponDisplayDamage");
				
				RegisterDetourFunction(hook_GetInventoryValue , RE::ID::BGSInventoryItemUtils::GetInventoryValue, &HookBGSInventoryItemUtilsGetInventoryValue, GetInventoryValueOriginal, "GetInventoryValue");
				RegisterDetourFunction(hook_QCurrentModChoiceData, RE::ID::WorkbenchMenuBase::QCurrentModChoiceData, &HookWorkbenchMenuBaseQCurrentModChoiceData, WorkbenchMenuBaseQCurrentModChoiceDataOriginal, "WorkbenchMenuBaseQCurrentModChoiceData");
				RegisterDetourFunction(hook_GetBuildConfirmQuestion, RE::ID::ExamineMenu::GetBuildConfirmQuestion, &HookExamineMenuGetBuildConfirmQuestion, ExamineMenuGetBuildConfirmQuestionOriginal, "ExamineMenuGetBuildConfirmQuestion");
				RegisterDetourFunction(hook_ShowBuildFailureMessage, RE::ID::WorkbenchMenuBase::ShowBuildFailureMessage, &HookWorkbenchMenuBaseShowBuildFailureMessage, WorkbenchMenuBaseShowBuildFailureMessageOriginal, "WorkbenchMenuBaseShowBuildFailureMessage");
				RegisterDetourFunction(hook_ExamineMenuBuildConfirmed, RE::ID::ExamineMenu::BuildConfirmed, &HookExamineMenuBuildConfirmed, ExamineMenuBuildConfirmedOriginal, "ExamineMenuBuildConfirmed");
				RegisterDetourFunction(hook_BuildWeaponScrappingArray, RE::ID::ExamineMenu::BuildWeaponScrappingArray, &HookBuildWeaponScrappingArray, BuildWeaponScrappingArrayOriginal, "BuildWeaponScrappingArray");
				
				InstallRemoveItemHook();
				// RegisterDetourFunction(hook_RemoveItem1, RE::ID::BGSInventoryList::RemoveItem1, &HookRemoveItem1, RemoveItem1Original, "RemoveItem1");
				
				// RegisterDetourFunction(hook_ConsumeSelectedItems, RE::ID::ExamineMenu::ConsumeSelectedItems, &HookConsumeSelectedItems, ConsumeSelectedItemsOriginal, "ConsumeSelectedItems");

				// RegisterDetourFunction(hook_ClosedownPipboy, RE::ID::PipboyManager::ClosedownPipboy, &Hook_ClosedownPipboy, ClosedownPipboyOriginal, "ClosedownPipboy");
				// RegisterDetourFunction(hook_LowerPipboy, RE::ID::PipboyManager::LowerPipboy, &Hook_LowerPipboy, LowerPipboyOriginal, "LowerPipboy");

				// RegisterDetourFunction(hook_EquipObject, RE::ID::ActorEquipManager::EquipObject, &Hook_EquipObject, EquipObjectOriginal, "EquipObject");
				// RegisterDetourFunction(hook_GetWeaponDisplayRateOfFire, RE::ID::CombatFormulas::GetWeaponDisplayRateOfFire, &Hook_GetWeaponDisplayRateOfFire, GetWeaponDisplayRateOfFireOriginal, "GetWeaponDisplayRateOfFire");
				// RegisterDetourFunction(hook_HandleItemEquip, RE::ID::Actor::HandleItemEquip, &Hook_HandleItemEquip, HandleItemEquipOriginal, "HandleItemEquipItem");
				// RegisterDetourFunction(hook_OnPipboyClosed, RE::ID::PipboyManager::OnPipboyClosed, &Hook_OnPipboyClosed, OnPipboyClosedOriginal, "OnPipboyClosed");
				// RegisterDetourFunction(hook_SetCurrentAmmoCount, RE::ID::Actor::SetCurrentAmmoCount, &Hook_SetCurrentAmmoCount, SetCurrentAmmoCountOriginal, "SetCurrentAmmoCount");
			}

			void Install()
			{
				auto& trampoline = REL::GetTrampoline();


			}
		}
	}
}

