#include "ItemDegradation.h"
#include <unordered_map>
#include <RE/T/TESAmmo.h>
#include "debugLog.h"
#include "IGlobalConfig.h"
#include "GameForms.h"
#include <RE/B/BSScript_IVirtualMachine.h>
#include <RE/T/TESDataHandler.h>
#include <format>
#include <RE/A/Actor.h>
#include <RE/B/BGSKeyword.h>
#include <RE/B/BGSListForm.h>
#include <RE/T/TESForm.h>
#include <RE/T/TESObjectARMO.h>
#include <RE/T/TESObjectREFR.h>
#include <RE/T/TESObjectWEAP.h>
#include <RE/E/ENUM_FORM_ID.h>
#include <RE/P/PlayerCharacter.h>
#include "Serialization.h"
#include <chrono>
#include <cstdint>
#include <RE/E/ExtraDataList.h>
#include <RE/S/Setting.h>
#include <RE/T/TBO_InstanceData.h>
#include <RE/T/TESEquipEvent.h>
#include <RE/E/EXTRA_DATA_TYPE.h>
#include "Hooks.h"
#include <variant>
#include <RE/B/BSFixedString.h>
#include <RE/I/IMenu.h>
// #include <Scaleform/G/GFx_Movie.h>
// #include <Scaleform/G/GFx_Value.h>
#include <RE/U/UI.h>
#include "skills.h"
#include "Shared.h"
#include <cmath>
#include <string>
#include <RE/B/BGSEquipIndex.h>
#include <RE/B/BGSObjectInstance.h>
#include <RE/W/WEAPON_FLAGS.h>
#include <RE/W/WEAPON_TYPE.h>
#include <RE/P/PipboyDataManager.h>
#include <RE/A/ActorEquipManager.h>
#include <RE/B/BSExtraData.h>
#include <RE/E/ExtraInstanceData.h>
#include <RE/T/TES.h>
#include <REX/LOG.h>
#include <RE/A/ActorValueInfo.h>
#include <RE/B/BGSDamageType.h>
#include <RE/B/BGSAction.h>
#include <RE/E/ExtraCharge.h>
#include <RE/E/ExtraHealth.h>
#include <RE/B/BGSBodyPartDefs.h>
#include <string_view>
#include <RE/B/BSRandom.h>
#include <RE/S/SendHUDMessage.h>
#include "Menus/HUD_Additions.h"
#include <Windows.h>
#include <vector>
#include <RE/B/BGSInventoryItem.h>
#include <RE/P/PowerArmor.h>
#include <Scaleform/G/GFx_Value.h>
#include "../src/Scaleform_PArroyo.h"

namespace PArroyo {
	namespace ItemDegradation {
		std::unordered_map<RE::TESObjectWEAP*, PAWeaponConditionInfo> WeaponConditionMapping;

		std::unordered_map<RE::TESAmmo*, float> AmmoDegradationMap;

		//	weapon condition game settings
		RE::Setting* fDamageWeaponMult;
		RE::Setting* fDamageGunWeapCondMult;
		RE::Setting* fDamageGunWeapCondBase;
		RE::Setting* fDamageMeleeWeapCondMult;
		RE::Setting* fDamageMeleeWeapCondBase;
		float	 fDamageSkillBase;
		float	 fDamageSkillMult;
		float	 fAVDMeleeDamageStrengthMult;
		float	 fAVDMeleeDamageStrengthOffset;
		RE::Setting* fDamageArmConditionBase;
		RE::Setting* fDamageArmConditionMult;

		//	weapon condition jam game settings https://geck.bethsoft.com/index.php?title=FWeaponConditionReloadJamXX
		RE::Setting* fWeaponConditionReloadJam1;
		RE::Setting* fWeaponConditionReloadJam2;
		RE::Setting* fWeaponConditionReloadJam3;
		RE::Setting* fWeaponConditionReloadJam4;
		RE::Setting* fWeaponConditionReloadJam5;
		RE::Setting* fWeaponConditionReloadJam6;
		RE::Setting* fWeaponConditionReloadJam7;
		RE::Setting* fWeaponConditionReloadJam8;
		RE::Setting* fWeaponConditionReloadJam9;
		RE::Setting* fWeaponConditionReloadJam10;

		bool bShouldTryAndJamWeapon = false;
		std::chrono::system_clock::time_point lastTimeJammed;

		//	weapon condition rate of fire game settings https://geck.bethsoft.com/index.php?title=FWeaponConditionRateOfFireXX
		RE::Setting* fWeaponConditionRateOfFire1;
		RE::Setting* fWeaponConditionRateOfFire2;
		RE::Setting* fWeaponConditionRateOfFire3;
		RE::Setting* fWeaponConditionRateOfFire4;
		RE::Setting* fWeaponConditionRateOfFire5;
		RE::Setting* fWeaponConditionRateOfFire6;
		RE::Setting* fWeaponConditionRateOfFire7;
		RE::Setting* fWeaponConditionRateOfFire8;
		RE::Setting* fWeaponConditionRateOfFire9;
		RE::Setting* fWeaponConditionRateOfFire10;

		ItemDegredation_Struct ItemDegredationForms;

		RE::BGSKeyword* noItemDegredationKeyword;
		RE::BGSKeyword* crWeaponRanged;

		RE::BGSKeyword* furntitureTypePowerArmor;

		//	armor type keywords
		RE::BGSKeyword* armorTypePower;
		RE::BGSKeyword* armorBodyPartChest;
		RE::BGSKeyword* armorBodyPartHead;
		RE::BGSKeyword* armorBodyPartLeftArm;
		RE::BGSKeyword* armorBodyPartRightArm;
		RE::BGSKeyword* armorBodyPartLeftLeg;
		RE::BGSKeyword* armorBodyPartRightLeg;

		bool bIgnoreNoSkillWeaponPrompt = false;
		bool bIgnoreMaxSkillsNPC = false;
		bool bIgnoreMaxSkillsPlayer = false;

		float tempWantedWeaponCondition = 0.0;


		ArmorConditionData::ArmorConditionData() : actor(nullptr), Form(nullptr), extraData(nullptr), instance(nullptr) {}

		ArmorConditionData::ArmorConditionData(RE::TESForm* form, RE::ExtraDataList* extradata)
		{
			actor = nullptr;
			Form = form;
			extraData = extradata;
			instance = GetArmorInstanceData(extraData);
		}

		ArmorConditionData::ArmorConditionData(RE::Actor* myActor, RE::TESForm* form, RE::ExtraDataList* extradata)
		{
			actor = myActor;
			Form = form;
			extraData = extradata;
			instance = GetArmorInstanceData(extraData);
		}

		ArmorConditionData::ArmorConditionData(RE::TESObjectREFR* refr)
		{
			if (refr)
			{
				actor = RE::PlayerCharacter::GetSingleton();	//	Going to set this to Player so that when a ref gets initialised it already has the Player's stats
				Form = refr;
				extraData = refr->extraList.get();
				instance = GetArmorInstanceData(extraData);
			}
		}

		ArmorConditionData::ArmorConditionData(RE::Actor* myActor)
		{
			if (myActor)
			{
				actor = myActor;
				//Form = WPNUtilities::GetEquippedWeaponForm(myActor);
				//extraData = WPNUtilities::GetEquippedWeaponExtraData(myActor);
				//instance = WPNUtilities::GetEquippedWeaponInstanceData(myActor);
			}
		}
	}

	namespace ItemDegradation {
		WeaponConditionData::WeaponConditionData() : actor(nullptr), Form(nullptr), extraData(nullptr), instance(nullptr) {}

		WeaponConditionData::WeaponConditionData(RE::TESForm* form, RE::ExtraDataList* extradata)
		{
			actor = nullptr;
			Form = form;
			extraData = extradata;
			invHandle = 0;

			instance = GetWeaponInstanceData(extraData);
		}

		WeaponConditionData::WeaponConditionData(RE::Actor* myActor, RE::TESForm* form, RE::ExtraDataList* extradata)
		{
			actor = myActor;
			Form = form;
			extraData = extradata;
			invHandle = 0;
			// invHandle = WPNUtilities::GetEquippedWeaponHandleID(myActor);

			instance = GetWeaponInstanceData(extraData);
		}

		WeaponConditionData::WeaponConditionData(RE::TESObjectREFR* refr)
		{
			if (refr)
			{
				actor = RE::PlayerCharacter::GetSingleton();	//	Going to set this to Player so that when a ref gets initialised it already has the Player's stats
				Form = refr;
				extraData = refr->extraList.get();
				invHandle = 0;

				instance = GetWeaponInstanceData(extraData);
			}
		}

		WeaponConditionData::WeaponConditionData(RE::Actor* myActor)
		{
			if (myActor)
			{
				auto currentWeapon = InventoryUtils::GetCurrentEquippedWeapon(myActor);
				// auto weaponObject = static_cast<RE::TESObjectWEAP*>(myActor->currentProcess->middleHigh->equippedItems[0].item.object);
				if (currentWeapon) {
					actor = myActor;
					Form = currentWeapon->object;
					extraData = currentWeapon->stackData->extra.get();
					// instance = GetWeaponInstanceData(extraData);
					//instance = myActor->currentProcess->middleHigh->equippedItems[0].item.instanceData.get();
					invHandle = 0;
					//invHandle = InventoryUtils::GetHandleIDByIndex();
					instance = GetWeaponInstanceData(extraData);
					// instance = static_cast<RE::TESObjectWEAP::InstanceData*>();

				}
				else {
					REX::WARN(std::format("Tried to get equipoped weapon from actor '{}' but didn't cast", myActor->GetDisplayFullName()));
				}
			}
		}

	}

	RE::TESObjectWEAP::InstanceData* GetWeaponInstanceData(RE::ExtraDataList* myExtraDataList)
	{
		RE::TESObjectWEAP::InstanceData* result = nullptr;
		RE::TBO_InstanceData* myInstanceData = nullptr;

		myExtraDataList->extraRWLock.lock_read();

		if (myExtraDataList)
		{
			RE::BSExtraData* myExtraData = myExtraDataList->HasType<RE::ExtraInstanceData>() ? myExtraDataList->GetByType<RE::ExtraInstanceData>() : nullptr;

			if (myExtraData)
			{
				RE::ExtraInstanceData* myExtraInstanceData = static_cast<RE::ExtraInstanceData*>(myExtraData);
				if (myExtraInstanceData)
				{
					myInstanceData = myExtraInstanceData->data.get();
				}
			}
		}

		myExtraDataList->extraRWLock.unlock_read();
		if (!myInstanceData)
			return nullptr;

		result = static_cast<RE::TESObjectWEAP::InstanceData*>(myInstanceData);
		return result;
	}

	RE::TESObjectARMO::InstanceData* GetArmorInstanceData(RE::ExtraDataList* myExtraDataList)
	{
		RE::TESObjectARMO::InstanceData* result = nullptr;
		RE::TBO_InstanceData* myInstanceData = nullptr;
		myExtraDataList->extraRWLock.lock_read();
		if (myExtraDataList)
		{
			RE::BSExtraData* myExtraData = myExtraDataList->HasType<RE::ExtraInstanceData>() ? myExtraDataList->GetByType<RE::ExtraInstanceData>() : nullptr;

			if (myExtraData)
			{
				RE::ExtraInstanceData* myExtraInstanceData = static_cast<RE::ExtraInstanceData*>(myExtraData);
				if (myExtraInstanceData)
				{
					myInstanceData = myExtraInstanceData->data.get();
				}
			}
		}
		myExtraDataList->extraRWLock.unlock_read();
		if (!myInstanceData)
			return nullptr;

		result = static_cast<RE::TESObjectARMO::InstanceData*>(myInstanceData);
		return result;
	}

	float GetWeaponConditionMaximum(ItemDegradation::WeaponConditionData Data)
	{
		if (!Data.extraData)
			return -1.0;
		if (!Data.extraData->HasType(RE::EXTRA_DATA_TYPE::kCharge))
			return -1.0;

		return ((RE::ExtraCharge*)Data.extraData->GetByType(RE::EXTRA_DATA_TYPE::kCharge))->charge;
	}

	float GetWeaponConditionMaximum(RE::TESObjectREFR* refr)
	{
		return refr ? GetWeaponConditionMaximum(ItemDegradation::WeaponConditionData(refr)) : -1.0f;
	}

	float GetWeaponConditionPercent(ItemDegradation::WeaponConditionData Data)
	{
		if (!Data.extraData)
			return -1.0;

		const float health = Data.extraData->GetHealthPerc();
		return health;
	}

	float GetWeaponConditionPercent(RE::TESObjectREFR* refr)
	{
		return refr ? GetWeaponConditionPercent(ItemDegradation::WeaponConditionData(refr)) : -1.0f;
	}

	float GetWeaponConditionCurrent(ItemDegradation::WeaponConditionData Data)
	{
		float Percent = GetWeaponConditionPercent(Data);
		float Maximum = GetWeaponConditionMaximum(Data);
		if ((0 > Percent) || (0 > Maximum))
			return -1.0;
		return Percent * Maximum;
	}

	float GetWeaponConditionCurrent(RE::TESObjectREFR* refr)
	{
		return refr ? GetWeaponConditionCurrent(ItemDegradation::WeaponConditionData(refr)) : -1.0f;
	}

	void SetWeaponConditionMaximum(ItemDegradation::WeaponConditionData Data, float Value)
	{
		if (!Data.extraData)
			return;
		if (!Data.extraData->HasType(RE::EXTRA_DATA_TYPE::kCharge))
			return;
		Value = Value > 1 ? Value : 1;
		((RE::ExtraCharge*)Data.extraData->GetByType(RE::EXTRA_DATA_TYPE::kCharge))->charge = Value;
		// SetExtraData(Data.extraData, Charge, charge, Value);
	}

	void SetWeaponConditionMaximum(RE::TESObjectREFR* refr, float Value)
	{
		if (!refr)
			return;

		SetWeaponConditionMaximum(ItemDegradation::WeaponConditionData(refr), Value);
	}

	void SetWeaponConditionPercent(ItemDegradation::WeaponConditionData Data, float Value)
	{
		Data.extraData->SetHealthPerc(Value);
	}

	void SetWeaponConditionPercent(RE::TESObjectREFR* refr, float Value)
	{
		SetWeaponConditionPercent(ItemDegradation::WeaponConditionData(refr), Value);
	}

	void SetWeaponConditionCurrent(ItemDegradation::WeaponConditionData Data, float Value)
	{}

	void SetWeaponConditionCurrent(RE::TESObjectREFR* refr, float Value)
	{
		if (!refr)
			return;

		SetWeaponConditionCurrent(ItemDegradation::WeaponConditionData(refr), Value);
	}

	float GetArmorConditionMaximum(ItemDegradation::ArmorConditionData Data)
	{
		return 0.0f;
	}
	float GetArmorConditionMaximum(RE::TESObjectREFR* refr)
	{
		if (!refr)
			return -1.0f;

		return GetArmorConditionMaximum(ItemDegradation::ArmorConditionData(refr));

	}
	float GetArmorConditionPercent(ItemDegradation::ArmorConditionData Data)
	{
		return 0.0f;
	}
	float GetArmorConditionPercent(RE::TESObjectREFR* refr)
	{
		if (!refr)
			return -1.0f;

		return GetArmorConditionPercent(ItemDegradation::ArmorConditionData(refr));
	}
	float GetArmorConditionCurrent(ItemDegradation::ArmorConditionData Data)
	{
		return 0.0f;
	}
	float GetArmorConditionCurrent(RE::TESObjectREFR* refr)
	{
		if (!refr)
			return -1.0f;

		return GetArmorConditionCurrent(ItemDegradation::ArmorConditionData(refr));
	}
	void SetArmorConditionMaximum(ItemDegradation::ArmorConditionData Data, float Value)
	{}
	void SetArmorConditionMaximum(RE::TESObjectREFR* refr, float Value)
	{
		if (!refr)
			return;
		SetArmorConditionMaximum(ItemDegradation::ArmorConditionData(refr), Value);
	}
	void SetArmorConditionPercent(ItemDegradation::ArmorConditionData Data, float Value)
	{
		Data.extraData->SetHealthPerc(Value);
	}

	void SetArmorConditionPercent(RE::TESObjectREFR* refr, float Value)
	{
		if (!refr)
			return;
		SetArmorConditionPercent(ItemDegradation::ArmorConditionData(refr), Value);
	}
	void SetArmorConditionCurrent(ItemDegradation::ArmorConditionData Data, float Value)
	{}
	void SetArmorConditionCurrent(RE::TESObjectREFR* refr, float Value)
	{
		if (!refr)
			return;
		SetArmorConditionCurrent(ItemDegradation::ArmorConditionData(refr), Value);
	}
	void ModWeaponCondition(ItemDegradation::WeaponConditionData Data, float Value)
	{}
	void ModWeaponCondition(RE::TESObjectREFR* refr, float Value)
	{
		if (!refr)
			return;
		ModWeaponCondition(ItemDegradation::WeaponConditionData(refr), Value);
	}
	void ModWeaponCondition(RE::Actor* actor)
	{}

	void InitializeWeaponCondition(ItemDegradation::WeaponConditionData myConditionData)
	{
		myConditionData.extraData->SetHealthPerc(RE::BSRandom::Float(0.45f, 0.85f));
	}
	void InitializeWeaponCondition(RE::TESObjectREFR* myRef)
	{
		if (!myRef)
			return;
		InitializeWeaponCondition(ItemDegradation::WeaponConditionData(myRef));
	}
	void InitializeArmorCondition(ItemDegradation::ArmorConditionData myConditionData)
	{
		myConditionData.extraData->SetHealthPerc(RE::BSRandom::Float(0.45f, 0.85f));
	}

	void InitializeArmorCondition(RE::TESObjectREFR* myRef)
	{
		if (!myRef)
			return;
		InitializeArmorCondition(ItemDegradation::ArmorConditionData(myRef));
	}


	void InitializeInventoryItemCondition(RE::TESObjectREFR* myRef, RE::TESForm* myForm)
	{}
	void InitializeInventoryCondition(RE::TESObjectREFR* myRef)
	{}
	void InitializePowerArmorFrameInventoryCondition(RE::TESObjectREFR* ref)
	{}
	void ItemDegredation_ItemEquipped(RE::TESEquipEvent* myEvent)
	{}
	void HandleWeaponJamming()
	{}
	void CheckForNPCWeaponConditionPerk(RE::TESObjectREFR* myRef)
	{}
}

void PArroyo::ItemDegradation::DefineItemDegradationFormsFromGame()
{
	LOG_INFO("Fetching Item Degradation Forms From Game...");
	RE::TESDataHandler* dataHandler = RE::TESDataHandler::GetSingleton();

	RE::TESAmmo* ammo10mm = dataHandler->LookupForm<RE::TESAmmo>(0x01F276, "Fallout4.esm");
	RE::TESAmmo* ammo2mmEC = dataHandler->LookupForm<RE::TESAmmo>(0x18ABDF, "Fallout4.esm");
	RE::TESAmmo* ammo308 = dataHandler->LookupForm<RE::TESAmmo>(0x01F66B, "Fallout4.esm");
	RE::TESAmmo* ammo38 = dataHandler->LookupForm<RE::TESAmmo>(0x04CE87, "Fallout4.esm");
	RE::TESAmmo* ammo44 = dataHandler->LookupForm<RE::TESAmmo>(0x09221C, "Fallout4.esm");
	RE::TESAmmo* ammo45 = dataHandler->LookupForm<RE::TESAmmo>(0x01F66A, "Fallout4.esm");

	crWeaponRanged = dataHandler->LookupForm<RE::BGSKeyword>(0x189348, "Fallout4.esm");

	//	Get Armor Type Keywords
	armorTypePower = dataHandler->LookupForm<RE::BGSKeyword>(0x04D8A1, "Fallout4.esm");
	armorBodyPartChest = dataHandler->LookupForm<RE::BGSKeyword>(0x06C0EC, "Fallout4.esm");
	armorBodyPartHead = dataHandler->LookupForm<RE::BGSKeyword>(0x10C418, "Fallout4.esm");

	//	Misc Degredation Stuff
	ItemDegredationForms.tempConditionVariable = dataHandler->LookupForm<RE::ActorValueInfo>(0x0DCE0F, "Fallout4.esm");


	//ItemDegredationForms.weaponConditionHealthMaxDMGT = dataHandler->LookupForm<RE::BGSDamageType>(, MOD_ESM);

	/*
	//	Actor Values that hold information on Items
	ItemDegredationForms.itemConditionMaxHealth = dataHandler->LookupForm<RE::ActorValueInfo>(, MOD_ESM);
	ItemDegredationForms.itemConditionMinHealth = dataHandler->LookupForm<RE::ActorValueInfo>(, MOD_ESM);
	ItemDegredationForms.itemConditionStartCond = dataHandler->LookupForm<RE::ActorValueInfo>(, MOD_ESM);
	*/

	//	this holds our starting condition percent on OMODs in the Creation Kit

	//ItemDegredationForms.weaponConditionHealthStartingDMGT = dataHandler->LookupForm<RE::BGSDamageType>(, MOD_ESM);

	ItemDegradation::AmmoDegradationMap[ammo10mm] = 0.005f;
	ItemDegradation::AmmoDegradationMap[ammo2mmEC] = 0.04f;
	ItemDegradation::AmmoDegradationMap[ammo308] = 0.0133f;
	ItemDegradation::AmmoDegradationMap[ammo38] = 0.003f;
	ItemDegradation::AmmoDegradationMap[ammo44] = 0.011f;
	ItemDegradation::AmmoDegradationMap[ammo45] = 0.0035f;

	//	Get Condition Game Settings
	fDamageWeaponMult = RE::GameSettingCollection::GetSingleton()->GetSetting("fDamageWeaponMult");
	fDamageGunWeapCondMult = RE::GameSettingCollection::GetSingleton()->GetSetting("fDamageGunWeapCondMult");
	fDamageGunWeapCondBase = RE::GameSettingCollection::GetSingleton()->GetSetting("fDamageGunWeapCondBase");
	fDamageMeleeWeapCondMult = RE::GameSettingCollection::GetSingleton()->GetSetting("fDamageMeleeWeapCondMult");
	fDamageMeleeWeapCondBase = RE::GameSettingCollection::GetSingleton()->GetSetting("fDamageMeleeWeapCondBase");
	//	getting custom GSTT doesn't seem to work? using floats instead
	//fDamageSkillBase = dataHandler->LookupForm<RE::Setting*>(GetFormFromIdentifier("CapitalWasteland.esm|D3BD1"));
	fDamageSkillBase = 0.5;
	//fDamageSkillMult = dataHandler->LookupForm<RE::Setting*>(GetFormFromIdentifier("CapitalWasteland.esm|D3BD2"));
	fDamageSkillMult = 0.5;
	//fAVDMeleeDamageStrengthMult = dataHandler->LookupForm<RE::Setting*>(GetFormFromIdentifier("CapitalWasteland.esm|D3BD3"));
	fAVDMeleeDamageStrengthMult = 0.5;
	//fAVDMeleeDamageStrengthOffset = dataHandler->LookupForm<RE::Setting*>(GetFormFromIdentifier("CapitalWasteland.esm|D3BD4"));
	fAVDMeleeDamageStrengthOffset = 0.0;
	fDamageArmConditionBase = RE::GameSettingCollection::GetSingleton()->GetSetting("fDamageArmConditionBase");
	fDamageArmConditionMult = RE::GameSettingCollection::GetSingleton()->GetSetting("fDamageArmConditionMult");

	fWeaponConditionReloadJam1 = RE::GameSettingCollection::GetSingleton()->GetSetting("fWeaponConditionReloadJam1");
	fWeaponConditionReloadJam2 = RE::GameSettingCollection::GetSingleton()->GetSetting("fWeaponConditionReloadJam2");
	fWeaponConditionReloadJam3 = RE::GameSettingCollection::GetSingleton()->GetSetting("fWeaponConditionReloadJam3");
	fWeaponConditionReloadJam4 = RE::GameSettingCollection::GetSingleton()->GetSetting("fWeaponConditionReloadJam4");
	fWeaponConditionReloadJam5 = RE::GameSettingCollection::GetSingleton()->GetSetting("fWeaponConditionReloadJam5");
	fWeaponConditionReloadJam6 = RE::GameSettingCollection::GetSingleton()->GetSetting("fWeaponConditionReloadJam6");
	fWeaponConditionReloadJam7 = RE::GameSettingCollection::GetSingleton()->GetSetting("fWeaponConditionReloadJam7");
	fWeaponConditionReloadJam8 = RE::GameSettingCollection::GetSingleton()->GetSetting("fWeaponConditionReloadJam8");
	fWeaponConditionReloadJam9 = RE::GameSettingCollection::GetSingleton()->GetSetting("fWeaponConditionReloadJam9");
	fWeaponConditionReloadJam10 = RE::GameSettingCollection::GetSingleton()->GetSetting("fWeaponConditionReloadJam10");

	fWeaponConditionRateOfFire1 = RE::GameSettingCollection::GetSingleton()->GetSetting("fWeaponConditionRateOfFire1");
	fWeaponConditionRateOfFire2 = RE::GameSettingCollection::GetSingleton()->GetSetting("fWeaponConditionRateOfFire2");
	fWeaponConditionRateOfFire3 = RE::GameSettingCollection::GetSingleton()->GetSetting("fWeaponConditionRateOfFire3");
	fWeaponConditionRateOfFire4 = RE::GameSettingCollection::GetSingleton()->GetSetting("fWeaponConditionRateOfFire4");
	fWeaponConditionRateOfFire5 = RE::GameSettingCollection::GetSingleton()->GetSetting("fWeaponConditionRateOfFire5");
	fWeaponConditionRateOfFire6 = RE::GameSettingCollection::GetSingleton()->GetSetting("fWeaponConditionRateOfFire6");
	fWeaponConditionRateOfFire7 = RE::GameSettingCollection::GetSingleton()->GetSetting("fWeaponConditionRateOfFire7");
	fWeaponConditionRateOfFire8 = RE::GameSettingCollection::GetSingleton()->GetSetting("fWeaponConditionRateOfFire8");
	fWeaponConditionRateOfFire9 = RE::GameSettingCollection::GetSingleton()->GetSetting("fWeaponConditionRateOfFire9");
	fWeaponConditionRateOfFire10 = RE::GameSettingCollection::GetSingleton()->GetSetting("fWeaponConditionRateOfFire10");


	//	Reload Jam Stuff
	//ItemDegredationForms.actionReloadJam = dataHandler->LookupForm<RE::BGSAction>(, MOD_ESM);

	//	Store current time
	lastTimeJammed = std::chrono::system_clock::now();

	//	Get Item Degredation misc keywords
	noItemDegredationKeyword = dataHandler->LookupForm<RE::BGSKeyword>(0x0D1BFE, MOD_ESM);

	armorBodyPartLeftArm = dataHandler->LookupForm<RE::BGSKeyword>(0x0D1C01, MOD_ESM);
	armorBodyPartRightArm = dataHandler->LookupForm<RE::BGSKeyword>(0x0D1C00, MOD_ESM);
	armorBodyPartLeftLeg = dataHandler->LookupForm<RE::BGSKeyword>(0x0D1BFF, MOD_ESM);
	armorBodyPartRightLeg = dataHandler->LookupForm<RE::BGSKeyword>(0x0D1BFB, MOD_ESM);

	//	Get INI Settings
	std::string myINI = "./Data/CapitalWasteland.ini";
	bIgnoreMaxSkillsNPC = GetPrivateProfileInt("Skills", "bIgnoreMaxSkillsNPC", 0, myINI.c_str());
	bIgnoreMaxSkillsPlayer = GetPrivateProfileInt("Skills", "bIgnoreMaxSkillsPlayer", 0, myINI.c_str());


	LOG_INFO("Item degradation forms fetched. Initializing weapon conditions...");

	InitializeWeaponConditionMappings(dataHandler);

	LOG_INFO("Weapon conditions initialized.");
}

float PArroyo::ItemDegradation::GetDegradationMapping(RE::TESAmmo* AmmoToCheck)
{
	auto result = AmmoDegradationMap.find(AmmoToCheck);
	if (AmmoDegradationMap.empty() || result == AmmoDegradationMap.end()) {
		// Did not find ammo in the map. Returning default value from Globals.
		return GetDefaultAmmoDegradation();
	}

	return result->second;
}

float PArroyo::ItemDegradation::GetDefaultAmmoDegradation()
{
	return PA_Globals.AmmoDefaultDegradation->GetValue();
}

PArroyo::ItemDegradation::PAWeaponConditionInfo PArroyo::ItemDegradation::GetCWWeaponConditionInfo(RE::TESObjectWEAP* weaponRef)
{
	if (!weaponRef)
		return PAWeaponConditionInfo();

	auto res = WeaponConditionMapping.find(weaponRef);
	if (res == WeaponConditionMapping.end())
		return PAWeaponConditionInfo();

	return res->second;
}

void PArroyo::ItemDegradation::InitializeWeaponConditionMappings(RE::TESDataHandler* dataHandler)
{
	// @TODO
}

bool PArroyo::ItemDegradation::RegisterDegradationFunctions(RE::BSScript::IVirtualMachine* vm)
{
	vm->BindNativeMethod("TCW:PArroyo", "AddWeaponRepairList", PArroyo::DegradationPapyrus::AddWeaponRepairList_Papyrus);
	vm->BindNativeMethod("TCW:PArroyo", "AddArmorRepairList", PArroyo::DegradationPapyrus::AddArmorRepairList_Papyrus);

	vm->BindNativeMethod("TCW:PArroyo", "UpdateInventoryCondition", PArroyo::DegradationPapyrus::UpdateInventoryCondition_Papyrus);
	vm->BindNativeMethod("TCW:PArroyo", "GetEquippedWeaponConditionPercent", PArroyo::DegradationPapyrus::GetEquippedWeaponConditionPercentage_Papyrus);
	vm->BindNativeMethod("TCW:PArroyo", "ModEquippedWeaponConditionPercent", PArroyo::DegradationPapyrus::ModEquippedWeaponConditionPercentage_Papyrus);

	vm->BindNativeMethod("TCW:PArroyo", "GetWeaponRepairList", PArroyo::DegradationPapyrus::GetWeaponRepairList_Papyrus);
	vm->BindNativeMethod("TCW:PArroyo", "GetWeaponRepairKeyword", PArroyo::DegradationPapyrus::GetWeaponRepairKeyword_Papyrus);
	vm->BindNativeMethod("TCW:PArroyo", "CanWeaponBeRepairedWithOther", PArroyo::DegradationPapyrus::CanWeaponBeRepairedWithOther_Papyrus);

	vm->BindNativeMethod("TCW:PArroyo", "UpdateWeaponRefStats", PArroyo::DegradationPapyrus::UpdateWeaponRefStats_Papyrus);

	vm->BindNativeMethod("TCW:PArroyo", "GetArmorRepairList", PArroyo::DegradationPapyrus::GetArmorRepairList_Papyrus);
	vm->BindNativeMethod("TCW:PArroyo", "GetArmorRepairKeyword", PArroyo::DegradationPapyrus::GetArmorRepairKeyword_Papyrus);
	vm->BindNativeMethod("TCW:PArroyo", "CanArmorBeRepairedWithOther", PArroyo::DegradationPapyrus::CanArmorBeRepairedWithOther_Papyrus);

	vm->BindNativeMethod("TCW:PArroyo", "UpdateArmorRefStats", PArroyo::DegradationPapyrus::UpdateArmorRefStats_Papyrus);

	// vm->BindNativeMethod("TCW:PArroyo", "AddItemHealthPercent", PArroyo::DegradationPapyrus::AddItemHealthPercent_Papyrus);

	return true;
}

// Papyrus Functions ==============================================================================================

bool PArroyo::DegradationPapyrus::AddWeaponRepairList_Papyrus(std::monostate, RE::BGSListForm* repairList, RE::BGSKeyword* keyword)
{
	return PArroyoSerialization::AddToWeaponRepairList(repairList, keyword);
}

bool PArroyo::DegradationPapyrus::AddArmorRepairList_Papyrus(std::monostate, RE::BGSListForm* repairList, RE::BGSKeyword* keyword)
{
	return PArroyoSerialization::AddToArmorRepairList(repairList, keyword);
}

void PArroyo::DegradationPapyrus::UpdateInventoryCondition_Papyrus(std::monostate, RE::TESObjectREFR* myRef)
{
	if (myRef == nullptr) return;

	LOG_INFO(std::format("Update Inventory Condition Called on %s from Papyrus", myRef->GetDisplayFullName()));
	InitializeInventoryCondition(myRef);
}

/*
void PArroyo::DegradationPapyrus::AddItemHealthPercent_Papyrus(std::monostate, RE::TESForm * form, RE::TESObjectREFR * containerRef, float health)
{
	switch (form->GetFormType())
	{
		case RE::ENUM_FORM_ID::kWEAP:
			WPNUtilities::AddItemHealthPercent(form, containerRef, health);
			break;

		//default:
			//Hooks::Hook_BGSInventoryListAddItem(containerRef->inventoryList, form->As<RE::TESBoundObject>(), 1, false);
			//break;
	}
}
*/

float PArroyo::DegradationPapyrus::GetEquippedWeaponConditionPercentage_Papyrus(std::monostate, RE::Actor* myActor)
{
	LOG_INFO("GetEquippedWeaponConditionPercentage called from Papyrus");

	ItemDegradation::WeaponConditionData actorData(myActor);

	if (actorData.Form == nullptr || actorData.instance == nullptr || actorData.extraData == nullptr)
		return -1.0;

	return GetWeaponConditionPercent(actorData);
}

void PArroyo::DegradationPapyrus::ModEquippedWeaponConditionPercentage_Papyrus(std::monostate, RE::Actor* myActor, float value)
{
	LOG_INFO("ModEquippedWeaponConditionPercentage called from Papyrus");

	ItemDegradation::WeaponConditionData actorData(myActor);
	RE::TESObjectWEAP* weaponObject = static_cast<RE::TESObjectWEAP*>(actorData.Form);

	auto& weapon = myActor->currentProcess->middleHigh->equippedItems[0];

	if (actorData.Form == nullptr || actorData.instance == nullptr || actorData.extraData == nullptr)
		return;

	float percentageInitial = GetWeaponConditionPercent(actorData);
	float percentageIncrease = value;

	percentageIncrease += percentageInitial;

	if (percentageIncrease > 1.0)
	{
		percentageIncrease = 1.0;
	}


	percentageIncrease = percentageIncrease > 0.0f ? percentageIncrease : 0.0f;
	if (percentageIncrease == 0.0f) {
		// Weapon breaks.
		RE::ActorEquipManager::GetSingleton()->UnequipItem(myActor, &weapon, false);
		RE::GameSettingCollection* gameSettingCollection = RE::GameSettingCollection::GetSingleton();
		RE::SendHUDMessage::ShowHUDMessage(gameSettingCollection->GetSetting("sWeaponBreak")->GetString().data(), "00DCUIWeaponBreak", true, true);
	}
	else if (percentageIncrease <= 0.0)
	{
		percentageIncrease = 0.0;

		myActor->inventoryList->rwLock.unlock_read();
		RE::ActorEquipManager::GetSingleton()->UnequipItem(myActor, &weapon, false);
		myActor->inventoryList->rwLock.lock_read();
	}

	weaponObject->weaponData.attackDelaySec = WPNUtilities::CalculateUpdatedRateOfFireValue(weaponObject, &weaponObject->weaponData, percentageIncrease);
	actorData.extraData->SetHealthPerc(percentageIncrease);

	RE::PipboyDataManager::GetSingleton()->inventoryData.RepopulateItemCardOnSection(RE::ENUM_FORM_ID::kWEAP);

	LOG_INFO(std::format("Initial Condition = {}, New Condition = {}", percentageInitial, percentageIncrease));

	// @TODO
	// WPNUtilities::UpdateWeaponStats(actorData);

	if (myActor == RE::PlayerCharacter::GetSingleton())
	{
		WPNUtilities::UpdateHUDCondition(actorData);
	}
}

RE::BGSListForm* PArroyo::DegradationPapyrus::GetWeaponRepairList_Papyrus(std::monostate, RE::TESObjectWEAP* myWeapon)
{
	RE::BGSKeyword* myRepairKeyword = WPNUtilities::GetWeaponRepairKeyword(myWeapon);

	if (myRepairKeyword)
	{
		return WPNUtilities::GetLinkedWeaponRepairListFromKeyword(myRepairKeyword);
	}

	return NULL;
}

RE::BGSKeyword* PArroyo::DegradationPapyrus::GetWeaponRepairKeyword_Papyrus(std::monostate, RE::TESObjectWEAP* myWeapon)
{
	return WPNUtilities::GetWeaponRepairKeyword(myWeapon);
}

bool PArroyo::DegradationPapyrus::CanWeaponBeRepairedWithOther_Papyrus(std::monostate, RE::TESObjectWEAP* weapon1, RE::TESObjectWEAP* weapon2)
{
	return WPNUtilities::CanWeaponBeRepairedWithOther(weapon1, weapon2);
}

void PArroyo::DegradationPapyrus::UpdateWeaponRefStats_Papyrus(std::monostate, RE::TESObjectREFR* weaponRef)
{
	InitializeWeaponCondition(weaponRef);
}

RE::BGSListForm* PArroyo::DegradationPapyrus::GetArmorRepairList_Papyrus(std::monostate, RE::TESObjectARMO* myArmor)
{
	RE::BGSKeyword* myRepairKeyword = ARMOUtilities::GetArmorRepairKeyword(myArmor);

	if (myRepairKeyword)
	{
		return ARMOUtilities::GetLinkedArmorRepairListFromKeyword(myRepairKeyword);
	}

	return nullptr;
}

RE::BGSKeyword* PArroyo::DegradationPapyrus::GetArmorRepairKeyword_Papyrus(std::monostate, RE::TESObjectARMO* myArmor)
{
	return ARMOUtilities::GetArmorRepairKeyword(myArmor);
}

bool PArroyo::DegradationPapyrus::CanArmorBeRepairedWithOther_Papyrus(std::monostate, RE::TESObjectARMO* armor1, RE::TESObjectARMO* armor2)
{
	return ARMOUtilities::CanArmorBeRepairedWithOther(armor1, armor2);
}

void PArroyo::DegradationPapyrus::UpdateArmorRefStats_Papyrus(std::monostate, RE::TESObjectREFR* armorRef)
{
	InitializeArmorCondition(armorRef);
}

// WEAPON UTILS =======================================================================================================================

bool PArroyo::WPNUtilities::UpdateHUDCondition(ItemDegradation::WeaponConditionData myConditionData)
{

	RE::BSFixedString menuString("HUDMenu");
	if (RE::UI::GetSingleton()->GetMenuOpen(menuString)) {

		auto myHudMenu = RE::UI::GetSingleton()->GetMenu(menuString).get();
		Scaleform::GFx::Value myConditionValue[1];

		float conditionValue = GetWeaponConditionPercent(myConditionData);

		myConditionValue[0] = Scaleform::GFx::Value(conditionValue);

		const bool result = myHudMenu->uiMovie->asMovieRoot->Invoke("root.CWHUD_loader.content.SetCondition", nullptr, myConditionValue, 1);

		return result;
	}
	// auto myMovieRoot = myHUDMenu->uiMovie->asMovieRoot;

	return false;

}

float PArroyo::WPNUtilities::CalculateSkillBonusFromActor(ItemDegradation::WeaponConditionData myConditionData)
{
	using namespace ItemDegradation;

	float actorSkillValue = 1.0;

	RE::Actor* actor = myConditionData.actor;

	RE::TESObjectWEAP* myWeapon = static_cast<RE::TESObjectWEAP*>(myConditionData.Form);

	if (myWeapon->weaponData.skill != nullptr)
	{
		if (actor == nullptr)
		{
			//	use player instead
			actor = RE::PlayerCharacter::GetSingleton();
		}

		actorSkillValue = (GetAVValue(actor, myWeapon->weaponData.skill) / 100);
	}

	if (actorSkillValue > 1.0)
	{
		if (actor == RE::PlayerCharacter::GetSingleton())
		{
			if (!bIgnoreMaxSkillsPlayer)
			{
				actorSkillValue = 1.0;
			}
		}
		else
		{
			if (!bIgnoreMaxSkillsNPC)
			{
				actorSkillValue = 1.0;
			}
		}
	}

	float result = (fDamageSkillBase + fDamageSkillMult * actorSkillValue);

	//REX::WARN("Calculate Skill Bonus From Actor = %f", result);

	return result;
}

float PArroyo::WPNUtilities::CalculateUpdatedDamageValue(float baseDamage, float minimum, float conditionPercent, float skillBonus)
{
	return (baseDamage * (minimum + conditionPercent * (1 - minimum)) * skillBonus);
}

float PArroyo::WPNUtilities::GetWeaponDamage(ItemDegradation::WeaponConditionData myConditionData)
{
	if (myConditionData.instance == nullptr)
	{
		return 0.0;
	}

	float baseDamage = myConditionData.instance->attackDamage;
	const float newMaxCondition = myConditionData.extraData->GetHealthPerc();

	if (newMaxCondition != -1.0f || newMaxCondition < 0.75f)
		baseDamage = baseDamage * (0.5f + min((0.5f * newMaxCondition) / 0.75f, 0.5f));

	return baseDamage;
}

double PArroyo::WPNUtilities::CalculateUpdatedRateOfFireValue(ItemDegradation::WeaponConditionData myConditionData, double currentCondition)
{
	return CalculateUpdatedRateOfFireValue(myConditionData.Form, myConditionData.instance, currentCondition);
}

double PArroyo::WPNUtilities::CalculateUpdatedRateOfFireValue(RE::TESForm* weaponForm, RE::TESObjectWEAP::InstanceData* weaponInstanceData, double currentCondition)
{
	using namespace ItemDegradation;
	// the calculation here is a bit weird in FO4, Fire Rate is linked to attack delay, with attack delay, the lower it is the higher the Fire Rate
		// instead of multiplying the base attack delay by a set amount instead we divide to make attack delay longer and in turn Fire Rate lower

	RE::TESObjectWEAP* baseWPNForm = static_cast<RE::TESObjectWEAP*>(weaponForm);

	if (!baseWPNForm->IsRangedWeapon() || baseWPNForm->weaponData.type == RE::WEAPON_TYPE::kGrenade || baseWPNForm->weaponData.type == RE::WEAPON_TYPE::kMine)
		return -1.0f;

	auto baseROF = GetCWWeaponConditionInfo(baseWPNForm);	//	unkC0 == fAttackSeconds;
	if (baseROF.BaseAttackDelay < 0) {
		LOG_WARNING(std::format("Weapon '{}' is not in WeaponConditionMapping. Ignoring Rate of Fire change...", weaponForm->GetFormEditorID()).c_str());
		return -1.0f;
	}

	double result = 1.0f;

	//if (baseROF == 0)
		//baseROF = 0.1f;

	//REX::WARN("%s: Base Attack Delay = %f", baseWPNForm->GetFullName(), baseROF);

	if (currentCondition <= 1.0 && currentCondition > 0.9)
	{
		result = fWeaponConditionRateOfFire10->GetFloat();
	}
	else if (currentCondition <= 0.9 && currentCondition > 0.8)
	{
		result = fWeaponConditionRateOfFire9->GetFloat();
	}
	else if (currentCondition <= 0.8 && currentCondition > 0.7)
	{
		result = fWeaponConditionRateOfFire8->GetFloat();
	}
	else if (currentCondition <= 0.7 && currentCondition > 0.6)
	{
		result = fWeaponConditionRateOfFire7->GetFloat();
	}
	else if (currentCondition <= 0.6 && currentCondition > 0.5)
	{
		result = fWeaponConditionRateOfFire6->GetFloat();
	}
	else if (currentCondition <= 0.5 && currentCondition > 0.4)
	{
		result = fWeaponConditionRateOfFire5->GetFloat();
	}
	else if (currentCondition <= 0.4 && currentCondition > 0.3)
	{
		result = fWeaponConditionRateOfFire4->GetFloat();
	}
	else if (currentCondition <= 0.3 && currentCondition > 0.2)
	{
		result = fWeaponConditionRateOfFire3->GetFloat();
	}
	else if (currentCondition <= 0.2 && currentCondition > 0.1)
	{
		result = fWeaponConditionRateOfFire2->GetFloat();
	}
	else if (currentCondition <= 0.1 && currentCondition > 0.0)
	{
		result = fWeaponConditionRateOfFire1->GetFloat();
	}


	//LOG_INFO(std::format("1: {} | 2: {}| 3: {}| 4: {}| 5: {}| 6: {}| 7: {}| 8: {}| 9: {}| 10: {}| ", fWeaponConditionRateOfFire1->GetFloat(), fWeaponConditionRateOfFire2->GetFloat(), fWeaponConditionRateOfFire3->GetFloat(), fWeaponConditionRateOfFire4->GetFloat(), fWeaponConditionRateOfFire5->GetFloat(),
	//	fWeaponConditionRateOfFire6->GetFloat(), fWeaponConditionRateOfFire7->GetFloat(), fWeaponConditionRateOfFire8->GetFloat(), fWeaponConditionRateOfFire9->GetFloat(), fWeaponConditionRateOfFire10->GetFloat()).c_str());

	// result = baseROF / result;
	/*
	if (baseWPNForm->weaponData.flags.underlying() & (std::uint32_t)RE::WEAPON_FLAGS::kAutomatic) {
		weaponInstanceData->speed = result;
		weaponInstanceData->attackSeconds = weaponInstanceData->attackDelaySec / result;

	}
	else {

	}
	*/
	weaponInstanceData->attackDelaySec = baseROF.BaseAttackDelay / result;
	baseWPNForm->weaponData.attackDelaySec = weaponInstanceData->attackDelaySec;
	//baseWPNForm->weaponData.attackSeconds = weaponInstanceData->attackSeconds;
	weaponInstanceData->speed = result;
	baseWPNForm->weaponData.speed = weaponInstanceData->speed;


	//REX::WARN("%s: Updated Attack Delay = %f", baseWPNForm->GetFullName(), result);

	return result;
}

RE::TESForm* PArroyo::WPNUtilities::GetEquippedWeaponForm(RE::Actor* actor)
{
	if (!actor->currentProcess || !actor->currentProcess->middleHigh || actor->currentProcess->middleHigh->equippedItems.size() == 0)
		return nullptr;

	return actor->currentProcess->middleHigh->equippedItems[0].item.object;
}

bool PArroyo::WPNUtilities::IsMeleeWeapon(ItemDegradation::WeaponConditionData myConditionData)
{
	return IsMeleeWeapon(myConditionData.instance->type.get());
}

bool PArroyo::WPNUtilities::IsMeleeWeapon(RE::WEAPON_TYPE weaponType)
{
	bool result = false;

	switch (weaponType)
	{
	case RE::WEAPON_TYPE::kHandToHand:
	case RE::WEAPON_TYPE::kOneHandAxe:
	case RE::WEAPON_TYPE::kOneHandDagger:
	case RE::WEAPON_TYPE::kOneHandMace:
	case RE::WEAPON_TYPE::kOneHandSword:
	case RE::WEAPON_TYPE::kStaff:
	case RE::WEAPON_TYPE::kTwoHandAxe:
	case RE::WEAPON_TYPE::kTwoHandSword:
		result = true;
		break;
	default:
		result = false;
		break;
	}

	return result;
}


/*
RE::TESObjectWEAP::InstanceData* PArroyo::WPNUtilities::CastInstanceData(RE::TBO_InstanceData * myInstanceData)
{
	return nullptr;
}
*/

RE::BGSKeyword* PArroyo::WPNUtilities::GetWeaponRepairKeyword(RE::TESObjectWEAP* myWeapon)
{
	if (!myWeapon)
		return nullptr;

	RepairListMap weaponRepairListMap = PArroyoSerialization::GetWeaponRepairList();

	for (auto it = weaponRepairListMap.begin(); it != weaponRepairListMap.end(); ++it)
	{
		if (myWeapon->HasKeyword(it->second))
			return it->second;
	}

	return nullptr;
}

RE::BGSListForm* PArroyo::WPNUtilities::GetLinkedWeaponRepairListFromKeyword(RE::BGSKeyword* myKeyword)
{
	if (!myKeyword)
		return nullptr;

	RepairListMap weaponRepairListMap = PArroyoSerialization::GetWeaponRepairList();

	for (auto it = weaponRepairListMap.begin(); it != weaponRepairListMap.end(); ++it)
	{
		if (it->second == myKeyword)
			return it->first;
	}

	return nullptr;
}

RE::BGSKeyword* PArroyo::WPNUtilities::GetLinkedWeaponRepairKeywordFromFormList(RE::BGSListForm* myList)
{
	if (!myList)
		return nullptr;

	RepairListMap weaponRepairListMap = PArroyoSerialization::GetWeaponRepairList();

	for (auto it = weaponRepairListMap.begin(); it != weaponRepairListMap.end(); ++it)
	{
		if (it->first == myList)
			return it->second;
	}

	return nullptr;
}

bool PArroyo::WPNUtilities::CanWeaponBeRepairedWithOther(RE::TESObjectWEAP* weapon1, RE::TESObjectWEAP* weapon2)
{
	if (weapon1 && weapon2)
	{
		RE::BGSKeyword* myArmorRepairKeyword = GetWeaponRepairKeyword(weapon1);

		if (myArmorRepairKeyword)
		{
			RE::BGSListForm* myArmorRepairList = GetLinkedWeaponRepairListFromKeyword(myArmorRepairKeyword);

			for (const RE::TESForm* weapon : myArmorRepairList->arrayOfForms) {
				if (weapon == weapon2)
					return true;
			}
		}

		else if (weapon1 == weapon2)
			return true;
	}

	return false;
}

// ARMOR UTILS =======================================================================================================================

RE::BGSKeyword* PArroyo::ARMOUtilities::GetPowerArmorTypeKeyword()
{
	return ItemDegradation::armorTypePower;
}

RE::BGSKeyword* PArroyo::ARMOUtilities::GetArmorRepairKeyword(RE::TESObjectARMO* myArmor)
{
	if (!myArmor)
		return nullptr;

	RepairListMap armorRepairListMap = PArroyoSerialization::GetArmorRepairList();

	for (auto it = armorRepairListMap.begin(); it != armorRepairListMap.end(); ++it)
	{
		if (myArmor->HasKeyword(it->second))
			return it->second;
	}

	return nullptr;
}

RE::BGSListForm* PArroyo::ARMOUtilities::GetLinkedArmorRepairListFromKeyword(RE::BGSKeyword* myKeyword)
{
	if (!myKeyword)
		return nullptr;

	RepairListMap armorRepairListMap = PArroyoSerialization::GetArmorRepairList();

	for (auto it = armorRepairListMap.begin(); it != armorRepairListMap.end(); ++it)
	{
		if (it->second == myKeyword)
			return it->first;
	}

	return nullptr;
}

RE::BGSKeyword* PArroyo::ARMOUtilities::GetLinkedArmorRepairKeywordFromFormList(RE::BGSListForm* myList)
{
	if (!myList)
		return nullptr;

	RepairListMap armorRepairListMap = PArroyoSerialization::GetArmorRepairList();

	for (auto it = armorRepairListMap.begin(); it != armorRepairListMap.end(); ++it)
	{
		if (it->first == myList)
			return it->second;
	}

	return nullptr;
}

bool PArroyo::ARMOUtilities::CanArmorBeRepairedWithOther(RE::TESObjectARMO* armor1, RE::TESObjectARMO* armor2)
{
	if (armor1 && armor2)
	{
		RE::BGSKeyword* myArmorRepairKeyword = GetArmorRepairKeyword(armor1);

		if (myArmorRepairKeyword)
		{
			RE::BGSListForm* myArmorRepairList = GetLinkedArmorRepairListFromKeyword(myArmorRepairKeyword);

			for (const RE::TESForm* armour : myArmorRepairList->arrayOfForms) {
				if (armour == armor2)
					return true;
			}
		}

		else if (armor1 == armor2)
			return true;
	}

	return false;
}



void PArroyo::ARMOUtilities::UpdateArmorStats(ItemDegradation::ArmorConditionData myConditionData)
{
	if (myConditionData.instance == nullptr)
	{
		return;
	}

	RE::TESObjectARMO* baseARMOForm = static_cast<RE::TESObjectARMO*>(myConditionData.Form);
	RE::TESObjectARMO::InstanceData* myInstance = myConditionData.instance;

	//	Ignore No Item Degredation keyworded items
	if (ArmorHasKeyword(baseARMOForm, Shared::noDegradation))
	{
		return;
	}

	//	Check if has Armor Body Part Keywords
	bool bHasBodyPartKeyword = false;

	if (ArmorHasKeyword(baseARMOForm, ItemDegradation::armorBodyPartChest))
		bHasBodyPartKeyword = true;

	if (ArmorHasKeyword(baseARMOForm, ItemDegradation::armorBodyPartHead))
		bHasBodyPartKeyword = true;

	if (ArmorHasKeyword(baseARMOForm, ItemDegradation::armorBodyPartLeftArm))
		bHasBodyPartKeyword = true;

	if (ArmorHasKeyword(baseARMOForm, ItemDegradation::armorBodyPartLeftLeg))
		bHasBodyPartKeyword = true;

	if (ArmorHasKeyword(baseARMOForm, ItemDegradation::armorBodyPartRightArm))
		bHasBodyPartKeyword = true;

	if (ArmorHasKeyword(baseARMOForm, ItemDegradation::armorBodyPartRightLeg))
		bHasBodyPartKeyword = true;

	if (!bHasBodyPartKeyword)
	{
		std::string armorFormIDString = GetFormIDAsString(baseARMOForm->formID);
		if (!IsFormIDStringBaseGame(armorFormIDString))
			REX::WARN("CONDITION: Trying to update a armor (%s) that has no Armor Condition Keywords. Please set up in the Creation Kit!", armorFormIDString.c_str());
		return;
	}

	float currentCondition = GetArmorConditionPercent(myConditionData);

	if (currentCondition == -1.0)
	{
		//	This will likely happen with armors not set up for Armor Condition (ignoring Base game and DLCs for now)
		//	try and initialise condition anyway as a failsafe
		InitializeArmorCondition(myConditionData);

		if (currentCondition == -1.0)
		{
			std::string armorFormIDString = GetFormIDAsString(baseARMOForm->formID);
			if (!IsFormIDStringBaseGame(armorFormIDString))
				REX::WARN("CONDITION: Trying to update a armor (%s) that has no Armor Condition Health. Please set up in the Creation Kit!", armorFormIDString.c_str());
			return;
		}
	}

	float baseValue = (float)baseARMOForm->armorData.value;

	//	Value
	float newValue = (baseValue * (pow(currentCondition, 1.5)));
	myInstance->value = (std::uint32_t)newValue;

	//	DR
	float maxDR = (float)baseARMOForm->armorData.rating;
	float newDR = ((0.2 * maxDR) + (currentCondition * 100) * ((0.8 * maxDR) / 100));
	//REX::WARN("Armor (%s): Max DR = %.4f, Health = %.4f%%, Current DR = %.4f", baseARMOForm->GetFullName(), maxDR, (currentCondition * 100), newDR);
	if (newDR < 1.0)
	{
		// minimum DR should be 1 so we don't have 0 DR on an item
		newDR = 1.0;
	}
	myInstance->rating = (std::uint16_t)newDR;
}


void PArroyo::ARMOUtilities::UpdateArmorStatsOnHit(RE::Actor* actor, std::uint32_t eDamageLimb, const bool isMelee, const bool isUnarmed, float fDamage)
{
	RE::BGSKeyword* checkKeyword = nullptr;

	if (actor != RE::PlayerCharacter::GetSingleton())
	{
		//	Ignoring any NPCs
		return;
	}

	bool isPowerArmor = RE::PowerArmor::ActorInPowerArmor(*actor);

	switch (static_cast<RE::BGSBodyPartDefs::LIMB_ENUM>(eDamageLimb))
	{
	case RE::BGSBodyPartDefs::LIMB_ENUM::kHead1:
	case RE::BGSBodyPartDefs::LIMB_ENUM::kHead2:
	case RE::BGSBodyPartDefs::LIMB_ENUM::kEye1:
	case RE::BGSBodyPartDefs::LIMB_ENUM::kBrain:
	{
		//_MESSAGE("Hit in Head");
		checkKeyword = ItemDegradation::armorBodyPartHead;
		break;
	}
	case RE::BGSBodyPartDefs::LIMB_ENUM::kTorso:
	case RE::BGSBodyPartDefs::LIMB_ENUM::kPelvis:
	{
		//_MESSAGE("Hit in Body");
		checkKeyword = ItemDegradation::armorBodyPartChest;
		break;
	}
	case RE::BGSBodyPartDefs::LIMB_ENUM::kLeftArm1:
	case RE::BGSBodyPartDefs::LIMB_ENUM::kLeftArm2:
	{
		//_MESSAGE("Hit in Left Arm");
		if (isPowerArmor)
		{
			checkKeyword = ItemDegradation::armorBodyPartLeftArm;
		}
		else
		{
			checkKeyword = ItemDegradation::armorBodyPartChest;
		}
		break;
	}
	case RE::BGSBodyPartDefs::LIMB_ENUM::kRightArm1:
	case RE::BGSBodyPartDefs::LIMB_ENUM::kRightArm2:
	{
		//_MESSAGE("Hit in Right Arm");
		if (isPowerArmor)
		{
			checkKeyword = ItemDegradation::armorBodyPartRightArm;
		}
		else
		{
			checkKeyword = ItemDegradation::armorBodyPartChest;
		}
		break;
	}
	case RE::BGSBodyPartDefs::LIMB_ENUM::kLeftLeg1:
	case RE::BGSBodyPartDefs::LIMB_ENUM::kLeftLeg2:
	case RE::BGSBodyPartDefs::LIMB_ENUM::kLeftLeg3:
	case RE::BGSBodyPartDefs::LIMB_ENUM::kLeftFoot:
	{
		//_MESSAGE("Hit in Left Leg");
		if (isPowerArmor)
		{
			checkKeyword = ItemDegradation::armorBodyPartLeftLeg;
		}
		else
		{
			checkKeyword = ItemDegradation::armorBodyPartChest;
		}
		break;
	}
	case RE::BGSBodyPartDefs::LIMB_ENUM::kRightLeg1:
	case RE::BGSBodyPartDefs::LIMB_ENUM::kRightLeg2:
	case RE::BGSBodyPartDefs::LIMB_ENUM::kRightLeg3:
	case RE::BGSBodyPartDefs::LIMB_ENUM::kRightFoot:
	{
		//_MESSAGE("Hit in Right Leg");
		if (isPowerArmor)
		{
			checkKeyword = ItemDegradation::armorBodyPartRightLeg;
		}
		else
		{
			checkKeyword = ItemDegradation::armorBodyPartChest;
		}
		break;
	}
	case RE::BGSBodyPartDefs::LIMB_ENUM::kWeapon:

	default:
	{
		//_MESSAGE("Unknown Hit Type, Assuming Body");
		checkKeyword = ItemDegradation::armorBodyPartChest;
		break;
	}
	}

	if (checkKeyword)
	{
		RE::BGSInventoryItem invItem = GetEquippedArmorInventoryItemBasedOnKeyword(actor, checkKeyword);

		if (invItem.object)
		{
			//_MESSAGE("Got Form 0x%08X for Armor Degredation", invItem.form->formID);
			ItemDegradation::ArmorConditionData conditionData = ItemDegradation::ArmorConditionData(actor, invItem.object, invItem.stackData->extra.get());
			if (!isUnarmed)
			{
				ItemDegradation::ArmorDegrade degrade{};
				degrade.fDamageDealt = fDamage;
				degrade.isMelee = isMelee;


				ModArmorCondition(conditionData, degrade);
			}
		}
	}
}

void PArroyo::ARMOUtilities::ModArmorCondition(ItemDegradation::ArmorConditionData conditionData, ItemDegradation::ArmorDegrade degradeData)
{
	/*
	double armourHealth = conditionData.extraData->GetHealthPerc();
	RE::TESObjectARMO* armor = static_cast<RE::TESObjectARMO*>(conditionData.Form);
	if (armourHealth > 0.0f) {
		REX::DEBUG("Armor condition of '{}' before degradation: {}", armor->GetFormEditorID(), armourHealth);

		const double degradationAmount = (degradeData.fDamageDealt - DR_ForHitLimb) / DR_ForHitLimb * Shared::fArmourConditionReductionPerPercentage->GetValue();
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
	else {
		InitializeArmorCondition(conditionData);
	}
	*/
}

std::vector<RE::BGSInventoryItem> PArroyo::ARMOUtilities::GetEquippedArmorInventoryItems(RE::Actor* actor)
{
	std::vector<RE::BGSInventoryItem> result{};

	if (!actor)
	{
		actor = RE::PlayerCharacter::GetSingleton();
	}

	auto inventory = actor->inventoryList;
	if (inventory)
	{
		inventory->rwLock.lock_read();
		inventory->rwLock.lock_write();

		for (int i = 0; i < inventory->data.size(); i++)
		{
			RE::BGSInventoryItem iter = inventory->data[i];

			if (iter.stackData->IsEquipped() && iter.object->GetFormType() == RE::ENUM_FORM_ID::kARMO)
			{
				result.push_back(iter);
			}
		}

		inventory->rwLock.unlock_read();
		inventory->rwLock.unlock_write();
	}


	return result;
}

RE::BGSInventoryItem PArroyo::ARMOUtilities::GetEquippedArmorInventoryItemBasedOnKeyword(RE::Actor* actor, RE::BGSKeyword* keyword)
{
	std::vector<RE::BGSInventoryItem> equippedArmors = GetEquippedArmorInventoryItems(actor);

	RE::BGSInventoryItem result{};

	for (int i = 0; i < equippedArmors.size(); i++)
	{
		RE::BGSInventoryItem invItem = equippedArmors[i];
		RE::TESObjectARMO* armor = static_cast<RE::TESObjectARMO*>(invItem.object);

		if (armor)
		{
			if (ArmorHasKeyword(armor, keyword))
			{
				return invItem;
			}
		}
	}

	return result;
}