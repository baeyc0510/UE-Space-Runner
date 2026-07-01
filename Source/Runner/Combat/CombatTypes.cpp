// Fill out your copyright notice in the Description page of Project Settings.

#include "CombatTypes.h"

/*~ GameplayTags - Data (메타 태그: Context 생성 시 설정) ~*/

UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Data_Critical, "Data.Critical", "크리티컬 발생 여부");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Data_Cooldown, "Data.Cooldown", "쿨타임 Set by Caller");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Data_BaseDamage, "Data.BaseDamage", "기본 데미지");

/*~ GameplayTags - Stat (캐릭터 고유 스탯) ~*/

UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Stat_MaxHealth, "Stat.MaxHealth", "최대 체력");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Stat_CurrentHealth, "Stat.CurrentHealth", "현재 체력");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Stat_Defense, "Stat.Defense", "방어력");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Stat_DamageBonus, "Stat.DamageBonus", "데미지 보너스%");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Stat_CriticalChance, "Stat.CriticalChance", "크리티컬 확률%");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Stat_CriticalBonus, "Stat.CriticalBonus", "크리티컬 데미지 보너스%");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Stat_LifeSteal, "Stat.LifeSteal", "흡혈 비율%");
