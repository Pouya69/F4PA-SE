#pragma once




namespace Skills {

	extern RE::BSTArray<RE::BGSPerk*> PAPerksLevelUp;
	extern RE::BSTArray<RE::ActorValueInfo*> PASkillsLevelUp;
	extern RE::BSTArray<RE::BGSSoundDescriptorForm*> ScaleformSkillSounds;
	extern RE::BSTArray<RE::BGSSoundDescriptorForm*> ScaleformPerkSounds;

	extern std::vector<RE::BGSPerk*> PANPCPerksList;
	extern std::vector<RE::ActorValueInfo*> SkillsList;
	extern std::unordered_map<std::string, RE::ActorValueInfo*> strSkillMap;

	extern std::unordered_map<const RE::ActorValueInfo*, std::vector<RE::ActorValueInfo*>> skillsToSpecialMap;
	extern std::unordered_map<const RE::ActorValueInfo*, std::vector<RE::ActorValueInfo*>> specialToSkillsMap;

	extern std::unordered_map<std::string, RE::ActorValueInfo*> PerksMap;

	void RegisterForSkillLink();

	std::vector<RE::ActorValueInfo*>* GetDependantAVs(const RE::ActorValueInfo* a_info);

	// Used for calculating the skill based on Fallout 3's formula. (Dependant * 2) + 2 + (Luck / 2)
	float CalculateSkillOffset(const RE::ActorValueOwner* a_actor, const RE::ActorValueInfo& a_info);

	void RegisterLinkedAV(RE::ActorValueInfo* AV, RE::ActorValueInfo::DerivationFunction_t* CalcFunction, RE::ActorValueInfo* linkedToSPECIAL, RE::ActorValueInfo* linkedToSPECIAL_02 = nullptr);
}

namespace SkillOffsetsCalcFunctions {

	float CalculateSmallGunsOffset(const RE::ActorValueOwner* a_actor, const RE::ActorValueInfo& a_info);

	float CalculateBigGuns(const RE::ActorValueOwner* a_actor, const RE::ActorValueInfo& a_info);

	float CalculateEnergyWeapons(const RE::ActorValueOwner* a_actor, const RE::ActorValueInfo& a_info);

	float CalculateThrowing(const RE::ActorValueOwner* a_actor, const RE::ActorValueInfo& a_info);

	float CalculateUnarmed(const RE::ActorValueOwner* a_actor, const RE::ActorValueInfo& a_info);

	float CalculateMelee(const RE::ActorValueOwner* a_actor, const RE::ActorValueInfo& a_info);

	float CalculateFirstAid(const RE::ActorValueOwner* a_actor, const RE::ActorValueInfo& a_info);

	float CalculateDoctor(const RE::ActorValueOwner* a_actor, const RE::ActorValueInfo& a_info);

	float CalculateSneak(const RE::ActorValueOwner* a_actor, const RE::ActorValueInfo& a_info);

	float CalculateLockpick(const RE::ActorValueOwner* a_actor, const RE::ActorValueInfo& a_info);

	float CalculateSteal(const RE::ActorValueOwner* a_actor, const RE::ActorValueInfo& a_info);

	float CalculateTraps(const RE::ActorValueOwner* a_actor, const RE::ActorValueInfo& a_info);

	float CalculateScience(const RE::ActorValueOwner* a_actor, const RE::ActorValueInfo& a_info);

	float CalculateRepair(const RE::ActorValueOwner* a_actor, const RE::ActorValueInfo& a_info);

	float CalculateSpeech(const RE::ActorValueOwner* a_actor, const RE::ActorValueInfo& a_info);

	float CalculateBarter(const RE::ActorValueOwner* a_actor, const RE::ActorValueInfo& a_info);

	float CalculateGambling(const RE::ActorValueOwner* a_actor, const RE::ActorValueInfo& a_info);

	float CalculateOutdoorsman(const RE::ActorValueOwner* a_actor, const RE::ActorValueInfo& a_info);
}

namespace PA_SkillsPapyrus {

	RE::ActorValueInfo* GetSkillByName_Papyrus(std::monostate, RE::BSFixedString skillToGet);
	uint32_t GetSkillValueByName_Papyrus(std::monostate, RE::Actor* Actor, RE::BSFixedString skillToGet);
	uint32_t GetBaseSkillValueByName_Papyrus(std::monostate, RE::Actor* Actor, RE::BSFixedString skillToGet);
	void ModPermanentSkillValue_Papyrus(std::monostate, RE::TESForm* Actor, RE::ActorValueInfo* AVToModify, float modAmount);

	// Debug
	void DEBUG_LogSkillsToConsole_Papyrus(std::monostate, RE::Actor* Actor);
	inline void DEBUG_LogSkillToConsole_Papyrus(std::monostate, RE::Actor* Actor, RE::ActorValueInfo* AV);
}

void InitializeGameVariables(RE::TESDataHandler* dataHandler, std::string mod_esm);
bool RegisterSkillFunctions(RE::BSScript::IVirtualMachine& vm);
void Initialize_TOBEMERGED_Vars(RE::TESDataHandler* dataHandler);

void InitializeGlobalVariables(RE::TESDataHandler* dataHandler);

void InitializePerks(RE::TESDataHandler* dataHandler, std::string mod_esm);

// Internal Functions
// ===========================================================================

void GetLevelUpFormsFromGame();

float GetAVValue(RE::Actor* Actor, RE::ActorValueInfo* valueInfo);
float GetBaseAVValue(RE::Actor* Actor, RE::ActorValueInfo* valueInfo);
void ModBaseAVValue(RE::Actor* Actor, RE::ActorValueInfo* valueInfo, int modAmount);
void SetBaseAVValue(RE::Actor* Actor, RE::ActorValueInfo* valueInfo, int setAmount);
void ModPermanentSkillValue(RE::TESForm* Actor, RE::ActorValueInfo* AVToModify, float modAmount);

RE::ActorValueInfo* GetSkillByName(std::string skillName);


float GetPlayerAVValue(RE::ActorValueInfo* valueInfo);
float GetPlayerBaseAVValue(RE::ActorValueInfo* valueInfo);
void ModPlayerBaseAVValue(RE::ActorValueInfo* valueInfo, int modAmount);
void SetPlayerBaseAVValue(RE::Actor* Actor, RE::ActorValueInfo* valueInfo, int setAmount);
