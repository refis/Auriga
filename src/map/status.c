/*
 * Copyright (C) 2002-2007  Auriga
 *
 * This file is part of Auriga.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include <time.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <math.h>

#include "db.h"
#include "malloc.h"
#include "nullpo.h"
#include "timer.h"
#include "utils.h"

#include "pc.h"
#include "map.h"
#include "path.h"
#include "pet.h"
#include "homun.h"
#include "mob.h"
#include "clif.h"
#include "skill.h"
#include "itemdb.h"
#include "battle.h"
#include "status.h"
#include "script.h"
#include "guild.h"
#include "unit.h"
#include "ranking.h"
#include "merc.h"
#include "elem.h"
#include "msg.h"
#include "bonus.h"

static struct job_db {
	int max_weight_base;
	int hp_base[MAX_LEVEL];
	int sp_base[MAX_LEVEL];
	int bonus[PC_UPPER_MAX][MAX_LEVEL];
	int aspd_base[WT_MAX+1];
} job_db[PC_JOB_MAX];

static int atkmods[MAX_SIZE_FIX][WT_MAX];	// ����ATK�T�C�Y�C��(size_fix.txt)

static struct refine_db {
	int safety_bonus[MAX_REFINE];
	int over_bonus;
	int limit;
	int per[MAX_REFINE];
} refine_db[MAX_WEAPON_LEVEL+1];

int current_equip_item_index;	// �X�e�[�^�X�v�Z�p
int current_equip_name_id;

#ifdef DYNAMIC_SC_DATA
struct status_change_data dummy_sc_data[MAX_STATUSCHANGE];
#endif

static int status_calc_amotion_pc(struct map_session_data *sd);	// PC�pamotion�v�Z
static int status_calc_speed_pc(struct map_session_data *sd,int speed);	// PC�pspeed�v�Z

static struct scdata_db scdata_db[MAX_STATUSCHANGE];	// �X�e�[�^�X�ُ�f�[�^�x�[�X

static int StatusIconChangeTable[MAX_STATUSCHANGE] = {
	/* 0- */
	SI_PROVOKE,SI_ENDURE,SI_TWOHANDQUICKEN,SI_CONCENTRATE,SI_BLANK,SI_CLOAKING,SI_ENCPOISON,SI_POISONREACT,SI_QUAGMIRE,SI_ANGELUS,
	/* 10- */
	SI_BLESSING,SI_SIGNUMCRUCIS,SI_INCREASEAGI,SI_DECREASEAGI,SI_SLOWPOISON,SI_IMPOSITIO,SI_SUFFRAGIUM,SI_ASPERSIO,SI_BENEDICTIO,SI_KYRIE,
	/* 20- */
	SI_MAGNIFICAT,SI_GLORIA,SI_AETERNA,SI_ADRENALINE,SI_WEAPONPERFECTION,SI_OVERTHRUST,SI_MAXIMIZEPOWER,SI_BLANK,SI_BLANK,SI_TRICKDEAD,
	/* 30- */
	SI_LOUD,SI_ENERGYCOAT,SI_BLANK,SI_BLANK,SI_BLANK,SI_BLANK,SI_BLANK,SI_SPEEDPOTION0,SI_SPEEDPOTION1,SI_SPEEDPOTION2,
	/* 40- */
	SI_MOVHASTE_HORSE,SI_MOVHASTE_POT,SI_BLANK,SI_MOVESLOW_POTION,SI_BLANK,SI_BLANK,SI_BLANK,SI_BLANK,SI_BLANK,SI_BLANK,
	/* 50- */
	SI_STRIPWEAPON,SI_STRIPSHIELD,SI_STRIPARMOR,SI_STRIPHELM,SI_CP_WEAPON,SI_CP_SHIELD,SI_CP_ARMOR,SI_CP_HELM,SI_AUTOGUARD,SI_REFLECTSHIELD,
	/* 60- */
	SI_DEVOTION,SI_PROVIDENCE,SI_DEFENDER,SI_BLANK,SI_BLANK,SI_AUTOSPELL,SI_BLANK,SI_BLANK,SI_SPEARQUICKEN,SI_BLANK,
	/* 70- */
	SI_BLANK,SI_BLANK,SI_BLANK,SI_BLANK,SI_BLANK,SI_BLANK,SI_BLANK,SI_BLANK,SI_BLANK,SI_BLANK,
	/* 80- */
	SI_BLANK,SI_BLANK,SI_BLANK,SI_BLANK,SI_BLANK,SI_BLANK,SI_EXPLOSIONSPIRITS,SI_STEELBODY,SI_BLANK,SI_COMBO,
	/* 90- */
	SI_FLAMELAUNCHER,SI_FROSTWEAPON,SI_LIGHTNINGLOADER,SI_SEISMICWEAPON,SI_BLANK,SI_BLANK,SI_BLANK,SI_BLANK,SI_BLANK,SI_BLANK,
	/* 100- */
	SI_BLANK,SI_BLANK,SI_WE_FEMALE,SI_AURABLADE,SI_PARRYING,SI_CONCENTRATION,SI_TENSIONRELAX,SI_BERSERK,SI_BLANK,SI_BLANK,
	/* 110- */
	SI_ASSUMPTIO,SI_BLANK,SI_BLANK,SI_MAGICPOWER,SI_EDP,SI_TRUESIGHT,SI_WINDWALK,SI_MELTDOWN,SI_CARTBOOST,SI_CHASEWALK,
	/* 120- */
	SI_REJECTSWORD,SI_MARIONETTE,SI_MARIONETTE2,SI_BLANK,SI_HEADCRUSH,SI_JOINTBEAT,SI_BLANK,SI_BLANK,SI_BLANK,SI_BLANK,
	/* 130- */
	SI_BLANK,SI_BLANK,SI_BLANK,SI_BLANK,SI_BLANK,SI_BLANK,SI_BLANK,SI_HEADCRUSH,SI_BLANK,SI_BLANK,
	/* 140- */
	SI_BLANK,SI_BLANK,SI_BLANK,SI_BLANK,SI_BLANK,SI_BLANK,SI_BLANK,SI_BLANK,SI_BLANK,SI_BLANK,
	/* 150- */
	SI_BLANK,SI_BLANK,SI_BLANK,SI_BLANK,SI_BLANK,SI_BLANK,SI_BLANK,SI_BLANK,SI_BLANK,SI_BLANK,
	/* 160- */
	SI_BLANK,SI_BLANK,SI_BLANK,SI_BLANK,SI_BLANK,SI_BLANK,SI_BLANK,SI_BLANK,SI_BLANK,SI_BLANK,
	/* 170- */
	SI_BLANK,SI_BLANK,SI_BLANK,SI_BLANK,SI_BLANK,SI_BLANK,SI_BLANK,SI_BLANK,SI_BLANK,SI_BLANK,
	/* 180- */
	SI_BLANK,SI_BLANK,SI_BLANK,SI_BLANK,SI_BLANK,SI_BLANK,SI_BLANK,SI_BLANK,SI_BLANK,SI_BLANK,
	/* 190- */
	SI_BLANK,SI_MAGNUM,SI_BLANK,SI_BLANK,SI_BLANK,SI_BLANK,SI_BLANK,SI_BLANK,SI_BLANK,SI_BLANK,
	/* 200- */
	SI_BLANK,SI_PRESERVE,SI_OVERTHRUSTMAX,SI_CHASEWALK_STR,SI_BLANK,SI_BLANK,SI_BLANK,SI_BLANK,SI_BLANK,SI_BLANK,
	/* 210- */
	SI_BLANK,SI_BLANK,SI_BLANK,SI_BLANK,SI_BLANK,SI_BLANK,SI_BLANK,SI_BLANK,SI_BLANK,SI_BLANK,
	/* 220- */
	SI_TAROTCARD,SI_TAROTCARD,SI_TAROTCARD,SI_TAROTCARD,SI_MEAL_INCSTR,SI_MEAL_INCAGI,SI_MEAL_INCVIT,SI_MEAL_INCINT,SI_MEAL_INCDEX,SI_MEAL_INCLUK,
	/* 230- */
	SI_RUN,SI_SPURT,SI_BLANK,SI_DODGE,SI_BLANK,SI_BLANK,SI_BLANK,SI_WARM,SI_BLANK,SI_BLANK,
	/* 240- */
	SI_SUN_COMFORT,SI_MOON_COMFORT,SI_STAR_COMFORT,SI_BLANK,SI_SOULLINK,SI_SOULLINK,SI_SOULLINK,SI_SOULLINK,SI_SOULLINK,SI_SOULLINK,
	/* 250- */
	SI_SOULLINK,SI_SOULLINK,SI_SOULLINK,SI_SOULLINK,SI_SOULLINK,SI_SOULLINK,SI_SOULLINK,SI_SOULLINK,SI_SOULLINK,SI_SOULLINK,
	/* 260- */
	SI_SOULLINK,SI_SOULLINK,SI_SOULLINK,SI_SOULLINK,SI_KAIZEL,SI_KAAHI,SI_KAUPE,SI_KAITE,SI_SMA,SI_BLANK,
	/* 270- */
	SI_BLANK,SI_BLANK,SI_ONEHAND,SI_READYSTORM,SI_READYDOWN,SI_READYTURN,SI_READYCOUNTER,SI_BLANK,SI_AUTOBERSERK,SI_BLANK,
	/* 280- */
	SI_DOUBLECASTING,SI_ELEMENTFIELD,SI_DARKELEMENT,SI_ATTENELEMENT,SI_MIRACLE,SI_BLANK,SI_BLANK,SI_BLANK,SI_BLANK,SI_BABY,
	/* 290- */
	SI_BLANK,SI_SHRINK,SI_CLOSECONFINE,SI_SIGHTBLASTER,SI_BLANK,SI_MEAL_INCHIT,SI_MEAL_INCFLEE,SI_BLANK,SI_MEAL_INCCRITICAL,SI_BLANK,
	/* 300- */
	SI_BLANK,SI_BLANK,SI_BLANK,SI_BLANK,SI_BLANK,SI_BLANK,SI_BLANK,SI_BLANK,SI_BLANK,SI_BLANK,
	/* 310- */
	SI_BLANK,SI_BLANK,SI_UNDEAD,SI_BLANK,SI_BLANK,SI_BLANK,SI_BLANK,SI_BLANK,SI_BLANK,SI_BLANK,
	/* 320- */
	SI_BLANK,SI_BLANK,SI_BLANK,SI_BLANK,SI_BLANK,SI_BLANK,SI_BLANK,SI_BLANK,SI_BLANK,SI_BLANK,
	/* 330- */
	SI_BLANK,SI_BLANK,SI_BLANK,SI_BLANK,SI_BLANK,SI_BLANK,SI_BLANK,SI_TIGEREYE,SI_BLANK,SI_BLANK,
	/* 340- */
	SI_BLANK,SI_BLANK,SI_BLANK,SI_BLANK,SI_STOP,SI_BLANK,SI_BLANK,SI_BLANK,SI_MADNESSCANCEL,SI_ADJUSTMENT,
	/* 350- */
	SI_INCREASING,SI_BLANK,SI_GATLINGFEVER,SI_BLANK,SI_BLANK,SI_UTSUSEMI,SI_BUNSINJYUTSU,SI_BLANK,SI_NEN,SI_BLANK,
	/* 360- */
	SI_BLANK,SI_BLANK,SI_BLANK,SI_BLANK,SI_BLANK,SI_ADRENALINE2,SI_BLANK,SI_BLANK,SI_COMBATHAN,SI_LIFEINSURANCE,
	/* 370- */
	SI_ITEMDROPRATE,SI_BOSSMAPINFO,SI_MEAL_INCSTR2,SI_MEAL_INCAGI2,SI_MEAL_INCVIT2,SI_MEAL_INCDEX2,SI_MEAL_INCINT2,SI_MEAL_INCLUK2,SI_SLOWCAST,SI_CRITICALWOUND,
	/* 380- */
	SI_BLANK,SI_BLANK,SI_BLANK,SI_BLANK,SI_HAPPY,SI_BLANK,SI_BLANK,SI_HELLPOWER,SI_ENCHANTBLADE,SI_BLANK,
	/* 390- */
	SI_NAUTHIZ,SI_THURISSUS,SI_HAGALAZ,SI_ISA,SI_OTHILA,SI_URUZ,SI_BLANK,SI_BLANK,SI_VENOMIMPRESS,SI_POISONINGWEAPON,
	/* 400- */
	SI_WEAPONBLOCKING,SI_BLANK,SI_BLANK,SI_HALLUCINATIONWALK,SI_BLANK,SI_ROLLINGCUTTER,SI_TOXIN,SI_PARALIZE,SI_VENOMBLEED,SI_MAGICMUSHROOM,
	/* 410- */
	SI_DEATHHURT,SI_PYREXIA,SI_OBLIVIONCURSE,SI_LEECHEND,SI_BLANK,SI_ORATIO,SI_BLANK,SI_BLANK,SI_RENOVATIO,SI_EXPIATIO,
	/* 420- */
	SI_DUPLELIGHT,SI_SACRAMENT,SI_BLANK,SI_FROSTMISTY,SI_MARSHOFABYSS,SI_RECOGNIZEDSPELL,SI_BLANK,SI_BLANK,SI_SUMMONBALL1,SI_SUMMONBALL2,
	/* 430- */
	SI_SUMMONBALL3,SI_SUMMONBALL4,SI_SUMMONBALL5,SI_BLANK,SI_FEARBREEZE,SI_BLANK,SI_WUGDASH,SI_BLANK,SI_CAMOUFLAGE,SI_ACCELERATION,
	/* 440- */
	SI_HOVERING,SI_OVERHEAT,SI_SHAPESHIFT,SI_INFRAREDSCAN,SI_ANALYZE,SI_BLANK,SI_BLANK,SI_NEUTRALBARRIER,SI_BLANK,SI_STEALTHFIELD,
	/* 450- */
	SI_MANU_ATK,SI_MANU_DEF,SI_MANU_MATK,SI_SPL_ATK,SI_SPL_DEF,SI_SPL_MATK,SI_REPRODUCE,SI_AUTOSHADOWSPELL,SI_SHADOWFORM,SI_BODYPAINT,
	/* 460- */
	SI_INVISIBILITY,SI_DEADLYINFECT,SI_ENERVATION,SI_GROOMY,SI_IGNORANCE,SI_LAZINESS,SI_UNLUCKY,SI_WEAKNESS,SI_STRIPACCESSARY,SI_MANHOLE,
	/* 470- */
	SI_BLOODYLUST,SI_REFLECTDAMAGE,SI_FORCEOFVANGUARD,SI_SHIELDSPELL_DEF,SI_SHIELDSPELL_MDEF,SI_SHIELDSPELL_REF,SI_EXEEDBREAK,SI_PRESTIGE,SI_BANDING,SI_SITDOWN_FORCE,
	/* 480- */
	SI_EARTHDRIVE,SI_INSPIRATION,SI_FALLENEMPIRE,SI_CRESCENTELBOW,SI_CURSEDCIRCLE_ATKER,SI_CURSEDCIRCLE_TARGET,SI_LIGHTNINGWALK,SI_RAISINGDRAGON,SI_GENTLETOUCH_ENERGYGAIN,SI_GENTLETOUCH_CHANGE,
	/* 490- */
	SI_GENTLETOUCH_REVITALIZE,SI_SWING,SI_SYMPHONY_LOVE,SI_MOONLIT_SERENADE,SI_RUSH_WINDMILL,SI_ECHOSONG,SI_HARMONIZE,SI_NETHERWORLD,SI_SIREN,SI_DEEP_SLEEP,
	/* 500- */
	SI_SIRCLEOFNATURE,SI_GLOOMYDAY,SI_SONG_OF_MANA,SI_DANCE_WITH_WUG,SI_SATURDAY_NIGHT_FEVER,SI_LERADS_DEW,SI_MELODYOFSINK,SI_BEYOND_OF_WARCRY,SI_UNLIMITED_HUMMING_VOICE,SI_PROPERTYWALK,
	/* 510- */
	SI_SPELLFIST,SI_DIAMONDDUST,SI_BLANK,SI_STRIKING,SI_WARMER,SI_VACUUM_EXTREME,SI_GN_CARTBOOST,SI_THORNS_TRAP,SI_BLOOD_SUCKER,SI_SPORE_EXPLOSION,
	/* 520- */
	SI_FIRE_EXPANSION_SMOKE_POWDER,SI_FIRE_EXPANSION_TEAR_GAS,SI_MANDRAGORA,SI_MELON_BOMB,SI_BANANA_BOMB,SI_MYSTERIOUS_POWDER,SI_BOOST500,SI_FULL_SWING_K,SI_MANA_PLUS,SI_MUSTLE_M,
	/* 530- */
	SI_LIFE_FORCE_F,SI_PROMOTE_HEALTH_RESERCH,SI_ENERGY_DRINK_RESERCH,SI_EXTRACT_WHITE_POTION_Z,SI_VITATA_500,SI_EXTRACT_SALAMINE_JUICE,SI_SAVAGE_STEAK,SI_COCKTAIL_WARG_BLOOD,SI_MINOR_BBQ,SI_SIROMA_ICE_TEA,
	/* 540- */
	SI_DROCERA_HERB_STEAMED,SI_PUTTI_TAILS_NOODLES,SI_STOMACHACHE,SI_MONSTER_TRANSFORM,SI_IZAYOI,SI_KG_KAGEHUMI,SI_KYOMU,SI_KAGEMUSYA,SI_AKAITSUKI,SI_ALL_RIDING,
	/* 550- */
	SI_MEIKYOUSISUI,SI_KYOUGAKU,SI_ODINS_POWER,SI_CPLUSONLYJOBEXP,SI_MER_FLEE,SI_MER_ATK,SI_MER_HP,SI_MER_SP,SI_MER_HIT,SI_ON_PUSH_CART,
	/* 560- */
	SI_ZENKAI,SI_KO_JYUMONJIKIRI,SI_SUMMON_AGNI,SI_CIRCLE_OF_FIRE,SI_CIRCLE_OF_FIRE_OPTION,SI_FIRE_CLOAK,SI_FIRE_CLOAK_OPTION,SI_WATER_SCREEN,SI_WATER_SCREEN_OPTION,SI_WATER_DROP,
	/* 570- */
	SI_WATER_DROP_OPTION,SI_WATER_BARRIER,SI_WIND_STEP,SI_WIND_STEP_OPTION,SI_WIND_CURTAIN,SI_WIND_CURTAIN_OPTION,SI_ZEPHYR,SI_SOLID_SKIN,SI_SOLID_SKIN_OPTION,SI_STONE_SHIELD,
	/* 580- */
	SI_STONE_SHIELD_OPTION,SI_POWER_OF_GAIA,SI_PYROTECHNIC,SI_PYROTECHNIC_OPTION,SI_HEATER,SI_HEATER_OPTION,SI_TROPIC,SI_TROPIC_OPTION,SI_AQUAPLAY,SI_AQUAPLAY_OPTION,
	/* 590- */
	SI_COOLER,SI_COOLER_OPTION,SI_CHILLY_AIR,SI_CHILLY_AIR_OPTION,SI_GUST,SI_GUST_OPTION,SI_BLAST,SI_BLAST_OPTION,SI_WILD_STORM,SI_WILD_STORM_OPTION,
	/* 600- */
	SI_PETROLOGY,SI_PETROLOGY_OPTION,SI_CURSED_SOIL,SI_CURSED_SOIL_OPTION,SI_UPHEAVAL,SI_UPHEAVAL_OPTION,SI_TIDAL_WEAPON,SI_TIDAL_WEAPON_OPTION,SI_ROCK_CRUSHER,SI_ROCK_CRUSHER_ATK,
	/* 610- */
	SI_FIRE_INSIGNIA,SI_WATER_INSIGNIA,SI_WIND_INSIGNIA,SI_EARTH_INSIGNIA,SI_HAT_EFFECT,SI_JP_EVENT01,SI_JP_EVENT02,SI_JP_EVENT03,SI_JP_EVENT04,SI_ACTIVE_MONSTER_TRANSFORM,
	/* 620- */
	SI_BLANK,SI_BLANK,SI_BLANK,SI_ZANGETSU,SI_GENSOU,SI_ASSUMPTIO2,SI_PHI_DEMON,SI_PLUSATTACKPOWER,SI_PLUSMAGICPOWER,SI_ALMIGHTY
};

/*==========================================
 * �ߏ萸�B�{�[�i�X
 *------------------------------------------
 */
int status_get_overrefine_bonus(int lv)
{
	if(lv >= 0 && lv <= MAX_WEAPON_LEVEL)
		return refine_db[lv].over_bonus;
	return 0;
}

/*==========================================
 * ���B������
 *------------------------------------------
 */
int status_percentrefinery(struct map_session_data *sd,struct item *item)
{
	int percent;

	nullpo_retr(0, item);

	if(item->refine < 0 || item->refine >= MAX_REFINE)	// �l���G���[�������͊��ɍő�l�Ȃ�0%
		return 0;

	percent = refine_db[itemdb_wlv(item->nameid)].per[(int)item->refine];

	if(battle_config.refinery_research_lv)
		percent += pc_checkskill(sd,BS_WEAPONRESEARCH);	// ���팤���X�L������

	// �m���̗L���͈̓`�F�b�N
	if(percent > 100) {
		percent = 100;
	}
	if(percent < 0) {
		percent = 0;
	}

	return percent;
}

/*==========================================
 * ���B������ �番��
 *------------------------------------------
 */
int status_percentrefinery_weaponrefine(struct map_session_data *sd,struct item *item)
{
	int percent;
	int joblv;
	int diff;

	nullpo_retr(0, sd);
	nullpo_retr(0, item);

	if(item->refine < 0 || item->refine >= MAX_REFINE)	// �l���G���[�������͊��ɍő�l�Ȃ�0%
		return 0;

	if(sd->status.class_ == PC_CLASS_NC || sd->status.class_ == PC_CLASS_NC_H || sd->status.class_ == PC_CLASS_NC_B)
		joblv = 70;
	else if(sd->status.job_level > 70)
		joblv = 70;
	else
		joblv = sd->status.job_level;

	diff = joblv - 50;
	percent = refine_db[itemdb_wlv(item->nameid)].per[(int)item->refine]*10 + diff + 4 * diff;

	if(battle_config.allow_weaponrearch_to_weaponrefine)
		percent += pc_checkskill(sd,BS_WEAPONRESEARCH)*10;	// ���팤���X�L������

	// �m���̗L���͈̓`�F�b�N
	if(percent > 1000) {
		percent = 1000;
	}
	if(percent < 0) {
		percent = 0;
	}

	return percent;
}

/*==========================================
 * �Z�[�u�\�ȃX�e�[�^�X�ُ킩�ǂ���
 *------------------------------------------
 */
int status_can_save(int type)
{
	if(type >= 0 && type < MAX_STATUSCHANGE) {
		if(scdata_db[type].save > 0)
			return 1;
	}
	return 0;
}

/*==========================================
 * diable�ȃX�e�[�^�X�ُ킩�ǂ���
 *------------------------------------------
 */
int status_is_disable(int type,int mask)
{
	if(type >= 0 && type < MAX_STATUSCHANGE) {
		if(scdata_db[type].disable & mask)
			return 1;
	}
	return 0;
}

/*==========================================
 * �p�����[�^�v�Z
 * first==0�̎��A�v�Z�Ώۂ̃p�����[�^���Ăяo���O����
 * �ω������ꍇ������send���邪�A
 * �\���I�ɕω��������p�����[�^�͎��O��send����悤��
 *------------------------------------------
 */
int status_calc_pc(struct map_session_data* sd,int first)
{
	// ���ӁF�����ł͕ϐ��̐錾�݂̂ɂƂǂ߁A��������L_RECALC�̌�ɂ�邱�ƁB
	int b_speed,b_max_hp,b_max_sp,b_hp,b_sp,b_weight,b_max_weight,b_paramb[6],b_paramc[6],b_hit,b_flee;
	int b_aspd,b_watk,b_def,b_watk2,b_def2,b_flee2,b_critical,b_attackrange,b_matk1,b_matk2,b_mdef,b_mdef2,b_class;
	int b_base_atk;
	int b_watk_,b_watk_2;
	int b_tigereye, b_endure, b_speedrate;
	struct skill b_skill[MAX_PCSKILL];
	int i,blv,calc_val,idx;
	int skill,wele,wele_,def_ele,refinedef;
	int pele,pdef_ele;
	int str,dstr,dex;
	int calclimit = 2; // �����use script���݂Ŏ��s
#ifndef PRE_RENEWAL
	int b_plus_atk,b_plus_matk;
#endif

	nullpo_retr(0, sd);

	if(sd->stop_status_calc_pc)
	{
		sd->call_status_calc_pc_while_stopping++;
		return 0;
	}

	sd->call_status_calc_pc_while_stopping = 0;

	// status_calc_pc �̏�������status_calc_pc���Ăяo�����ƍŏ��Ɍv�Z���Ă���
	// �l�������\��������B�܂��A���̊֐����Ă΂��Ƃ������Ƃ́A�L�����̏�Ԃ�
	// �ω����Ă��邱�Ƃ��Î����Ă���̂ŁA�v�Z���ʂ��̂ĂčČv�Z���Ȃ���΂����Ȃ��B
	// �֐��̏I�����_�ŌĂяo��������΁AL_RECALC�ɔ�΂��悤�ɂ���B
	if( sd->status_calc_pc_process++ ) return 0;

	// �ȑO�̏�Ԃ̕ۑ�
	b_speed      = sd->speed;
	b_max_hp     = sd->status.max_hp;
	b_max_sp     = sd->status.max_sp;
	b_hp         = sd->status.hp;
	b_sp         = sd->status.sp;
	b_weight     = sd->weight;
	b_max_weight = sd->max_weight;
	memcpy(b_paramb, &sd->paramb, sizeof(b_paramb));
	memcpy(b_paramc, &sd->paramc, sizeof(b_paramc));
	memcpy(b_skill, &sd->status.skill, sizeof(b_skill));
	b_hit         = sd->hit;
	b_flee        = sd->flee;
	b_aspd        = sd->aspd;
	b_watk        = sd->watk;
	b_def         = sd->def;
	b_watk2       = sd->watk2;
	b_def2        = sd->def2;
	b_flee2       = sd->flee2;
	b_critical    = sd->critical;
	b_attackrange = sd->range.attackrange;
	b_matk1       = sd->matk1;
	b_matk2       = sd->matk2;
	b_mdef        = sd->mdef;
	b_mdef2       = sd->mdef2;
	b_class       = sd->view_class;
	b_base_atk    = sd->base_atk;
	b_tigereye    = sd->special_state.infinite_tigereye;
	b_endure      = sd->special_state.infinite_endure;
	b_speedrate   = sd->speed_rate;
	b_watk_       = sd->watk_;
	b_watk_2      = sd->watk_2;
#ifndef PRE_RENEWAL
	b_plus_atk    = sd->plus_atk;
	b_plus_matk   = sd->plus_matk;
#endif

L_RECALC:
	// �{���̌v�Z�J�n(���̃p�����[�^���X�V���Ȃ��̂́A�v�Z���Ɍv�Z�������Ă΂ꂽ�Ƃ���
	// ���f�����V���ɑ��M���邽��)�B

	pele      = ELE_NEUTRAL;
	pdef_ele  = ELE_NEUTRAL;
	refinedef = 0;
	sd->view_class = sd->status.class_;
	sd->race = RCT_HUMAN;
	sd->ranker_weapon_bonus  = 0;
	sd->ranker_weapon_bonus_ = 0;
	sd->special_state.infinite_tigereye = 0;
	sd->special_state.infinite_endure   = 0;

	pc_calc_skilltree(sd);	// �X�L���c���[�̌v�Z

	sd->max_weight = job_db[sd->s_class.job].max_weight_base+sd->status.str*battle_config.str_weight;

	if(battle_config.baby_weight_rate != 100 && pc_isbaby(sd))
		sd->max_weight = sd->max_weight*battle_config.baby_weight_rate/100;
	else if(battle_config.upper_weight_rate != 100 && pc_isupper(sd))
		sd->max_weight = sd->max_weight*battle_config.upper_weight_rate/100;
	else if(battle_config.normal_weight_rate != 100)
		sd->max_weight = sd->max_weight*battle_config.normal_weight_rate/100;

	// �y�R�R�掞������悤�ړ�
	if(pc_isriding(sd))	// �y�R�y�R�E�O���t�H��
		sd->max_weight += battle_config.riding_weight;

	if(pc_isdragon(sd))	// �h���S��
		sd->max_weight += 5000 + 2000 * pc_checkskill(sd,RK_DRAGONTRAINING);

	if((skill = pc_checkskill(sd,MC_INCCARRY)) > 0)	// �����ʑ���
		sd->max_weight += skill*2000;

	if((skill = pc_checkskill(sd,ALL_INCCARRAY)) > 0)	// �����ʑ���R
		sd->max_weight += skill*2000;

	if((skill = pc_checkskill(sd,SG_KNOWLEDGE)) > 0)	// ���z�ƌ��Ɛ��̒m��
	{
	 	if(battle_config.check_knowlege_map) {	// �ꏊ�`�F�b�N���s�Ȃ�
			if(sd->bl.m == sd->feel_index[0] || sd->bl.m == sd->feel_index[1] || sd->bl.m == sd->feel_index[2])
				sd->max_weight += sd->max_weight*skill/10;
		} else {
			sd->max_weight += sd->max_weight*skill/10;
		}
	}

	if(first&1) {
		sd->weight = 0;
		for(i=0; i<MAX_INVENTORY; i++) {
			if(sd->status.inventory[i].nameid == 0 || sd->inventory_data[i] == NULL)
				continue;
			sd->weight += sd->inventory_data[i]->weight*sd->status.inventory[i].amount;
		}
		sd->cart_max_weight = battle_config.max_cart_weight;
		if((skill = pc_checkskill(sd,GN_REMODELING_CART)) > 0)	// �J�[�g����
			sd->cart_max_weight += skill*5000;
		sd->cart_weight     = 0;
		sd->cart_max_num    = MAX_CART;
		sd->cart_num        = 0;
		for(i=0; i<MAX_CART; i++) {
			if(sd->status.cart[i].nameid == 0)
				continue;
			sd->cart_weight += itemdb_weight(sd->status.cart[i].nameid)*sd->status.cart[i].amount;
			sd->cart_num++;
		}
	}

	memset(sd->paramb,0,sizeof(sd->paramb));
	memset(sd->parame,0,sizeof(sd->parame));
	sd->hit                 = 0;
	sd->flee                = 0;
	sd->flee2               = 0;
	sd->critical            = 0;
	sd->aspd                = 0;
	sd->watk                = 0;
	sd->def                 = 0;
	sd->mdef                = 0;
	sd->watk2               = 0;
	sd->def2                = 0;
	sd->mdef2               = 0;
	sd->status.max_hp       = 0;
	sd->status.max_sp       = 0;
	sd->range.attackrange   = 0;
	sd->range.attackrange_  = 0;
	sd->atk_ele             = ELE_NEUTRAL;
	sd->def_ele             = ELE_NEUTRAL;
	sd->star                = 0;
	sd->overrefine          = 0;
	sd->matk1               = 0;
	sd->matk2               = 0;
	sd->speed               = DEFAULT_WALK_SPEED;
	sd->hprate              = battle_config.hp_rate;
	sd->sprate              = battle_config.sp_rate;
	sd->castrate            = 100;
	sd->fixcastrate         = 0;
	sd->fixcastrate_        = 0;
	sd->dsprate             = 100;
	sd->base_atk            = 0;
#ifndef PRE_RENEWAL
	sd->plus_atk            = 0;
	sd->plus_matk           = 0;
#endif
	sd->arrow_atk           = 0;
	sd->arrow_ele           = ELE_NEUTRAL;
	sd->arrow_hit           = 0;
	sd->arrow_range         = 0;
	sd->nhealhp = sd->nhealsp = sd->nshealhp = sd->nshealsp = sd->nsshealhp = sd->nsshealsp = 0;
	memset(sd->addele,0,sizeof(sd->addele));
	memset(sd->addrace,0,sizeof(sd->addrace));
	memset(sd->addenemy,0,sizeof(sd->addenemy));
	memset(sd->addsize,0,sizeof(sd->addsize));
	memset(sd->addele_,0,sizeof(sd->addele_));
	memset(sd->addrace_,0,sizeof(sd->addrace_));
	memset(sd->addenemy_,0,sizeof(sd->addenemy_));
	memset(sd->addsize_,0,sizeof(sd->addsize_));
	memset(sd->subele,0,sizeof(sd->subele));
	memset(sd->subrace,0,sizeof(sd->subrace));
	memset(sd->subenemy,0,sizeof(sd->subenemy));
	memset(sd->addeff,0,sizeof(sd->addeff));
	memset(sd->addeff2,0,sizeof(sd->addeff2));
	memset(sd->reseff,0,sizeof(sd->reseff));
	memset(sd->addeff_range_flag,0,sizeof(sd->addeff));
	memset(&sd->special_state,0,sizeof(sd->special_state));
	memset(sd->weapon_coma_ele,0,sizeof(sd->weapon_coma_ele));
	memset(sd->weapon_coma_race,0,sizeof(sd->weapon_coma_race));
	memset(sd->weapon_coma_ele2,0,sizeof(sd->weapon_coma_ele2));
	memset(sd->weapon_coma_race2,0,sizeof(sd->weapon_coma_race2));
	memset(sd->weapon_atk,0,sizeof(sd->weapon_atk));
	memset(sd->weapon_atk_rate,0,sizeof(sd->weapon_atk_rate));
	memset(sd->auto_status_calc_pc,0,sizeof(sd->auto_status_calc_pc));
	memset(sd->eternal_status_change,0,sizeof(sd->eternal_status_change));

	sd->watk_       = 0;	// �񓁗��p
	sd->watk_2      = 0;
	sd->atk_ele_    = ELE_NEUTRAL;
	sd->star_       = 0;
	sd->overrefine_ = 0;

	sd->aspd_add       = 0;
	sd->aspd_rate      = 0;
	sd->aspd_add_rate  = 0;
	sd->speed_rate     = 0;
	sd->speed_add_rate = 0;
	sd->hprecov_rate   = 100;
	sd->sprecov_rate   = 100;
	sd->critical_def   = 0;
	sd->double_rate    = 0;
	sd->near_attack_def_rate = sd->long_attack_def_rate = 0;
	sd->atk_rate = sd->matk_rate = sd->matk2_rate = 100;
	memset(sd->ignore_def_ele,0,sizeof(sd->ignore_def_ele));
	memset(sd->ignore_def_race,0,sizeof(sd->ignore_def_race));
	memset(sd->ignore_def_enemy,0,sizeof(sd->ignore_def_enemy));
	memset(sd->ignore_def_ele_,0,sizeof(sd->ignore_def_ele_));
	memset(sd->ignore_def_race_,0,sizeof(sd->ignore_def_race_));
	memset(sd->ignore_def_enemy_,0,sizeof(sd->ignore_def_enemy_));
	memset(sd->ignore_mdef_ele,0,sizeof(sd->ignore_mdef_ele));
	memset(sd->ignore_mdef_race,0,sizeof(sd->ignore_mdef_race));
	memset(sd->ignore_mdef_enemy,0,sizeof(sd->ignore_mdef_enemy));
	sd->arrow_cri = 0;
	sd->magic_def_rate = sd->misc_def_rate = 0;
	memset(sd->arrow_addele,0,sizeof(sd->arrow_addele));
	memset(sd->arrow_addrace,0,sizeof(sd->arrow_addrace));
	memset(sd->arrow_addenemy,0,sizeof(sd->arrow_addenemy));
	memset(sd->arrow_addsize,0,sizeof(sd->arrow_addsize));
	memset(sd->arrow_addeff,0,sizeof(sd->arrow_addeff));
	memset(sd->arrow_addeff2,0,sizeof(sd->arrow_addeff2));
	memset(sd->magic_addele,0,sizeof(sd->magic_addele));
	memset(sd->magic_addrace,0,sizeof(sd->magic_addrace));
	memset(sd->magic_addenemy,0,sizeof(sd->magic_addenemy));
	memset(sd->magic_addeff,0,sizeof(sd->magic_addeff));
	memset(sd->magic_subrace,0,sizeof(sd->magic_subrace));
	sd->perfect_hit = 0;
	sd->critical_rate = sd->hit_rate = sd->flee_rate = sd->flee2_rate = 100;
	sd->def_rate = sd->def2_rate = sd->mdef_rate = sd->mdef2_rate = 100;
	sd->def_ratio_atk_ele = sd->def_ratio_atk_race = sd->def_ratio_atk_enemy = 0;
	sd->def_ratio_atk_ele_ = sd->def_ratio_atk_race_ = sd->def_ratio_atk_enemy_ = 0;
	sd->get_zeny_num = sd->get_zeny_num2 = 0;
	sd->add_damage_class_count = sd->add_damage_class_count_ = sd->add_magic_damage_class_count = 0;
	sd->add_def_class_count = sd->add_mdef_class_count = 0;
	sd->monster_drop_item_count = 0;
	memset(sd->add_damage_classrate,0,sizeof(sd->add_damage_classrate));
	memset(sd->add_damage_classrate_,0,sizeof(sd->add_damage_classrate_));
	memset(sd->add_magic_damage_classrate,0,sizeof(sd->add_magic_damage_classrate));
	memset(sd->add_def_classrate,0,sizeof(sd->add_def_classrate));
	memset(sd->add_mdef_classrate,0,sizeof(sd->add_mdef_classrate));
	memset(sd->monster_drop_race,0,sizeof(sd->monster_drop_race));
	memset(sd->monster_drop_itemrate,0,sizeof(sd->monster_drop_itemrate));
	sd->sp_gain_value = 0;
	sd->hp_gain_value = 0;
	sd->double_add_rate = sd->perfect_hit_add = sd->get_zeny_add_num = sd->get_zeny_add_num2 = 0;
	sd->splash_range = sd->splash_add_range = 0;
	memset(&sd->hp_drain,0,sizeof(sd->hp_drain));
	memset(&sd->sp_drain,0,sizeof(sd->sp_drain));
	memset(&sd->hp_drain_,0,sizeof(sd->hp_drain_));
	memset(&sd->sp_drain_,0,sizeof(sd->sp_drain_));
	sd->short_weapon_damage_return = sd->long_weapon_damage_return = sd->magic_damage_return = 0;
	sd->break_weapon_rate = sd->break_armor_rate = 0;
	sd->add_steal_rate    = 0;
	sd->unbreakable_equip = LOC_NOTHING;
	sd->critical_damage   = 0;
	sd->hp_recov_stop = sd->sp_recov_stop = 0;
	memset(sd->critical_race,0,sizeof(sd->critical_race));
	memset(sd->critical_race_rate,0,sizeof(sd->critical_race_rate));
	memset(sd->subsize,0,sizeof(sd->subsize));
	memset(sd->magic_subsize,0,sizeof(sd->magic_subsize));
	memset(sd->exp_rate,0,sizeof(sd->exp_rate));
	memset(sd->job_rate,0,sizeof(sd->job_rate));
	memset(sd->hp_drain_rate_race,0,sizeof(sd->hp_drain_rate_race));
	memset(sd->sp_drain_rate_race,0,sizeof(sd->sp_drain_rate_race));
	memset(sd->hp_drain_value_race,0,sizeof(sd->hp_drain_value_race));
	memset(sd->sp_drain_value_race,0,sizeof(sd->sp_drain_value_race));
	memset(sd->addreveff,0,sizeof(sd->addreveff));
	sd->addreveff_flag = 0;
	memset(sd->addgroup,0,sizeof(sd->addgroup));
	memset(sd->addgroup_,0,sizeof(sd->addgroup_));
	memset(sd->magic_addgroup,0,sizeof(sd->magic_addgroup));
	memset(sd->arrow_addgroup,0,sizeof(sd->arrow_addgroup));
	memset(sd->subgroup,0,sizeof(sd->subgroup));
	sd->hp_penalty_time  = 0;
	sd->hp_penalty_value = 0;
	sd->sp_penalty_time  = 0;
	sd->sp_penalty_value = 0;
	memset(sd->hp_penalty_unrig_value,0,sizeof(sd->hp_penalty_unrig_value));
	memset(sd->sp_penalty_unrig_value,0,sizeof(sd->sp_penalty_unrig_value));
	memset(sd->hp_rate_penalty_unrig,0,sizeof(sd->hp_rate_penalty_unrig));
	memset(sd->sp_rate_penalty_unrig,0,sizeof(sd->sp_rate_penalty_unrig));
	sd->mob_class_change_rate = 0;
	memset(&sd->skill_dmgup,0,sizeof(sd->skill_dmgup));
	memset(&sd->skill_blow,0,sizeof(sd->skill_blow));
	memset(&sd->autospell,0,sizeof(sd->autospell));
	memset(&sd->itemheal_rate,0,sizeof(sd->itemheal_rate));
	memset(&sd->autoraise,0,sizeof(sd->autoraise));
	memset(&sd->hp_vanish,0,sizeof(sd->hp_vanish));
	memset(&sd->sp_vanish,0,sizeof(sd->sp_vanish));
	sd->bonus_damage      = 0;
	sd->curse_by_muramasa = 0;
	memset(sd->loss_equip_rate_when_die,0,sizeof(sd->loss_equip_rate_when_die));
	memset(sd->loss_equip_rate_when_attack,0,sizeof(sd->loss_equip_rate_when_attack));
	memset(sd->loss_equip_rate_when_hit,0,sizeof(sd->loss_equip_rate_when_hit));
	memset(sd->break_myequip_rate_when_attack,0,sizeof(sd->break_myequip_rate_when_attack));
	memset(sd->break_myequip_rate_when_hit,0,sizeof(sd->break_myequip_rate_when_hit));
	sd->loss_equip_flag = 0;
	sd->short_weapon_damege_rate = sd->long_weapon_damege_rate = 0;
	sd->range.add_attackrange = 0;
	sd->range.add_attackrange_rate = 100;
	sd->special_state.item_no_use = 0;
	sd->skill_delay_rate = 0;
	memset(&sd->fix_status,0,sizeof(sd->fix_status));
	memset(&sd->skill_fixcastrate,0,sizeof(sd->skill_fixcastrate));
	memset(&sd->skill_addcastrate,0,sizeof(sd->skill_addcastrate));
	memset(&sd->skill_healup,0,sizeof(sd->skill_healup));
	memset(&sd->activeitem,0,sizeof(sd->activeitem));
	memset(sd->def_eleenemy,0,sizeof(sd->def_eleenemy));
	memset(&sd->skill_addspcost,0,sizeof(sd->skill_addspcost));
	memset(&sd->skill_subhealup,0,sizeof(sd->skill_subhealup));
	memset(&sd->skill_addcast,0,sizeof(sd->skill_addcast));
	memset(&sd->skill_cooldown,0,sizeof(sd->skill_cooldown));
	memset(&sd->skill_eleweapon_dmgup,0,sizeof(sd->skill_eleweapon_dmgup));
	memset(&sd->skill_elemagic_dmgup,0,sizeof(sd->skill_elemagic_dmgup));
	sd->hp_rate_penalty_time  = 0;
	sd->hp_rate_penalty_value = 0;
	sd->sp_rate_penalty_time  = 0;
	sd->sp_rate_penalty_value = 0;

	for(i=0; i<EQUIP_INDEX_MAX; i++) {
		if(i == EQUIP_INDEX_ARROW)
			continue;
		idx = sd->equip_index[i];
		current_equip_item_index = i;	// ���ʃ`�F�b�N�p
		if(idx < 0)
			continue;
		if(i == EQUIP_INDEX_RARM && sd->equip_index[EQUIP_INDEX_LARM] == idx)
			continue;
		if(i == EQUIP_INDEX_HEAD3 && sd->equip_index[EQUIP_INDEX_HEAD] == idx)
			continue;
		if(i == EQUIP_INDEX_HEAD2 && (sd->equip_index[EQUIP_INDEX_HEAD3] == idx || sd->equip_index[EQUIP_INDEX_HEAD] == idx))
			continue;
		if(i == EQUIP_INDEX_COSTUME_HEAD3 && sd->equip_index[EQUIP_INDEX_COSTUME_HEAD] == idx)
			continue;
		if(i == EQUIP_INDEX_COSTUME_HEAD2 && (sd->equip_index[EQUIP_INDEX_COSTUME_HEAD3] == idx || sd->equip_index[EQUIP_INDEX_COSTUME_HEAD] == idx))
			continue;

		if(sd->inventory_data[idx]) {
			if(itemdb_isweapon(sd->inventory_data[idx]->nameid)) {
				int j;
				if( !itemdb_isspecial(sd->status.inventory[idx].card[0]) ) {
					int c;
					for(j=0; j<4; j++) {	// �J�[�h
						if((c = sd->status.inventory[idx].card[j]) <= 0)
							continue;
						if(sd->inventory_data[idx]->slot < j && itemdb_cardtype(c) != 2)
							continue;
						current_equip_name_id = c;		// �I�[�g�X�y��(�d���`�F�b�N�p)
						if(i == EQUIP_INDEX_LARM && sd->status.inventory[idx].equip == LOC_LARM)
							sd->state.lr_flag = 1;
						if(calclimit == 2)
							run_script(itemdb_usescript(c),0,sd->bl.id,0);
						run_script(itemdb_equipscript(c),0,sd->bl.id,0);
						sd->state.lr_flag = 0;
					}
				}
				for(j=0; j<5; j++) {	// �����_���I�v�V����
					if(sd->status.inventory[idx].opt[j].id <= 0)
						continue;
					if(i == EQUIP_INDEX_LARM && sd->status.inventory[idx].equip == LOC_LARM)
						sd->state.lr_flag = 1;
					bonus_randopt(sd, sd->status.inventory[idx].opt[j].id, sd->status.inventory[idx].opt[j].val);
					sd->state.lr_flag = 0;
				}
			}
			else if(itemdb_isarmor(sd->inventory_data[idx]->nameid)) { // �h��
				int j;
				if( !itemdb_isspecial(sd->status.inventory[idx].card[0]) ) {
					int c;
					for(j=0; j<4; j++) {	// �J�[�h
						if((c = sd->status.inventory[idx].card[j]) <= 0)
							continue;
						if(sd->inventory_data[idx]->slot < j && itemdb_cardtype(c) != 2)
							continue;
						current_equip_name_id = c;		// �I�[�g�X�y��(�d���`�F�b�N�p)
						if(calclimit == 2)
							run_script(itemdb_usescript(c),0,sd->bl.id,0);
						run_script(itemdb_equipscript(c),0,sd->bl.id,0);
					}
				}
				for(j=0; j<5; j++) {	// �����_���I�v�V����
					if(sd->status.inventory[idx].opt[j].id <= 0)
						continue;
					bonus_randopt(sd, sd->status.inventory[idx].opt[j].id, sd->status.inventory[idx].opt[j].val);
				}
			}
		}
	}

	wele    = sd->atk_ele;
	wele_   = sd->atk_ele_;
	def_ele = sd->def_ele;
	if(battle_config.pet_status_support) {
		if(sd->status.pet_id > 0 && sd->petDB && sd->pet.intimate > 0) {
			run_script(sd->petDB->script,0,sd->bl.id,0);
		}
		pele        = sd->atk_ele;
		pdef_ele    = sd->def_ele;
		sd->atk_ele = sd->def_ele = ELE_NEUTRAL;
	}
	memcpy(sd->paramcard,sd->parame,sizeof(sd->paramcard));

	// �����i�ɂ��X�e�[�^�X�ω��͂����Ŏ��s
	for(i=0; i<EQUIP_INDEX_MAX; i++) {
		if(i == EQUIP_INDEX_ARROW)
			continue;
		idx = sd->equip_index[i];
		current_equip_item_index = i;	// ���ʃ`�F�b�N�p
		if(idx < 0)
			continue;
		if(i == EQUIP_INDEX_RARM && sd->equip_index[EQUIP_INDEX_LARM] == idx)
			continue;
		if(i == EQUIP_INDEX_HEAD3 && sd->equip_index[EQUIP_INDEX_HEAD] == idx)
			continue;
		if(i == EQUIP_INDEX_HEAD2 && (sd->equip_index[EQUIP_INDEX_HEAD3] == idx || sd->equip_index[EQUIP_INDEX_HEAD] == idx))
			continue;
		if(i == EQUIP_INDEX_COSTUME_HEAD3 && sd->equip_index[EQUIP_INDEX_COSTUME_HEAD] == idx)
			continue;
		if(i == EQUIP_INDEX_COSTUME_HEAD2 && (sd->equip_index[EQUIP_INDEX_COSTUME_HEAD3] == idx || sd->equip_index[EQUIP_INDEX_COSTUME_HEAD] == idx))
			continue;
		if(sd->inventory_data[idx]) {
			current_equip_name_id = sd->inventory_data[idx]->nameid;
			sd->matk1 += sd->inventory_data[idx]->matk;
#ifdef PRE_RENEWAL
			sd->matk2 += sd->inventory_data[idx]->matk;
#endif
			sd->def += sd->inventory_data[idx]->def;
			sd->mdef += sd->inventory_data[idx]->mdef;
			if(itemdb_isweapon(sd->inventory_data[idx]->nameid)) {
				int r,wlv = sd->inventory_data[idx]->wlv;
				if(i == EQUIP_INDEX_LARM && sd->status.inventory[idx].equip == LOC_LARM) {
					// �񓁗��p�f�[�^����
					sd->watk_ += sd->inventory_data[idx]->atk;
					if((r = sd->status.inventory[idx].refine) > 0)
						sd->watk_2 = refine_db[wlv].safety_bonus[r-1];	// ���B�U����
#ifndef PRE_RENEWAL
					if(sd->status.weapon != WT_BOW && r > 0)	// �|�ɂ͐��BMATK�{�[�i�X���Ȃ�
						sd->matk1 += refine_db[wlv].safety_bonus[r-1];
#endif
					if((r -= refine_db[wlv].limit) > 0)	// �ߏ萸�B�{�[�i�X
						sd->overrefine_ = r*refine_db[wlv].over_bonus;

					if(sd->status.inventory[idx].card[0] == 0x00ff) {	// ��������
						sd->star_ = (sd->status.inventory[idx].card[1]>>8);	// ���̂�����
						if(sd->star_ == 15)
							sd->star_ = 40;
						wele_= sd->status.inventory[idx].card[1] & 0x0f;	// �� ��
						// �����L���O�{�[�i�X
						if(ranking_get_id2rank(*((int *)(&sd->status.inventory[idx].card[2])), RK_BLACKSMITH))
							sd->ranker_weapon_bonus_ = battle_config.ranker_weapon_bonus;
					}
					sd->range.attackrange_ += sd->inventory_data[idx]->range;
					sd->state.lr_flag = 1;
					if(calclimit == 2)
						run_script(sd->inventory_data[idx]->use_script,0,sd->bl.id,0);
					run_script(sd->inventory_data[idx]->equip_script,0,sd->bl.id,0);
					sd->state.lr_flag = 0;
				} else {
					// �񓁗�����ȊO
					sd->watk  += sd->inventory_data[idx]->atk;
					if((r = sd->status.inventory[idx].refine) > 0)
						sd->watk2 += refine_db[wlv].safety_bonus[r-1];	// ���B�U����
#ifndef PRE_RENEWAL
					if(sd->status.weapon != WT_BOW && r > 0)	// �|�ɂ͐��BMATK�{�[�i�X���Ȃ�
						sd->matk1 += refine_db[wlv].safety_bonus[r-1];
#endif
					if((r -= refine_db[wlv].limit) > 0)	// �ߏ萸�B�{�[�i�X
						sd->overrefine += r*refine_db[wlv].over_bonus;

					if(sd->status.inventory[idx].card[0] == 0x00ff) {	// ��������
						sd->star += (sd->status.inventory[idx].card[1]>>8);	// ���̂�����
						if(sd->star == 15)
							sd->star = 40;
						wele = sd->status.inventory[idx].card[1] & 0x0f;	// �� ��
						// �����L���O�{�[�i�X
						if(ranking_get_id2rank(*((int *)(&sd->status.inventory[idx].card[2])),RK_BLACKSMITH))
							sd->ranker_weapon_bonus = battle_config.ranker_weapon_bonus;
					}
					sd->range.attackrange += sd->inventory_data[idx]->range;
					if(calclimit == 2)
						run_script(sd->inventory_data[idx]->use_script,0,sd->bl.id,0);
					run_script(sd->inventory_data[idx]->equip_script,0,sd->bl.id,0);
				}
			} else if(itemdb_isarmor(sd->inventory_data[idx]->nameid)) {
				sd->watk  += sd->inventory_data[idx]->atk;
				if(sd->status.inventory[idx].refine > 0)
					refinedef += refine_db[0].safety_bonus[sd->status.inventory[idx].refine - 1];
				if(calclimit == 2)
					run_script(sd->inventory_data[idx]->use_script,0,sd->bl.id,0);
				run_script(sd->inventory_data[idx]->equip_script,0,sd->bl.id,0);
			}
		}
	}

	if(sd->equip_index[EQUIP_INDEX_ARROW] >= 0) { // ��
		idx = sd->equip_index[EQUIP_INDEX_ARROW];
		if(sd->inventory_data[idx]) {		// �܂������������Ă��Ȃ�
			current_equip_name_id = sd->inventory_data[idx]->nameid;
			sd->state.lr_flag = 2;
			run_script(sd->inventory_data[idx]->equip_script,0,sd->bl.id,0);
			sd->state.lr_flag = 0;
			sd->arrow_atk += sd->inventory_data[idx]->atk;
		}
	}

	sd->def += (refinedef+50)/100;

	if(sd->range.attackrange < 1)  sd->range.attackrange  = 1;
	if(sd->range.attackrange_ < 1) sd->range.attackrange_ = 1;
	if(sd->range.attackrange < sd->range.attackrange_)
		sd->range.attackrange = sd->range.attackrange_;
	if(sd->status.weapon == WT_BOW)
		sd->range.attackrange += sd->arrow_range;
	if(wele > ELE_NEUTRAL)
		sd->atk_ele = wele;
	if(wele_ > ELE_NEUTRAL)
		sd->atk_ele_ = wele_;
	if(def_ele > ELE_NEUTRAL)
		sd->def_ele = def_ele;
	if(battle_config.pet_status_support) {
		if(pele > ELE_NEUTRAL && !sd->atk_ele)
			sd->atk_ele = pele;
		if(pdef_ele > ELE_NEUTRAL && !sd->def_ele)
			sd->def_ele = pdef_ele;
	}
	sd->double_rate += sd->double_add_rate;
	sd->perfect_hit += sd->perfect_hit_add;
	sd->get_zeny_num = (sd->get_zeny_num + sd->get_zeny_add_num > 100) ? 100 : (sd->get_zeny_num + sd->get_zeny_add_num);
	sd->get_zeny_num2 = (sd->get_zeny_num2 + sd->get_zeny_add_num2 > 100) ? 100 : (sd->get_zeny_num2 + sd->get_zeny_add_num2);
	sd->splash_range += sd->splash_add_range;

	// ����ATK�T�C�Y�␳
	for(i=0; i<MAX_SIZE_FIX; i++) {
		sd->atkmods[i]  = atkmods[i][sd->weapontype1];	// �E��
		sd->atkmods_[i] = atkmods[i][sd->weapontype2];	// ����
	}

	// job�{�[�i�X��
	for(i=0; i<sd->status.job_level && i<MAX_LEVEL; i++) {
		if(job_db[sd->s_class.job].bonus[sd->s_class.upper][i] & 0x01)
			sd->paramb[0]++;
		if(job_db[sd->s_class.job].bonus[sd->s_class.upper][i] & 0x02)
			sd->paramb[1]++;
		if(job_db[sd->s_class.job].bonus[sd->s_class.upper][i] & 0x04)
			sd->paramb[2]++;
		if(job_db[sd->s_class.job].bonus[sd->s_class.upper][i] & 0x08)
			sd->paramb[3]++;
		if(job_db[sd->s_class.job].bonus[sd->s_class.upper][i] & 0x10)
			sd->paramb[4]++;
		if(job_db[sd->s_class.job].bonus[sd->s_class.upper][i] & 0x20)
			sd->paramb[5]++;
	}

	if((skill = pc_checkskill(sd,AC_OWL)) > 0) {	// �ӂ��낤�̖�
		sd->paramb[4] += skill;
	}
	if(pc_checkskill(sd,BS_HILTBINDING)) {	// �q���g�o�C���f�B���O
		sd->paramb[0] += 1;
		// �{�I�������̂��߃R�����g�A�E�g
		//sd->watk += 4;
	}
	if((skill = pc_checkskill(sd,SA_DRAGONOLOGY)) > 0) {	// �h���S�m���W�[
		sd->paramb[3] += (skill+1)>>1;
	}
	if((skill = pc_checkskill(sd,RA_RESEARCHTRAP)) > 0) {	// �g���b�v����
		sd->paramb[3] += skill;
	}

	// �}�[�_���[�{�[�i�X
	if(battle_config.pk_murderer_point > 0) {
		int point = ranking_get_point(sd,RK_PK);
		if(point >= battle_config.pk_murderer_point * 4) {
			sd->paramb[0] += 5;
			sd->paramb[1] += 5;
			sd->paramb[2] += 5;
			sd->paramb[3] += 5;
			sd->paramb[4] += 5;
			sd->paramb[5] += 5;
			sd->atk_rate  += 10;
			sd->matk_rate += 10;
		} else if(point >= battle_config.pk_murderer_point) {
			sd->paramb[0] += 3;
			sd->paramb[1] += 3;
			sd->paramb[2] += 3;
			sd->paramb[3] += 3;
			sd->paramb[4] += 3;
			sd->paramb[5] += 3;
			sd->atk_rate  += 10;
			sd->matk_rate += 10;
		}
	}
	// 1�x������łȂ�Job70�X�p�m�ror�g���X�p�m�r��+10
	if(((sd->s_class.job == PC_JOB_SNV && sd->status.job_level >= 70) || sd->s_class.job == PC_JOB_ESNV) && sd->status.die_counter == 0) {
		sd->paramb[0] += 10;
		sd->paramb[1] += 10;
		sd->paramb[2] += 10;
		sd->paramb[3] += 10;
		sd->paramb[4] += 10;
		sd->paramb[5] += 10;
	}

	// �M���h�X�L��
	if(battle_config.guild_hunting_skill_available)
	{
		struct guild *g = guild_search(sd->status.guild_id);	// �M���h�擾
		struct map_session_data *gmsd = NULL;

		if(g)
			gmsd = guild_get_guildmaster_sd(g);

		// �M���h�L && �}�X�^�[�ڑ� && ����!=�}�X�^�[ && �����}�b�v
		if(g && gmsd && (battle_config.allow_me_guild_skill == 1 || gmsd != sd) && sd->bl.m == gmsd->bl.m)
		{
			if(battle_config.guild_skill_check_range) {	// ����������s��
				int dx = abs(sd->bl.x - gmsd->bl.x);
				int dy = abs(sd->bl.y - gmsd->bl.y);
				int range;

				if(battle_config.guild_skill_effective_range > 0) {	// ���ꋗ���Ōv�Z
					range = battle_config.guild_skill_effective_range;
					if(dx <= range && dy <= range) {
						sd->paramb[0] += guild_checkskill(g,GD_LEADERSHIP);	// str
						sd->paramb[1] += guild_checkskill(g,GD_SOULCOLD);	// agi
						sd->paramb[2] += guild_checkskill(g,GD_GLORYWOUNDS);	// vit
						sd->paramb[4] += guild_checkskill(g,GD_HAWKEYES);	// dex
						sd->under_the_influence_of_the_guild_skill = range+1;	// 0>�ŉe����,�d�Ȃ�ꍇ������̂�+1
					} else {
						sd->under_the_influence_of_the_guild_skill = 0;
					}
				} else {	// �ʋ���
					int min_range = 0x7fffffff;
					range = skill_get_range(GD_LEADERSHIP,guild_skill_get_lv(g,GD_LEADERSHIP));
					if(dx <= range && dy <= range) {
						sd->paramb[0] += guild_checkskill(g,GD_LEADERSHIP);	// str
						if(min_range > range) min_range = range;
					}
					range = skill_get_range(GD_SOULCOLD,guild_skill_get_lv(g,GD_SOULCOLD));
					if(dx <= range && dy <= range) {
						sd->paramb[1] += guild_checkskill(g,GD_SOULCOLD);	// agi
						if(min_range > range) min_range = range;
					}
					range = skill_get_range(GD_GLORYWOUNDS,guild_skill_get_lv(g,GD_GLORYWOUNDS));
					if(dx <= range && dy <= range) {
						sd->paramb[2] += guild_checkskill(g,GD_GLORYWOUNDS);	// vit
						if(min_range > range) min_range = range;
					}

					range = skill_get_range(GD_HAWKEYES,guild_skill_get_lv(g,GD_HAWKEYES));
					if(dx <= range && dy <= range) {
						sd->paramb[4] += guild_checkskill(g,GD_HAWKEYES);	// dex
						if(min_range > range) min_range = range;
					}
					if(min_range == 0x7fffffff)
						sd->under_the_influence_of_the_guild_skill = 0;
					else
						sd->under_the_influence_of_the_guild_skill = min_range+1;
				}
			} else {	// �}�b�v�S��
				sd->paramb[0] += guild_checkskill(g,GD_LEADERSHIP);	// str
				sd->paramb[1] += guild_checkskill(g,GD_SOULCOLD);	// agi
				sd->paramb[2] += guild_checkskill(g,GD_GLORYWOUNDS);	// vit
				sd->paramb[4] += guild_checkskill(g,GD_HAWKEYES);	// dex
				sd->under_the_influence_of_the_guild_skill = battle_config.guild_skill_effective_range+1;
			}
		} else {
			sd->under_the_influence_of_the_guild_skill = 0;
		}
	} else {	// �}�b�v���������c������������
		sd->under_the_influence_of_the_guild_skill = 0;
	}

	// �X�e�[�^�X�ω��ɂ���{�p�����[�^�␳
	if(sd->sc.count) {
		// �W���͌���
		if(sd->sc.data[SC_CONCENTRATE].timer != -1 && sd->sc.data[SC_QUAGMIRE].timer == -1) {
			sd->paramb[1] += (sd->status.agi+sd->paramb[1]+sd->parame[1]-sd->paramcard[1])*(2+sd->sc.data[SC_CONCENTRATE].val1)/100;
			sd->paramb[4] += (sd->status.dex+sd->paramb[4]+sd->parame[4]-sd->paramcard[4])*(2+sd->sc.data[SC_CONCENTRATE].val1)/100;
		}
		// �S�X�y��ALL+20
		if(sd->sc.data[SC_INCALLSTATUS].timer != -1) {
			sd->paramb[0] += sd->sc.data[SC_INCALLSTATUS].val1;
			sd->paramb[1] += sd->sc.data[SC_INCALLSTATUS].val1;
			sd->paramb[2] += sd->sc.data[SC_INCALLSTATUS].val1;
			sd->paramb[3] += sd->sc.data[SC_INCALLSTATUS].val1;
			sd->paramb[4] += sd->sc.data[SC_INCALLSTATUS].val1;
			sd->paramb[5] += sd->sc.data[SC_INCALLSTATUS].val1;
		}

		// ��ʈꎟ�E�̍�
		if(sd->sc.data[SC_HIGH].timer != -1) {
			if(sd->status.base_level < 60) {
				if(sd->status.str < sd->status.base_level-10)  sd->paramb[0] += sd->status.base_level-10 - sd->status.str;
				if(sd->status.agi < sd->status.base_level-10)  sd->paramb[1] += sd->status.base_level-10 - sd->status.agi;
				if(sd->status.vit < sd->status.base_level-10)  sd->paramb[2] += sd->status.base_level-10 - sd->status.vit;
				if(sd->status.int_ < sd->status.base_level-10) sd->paramb[3] += sd->status.base_level-10 - sd->status.int_;
				if(sd->status.dex < sd->status.base_level-10)  sd->paramb[4] += sd->status.base_level-10 - sd->status.dex;
				if(sd->status.luk < sd->status.base_level-10)  sd->paramb[5] += sd->status.base_level-10 - sd->status.luk;
			}
			else {
				if(sd->status.str < 50)  sd->paramb[0] += 50 - sd->status.str;
				if(sd->status.agi < 50)  sd->paramb[1] += 50 - sd->status.agi;
				if(sd->status.vit < 50)  sd->paramb[2] += 50 - sd->status.vit;
				if(sd->status.int_ < 50) sd->paramb[3] += 50 - sd->status.int_;
				if(sd->status.dex < 50)  sd->paramb[4] += 50 - sd->status.dex;
				if(sd->status.luk < 50)  sd->paramb[5] += 50 - sd->status.luk;
			}
		}

		// �H���p
		if(sd->sc.data[SC_MEAL_INCSTR2].timer != -1)
			sd->paramb[0] += sd->sc.data[SC_MEAL_INCSTR2].val1;
		else if(sd->sc.data[SC_MEAL_INCSTR].timer != -1)
			sd->paramb[0] += sd->sc.data[SC_MEAL_INCSTR].val1;
		if(sd->sc.data[SC_MEAL_INCAGI2].timer != -1)
			sd->paramb[1] += sd->sc.data[SC_MEAL_INCAGI2].val1;
		else if(sd->sc.data[SC_MEAL_INCAGI].timer != -1)
			sd->paramb[1] += sd->sc.data[SC_MEAL_INCAGI].val1;
		if(sd->sc.data[SC_MEAL_INCVIT2].timer != -1)
			sd->paramb[2] += sd->sc.data[SC_MEAL_INCVIT2].val1;
		else if(sd->sc.data[SC_MEAL_INCVIT].timer != -1)
			sd->paramb[2] += sd->sc.data[SC_MEAL_INCVIT].val1;
		if(sd->sc.data[SC_MEAL_INCINT2].timer != -1)
			sd->paramb[3] += sd->sc.data[SC_MEAL_INCINT2].val1;
		else if(sd->sc.data[SC_MEAL_INCINT].timer != -1)
			sd->paramb[3] += sd->sc.data[SC_MEAL_INCINT].val1;
		if(sd->sc.data[SC_MEAL_INCDEX2].timer != -1)
			sd->paramb[4] += sd->sc.data[SC_MEAL_INCDEX2].val1;
		else if(sd->sc.data[SC_MEAL_INCDEX].timer != -1)
			sd->paramb[4] += sd->sc.data[SC_MEAL_INCDEX].val1;
		if(sd->sc.data[SC_MEAL_INCLUK2].timer != -1)
			sd->paramb[5] += sd->sc.data[SC_MEAL_INCLUK2].val1;
		else if(sd->sc.data[SC_MEAL_INCLUK].timer != -1)
			sd->paramb[5] += sd->sc.data[SC_MEAL_INCLUK].val1;

		// �^�C���M�̃X�p�[�g��� STR+10
		if(sd->sc.data[SC_SPURT].timer != -1)
			sd->paramb[0] += 10;

		// �M���h�X�L�� �Ր�Ԑ�
		if(sd->sc.data[SC_BATTLEORDER].timer != -1) {
			sd->paramb[0] += 5*sd->sc.data[SC_BATTLEORDER].val1;
			sd->paramb[3] += 5*sd->sc.data[SC_BATTLEORDER].val1;
			sd->paramb[4] += 5*sd->sc.data[SC_BATTLEORDER].val1;
		}

		if(sd->sc.data[SC_CHASEWALK_STR].timer != -1)
			sd->paramb[0] += sd->sc.data[SC_CHASEWALK_STR].val1;

		if(sd->sc.data[SC_INCREASEAGI].timer != -1)	// ���x����
			sd->paramb[1] += 2+sd->sc.data[SC_INCREASEAGI].val1;

		if(sd->sc.data[SC_DECREASEAGI].timer != -1)	// ���x�����i�I�[�o�[�X�L���d�l��AGI-50�j
			sd->paramb[1] -= (sd->sc.data[SC_DECREASEAGI].val2)? 50: 2+sd->sc.data[SC_DECREASEAGI].val1;

		if(sd->sc.data[SC_BLESSING].timer != -1) {	// �u���b�V���O
			sd->paramb[0] += sd->sc.data[SC_BLESSING].val1;
			sd->paramb[3] += sd->sc.data[SC_BLESSING].val1;
			sd->paramb[4] += sd->sc.data[SC_BLESSING].val1;
		}
		if(sd->sc.data[SC_NEN].timer != -1) {	// �O
			sd->paramb[0] += sd->sc.data[SC_NEN].val1;
			sd->paramb[3] += sd->sc.data[SC_NEN].val1;
		}
		if(sd->sc.data[SC_SUITON].timer != -1) {	// ����
			if(sd->sc.data[SC_SUITON].val3)
				sd->paramb[1] += sd->sc.data[SC_SUITON].val3;
		}

		if(sd->sc.data[SC_GLORIA].timer != -1)	// �O�����A
			sd->paramb[5] += 30;

		if(sd->sc.data[SC_LOUD].timer != -1 && sd->sc.data[SC_QUAGMIRE].timer == -1)	// ���E�h�{�C�X
			sd->paramb[0] += 4;

		if(sd->sc.data[SC_TRUESIGHT].timer != -1) {	// �g�D���[�T�C�g
			sd->paramb[0] += 5;
			sd->paramb[1] += 5;
			sd->paramb[2] += 5;
			sd->paramb[3] += 5;
			sd->paramb[4] += 5;
			sd->paramb[5] += 5;
		}
		if(sd->sc.data[SC_INCREASING].timer != -1) { // �C���N���[�V���O�A�L���A���V�[
			sd->paramb[1] += 4;
			sd->paramb[4] += 4;
		}

		// �f�B�t�F���X
		if(sd->sc.data[SC_DEFENCE].timer != -1)
			sd->paramb[2] += sd->sc.data[SC_DEFENCE].val1*2;

		if(sd->sc.data[SC_QUAGMIRE].timer != -1) {	// �N�@�O�}�C�A
			short subagi = 0;
			short subdex = 0;
#ifdef PRE_RENEWAL
			subagi = sd->status.agi * sd->sc.data[SC_QUAGMIRE].val1*10 / 100;
			subdex = sd->status.dex * sd->sc.data[SC_QUAGMIRE].val1*10 / 100;
#else
			subagi = (sd->status.agi/2 < sd->sc.data[SC_QUAGMIRE].val1*10) ? sd->status.agi/2 : sd->sc.data[SC_QUAGMIRE].val1*10;
			subdex = (sd->status.dex/2 < sd->sc.data[SC_QUAGMIRE].val1*10) ? sd->status.dex/2 : sd->sc.data[SC_QUAGMIRE].val1*10;
#endif
			if(map[sd->bl.m].flag.pvp || map[sd->bl.m].flag.gvg) {
				subagi /= 2;
				subdex /= 2;
			}
			sd->paramb[1] -= subagi;
			sd->paramb[4] -= subdex;
		}

		// �}�[�V���I�u�A�r�X
		if(sd->sc.data[SC_MARSHOFABYSS].timer != -1) {
			sd->paramb[1] -= sd->status.agi * (sd->sc.data[SC_MARSHOFABYSS].val3 / 2) / 100;
			sd->paramb[4] -= sd->status.dex * (sd->sc.data[SC_MARSHOFABYSS].val3 / 2) / 100;
		}

		if(sd->sc.data[SC_MARIONETTE].timer != -1) {
			sd->paramb[0] -= sd->status.str/2;
			sd->paramb[1] -= sd->status.agi/2;
			sd->paramb[2] -= sd->status.vit/2;
			sd->paramb[3] -= sd->status.int_/2;
			sd->paramb[4] -= sd->status.dex/2;
			sd->paramb[5] -= sd->status.luk/2;
		}

		if(sd->sc.data[SC_MARIONETTE2].timer != -1)
		{
			struct map_session_data* ssd = map_id2sd(sd->sc.data[SC_MARIONETTE2].val2);
			if(ssd) {
				if(battle_config.marionette_type) {
					sd->paramb[0] += ssd->status.str/2;
					sd->paramb[1] += ssd->status.agi/2;
					sd->paramb[2] += ssd->status.vit/2;
					sd->paramb[3] += ssd->status.int_/2;
					sd->paramb[4] += ssd->status.dex/2;
					sd->paramb[5] += ssd->status.luk/2;
				} else if(map[sd->bl.m].flag.pk) {
					// �o�j�}�b�v��MC����
					// str
					if(sd->paramb[0]+sd->parame[0]+sd->status.str < battle_config.max_marionette_pk_str)
					{
						sd->paramb[0] += ssd->status.str/2;
						if(sd->paramb[0]+sd->parame[0]+sd->status.str > battle_config.max_marionette_pk_str)
							sd->paramb[0] = battle_config.max_marionette_pk_str - sd->status.str;
					}
					// agi
					if(sd->paramb[1]+sd->parame[1]+sd->status.agi < battle_config.max_marionette_pk_agi)
					{
						sd->paramb[1] += ssd->status.agi/2;
						if(sd->paramb[1]+sd->parame[1]+sd->status.agi > battle_config.max_marionette_pk_agi)
							sd->paramb[1] = battle_config.max_marionette_pk_agi - sd->status.agi;
					}
					// vit
					if(sd->paramb[2]+sd->parame[2]+sd->status.vit < battle_config.max_marionette_pk_vit)
					{
						sd->paramb[2] += ssd->status.vit/2;
						if(sd->paramb[2]+sd->parame[2]+sd->status.vit > battle_config.max_marionette_pk_vit)
							sd->paramb[2] = battle_config.max_marionette_pk_vit - sd->status.vit;
					}
					// int
					if(sd->paramb[3]+sd->parame[3]+sd->status.int_ < battle_config.max_marionette_pk_int)
					{
						sd->paramb[3] += ssd->status.int_/2;
						if(sd->paramb[3]+sd->parame[3]+sd->status.int_ > battle_config.max_marionette_pk_int)
							sd->paramb[3] = battle_config.max_marionette_pk_int - sd->status.int_;
					}
					// dex
					if(sd->paramb[4]+sd->parame[4]+sd->status.dex < battle_config.max_marionette_pk_dex)
					{
						sd->paramb[4] += ssd->status.dex/2;
						if(sd->paramb[4]+sd->parame[4]+sd->status.dex > battle_config.max_marionette_pk_dex)
							sd->paramb[4] = battle_config.max_marionette_pk_dex - sd->status.dex;
					}
					// luk
					if(sd->paramb[5]+sd->parame[5]+sd->status.luk < battle_config.max_marionette_pk_luk)
					{
						sd->paramb[5] += ssd->status.luk/2;
						if(sd->paramb[5]+sd->parame[5]+sd->status.luk > battle_config.max_marionette_pk_luk)
							sd->paramb[5] = battle_config.max_marionette_pk_luk - sd->status.luk;
					}
				} else if(map[sd->bl.m].flag.pvp) {
					// �o�u�o�}�b�v��MC����
					// str
					if(sd->paramb[0]+sd->parame[0]+sd->status.str < battle_config.max_marionette_pvp_str)
					{
						sd->paramb[0] += ssd->status.str/2;
						if(sd->paramb[0]+sd->parame[0]+sd->status.str > battle_config.max_marionette_pvp_str)
							sd->paramb[0] = battle_config.max_marionette_pvp_str - sd->status.str;
					}
					// agi
					if(sd->paramb[1]+sd->parame[1]+sd->status.agi < battle_config.max_marionette_pvp_agi)
					{
						sd->paramb[1] += ssd->status.agi/2;
						if(sd->paramb[1]+sd->parame[1]+sd->status.agi > battle_config.max_marionette_pvp_agi)
							sd->paramb[1] = battle_config.max_marionette_pvp_agi - sd->status.agi;
					}
					// vit
					if(sd->paramb[2]+sd->parame[2]+sd->status.vit < battle_config.max_marionette_pvp_vit)
					{
						sd->paramb[2] += ssd->status.vit/2;
						if(sd->paramb[2]+sd->parame[2]+sd->status.vit > battle_config.max_marionette_pvp_vit)
							sd->paramb[2] = battle_config.max_marionette_pvp_vit - sd->status.vit;
					}
					// int
					if(sd->paramb[3]+sd->parame[3]+sd->status.int_ < battle_config.max_marionette_pvp_int)
					{
						sd->paramb[3] += ssd->status.int_/2;
						if(sd->paramb[3]+sd->parame[3]+sd->status.int_ > battle_config.max_marionette_pvp_int)
							sd->paramb[3] = battle_config.max_marionette_pvp_int - sd->status.int_;
					}
					// dex
					if(sd->paramb[4]+sd->parame[4]+sd->status.dex < battle_config.max_marionette_pvp_dex)
					{
						sd->paramb[4] += ssd->status.dex/2;
						if(sd->paramb[4]+sd->parame[4]+sd->status.dex > battle_config.max_marionette_pvp_dex)
							sd->paramb[4] = battle_config.max_marionette_pvp_dex - sd->status.dex;
					}
					// luk
					if(sd->paramb[5]+sd->parame[5]+sd->status.luk < battle_config.max_marionette_pvp_luk)
					{
						sd->paramb[5] += ssd->status.luk/2;
						if(sd->paramb[5]+sd->parame[5]+sd->status.luk > battle_config.max_marionette_pvp_luk)
							sd->paramb[5] = battle_config.max_marionette_pvp_luk - sd->status.luk;
					}
				} else if(map[sd->bl.m].flag.gvg) {
					// �f�u�f�}�b�v��MC����
					// str
					if(sd->paramb[0]+sd->parame[0]+sd->status.str < battle_config.max_marionette_gvg_str)
					{
						sd->paramb[0] += ssd->status.str/2;
						if(sd->paramb[0]+sd->parame[0]+sd->status.str > battle_config.max_marionette_gvg_str)
							sd->paramb[0] = battle_config.max_marionette_gvg_str - sd->status.str;
					}
					// agi
					if(sd->paramb[1]+sd->parame[1]+sd->status.agi < battle_config.max_marionette_gvg_agi)
					{
						sd->paramb[1] += ssd->status.agi/2;
						if(sd->paramb[1]+sd->parame[1]+sd->status.agi > battle_config.max_marionette_gvg_agi)
							sd->paramb[1] = battle_config.max_marionette_gvg_agi - sd->status.agi;
					}
					// vit
					if(sd->paramb[2]+sd->parame[2]+sd->status.vit < battle_config.max_marionette_gvg_vit)
					{
						sd->paramb[2] += ssd->status.vit/2;
						if(sd->paramb[2]+sd->parame[2]+sd->status.vit > battle_config.max_marionette_gvg_vit)
							sd->paramb[2] = battle_config.max_marionette_gvg_vit - sd->status.vit;
					}
					// int
					if(sd->paramb[3]+sd->parame[3]+sd->status.int_ < battle_config.max_marionette_gvg_int)
					{
						sd->paramb[3] += ssd->status.int_/2;
						if(sd->paramb[3]+sd->parame[3]+sd->status.int_ > battle_config.max_marionette_gvg_int)
							sd->paramb[3] = battle_config.max_marionette_gvg_int - sd->status.int_;
					}
					// dex
					if(sd->paramb[4]+sd->parame[4]+sd->status.dex < battle_config.max_marionette_gvg_dex)
					{
						sd->paramb[4] += ssd->status.dex/2;
						if(sd->paramb[4]+sd->parame[4]+sd->status.dex > battle_config.max_marionette_gvg_dex)
							sd->paramb[4] = battle_config.max_marionette_gvg_dex - sd->status.dex;
					}
					// luk
					if(sd->paramb[5]+sd->parame[5]+sd->status.luk < battle_config.max_marionette_gvg_luk)
					{
						sd->paramb[5] += ssd->status.luk/2;
						if(sd->paramb[5]+sd->parame[5]+sd->status.luk > battle_config.max_marionette_gvg_luk)
							sd->paramb[5] = battle_config.max_marionette_gvg_luk - sd->status.luk;
					}
				} else {
					// �ʏ��MC����
					// str
					if(sd->paramb[0]+sd->parame[0]+sd->status.str < battle_config.max_marionette_str)
					{
						sd->paramb[0] += ssd->status.str/2;
						if(sd->paramb[0]+sd->parame[0]+sd->status.str > battle_config.max_marionette_str)
							sd->paramb[0] = battle_config.max_marionette_str - sd->status.str;
					}
					// agi
					if(sd->paramb[1]+sd->parame[1]+sd->status.agi < battle_config.max_marionette_agi)
					{
						sd->paramb[1] += ssd->status.agi/2;
						if(sd->paramb[1]+sd->parame[1]+sd->status.agi > battle_config.max_marionette_agi)
							sd->paramb[1] = battle_config.max_marionette_agi - sd->status.agi;
					}
					// vit
					if(sd->paramb[2]+sd->parame[2]+sd->status.vit < battle_config.max_marionette_vit)
					{
						sd->paramb[2] += ssd->status.vit/2;
						if(sd->paramb[2]+sd->parame[2]+sd->status.vit > battle_config.max_marionette_vit)
							sd->paramb[2] = battle_config.max_marionette_vit - sd->status.vit;
					}
					// int
					if(sd->paramb[3]+sd->parame[3]+sd->status.int_ < battle_config.max_marionette_int)
					{
						sd->paramb[3] += ssd->status.int_/2;
						if(sd->paramb[3]+sd->parame[3]+sd->status.int_ > battle_config.max_marionette_int)
							sd->paramb[3] = battle_config.max_marionette_int - sd->status.int_;
					}
					// dex
					if(sd->paramb[4]+sd->parame[4]+sd->status.dex < battle_config.max_marionette_dex)
					{
						sd->paramb[4] += ssd->status.dex/2;
						if(sd->paramb[4]+sd->parame[4]+sd->status.dex > battle_config.max_marionette_dex)
							sd->paramb[4] = battle_config.max_marionette_dex - sd->status.dex;
					}
					// luk
					if(sd->paramb[5]+sd->parame[5]+sd->status.luk < battle_config.max_marionette_luk)
					{
						sd->paramb[5] += ssd->status.luk/2;
						if(sd->paramb[5]+sd->parame[5]+sd->status.luk > battle_config.max_marionette_luk)
							sd->paramb[5] = battle_config.max_marionette_luk - sd->status.luk;
					}
				}
			}
		}
		if(sd->sc.data[SC_WE_FEMALE].timer != -1) {	// ���Ȃ��ɐs�����܂�
			if(sd->sc.data[SC_WE_FEMALE].val2 == 1) {
				// �����͑S��-1
				sd->paramb[0]--;
				sd->paramb[1]--;
				sd->paramb[2]--;
				sd->paramb[3]--;
				sd->paramb[4]--;
				sd->paramb[5]--;
			} else if(sd->sc.data[SC_WE_FEMALE].val2 == 2) {
				// ����͑S��+1
				sd->paramb[0]++;
				sd->paramb[1]++;
				sd->paramb[2]++;
				sd->paramb[3]++;
				sd->paramb[4]++;
				sd->paramb[5]++;
			}
		}
		if(sd->sc.data[SC_TURISUSS].timer != -1)	// �W���C�A���g�O���[�X
			sd->paramb[0] += 30;
		if(sd->sc.data[SC_LAUDAAGNUS].timer != -1)	// ���E�_�A�O�k�X
			sd->paramb[2] += sd->sc.data[SC_LAUDAAGNUS].val2;
		if(sd->sc.data[SC_LAUDARAMUS].timer != -1)	// ���E�_�����X
			sd->paramb[5] += sd->sc.data[SC_LAUDARAMUS].val2;
		if(sd->sc.data[SC_INSPIRATION].timer != -1) {	// �C���X�s���[�V����
			int param = sd->status.base_level / 10 + sd->status.job_level / 5;
			sd->paramb[0] += param;
			sd->paramb[1] += param;
			sd->paramb[2] += param;
			sd->paramb[3] += param;
			sd->paramb[4] += param;
			sd->paramb[5] += param;
		}
		if(sd->sc.data[SC_MANDRAGORA].timer != -1)	// �n�E�����O�I�u�}���h���S��
			sd->paramb[3] -= sd->sc.data[SC_MANDRAGORA].val2;
		if(sd->sc.data[SC_BANANA_BOMB].timer != -1)		// �o�i�i���e
			sd->paramb[5] -= sd->paramb[5] * sd->sc.data[SC_BANANA_BOMB].val1 / 100;
		if(sd->sc.data[SC_SAVAGE_STEAK].timer != -1)			// �T�x�[�W�̊ۏĂ�
			sd->paramb[0] += sd->sc.data[SC_SAVAGE_STEAK].val1;
		if(sd->sc.data[SC_DROCERA_HERB_STEAMED].timer != -1)	// �h���Z���̃n�[�u��
			sd->paramb[1] += sd->sc.data[SC_DROCERA_HERB_STEAMED].val1;
		if(sd->sc.data[SC_MINOR_BBQ].timer != -1)				// �~�m�^�E���X�̋��J���r
			sd->paramb[2] += sd->sc.data[SC_MINOR_BBQ].val1;
		if(sd->sc.data[SC_COCKTAIL_WARG_BLOOD].timer != -1)		// �J�N�e���E�H�[�O�u���b�h
			sd->paramb[3] += sd->sc.data[SC_COCKTAIL_WARG_BLOOD].val1;
		if(sd->sc.data[SC_SIROMA_ICE_TEA].timer != -1)			// �V���}�A�C�X�e�B�[
			sd->paramb[4] += sd->sc.data[SC_SIROMA_ICE_TEA].val1;
		if(sd->sc.data[SC_PUTTI_TAILS_NOODLES].timer != -1)		// �v�e�B�b�g�̂����ۖ�
			sd->paramb[5] += sd->sc.data[SC_PUTTI_TAILS_NOODLES].val1;
		if(sd->sc.data[SC_STOMACHACHE].timer != -1) {		// ����
			sd->paramb[0] -= sd->sc.data[SC_STOMACHACHE].val1;
			sd->paramb[1] -= sd->sc.data[SC_STOMACHACHE].val1;
			sd->paramb[2] -= sd->sc.data[SC_STOMACHACHE].val1;
			sd->paramb[3] -= sd->sc.data[SC_STOMACHACHE].val1;
			sd->paramb[4] -= sd->sc.data[SC_STOMACHACHE].val1;
			sd->paramb[5] -= sd->sc.data[SC_STOMACHACHE].val1;
		}
		if(sd->sc.data[SC_HARMONIZE].timer != -1) {	// �n�[���i�C�Y
			sd->parame[0] = 0;
			sd->parame[1] = 0;
			sd->parame[2] = 0;
			sd->parame[3] = 0;
			sd->parame[4] = 0;
			sd->parame[5] = 0;
			sd->paramb[0] = sd->sc.data[SC_HARMONIZE].val2;
			sd->paramb[1] = sd->sc.data[SC_HARMONIZE].val2;
			sd->paramb[2] = sd->sc.data[SC_HARMONIZE].val2;
			sd->paramb[3] = sd->sc.data[SC_HARMONIZE].val2;
			sd->paramb[4] = sd->sc.data[SC_HARMONIZE].val2;
			sd->paramb[5] = sd->sc.data[SC_HARMONIZE].val2;
		}
		if(sd->sc.data[SC_KYOUGAKU].timer != -1) {	// ���p -����-
			sd->paramb[0] -= sd->sc.data[SC_KYOUGAKU].val2;
			sd->paramb[1] -= sd->sc.data[SC_KYOUGAKU].val2;
			sd->paramb[2] -= sd->sc.data[SC_KYOUGAKU].val2;
			sd->paramb[3] -= sd->sc.data[SC_KYOUGAKU].val2;
			sd->paramb[4] -= sd->sc.data[SC_KYOUGAKU].val2;
			sd->paramb[5] -= sd->sc.data[SC_KYOUGAKU].val2;
		}
	}

	sd->paramc[0] = sd->status.str  + sd->paramb[0] + sd->parame[0];
	sd->paramc[1] = sd->status.agi  + sd->paramb[1] + sd->parame[1];
	sd->paramc[2] = sd->status.vit  + sd->paramb[2] + sd->parame[2];
	sd->paramc[3] = sd->status.int_ + sd->paramb[3] + sd->parame[3];
	sd->paramc[4] = sd->status.dex  + sd->paramb[4] + sd->parame[4];
	sd->paramc[5] = sd->status.luk  + sd->paramb[5] + sd->parame[5];

	for(i=0; i<6; i++) {
		if(sd->paramc[i] < 0)
			sd->paramc[i] = 0;
	}

	// BASEATK�v�Z
	if( sd->status.weapon == WT_BOW ||
	    sd->status.weapon == WT_MUSICAL ||
	    sd->status.weapon == WT_WHIP ||
	    (sd->status.weapon >= WT_HANDGUN && sd->status.weapon <= WT_GRENADE) ) {
		str = sd->paramc[4];
		dex = sd->paramc[0];
	} else {
		str = sd->paramc[0];
		dex = sd->paramc[4];
	}
#ifdef PRE_RENEWAL
	dstr = str/10;

	sd->base_atk += str + dstr*dstr + dex/5 + sd->paramc[5]/5;
	sd->matk1 += sd->paramc[3]+(sd->paramc[3]/5)*(sd->paramc[3]/5);
	sd->matk2 += sd->paramc[3]+(sd->paramc[3]/7)*(sd->paramc[3]/7);

	if(sd->matk1 < sd->matk2) {
		int temp = sd->matk2;
		sd->matk2 = sd->matk1;
		sd->matk1 = temp;
	}

	sd->hit      += sd->paramc[4] + sd->status.base_level;
	sd->flee     += sd->paramc[1] + sd->status.base_level;
	sd->def2     += sd->paramc[2];
	sd->mdef2    += sd->paramc[3];
	sd->flee2    += sd->paramc[5] + 10;
	sd->critical += sd->paramc[5] * 3 + 10;
#else
	dstr = sd->paramc[3]/8;

	sd->base_atk += (int)(str + dex/5. + sd->paramc[5]/3. + sd->status.base_level/4.);
	sd->matk2    += sd->paramc[3] + (dstr*dstr*dstr)/40 + sd->paramc[5]/3;

	sd->hit      += 175 + sd->paramc[4] + sd->paramc[5]/3 + sd->status.base_level;
	sd->flee     += 100 + sd->paramc[1] + sd->paramc[5]/5 + sd->status.base_level;
	sd->def2     += (int)(sd->paramc[2]/2. + sd->status.base_level/2. + sd->paramc[1]/5.);
	sd->mdef2    += (int)(sd->paramc[3] + sd->status.base_level/4. + sd->paramc[2]/5. + sd->paramc[4]/5.);
	sd->flee2    += sd->paramc[5] + 10;
	sd->critical += sd->paramc[5]*33/10 + 10;
#endif

	// �A�C�e���␳
	if(sd->sc.count > 0) {
		if(sd->sc.data[SC_MEAL_INCATK].timer != -1)
			sd->base_atk += sd->sc.data[SC_MEAL_INCATK].val1;
		if(sd->sc.data[SC_MEAL_INCMATK].timer != -1) {
			sd->matk1 += sd->sc.data[SC_MEAL_INCMATK].val1;
			sd->matk2 += sd->sc.data[SC_MEAL_INCMATK].val1;
		}
		if(sd->sc.data[SC_MEAL_INCHIT].timer != -1)
			sd->hit += sd->sc.data[SC_MEAL_INCHIT].val1;
		if(sd->sc.data[SC_MEAL_INCFLEE].timer != -1)
			sd->flee += sd->sc.data[SC_MEAL_INCFLEE].val1;
		if(sd->sc.data[SC_MEAL_INCFLEE2].timer != -1)
			sd->flee2 += sd->sc.data[SC_MEAL_INCFLEE2].val1;
		if(sd->sc.data[SC_MEAL_INCCRITICAL].timer != -1)
			sd->critical += sd->sc.data[SC_MEAL_INCCRITICAL].val1*10;
		if(sd->sc.data[SC_MEAL_INCDEF].timer != -1)
			sd->def += sd->sc.data[SC_MEAL_INCDEF].val1;
		if(sd->sc.data[SC_MEAL_INCMDEF].timer != -1)
			sd->mdef += sd->sc.data[SC_MEAL_INCMDEF].val1;
		if(sd->sc.data[SC_MONSTER_TRANSFORM].timer != -1 && sd->sc.data[SC_MONSTER_TRANSFORM].val1 == 1109)
			sd->hit += 5;
	}

	if(sd->sc.data[SC_MADNESSCANCEL].timer != -1) {	// �}�b�h�l�X�L�����Z���[
		sd->base_atk += 100;
	}
	if(sd->sc.data[SC_GATLINGFEVER].timer != -1) {	// �K�g�����O�t�B�[�o�[
		sd->base_atk += 20+(sd->sc.data[SC_GATLINGFEVER].val1*10);
	}
	if(sd->sc.data[SC_VOLCANO].timer != -1
#ifdef PRE_RENEWAL
		&& sd->def_ele == ELE_FIRE
#endif
	) {	// �{���P�[�m
		sd->base_atk += sd->sc.data[SC_VOLCANO].val3;
	}
#ifdef PRE_RENEWAL
	if(sd->sc.data[SC_DRUMBATTLE].timer != -1) {	// �푾�ۂ̋���
		sd->base_atk += sd->sc.data[SC_DRUMBATTLE].val2;
		//idx = sd->equip_index[EQUIP_INDEX_LARM];
		// ����ɂ͓K�p���Ȃ�
		//if(idx >= 0 && sd->inventory_data[idx] && sd->inventory_data[idx]->type == ITEMTYPE_ARMOR)
		//	sd->watk_ += sd->sc.data[SC_DRUMBATTLE].val2;
	}
#endif

	if(sd->base_atk < 1)
		sd->base_atk = 1;
	if(sd->critical_rate != 100)
		sd->critical = (sd->critical*sd->critical_rate)/100;
	if(sd->critical < 10)
		sd->critical = 10;
	if(sd->hit_rate != 100)
		sd->hit = (sd->hit*sd->hit_rate)/100;
	if(sd->hit < 1) sd->hit = 1;
	if(sd->flee_rate != 100)
		sd->flee = (sd->flee*sd->flee_rate)/100;
	if(sd->flee < 1) sd->flee = 1;
	if(sd->flee2_rate != 100)
		sd->flee2 = (sd->flee2*sd->flee2_rate)/100;
	if(sd->flee2 < 10) sd->flee2 = 10;
	if(sd->def_rate != 100)
		sd->def = (sd->def*sd->def_rate)/100;
	if(sd->def2_rate != 100)
		sd->def2 = (sd->def2*sd->def2_rate)/100;
	if(sd->def2 < 1) sd->def2 = 1;
	if(sd->mdef_rate != 100)
		sd->mdef = (sd->mdef*sd->mdef_rate)/100;
	if(sd->mdef2_rate != 100)
		sd->mdef2 = (sd->mdef2*sd->mdef2_rate)/100;
	if(sd->mdef2 < 1) sd->mdef2 = 1;

	// �V���O���A�N�V����
	if(sd->status.weapon >= WT_HANDGUN && sd->status.weapon <= WT_GRENADE && (skill = pc_checkskill(sd,GS_SINGLEACTION)) > 0)
	{
		sd->hit += skill*2;
	}
	// ���z�ƌ��Ɛ��̈���
	if((skill = pc_checkskill(sd,SG_DEVIL)) > 0 && sd->status.job_level >= 50)
	{
		clif_status_load_id(sd,SI_DEVIL,1);
	}

	// ���z�ƌ��Ɛ��̗Z��
	if(sd->sc.data[SC_FUSION].timer != -1)
	{
		sd->perfect_hit += 100;
	}

	if(sd->sc.data[SC_WEDDING].timer != -1)
		b_class = PC_CLASS_WE;
	else if(sd->sc.data[SC_SANTA].timer != -1)
		b_class = PC_CLASS_ST;
	else if(sd->sc.data[SC_SUMMER].timer != -1)
		b_class = PC_CLASS_SU;

	if((skill = pc_checkskill(sd,AC_VULTURE)) > 0) {	// ���V�̖�
		sd->hit += skill;
		if(sd->status.weapon == WT_BOW)
			sd->range.attackrange += skill;
	}
	if((skill = pc_checkskill(sd,GS_SNAKEEYE)) > 0) {	// �X�l�[�N�A�C
		if(sd->status.weapon >= WT_HANDGUN && sd->status.weapon <= WT_GRENADE)
		{
			sd->range.attackrange += skill;
			sd->hit += skill;
		}
	}
	if((skill = pc_checkskill(sd,BS_WEAPONRESEARCH)) > 0)	// ���팤���̖���������
		sd->hit += skill*2;
	if((sd->status.weapon == WT_DAGGER || sd->status.weapon == WT_1HSWORD) && ((skill = pc_checkskill(sd,GN_TRAINING_SWORD)) > 0))	// ���C���̖���������
		sd->hit += skill*3;

	if((sd->s_class.job == PC_JOB_SNV || sd->s_class.job == PC_JOB_ESNV) && sd->status.base_level >= 99)
	{
		if(pc_isupper(sd))
			sd->status.max_hp += 2000*(100 + sd->paramc[2])/100 * battle_config.upper_hp_rate/100;
		else if(pc_isbaby(sd))	// �{�q�̏ꍇ�ő�HP70%
			sd->status.max_hp += 2000*(100 + sd->paramc[2])/100 * battle_config.baby_hp_rate/100;
		else
			sd->status.max_hp += 2000*(100 + sd->paramc[2])/100 * battle_config.normal_hp_rate/100;
	}

	if((skill = pc_checkskill(sd,CR_TRUST)) > 0) { // �t�F�C�X
		sd->status.max_hp    += skill*200;
		sd->subele[ELE_HOLY] += skill*5;
	}

	if((skill = pc_checkskill(sd,BS_SKINTEMPER)) > 0) { // �X�L���e���p�����O
		sd->subele[ELE_FIRE]    += skill*4;
		sd->subele[ELE_NEUTRAL] += skill*1;
	}

	// bAtkRange2,bAtkRangeRate2�̎˒��v�Z
	sd->range.attackrange  += sd->range.add_attackrange;
	sd->range.attackrange_ += sd->range.add_attackrange;
	sd->range.attackrange  = sd->range.attackrange  * sd->range.add_attackrange_rate / 100;
	sd->range.attackrange_ = sd->range.attackrange_ * sd->range.add_attackrange_rate / 100;
	if(sd->range.attackrange < 1)  sd->range.attackrange  = 1;
	if(sd->range.attackrange_ < 1) sd->range.attackrange_ = 1;
	if(sd->range.attackrange < sd->range.attackrange_)
		sd->range.attackrange = sd->range.attackrange_;

	blv = (sd->status.base_level > 0)? sd->status.base_level - 1: 0;

	// �ő�HP�v�Z
	calc_val = job_db[sd->s_class.job].hp_base[blv] * (100 + sd->paramc[2]) / 100;

	if(pc_isupper(sd))	// �]���E�̏ꍇ�ő�HP25%UP
		sd->status.max_hp += calc_val * battle_config.upper_hp_rate / 100;
	else if(pc_isbaby(sd))	// �{�q�̏ꍇ�ő�HP70%
		sd->status.max_hp += calc_val * battle_config.baby_hp_rate / 100;
	else
		sd->status.max_hp += calc_val * battle_config.normal_hp_rate / 100;

	sd->status.max_hp += sd->parame[2] - sd->paramcard[2];

	if(sd->hprate != 100)
		sd->status.max_hp = sd->status.max_hp * sd->hprate / 100;

	if(sd->sc.data[SC_VENOMBLEED].timer != -1) {	// �x�i���u���[�h
		sd->status.max_hp -= (int)((atn_bignumber)sd->status.max_hp * sd->sc.data[SC_VENOMBLEED].val2 / 100);
	}
	if(sd->sc.data[SC__WEAKNESS].timer != -1) {	// �}�X�J���[�h �F �E�B�[�N�l�X
		sd->status.max_hp -= (int)((atn_bignumber)sd->status.max_hp * (sd->sc.data[SC__WEAKNESS].val1 * 10) / 100);
	}
	if(sd->sc.data[SC_BERSERK].timer != -1) {	// �o�[�T�[�N
		sd->status.max_hp *= 3;
	}
	if(sd->sc.data[SC_INCMHP2].timer != -1) {
		sd->status.max_hp = (int)((atn_bignumber)sd->status.max_hp * (100 + sd->sc.data[SC_INCMHP2].val1) / 100);
	}
	if(sd->sc.data[SC_EPICLESIS].timer != -1) {		// �G�s�N���V�X
		sd->status.max_hp = (int)((atn_bignumber)sd->status.max_hp * (100 + sd->sc.data[SC_EPICLESIS].val2) / 100);
	}

	// �ő�SP�v�Z
	calc_val = job_db[sd->s_class.job].sp_base[blv] * (100 + sd->paramc[3]) / 100 + (sd->parame[3] - sd->paramcard[3]);

	if(pc_isupper(sd))	// �]���E�̏ꍇ�ő�SP125%
		sd->status.max_sp += calc_val * battle_config.upper_sp_rate / 100;
	else if(pc_isbaby(sd))	// �{�q�̏ꍇ�ő�SP70%
		sd->status.max_sp += calc_val * battle_config.baby_sp_rate / 100;
	else
		sd->status.max_sp += calc_val * battle_config.normal_sp_rate / 100;

	if(sd->sprate != 100)
		sd->status.max_sp = sd->status.max_sp * sd->sprate / 100;

	if((skill = pc_checkskill(sd,HP_MEDITATIO)) > 0) // ���f�B�^�e�B�I
		sd->status.max_sp += sd->status.max_sp * skill / 100;
	if((skill = pc_checkskill(sd,HW_SOULDRAIN)) > 0) // �\�E���h���C��
		sd->status.max_sp += sd->status.max_sp * 2 * skill / 100;
	if((skill = pc_checkskill(sd,SL_KAINA)) > 0)	// �J�C�i
		sd->status.max_sp += 30 * skill;
	if((skill = pc_checkskill(sd,RA_RESEARCHTRAP)) > 0)	// �g���b�v����
		sd->status.max_sp += 200 + (skill*20);
	if((skill = pc_checkskill(sd,WM_LESSON)) > 0)	// ���b�X��
		sd->status.max_sp += skill * 30;

	if(sd->sc.data[SC_INCMSP2].timer != -1) {
		sd->status.max_sp = (int)((atn_bignumber)sd->status.max_sp * (100 + sd->sc.data[SC_INCMSP2].val1) / 100);
	}

	// SP����
	if((skill = pc_checkskill(sd,HP_MANARECHARGE)) > 0) {	// �}�i���`���[�W
		sd->dsprate -= skill * 4;
		if(sd->dsprate < 0)
			sd->dsprate = 0;
	}

	// ���R��HP
	sd->nhealhp = 1 + (sd->paramc[2]/5) + (sd->status.max_hp/200);
	if((skill = pc_checkskill(sd,SM_RECOVERY)) > 0) {	// HP�񕜗͌���
		sd->nshealhp = skill * 5 + sd->status.max_hp * skill / 500;
		if(sd->nshealhp > 0x7fff)
			sd->nshealhp = 0x7fff;
	}
	if((skill = pc_checkskill(sd,TK_HPTIME)) > 0) {	// ���炩�ȋx��
		sd->regen.tk_hp = skill*30 + (sd->status.max_hp*skill/500);
		if(sd->regen.tk_hp > 0x7fff)
			sd->regen.tk_hp = 0x7fff;
	}
	if(sd->sc.data[SC_GENTLETOUCH_REVITALIZE].timer != -1) {	// �_�� -��-
		sd->nhealhp += sd->nhealhp * (50 + sd->sc.data[SC_GENTLETOUCH_REVITALIZE].val1 * 30) / 100;
		if(sd->nhealhp > 0x7fff)
			sd->nhealhp = 0x7fff;
	}
	if(sd->sc.data[SC_EXTRACT_WHITE_POTION_Z].timer != -1) {	// �Z�k�z���C�g�|�[�V����Z
		sd->nhealhp += sd->nhealhp * sd->sc.data[SC_EXTRACT_WHITE_POTION_Z].val1 / 100;
		if(sd->nhealhp > 0x7fff)
			sd->nhealhp = 0x7fff;
	}
	if(sd->sc.data[SC_BERSERK].timer != -1) {
		sd->nhealhp = 0;
	}
	// ���R��SP
	sd->nhealsp = 1 + (sd->paramc[3]/6) + (sd->status.max_sp/100);
	if(sd->paramc[3] >= 120)
		sd->nhealsp += ((sd->paramc[3]-120)>>1) + 4;
	if((skill = pc_checkskill(sd,MG_SRECOVERY)) > 0) {	// SP�񕜗͌���
		sd->nshealsp = skill*3 + (sd->status.max_sp*skill/500);
		if(sd->nshealsp > 0x7fff)
			sd->nshealsp = 0x7fff;
	}
	if((skill = pc_checkskill(sd,NJ_NINPOU)) > 0) {	// �E�@�C��
		sd->nshealsp = skill*3 + (sd->status.max_sp*skill/500);
		if(sd->nshealsp > 0x7fff)
			sd->nshealsp = 0x7fff;
	}
	if((skill = pc_checkskill(sd,WM_LESSON)) > 0) {	// ���b�X��
		sd->nshealsp = 3 + skill * 3;
		if(sd->nshealsp > 0x7fff)
			sd->nshealsp = 0x7fff;
	}

	if((skill = pc_checkskill(sd,MO_SPIRITSRECOVERY)) > 0) {
		sd->nsshealhp = skill*4 + (sd->status.max_hp*skill/500);
		sd->nsshealsp = skill*2 + (sd->status.max_sp*skill/500);
		if(sd->nsshealhp > 0x7fff)
			sd->nsshealhp = 0x7fff;
		if(sd->nsshealsp > 0x7fff)
			sd->nsshealsp = 0x7fff;
	}
	if((skill = pc_checkskill(sd,TK_SPTIME)) > 0) { // �y�����x��
		sd->regen.tk_sp = skill*3 + (sd->status.max_sp*skill/500);
		if(sd->regen.tk_sp > 0x7fff)
			sd->regen.tk_sp = 0x7fff;
	}
	if(sd->hprecov_rate != 100) {
		sd->nhealhp = sd->nhealhp*sd->hprecov_rate/100;
		if(sd->nhealhp < 1)
			sd->nhealhp = 1;
	}
	if(sd->sprecov_rate != 100) {
		sd->nhealsp = sd->nhealsp*sd->sprecov_rate/100;
		if(sd->nhealsp < 1)
			sd->nhealsp = 1;
	}
	if((skill = pc_checkskill(sd,HP_MEDITATIO)) > 0) {
		// ���f�B�^�e�B�I��SPR�ł͂Ȃ����R�񕜂ɂ�����
		sd->nhealsp += (sd->nhealsp)*3*skill/100;
		if(sd->nhealsp > 0x7fff)
			sd->nhealsp = 0x7fff;
	}
	if(sd->sc.data[SC_VITATA_500].timer != -1) {	// �r�^�^500
		sd->nhealsp += sd->nhealsp * sd->sc.data[SC_VITATA_500].val1 / 100;
		if(sd->nhealsp > 0x7fff)
			sd->nhealsp = 0x7fff;
	}

	// �푰�ϐ��i����ł����́H �f�B�o�C���v���e�N�V�����Ɠ������������邩���j
	if((skill = pc_checkskill(sd,SA_DRAGONOLOGY)) > 0) {	// �h���S�m���W�[
		skill = skill*4;
		sd->addrace[RCT_DRAGON]  += skill;
		sd->addrace_[RCT_DRAGON] += skill;
		sd->subrace[RCT_DRAGON]  += skill;
	}
	// Flee�㏸
	if((skill = pc_checkskill(sd,TF_MISS)) > 0) {	// ��𗦑���
		if(sd->s_class.job == PC_JOB_AS || sd->s_class.job == PC_JOB_RG || sd->s_class.job == PC_JOB_GC || sd->s_class.job == PC_JOB_SC)
			sd->flee += skill*4;
		else
			sd->flee += skill*3;
	}
	if((skill = pc_checkskill(sd,MO_DODGE)) > 0)	// ���؂�
		sd->flee += (skill*3)>>1;
	if(sd->sc.count > 0) {
		if(sd->sc.data[SC_INCFLEE].timer != -1)
			sd->flee += sd->sc.data[SC_INCFLEE].val1;
		if(sd->sc.data[SC_INCFLEE2].timer != -1)
			sd->flee += sd->sc.data[SC_INCFLEE2].val1;
	}
	// Def
	if(pc_isgear(sd) && (skill = pc_checkskill(sd,NC_MAINFRAME)) > 0) {		// �����M�A�����C���t���[������
		if(skill == 1)
			sd->def += 4;
		else
			sd->def += (skill*4) - 1;
	}
#ifdef PRE_RENEWAL
	// MATK��Z����(��␳�ȊO)
	if(sd->matk_rate != 100) {
		sd->matk1 = sd->matk1 * sd->matk_rate / 100;
		sd->matk2 = sd->matk2 * sd->matk_rate / 100;
	}
#endif
	// amotion�̌v�Z
	sd->amotion = status_calc_amotion_pc(sd);
	sd->aspd = sd->amotion<<1;
	// speed�̌v�Z
	sd->speed = status_calc_speed_pc(sd,sd->speed);

	// �X�L����X�e�[�^�X�ُ�ɂ��c��̃p�����[�^�␳
	if(sd->sc.count > 0) {
		// ���z�̈��y DEF����
		if(sd->sc.data[SC_SUN_COMFORT].timer != -1)
			sd->def2 += (sd->status.base_level + sd->status.dex + sd->status.luk)/2;
			//sd->def += (sd->status.base_level + sd->status.dex + sd->status.luk + sd->paramb[4] + sd->paramb[5])/10;

		// ���̈��y
		if(sd->sc.data[SC_MOON_COMFORT].timer != -1 && (sd->bl.m == sd->feel_index[1] || sd->sc.data[SC_MIRACLE].timer != -1))
			sd->flee += (sd->status.base_level + sd->status.dex + sd->status.luk)/10;
			//sd->flee += (sd->status.base_level + sd->status.dex + sd->status.luk + sd->paramb[4] + sd->paramb[5])/10;

		// �N���[�Y�R���t�@�C��
		if(sd->sc.data[SC_CLOSECONFINE].timer != -1)
			sd->flee += 10;

		// ATK/DEF�ω��`
		if(sd->sc.data[SC_ANGELUS].timer != -1) {	// �G���W�F���X
#ifdef PRE_RENEWAL
			sd->def2 = sd->def2*(110+5*sd->sc.data[SC_ANGELUS].val1)/100;
#else
			sd->def2 = sd->def2+sd->paramc[2]/2*(100+5*sd->sc.data[SC_ANGELUS].val1)/100;
#endif
		}
#ifdef PRE_RENEWAL
		if(sd->sc.data[SC_IMPOSITIO].timer != -1) {// �C���|�V�e�B�I�}�k�X
			sd->watk += sd->sc.data[SC_IMPOSITIO].val1*5;
			// ����ɂ͓K�p���Ȃ�
			//idx = sd->equip_index[EQUIP_INDEX_LARM];
			//if(idx >= 0 && sd->inventory_data[idx] && sd->inventory_data[idx]->type == ITEMTYPE_ARMOR)
			//	sd->watk_ += sd->sc.data[SC_IMPOSITIO].val1*5;
		}
#endif
		if(sd->sc.data[SC__BLOODYLUST].timer != -1) {	// �u���b�f�B���X�g
			sd->def2 = sd->def2*(100 - 55) / 100;
#ifdef PRE_RENEWAL
			sd->base_atk = sd->base_atk*(100 + 32) / 100;
			sd->watk = sd->watk*(100 + 32) / 100;
#endif
			// ����ɂ͓K�p���Ȃ�
			//idx = sd->equip_index[EQUIP_INDEX_LARM];
			//if(idx >= 0 && sd->inventory_data[idx] && sd->inventory_data[idx]->type == ITEMTYPE_ARMOR)
			//	sd->watk_ = sd->watk_*(100+32)/100;
		}
		else if(sd->sc.data[SC_PROVOKE].timer != -1) {	// �v���{�b�N
			sd->def2 = sd->def2*(100 - 5 - 5 * sd->sc.data[SC_PROVOKE].val1) / 100;
#ifdef PRE_RENEWAL
			sd->base_atk = sd->base_atk*(100 + 2 + 3 * sd->sc.data[SC_PROVOKE].val1) / 100;
			sd->watk = sd->watk*(100 + 2 + 3 * sd->sc.data[SC_PROVOKE].val1) / 100;
#endif
			// ����ɂ͓K�p���Ȃ�
			//idx = sd->equip_index[EQUIP_INDEX_LARM];
			//if(idx >= 0 && sd->inventory_data[idx] && sd->inventory_data[idx]->type == ITEMTYPE_ARMOR)
			//	sd->watk_ = sd->watk_*(100+2+3*sd->sc.data[SC_PROVOKE].val1)/100;
		}
		if(sd->sc.data[SC_POISON].timer != -1)	// �ŏ��
			sd->def2 = sd->def2*75/100;

		// �^���̃^���b�g�J�[�h
		if(sd->sc.data[SC_THE_MAGICIAN].timer != -1) {
			// ATK����
			sd->base_atk = sd->base_atk * 50/100;
			sd->watk = sd->watk * 50/100;
			idx = sd->equip_index[EQUIP_INDEX_LARM];
			if(idx >= 0 && sd->inventory_data[idx] && itemdb_isweapon(sd->inventory_data[idx]->nameid))
				sd->watk_ = sd->watk_ * 50/100;
		}
		if(sd->sc.data[SC_STRENGTH].timer != -1) {
			// MATK����
			sd->matk1 = sd->matk1*50/100;
			sd->matk2 = sd->matk2*50/100;
		}
		if(sd->sc.data[SC_THE_DEVIL].timer != -1) {
			// ATK�����AMATK����
			sd->base_atk = sd->base_atk * 50/100;
			sd->watk = sd->watk * 50/100;
			idx = sd->equip_index[EQUIP_INDEX_LARM];
			if(idx >= 0 && sd->inventory_data[idx] && itemdb_isweapon(sd->inventory_data[idx]->nameid)) {
				sd->watk_ = sd->watk_ * 50/100;
			}
			sd->matk1 = sd->matk1*50/100;
			sd->matk2 = sd->matk2*50/100;
		}
		if(sd->sc.data[SC_THE_SUN].timer != -1) {
			// ATK�AMATK�A����A�����A�h��͂��S��20%����������
			sd->base_atk = sd->base_atk * 80/100;
			sd->watk = sd->watk * 80/100;
			idx = sd->equip_index[EQUIP_INDEX_LARM];
			if(idx >= 0 && sd->inventory_data[idx] && itemdb_isweapon(sd->inventory_data[idx]->nameid)) {
				sd->watk_ = sd->watk_ * 80/100;
			}
			sd->matk1 = sd->matk1*80/100;
			sd->matk2 = sd->matk2*80/100;
			sd->flee  = sd->flee * 80/100;
			sd->hit   = sd->hit * 80/100;

			sd->def  = sd->def * 80/100;
			sd->def2 = sd->def2 * 80/100;
		}

		if(sd->sc.data[SC_DRUMBATTLE].timer != -1) {	// �푾�ۂ̋���
			sd->def  += sd->sc.data[SC_DRUMBATTLE].val3;
		}
#ifdef PRE_RENEWAL
		if(sd->sc.data[SC_NIBELUNGEN].timer != -1) {	// �j�[�x�����O�̎w��
			idx = sd->equip_index[EQUIP_INDEX_RARM];
			if(idx >= 0 && sd->inventory_data[idx] && sd->inventory_data[idx]->wlv >= 4)
				sd->watk += sd->sc.data[SC_NIBELUNGEN].val2;
			// ����ɂ͓K�p���Ȃ�
			//idx = sd->equip_index[EQUIP_INDEX_LARM];
			//if(idx >= 0 && sd->inventory_data[idx] && sd->inventory_data[idx]->wlv >= 4)
			//	sd->watk_ += sd->sc.data[SC_NIBELUNGEN].val2;
		}

		if(sd->sc.data[SC_INCATK2].timer != -1) {
			sd->watk = sd->watk*(100+sd->sc.data[SC_INCATK2].val1)/100;
		}
#endif

		if(sd->sc.data[SC_SIGNUMCRUCIS].timer != -1)
			sd->def = sd->def * (100 - sd->sc.data[SC_SIGNUMCRUCIS].val2)/100;
		if(sd->sc.data[SC_ETERNALCHAOS].timer != -1)	// �G�^�[�i���J�I�X
			sd->def2 = 0;

		if(sd->sc.data[SC_CONCENTRATION].timer != -1) {	// �R���Z���g���[�V����
#ifdef PRE_RENEWAL
			sd->base_atk = sd->base_atk * (100 + 5*sd->sc.data[SC_CONCENTRATION].val1)/100;
			sd->watk = sd->watk * (100 + 5*sd->sc.data[SC_CONCENTRATION].val1)/100;
			idx = sd->equip_index[EQUIP_INDEX_LARM];
			if(idx >= 0 && sd->inventory_data[idx] && itemdb_isweapon(sd->inventory_data[idx]->nameid))
				sd->watk_ = sd->watk_ * (100 + 5*sd->sc.data[SC_CONCENTRATION].val1)/100;
#endif
			sd->def = sd->def * (100 - 5*sd->sc.data[SC_CONCENTRATION].val1)/100;
			sd->def2 = sd->def2 * (100 - 5*sd->sc.data[SC_CONCENTRATION].val1)/100;
		}

		if(sd->sc.data[SC_INCATK].timer != -1) {	// ATK�㏸ (�_��p)
			sd->watk += sd->sc.data[SC_INCATK].val1;
		}
		if(sd->sc.data[SC_ATKPOTION].timer != -1) {	// ATK�㏸
#ifdef PRE_RENEWAL
			sd->watk += sd->sc.data[SC_ATKPOTION].val1;
#else
			sd->plus_atk += sd->sc.data[SC_ATKPOTION].val1;
#endif
		}
		if(sd->sc.data[SC_ALMIGHTY].timer != -1) {	// ATK�㏸ (�H���p)
#ifdef PRE_RENEWAL
			sd->watk += sd->sc.data[SC_ALMIGHTY].val1;
#else
			sd->plus_atk += sd->sc.data[SC_ALMIGHTY].val1;
#endif
		}
		if(sd->sc.data[SC_INCMATK].timer != -1) {	// MATK�㏸ (�_��̑��p)
			sd->matk1 += sd->sc.data[SC_INCMATK].val1;
			sd->matk2 += sd->sc.data[SC_INCMATK].val1;
		}
		if(sd->sc.data[SC_MATKPOTION].timer != -1) {	// MATK�㏸
#ifdef PRE_RENEWAL
			sd->matk1 += sd->sc.data[SC_MATKPOTION].val1;
			sd->matk2 += sd->sc.data[SC_MATKPOTION].val1;
#else
			sd->plus_matk += sd->sc.data[SC_MATKPOTION].val1;
#endif
		}
		if(sd->sc.data[SC_ALMIGHTY].timer != -1) {	// MATK�㏸ (�H���p)
#ifdef PRE_RENEWAL
			sd->matk1 += sd->sc.data[SC_ALMIGHTY].val2;
			sd->matk2 += sd->sc.data[SC_ALMIGHTY].val2;
#else
			sd->plus_matk += sd->sc.data[SC_ALMIGHTY].val2;
#endif
		}
		if(sd->sc.data[SC_MINDBREAKER].timer != -1) {
#ifdef PRE_RENEWAL
			sd->matk1 += (sd->matk1*20*sd->sc.data[SC_MINDBREAKER].val1)/100;
			sd->matk2 += (sd->matk2*20*sd->sc.data[SC_MINDBREAKER].val1)/100;
#endif
			sd->mdef2 -= (sd->mdef2*12*sd->sc.data[SC_MINDBREAKER].val1)/100;
		}
		// MATK�㏸ (�����X�^�[�ϐg�̃}���h�D�[�N�A�o���V�[�p)
		if(sd->sc.data[SC_MONSTER_TRANSFORM].timer != -1 && (sd->sc.data[SC_MONSTER_TRANSFORM].val1 == 1140 || sd->sc.data[SC_MONSTER_TRANSFORM].val1 == 1867)) {
			sd->matk1 += 25;
			sd->matk2 += 25;
		}
		if(sd->sc.data[SC_ENDURE].timer != -1) {
			sd->mdef += sd->sc.data[SC_ENDURE].val1;
		}
		if(sd->sc.data[SC_ANALYZE].timer != -1) {	// �A�i���C�Y
			sd->def2  -= (sd->def2 * 14 * sd->sc.data[SC_ANALYZE].val1) / 100;
			sd->mdef2 -= (sd->mdef2 * 14 * sd->sc.data[SC_ANALYZE].val1) / 100;
		}
		if(sd->sc.data[SC_NEUTRALBARRIER].timer != -1) {	// �j���[�g�����o���A�[
			sd->def2  += (sd->def2 * (10 + 5 * sd->sc.data[SC_NEUTRALBARRIER].val1)) / 100;
			sd->mdef2 += (sd->mdef2 * (10 + 5 * sd->sc.data[SC_NEUTRALBARRIER].val1)) / 100;
		}

		// HIT/FLEE�ω��n
#ifdef PRE_RENEWAL
		if(sd->sc.data[SC_WHISTLE].timer != -1) {  // ���J
			sd->flee += sd->sc.data[SC_WHISTLE].val1 + sd->sc.data[SC_WHISTLE].val2 + sd->sc.data[SC_WHISTLE].val3;
		} else if(sd->sc.data[SC_WHISTLE_].timer != -1) {  // ���J
			sd->flee += sd->sc.data[SC_WHISTLE_].val1 + sd->sc.data[SC_WHISTLE_].val2 + sd->sc.data[SC_WHISTLE_].val3;
		}
#else
		if(sd->sc.data[SC_WHISTLE].timer != -1) {  // ���J
			sd->flee += sd->sc.data[SC_WHISTLE].val1*3 + sd->sc.data[SC_WHISTLE].val2 + sd->sc.data[SC_WHISTLE].val3;
		} else if(sd->sc.data[SC_WHISTLE_].timer != -1) {  // ���J
			sd->flee += sd->sc.data[SC_WHISTLE_].val1*3 + sd->sc.data[SC_WHISTLE_].val2 + sd->sc.data[SC_WHISTLE_].val3;
		}
#endif

#ifdef PRE_RENEWAL
		if(sd->sc.data[SC_HUMMING].timer != -1) {  // �n�~���O
			sd->hit += 10+sd->sc.data[SC_HUMMING].val1*2+sd->sc.data[SC_HUMMING].val2+sd->sc.data[SC_HUMMING].val3;
		} else if(sd->sc.data[SC_HUMMING_].timer != -1) {  // �n�~���O
			sd->hit += 10+sd->sc.data[SC_HUMMING_].val1*2+sd->sc.data[SC_HUMMING_].val2+sd->sc.data[SC_HUMMING_].val3;
		}
#else
		if(sd->sc.data[SC_HUMMING].timer != -1) {  // �n�~���O
			sd->hit += 20+sd->sc.data[SC_HUMMING].val1*2+sd->sc.data[SC_HUMMING].val2+sd->sc.data[SC_HUMMING].val3;
		} else if(sd->sc.data[SC_HUMMING_].timer != -1) {  // �n�~���O
			sd->hit += 20+sd->sc.data[SC_HUMMING_].val1*2+sd->sc.data[SC_HUMMING_].val2+sd->sc.data[SC_HUMMING_].val3;
		}
#endif

		if(sd->sc.data[SC_VIOLENTGALE].timer != -1
#ifdef PRE_RENEWAL
			&& sd->def_ele == ELE_WIND
#endif
		) {	// �o�C�I�����g�Q�C��
			sd->flee += sd->flee*sd->sc.data[SC_VIOLENTGALE].val3/100;
		}
		if(sd->sc.data[SC_BLIND].timer != -1) {	// �Í�
			sd->hit  -= sd->hit*25/100;
			sd->flee -= sd->flee*25/100;
		}
		if(sd->sc.data[SC_WINDWALK].timer != -1)	// �E�B���h�E�H�[�N
			sd->flee += sd->sc.data[SC_WINDWALK].val2;
		if(sd->sc.data[SC_SPIDERWEB].timer != -1)	// �X�p�C�_�[�E�F�u
			sd->flee -= 50;
		if(sd->sc.data[SC_TRUESIGHT].timer != -1)	// �g�D���[�T�C�g
			sd->hit += 3*(sd->sc.data[SC_TRUESIGHT].val1);
		if(sd->sc.data[SC_CONCENTRATION].timer != -1)	// �R���Z���g���[�V����
			sd->hit += 10*(sd->sc.data[SC_CONCENTRATION].val1);
		if(sd->sc.data[SC_INCHIT].timer != -1)
			sd->hit += sd->sc.data[SC_INCHIT].val1;
		if(sd->sc.data[SC_INCHIT2].timer != -1)
			sd->hit = sd->hit * (100+sd->sc.data[SC_INCHIT2].val1)/100;
		if(sd->sc.data[SC_BERSERK].timer != -1)
			sd->flee -= sd->flee*50/100;
		if(sd->sc.data[SC_INCFLEE].timer != -1)	// ���x����
			sd->flee += sd->flee*(sd->sc.data[SC_INCFLEE].val2)/100;
		if(sd->sc.data[SC_HALLUCINATIONWALK].timer != -1)	// �n���V�l�[�V�����E�H�[�N
			sd->flee += sd->sc.data[SC_HALLUCINATIONWALK].val1 * 50;
		if(sd->sc.data[SC_INFRAREDSCAN].timer != -1)	// �C���t�����b�h�X�L����
			sd->flee -= sd->flee*30/100;
#ifndef PRE_RENEWAL
		if(sd->sc.data[SC_SPEARQUICKEN].timer != -1)   // �X�s�A�N�C�b�P��
			sd->flee += 2*(sd->sc.data[SC_SPEARQUICKEN].val1);
#endif

		// �K���X�����K�[�X�L��
		if(sd->sc.data[SC_FLING].timer != -1) {		// �t���C���O
			sd->def = sd->def * (100 - 5*sd->sc.data[SC_FLING].val1)/100;
		}
		if(sd->sc.data[SC_ADJUSTMENT].timer != -1) {	// �A�W���X�g�����g
			sd->hit  -= 30;
			sd->flee += 30;
		}
		if(sd->sc.data[SC_INCREASING].timer != -1) {	// �C���N���[�V���O�A�L���A���V�[
			sd->hit += 20;
		}
		if(sd->sc.data[SC_GATLINGFEVER].timer != -1) {	// �K�g�����O�t�B�[�o�[
			sd->flee     -= sd->sc.data[SC_GATLINGFEVER].val1*5;
		}

#ifdef PRE_RENEWAL
		// �X�g�[���X�L��
		if(sd->sc.data[SC_STONESKIN].timer != -1) {
			sd->def  = sd->def  * (100 + 20 * sd->sc.data[SC_STONESKIN].val1) / 100;
			sd->mdef = sd->mdef * (100 - 20 * sd->sc.data[SC_STONESKIN].val1) / 100;
		}
		// �A���`�}�W�b�N
		if(sd->sc.data[SC_ANTIMAGIC].timer != -1) {
			sd->def  = sd->def  * (100 - 20 * sd->sc.data[SC_ANTIMAGIC].val1) / 100;
			sd->mdef = sd->mdef * (100 + 20 * sd->sc.data[SC_ANTIMAGIC].val1) / 100;
		}
#endif

		// �ϐ�
		if(sd->sc.data[SC_RESISTWATER].timer != -1)
			sd->subele[ELE_WATER] += sd->sc.data[SC_RESISTWATER].val1;
		if(sd->sc.data[SC_RESISTGROUND].timer != -1)
			sd->subele[ELE_EARTH] += sd->sc.data[SC_RESISTGROUND].val1;
		if(sd->sc.data[SC_RESISTFIRE].timer != -1)
			sd->subele[ELE_FIRE] += sd->sc.data[SC_RESISTFIRE].val1;
		if(sd->sc.data[SC_RESISTWIND].timer != -1)
			sd->subele[ELE_WIND] += sd->sc.data[SC_RESISTWIND].val1;
		if(sd->sc.data[SC_RESISTPOISON].timer != -1)
			sd->subele[ELE_POISON] += sd->sc.data[SC_RESISTPOISON].val1;
		if(sd->sc.data[SC_RESISTHOLY].timer != -1)
			sd->subele[ELE_HOLY] += sd->sc.data[SC_RESISTHOLY].val1;
		if(sd->sc.data[SC_RESISTDARK].timer != -1)
			sd->subele[ELE_DARK] += sd->sc.data[SC_RESISTDARK].val1;
		if(sd->sc.data[SC_RESISTTELEKINESIS].timer != -1)
			sd->subele[ELE_GHOST] += sd->sc.data[SC_RESISTTELEKINESIS].val1;
		if(sd->sc.data[SC_RESISTUNDEAD].timer != -1)
			sd->subele[ELE_UNDEAD] += sd->sc.data[SC_RESISTUNDEAD].val1;

		// �ϐ�
		if(sd->sc.data[SC_RESISTALL].timer != -1) {
			for(i=ELE_WATER; i<ELE_MAX; i++)
				sd->subele[i] += sd->sc.data[SC_RESISTALL].val1;	// �S�Ăɑϐ�����
		}
		// �s���g�̃W�[�N�t���[�h
		if(sd->sc.data[SC_SIEGFRIED].timer != -1) {
			for(i=ELE_WATER; i<ELE_MAX; i++)
				sd->subele[i] += sd->sc.data[SC_SIEGFRIED].val2;	// �S�Ăɑϐ�����
		}
		// �v�����B�f���X
		if(sd->sc.data[SC_PROVIDENCE].timer != -1) {
			sd->subele[ELE_HOLY]   += sd->sc.data[SC_PROVIDENCE].val2;	// �ΐ�����
			sd->subrace[RCT_DEMON] += sd->sc.data[SC_PROVIDENCE].val2;	// �Έ���
		}

		// ���̑�
		if(sd->sc.data[SC_BERSERK].timer != -1) {	// �o�[�T�[�N
			sd->def   = 0;
			sd->def2  = 0;
			sd->mdef  = 0;
			sd->mdef2 = 0;
		}
		if(sd->sc.data[SC_JOINTBEAT].timer != -1) {	// �W���C���g�r�[�g
			switch (sd->sc.data[SC_JOINTBEAT].val4) {
				case 0:		// ����
					break;
				case 1:		// ���
					break;
				case 2:		// �G
					break;
				case 3:		// ��
					sd->def2 -= (sd->def2 * 50)/100;
					break;
				case 4:		// ��
					sd->def2     -= (sd->def2 * 25)/100;
					sd->base_atk -= (sd->base_atk * 25)/100;
					break;
				case 5:		// ��
					sd->critical_def -= (sd->critical_def * 50)/100;
					break;
			}
		}
		if(sd->sc.data[SC_APPLEIDUN].timer != -1) {	// �C�h�D���̗ь�
			sd->status.max_hp += ((5+sd->sc.data[SC_APPLEIDUN].val1*2+sd->sc.data[SC_APPLEIDUN].val2
						+sd->sc.data[SC_APPLEIDUN].val3/10) * sd->status.max_hp)/100;

		} else if(sd->sc.data[SC_APPLEIDUN_].timer != -1) {	// �C�h�D���̗ь�
			sd->status.max_hp += ((5+sd->sc.data[SC_APPLEIDUN_].val1*2+sd->sc.data[SC_APPLEIDUN_].val2
						+sd->sc.data[SC_APPLEIDUN_].val3/10) * sd->status.max_hp)/100;
		}

		if(sd->sc.data[SC_DELUGE].timer != -1
#ifdef PRE_RENEWAL
			&& sd->def_ele == ELE_WATER
#endif
		) {	// �f�����[�W
			sd->status.max_hp += sd->status.max_hp*sd->sc.data[SC_DELUGE].val3/100;
		}
		if(sd->sc.data[SC_SERVICE4U].timer != -1) {	// �T�[�r�X�t�H�[���[
			sd->status.max_sp += sd->status.max_sp*(15+sd->sc.data[SC_SERVICE4U].val1+sd->sc.data[SC_SERVICE4U].val2
						+sd->sc.data[SC_SERVICE4U].val3)/100;
			sd->dsprate -= 20+sd->sc.data[SC_SERVICE4U].val1*3+sd->sc.data[SC_SERVICE4U].val2+sd->sc.data[SC_SERVICE4U].val3;
			if(sd->dsprate < 0)
				sd->dsprate = 0;
		} else if(sd->sc.data[SC_SERVICE4U_].timer != -1) {	// �T�[�r�X�t�H�[���[
			sd->status.max_sp += sd->status.max_sp*(15+sd->sc.data[SC_SERVICE4U_].val1+sd->sc.data[SC_SERVICE4U_].val2
						+sd->sc.data[SC_SERVICE4U_].val3)/100;
			sd->dsprate -= 20+sd->sc.data[SC_SERVICE4U_].val1*3+sd->sc.data[SC_SERVICE4U_].val2+sd->sc.data[SC_SERVICE4U_].val3;
			if(sd->dsprate < 0)
				sd->dsprate = 0;
		}

		if(sd->sc.data[SC_FORTUNE].timer != -1) {	// �K�^�̃L�X
			sd->critical += (10+sd->sc.data[SC_FORTUNE].val1+sd->sc.data[SC_FORTUNE].val2+sd->sc.data[SC_FORTUNE].val3)*10;
		} else if(sd->sc.data[SC_FORTUNE_].timer != -1) {	// �K�^�̃L�X
			sd->critical += (10+sd->sc.data[SC_FORTUNE_].val1+sd->sc.data[SC_FORTUNE_].val2+sd->sc.data[SC_FORTUNE_].val3)*10;
		}

		if(sd->sc.data[SC_EXPLOSIONSPIRITS].timer != -1) {	// �����g��
			if(sd->s_class.job == PC_JOB_SNV || sd->s_class.job == PC_JOB_ESNV)
				sd->critical += sd->sc.data[SC_EXPLOSIONSPIRITS].val1*100;
			else
				sd->critical += sd->sc.data[SC_EXPLOSIONSPIRITS].val2;
		}

#ifdef PRE_RENEWAL
		if(sd->sc.data[SC_STEELBODY].timer != -1) {	// ����
			sd->def = 90;
			sd->mdef = 90;
		}
#endif
		if(sd->sc.data[SC_ENCPOISON].timer != -1)
			sd->addeff[4] += sd->sc.data[SC_ENCPOISON].val2;
		if(sd->sc.data[SC_TRUESIGHT].timer != -1)	// �g�D���[�T�C�g
			sd->critical += 10*(sd->sc.data[SC_TRUESIGHT].val1);
#ifndef PRE_RENEWAL
		if(sd->sc.data[SC_SPEARQUICKEN].timer != -1)   // �X�s�A�N�C�b�P��
			sd->critical += 30*(sd->sc.data[SC_SPEARQUICKEN].val1);
#endif

		/*
		if(sd->sc.data[SC_VOLCANO].timer != -1)	// �G���`�����g�|�C�Y��(������battle.c��)
			sd->addeff[2] += sd->sc.data[SC_VOLCANO].val2;	// % of granting
		if(sd->sc.data[SC_DELUGE].timer != -1)	// �G���`�����g�|�C�Y��(������battle.c��)
			sd->addeff[0] += sd->sc.data[SC_DELUGE].val2;	// % of granting
		*/

		// �t�@�C�e�B���O�X�s���b�g
		if(sd->sc.data[SC_EISIR].timer != -1) {
#ifdef PRE_RENEWAL
			sd->base_atk += sd->sc.data[SC_EISIR].val2;
#else
			sd->plus_atk += sd->sc.data[SC_EISIR].val2;
#endif
		}
		// ���|
		if(sd->sc.data[SC_FEAR].timer != -1) {
			sd->hit  = sd->hit * 80/100;
			sd->flee = sd->flee * 80/100;
		}
		// �C���r�W�r���e�B
		if(sd->sc.data[SC__INVISIBILITY].timer != -1) {
			sd->critical += sd->critical * (sd->sc.data[SC__INVISIBILITY].val1 * 20) / 100;
		}
		// �}�X�J���[�h �F �G�i�[�x�[�V����
		if(sd->sc.data[SC__ENERVATION].timer != -1) {
			sd->base_atk -= sd->base_atk * (20 + sd->sc.data[SC__ENERVATION].val1 * 10) / 100;
			sd->watk -= sd->watk * (20 + sd->sc.data[SC__ENERVATION].val1 * 10) / 100;
		}
		// �}�X�J���[�h �F �O���[�~�[
		if(sd->sc.data[SC__GROOMY].timer != -1) {
			sd->hit -= sd->hit * (sd->sc.data[SC__GROOMY].val1 * 20) / 100;
		}
		// �}�X�J���[�h �F ���C�W�[�l�X
		if(sd->sc.data[SC__LAZINESS].timer != -1) {
			sd->flee -= sd->flee*10/100;
		}
		// �}�X�J���[�h �F �A�����b�L�[
		if(sd->sc.data[SC__UNLUCKY].timer != -1) {
			sd->critical -= sd->critical * (sd->sc.data[SC__UNLUCKY].val1 * 10) / 100;
			sd->flee2 -= sd->sc.data[SC__UNLUCKY].val1 * 10;
		}
		// �V�[���h�X�y��(DEF)
		if(sd->sc.data[SC_SHIELDSPELL_DEF].timer != -1 && sd->sc.data[SC_SHIELDSPELL_DEF].val2 == 2) {
#ifdef PRE_RENEWAL
			sd->base_atk += sd->sc.data[SC_SHIELDSPELL_DEF].val3;
#else
			sd->plus_atk += sd->sc.data[SC_SHIELDSPELL_DEF].val3;
#endif
		}
		// �V�[���h�X�y��(���B)
		if(sd->sc.data[SC_SHIELDSPELL_REF].timer != -1 && sd->sc.data[SC_SHIELDSPELL_REF].val2 == 2) {
#ifdef PRE_RENEWAL
			sd->def2 += sd->sc.data[SC_SHIELDSPELL_REF].val3;
#else
			sd->def += sd->sc.data[SC_SHIELDSPELL_REF].val3;
#endif
		}
		// �t�H�[�X�I�u�o���K�[�h
		if(sd->sc.data[SC_FORCEOFVANGUARD].timer != -1) {
			sd->status.max_hp += (sd->status.max_hp * sd->sc.data[SC_FORCEOFVANGUARD].val1 * 3) / 100;
		}
		// �v���X�e�B�[�W
		if(sd->sc.data[SC_PRESTIGE].timer != -1) {
#ifdef PRE_RENEWAL
			sd->def2 += sd->sc.data[SC_PRESTIGE].val2;
#else
			sd->def += sd->sc.data[SC_PRESTIGE].val2;
#endif
		}
		// �o���f�B���O
		if(sd->sc.data[SC_BANDING].timer != -1 && sd->sc.data[SC_BANDING].val2 > 1) {
#ifdef PRE_RENEWAL
			sd->base_atk += (10 + sd->sc.data[SC_BANDING].val1 * 10) * sd->sc.data[SC_BANDING].val2;
			sd->def2 += (5 + sd->sc.data[SC_BANDING].val1) * sd->sc.data[SC_BANDING].val2;
#else
			sd->plus_atk += (10 + sd->sc.data[SC_BANDING].val1 * 10) * sd->sc.data[SC_BANDING].val2;
			sd->def += (5 + sd->sc.data[SC_BANDING].val1) * sd->sc.data[SC_BANDING].val2;
#endif
		}
		// �A�[�X�h���C�u
		if(sd->sc.data[SC_EARTHDRIVE].timer != -1) {
			sd->def -= sd->def * 25 / 100;
		}
		// �C���X�s���[�V����
		if(sd->sc.data[SC_INSPIRATION].timer != -1) {
			sd->status.max_hp += (600 + sd->status.max_hp / 20) * sd->sc.data[SC_INSPIRATION].val1;
#ifdef PRE_RENEWAL
			sd->base_atk += sd->sc.data[SC_INSPIRATION].val1 * 40 + sd->status.job_level * 3;
#else
			sd->plus_atk += sd->sc.data[SC_INSPIRATION].val1 * 40 + sd->status.job_level * 3;
#endif
			sd->hit += 25 + sd->sc.data[SC_INSPIRATION].val1 * 5;
		}
		// �������V
		if(sd->sc.data[SC_RAISINGDRAGON].timer != -1) {
			sd->status.max_hp += sd->status.max_hp * sd->sc.data[SC_RAISINGDRAGON].val3 / 100;
			sd->status.max_sp += sd->status.max_sp * sd->sc.data[SC_RAISINGDRAGON].val3 / 100;
		}
		// �_�� -��-
		if(sd->sc.data[SC_GENTLETOUCH_CHANGE].timer != -1) {
			sd->status.max_hp -= sd->status.max_hp * (sd->sc.data[SC_GENTLETOUCH_CHANGE].val1 * 2) / 100;
		}
		// �_�� -��-
		if(sd->sc.data[SC_GENTLETOUCH_REVITALIZE].timer != -1) {
			sd->status.max_hp += sd->status.max_hp * (sd->sc.data[SC_GENTLETOUCH_REVITALIZE].val1 * 3) / 100;
		}
		// �������R���[
		if(sd->sc.data[SC_GLOOMYDAY].timer != -1) {
			sd->flee -= sd->sc.data[SC_GLOOMYDAY].val1 * 5;
		}
		// ���[���Y�̘I
		if(sd->sc.data[SC_LERADS_DEW].timer != -1) {
			sd->status.max_hp += sd->sc.data[SC_LERADS_DEW].val1 * 200 + sd->sc.data[SC_LERADS_DEW].val4 * 300;
		}
		// �t���C�f�[�i�C�g�t�B�[�o�[
		if(sd->sc.data[SC_SATURDAY_NIGHT_FEVER].timer != -1) {
			sd->watk += 100 * sd->sc.data[SC_SATURDAY_NIGHT_FEVER].val1;
			sd->def -= sd->def * (10 + 10 * sd->sc.data[SC_SATURDAY_NIGHT_FEVER].val1) / 100;
			sd->flee -= sd->flee * (40 + 10 * sd->sc.data[SC_SATURDAY_NIGHT_FEVER].val1) / 100;
		}
		// �X�g���C�L���O
		if(sd->sc.data[SC_STRIKING].timer != -1) {
			sd->critical += 10 * sd->sc.data[SC_STRIKING].val1;
		}
		// �I�[�f�B���̗�
		if(sd->sc.data[SC_ODINS_POWER].timer != -1) {
#ifdef PRE_RENEWAL
			sd->watk += 60 + 10 * sd->sc.data[SC_ODINS_POWER].val1;
			sd->matk1 += 60 + 10 * sd->sc.data[SC_ODINS_POWER].val1;
			sd->matk2 += 60 + 10 * sd->sc.data[SC_ODINS_POWER].val1;
#else
			sd->plus_atk += 60 + 10 * sd->sc.data[SC_ODINS_POWER].val1;
			sd->plus_matk += 60 + 10 * sd->sc.data[SC_ODINS_POWER].val1;
#endif
			sd->def -= 10 + 10 * sd->sc.data[SC_ODINS_POWER].val1;
			sd->mdef -= 10 + 10 * sd->sc.data[SC_ODINS_POWER].val1;
		}
		// �t�@�C�A�[�G�N�X�p���V����(����)
		if(sd->sc.data[SC_FIRE_EXPANSION_SMOKE_POWDER].timer != -1) {
			sd->flee += sd->flee * sd->sc.data[SC_FIRE_EXPANSION_SMOKE_POWDER].val3 / 100;
		}
		// �t�@�C�A�[�G�N�X�p���V����(�×�)
		if(sd->sc.data[SC_FIRE_EXPANSION_TEAR_GAS].timer != -1) {
			sd->hit -= sd->hit * sd->sc.data[SC_FIRE_EXPANSION_TEAR_GAS].val2 / 100;
			sd->flee -= sd->flee * sd->sc.data[SC_FIRE_EXPANSION_TEAR_GAS].val2 / 100;
		}
		// HP�����|�[�V����
		if(sd->sc.data[SC_PROMOTE_HEALTH_RESERCH].timer != -1) {
			sd->status.max_hp += 500 + sd->sc.data[SC_PROMOTE_HEALTH_RESERCH].val1 * 1000 + 4 / 3 * sd->status.base_level;
		}
		// SP�����|�[�V����
		if(sd->sc.data[SC_ENERGY_DRINK_RESERCH].timer != -1) {
			sd->status.max_sp += sd->status.max_sp * ((sd->sc.data[SC_ENERGY_DRINK_RESERCH].val1 * 5) - 5 + sd->status.base_level / 10) / 100;
		}
		// �r�^�^500
		if(sd->sc.data[SC_VITATA_500].timer != -1) {
			sd->status.max_sp += sd->status.max_sp * sd->sc.data[SC_VITATA_500].val2 / 100;
		}
		// �t�@�C�A�[�N���[�N
		if(sd->sc.data[SC_FIRE_CLOAK].timer != -1) {
			sd->subele[ELE_FIRE] += sd->sc.data[SC_FIRE_CLOAK].val2;
			sd->subele[ELE_WATER] -= sd->sc.data[SC_FIRE_CLOAK].val3;
		}
		// �E�H�[�^�[�h���b�v
		if(sd->sc.data[SC_WATER_DROP].timer != -1) {
			sd->subele[ELE_WATER] += sd->sc.data[SC_WATER_DROP].val2;
			sd->subele[ELE_WIND] -= sd->sc.data[SC_WATER_DROP].val3;
		}
		// �E�B���h�X�e�b�v
		if(sd->sc.data[SC_WIND_STEP].timer != -1) {
			sd->flee += sd->sc.data[SC_WIND_STEP].val2;
		}
		// �E�B���h�J�[�e��
		if(sd->sc.data[SC_WIND_CURTAIN].timer != -1) {
			sd->subele[ELE_WIND] += sd->sc.data[SC_WIND_CURTAIN].val2;
			sd->subele[ELE_EARTH] -= sd->sc.data[SC_WIND_CURTAIN].val3;
		}
		// �\���b�h�X�L��
		if(sd->sc.data[SC_SOLID_SKIN].timer != -1) {
			sd->def += sd->def * sd->sc.data[SC_SOLID_SKIN].val2 / 100;
			sd->status.max_hp += sd->status.max_hp * sd->sc.data[SC_SOLID_SKIN].val3 / 100;
		}
		// �X�g�[���V�[���h
		if(sd->sc.data[SC_STONE_SHIELD].timer != -1) {
			sd->subele[ELE_EARTH] += sd->sc.data[SC_STONE_SHIELD].val2;
			sd->subele[ELE_FIRE] -= sd->sc.data[SC_STONE_SHIELD].val3;
		}
		// �p�C���e�N�j�b�N
		if(sd->sc.data[SC_PYROTECHNIC].timer != -1) {
			sd->watk += sd->sc.data[SC_PYROTECHNIC].val2;
		}
		// �q�[�^�[
		if(sd->sc.data[SC_HEATER].timer != -1) {
			sd->watk += sd->sc.data[SC_HEATER].val2;
		}
		// �g���s�b�N
		if(sd->sc.data[SC_TROPIC].timer != -1) {
			sd->watk += sd->sc.data[SC_TROPIC].val2;
		}
		// �A�N�A�v���C
		if(sd->sc.data[SC_AQUAPLAY].timer != -1) {
			sd->matk1 += sd->sc.data[SC_AQUAPLAY].val2;
			sd->matk2 += sd->sc.data[SC_AQUAPLAY].val2;
		}
		// �N�[���[
		if(sd->sc.data[SC_COOLER].timer != -1) {
			sd->matk1 += sd->sc.data[SC_COOLER].val2;
			sd->matk2 += sd->sc.data[SC_COOLER].val2;
		}
		// �N�[���G�A�[
		if(sd->sc.data[SC_CHILLY_AIR].timer != -1) {
			sd->matk1 += sd->sc.data[SC_CHILLY_AIR].val2;
			sd->matk2 += sd->sc.data[SC_CHILLY_AIR].val2;
		}
		// �y�g���W�[
		if(sd->sc.data[SC_PETROLOGY].timer != -1) {
			sd->status.max_hp += sd->status.max_hp * sd->sc.data[SC_PETROLOGY].val2 / 100;
		}
		// �J�[�X�h�\�C��
		if(sd->sc.data[SC_CURSED_SOIL].timer != -1) {
			sd->status.max_hp += sd->status.max_hp * sd->sc.data[SC_CURSED_SOIL].val2 / 100;
		}
		// �A�b�v�w�C�o��
		if(sd->sc.data[SC_UPHEAVAL].timer != -1) {
			sd->status.max_hp += sd->status.max_hp * sd->sc.data[SC_UPHEAVAL].val2 / 100;
		}
		// �\�Z��
		if(sd->sc.data[SC_IZAYOI].timer != -1) {
#ifdef PRE_RENEWAL
			sd->matk1 += sd->sc.data[SC_IZAYOI].val2;
			sd->matk2 += sd->sc.data[SC_IZAYOI].val2;
#else
			sd->plus_matk += sd->sc.data[SC_IZAYOI].val2;
#endif
		}
		// ���p -�c��-
		if(sd->sc.data[SC_ZANGETSU].timer != -1) {
#ifdef PRE_RENEWAL
			sd->base_atk += sd->sc.data[SC_ZANGETSU].val2;
			sd->matk1 += sd->sc.data[SC_ZANGETSU].val3;
			sd->matk2 += sd->sc.data[SC_ZANGETSU].val3;
#else
			sd->plus_atk += sd->sc.data[SC_ZANGETSU].val2;
			sd->plus_matk += sd->sc.data[SC_ZANGETSU].val3;
#endif
		}
	}

	// �e�R�������J�[�{�[�i�X
	if(sd->status.class_ == PC_CLASS_TK && sd->status.base_level >= 90 && ranking_get_pc_rank(sd,RK_TAEKWON) > 0)
	{
		sd->status.max_hp *= 3;
		sd->status.max_sp *= 3;
	}

#ifdef PRE_RENEWAL
	// �y���F����
	if(sd->elementball.num && sd->elementball.ele == ELE_EARTH) {
		sd->watk += sd->watk * sd->elementball.num * 10 / 100;
	}
#endif

	// MATK��Z����(��␳)
	if(sd->matk2_rate != 100) {
		sd->matk1 = sd->matk1 * sd->matk2_rate / 100;
		sd->matk2 = sd->matk2 * sd->matk2_rate / 100;
	}

	// �X�e�[�^�X�Œ�
	if(sd->fix_status.max_hp > 0) {
		sd->status.max_hp = sd->fix_status.max_hp;
	}
	if(sd->fix_status.max_sp > 0) {
		sd->status.max_sp = sd->fix_status.max_sp;
	}
	if(sd->fix_status.atk > 0) {
		sd->base_atk = sd->fix_status.atk;
		sd->watk = sd->watk2 = 0;
	}
	if(sd->fix_status.matk > 0) {
		sd->matk1 = sd->matk2 = sd->fix_status.matk;
		// MATK��Z����(�Œ�l*(��␳+��␳�ȊO))
#ifdef PRE_RENEWAL
		if(sd->matk_rate != 100 || sd->matk2_rate != 100) {
			sd->matk1 = sd->matk1 * (sd->matk_rate + sd->matk2_rate - 100) / 100;
			sd->matk2 = sd->matk2 * (sd->matk_rate + sd->matk2_rate - 100) / 100;
		}
#else
		if(sd->matk2_rate != 100) {
			sd->matk1 = sd->matk1 * sd->matk2_rate / 100;
			sd->matk2 = sd->matk2 * sd->matk2_rate / 100;
		}
#endif
	}
	if(sd->fix_status.def > 0 && sd->fix_status.def <= 100) {
		sd->def = sd->fix_status.def;
	}
	if(sd->fix_status.mdef > 0 && sd->fix_status.mdef <= 100) {
		sd->mdef = sd->fix_status.mdef;
	}
	if(sd->fix_status.hit > 0) {
		sd->hit = sd->fix_status.hit;
	}
	if(sd->fix_status.critical > 0) {
		sd->critical = 10 * sd->fix_status.critical;
	}
	if(sd->fix_status.flee > 0) {
		sd->flee = sd->fix_status.flee;
	}

	if(pc_is3rdclass(sd) && sd->aspd < battle_config.third_max_aspd) {
		sd->aspd = battle_config.third_max_aspd;
		sd->amotion = sd->aspd>>1;
	}
	else if(sd->s_class.job == PC_JOB_ESNV && sd->aspd < battle_config.esnv_max_aspd) {
		sd->aspd = battle_config.esnv_max_aspd;
		sd->amotion = sd->aspd>>1;
	}
	else if((sd->s_class.job == PC_JOB_KG || sd->s_class.job == PC_JOB_OB) && sd->aspd < battle_config.ko_max_aspd) {
		sd->aspd = battle_config.ko_max_aspd;
		sd->amotion = sd->aspd>>1;
	}
	else if(!pc_is3rdclass(sd) && sd->aspd < battle_config.max_aspd) {
		sd->aspd = battle_config.max_aspd;
		sd->amotion = sd->aspd>>1;
	}

	if(battle_config.min_lv100_aspdfix && sd->status.base_level < 100 && sd->aspd < battle_config.max_aspd) {
		sd->aspd = battle_config.max_aspd;
		sd->amotion = sd->aspd>>1;
	}

	if(map[sd->bl.m].flag.pk) {
		if(sd->aspd < battle_config.pk_max_aspd) {
			sd->aspd = battle_config.pk_max_aspd;
			sd->amotion = sd->aspd>>1;
		}
	}
	if(map[sd->bl.m].flag.gvg) {
		if(sd->aspd < battle_config.gvg_max_aspd) {
			sd->aspd = battle_config.gvg_max_aspd;
			sd->amotion = sd->aspd>>1;
		}
	}
	if(map[sd->bl.m].flag.pvp) {
		if(sd->aspd < battle_config.pvp_max_aspd) {
			sd->aspd = battle_config.pvp_max_aspd;
			sd->amotion = sd->aspd>>1;
		}
	}
	sd->dmotion = 800-sd->paramc[1]*4;
	if(sd->dmotion < 400)
		sd->dmotion = 400;

	// MATK�Œ�l�ۏ�
#ifdef PRE_RENEWAL
	if(sd->matk1 < 1)
	 	sd->matk1 = 1;
#else
	if(sd->matk1 < 1)
	 	sd->matk1 = 0;
#endif
	if(sd->matk2 < 1)
		sd->matk2 = 1;

	if(sd->status.max_hp > battle_config.max_hp)
		sd->status.max_hp = battle_config.max_hp;
	if(sd->status.max_sp > battle_config.max_sp)
		sd->status.max_sp = battle_config.max_sp;

	if(sd->status.max_hp <= 0)
		sd->status.max_hp = 1;
	if(sd->status.max_sp <= 0)
		sd->status.max_sp = 1;

	if(sd->status.hp > sd->status.max_hp)
		sd->status.hp = sd->status.max_hp;
	if(sd->status.sp > sd->status.max_sp)
		sd->status.sp = sd->status.max_sp;

	// bTigereye���Ȃ��Ȃ��Ă�����p�P�b�g�����Č��ɖ߂�
	if(b_tigereye == 1 && sd->special_state.infinite_tigereye == 0 && sd->sc.data[SC_TIGEREYE].timer == -1)
		clif_status_load_id(sd,SI_TIGEREYE,0);

	// bInfiniteEndure���Ȃ��Ȃ��Ă�����p�P�b�g�𑗂��Č��ɖ߂�
	if(b_endure == 1 && sd->special_state.infinite_endure == 0)
		clif_status_load_id(sd,SI_ENDURE,0);
	// bSpeedRate���Ȃ��Ȃ��Ă�����p�P�b�g�𑗂��Č��ɖ߂�
	if(b_speedrate != 0 && sd->speed_rate == 0)
		clif_status_load_id(sd,SI_MOVHASTE_INFINITY,0);
	else if(b_speedrate < sd->speed_rate)
		clif_status_load_id(sd,SI_MOVHASTE_INFINITY,1);

	// �v�Z���������܂�
	if( sd->status_calc_pc_process > 1 ) {
		// ���̊֐����ċA�I�ɌĂ΂ꂽ�̂ŁA�Čv�Z����
		if( --calclimit ) {
			sd->status_calc_pc_process = 1;
			goto L_RECALC;
		} else {
			// �������[�v�ɂȂ����̂Ōv�Z�ł��؂�
			printf("status_calc_pc: infinity loop!\n");
		}
	}
	sd->status_calc_pc_process = 0;

	if(first&4)
		return 0;
	if(first&3) {
		clif_updatestatus(sd,SP_SPEED);
		clif_updatestatus(sd,SP_MAXHP);
		clif_updatestatus(sd,SP_MAXSP);
		if(first&1) {
			clif_updatestatus(sd,SP_HP);
			clif_updatestatus(sd,SP_SP);
		}
		return 0;
	}

	if(b_class != sd->view_class) {
		clif_changelook(&sd->bl,LOOK_BASE,sd->view_class);
#if PACKETVER < 4
		clif_changelook(&sd->bl,LOOK_WEAPON,sd->status.weapon);
		clif_changelook(&sd->bl,LOOK_SHIELD,sd->status.shield);
#else
		clif_changelook(&sd->bl,LOOK_WEAPON,0);
#endif
	}

	if( memcmp(b_skill,sd->status.skill,sizeof(sd->status.skill)) || b_attackrange != sd->range.attackrange ) {
		int type;
		for(i=0; i<MAX_PCSKILL; i++) {
			// �J�[�h�X�L�������X�g�����Ƃ����������^�Ȃ��Ԉُ������
			if(b_skill[i].flag == 1 && b_skill[i].lv > 0 && sd->status.skill[i].lv <= 0 && skill_get_inf(i) & INF_SELF) {
				type = GetSkillStatusChangeTable(i);
				if(type >= 0 && sd->sc.data[type].timer != -1)
					status_change_end(&sd->bl, type, -1);
			}
		}
		clif_skillinfoblock(sd);	// �X�L�����M
	}

	if(b_speed != sd->speed) {
		sd->ud.state.change_speed = 1;
		clif_updatestatus(sd,SP_SPEED);
	}
	if(b_weight != sd->weight)
		clif_updatestatus(sd,SP_WEIGHT);
	if(b_max_weight != sd->max_weight) {
		clif_updatestatus(sd,SP_MAXWEIGHT);
		pc_checkweighticon(sd);
	}

	for(i=0; i<6; i++) {
		if(b_paramb[i] + b_paramc[i] != sd->paramb[i] + sd->paramc[i])
			clif_updatestatus(sd,SP_STR+i);
	}

	if(b_hit != sd->hit)
		clif_updatestatus(sd,SP_HIT);
	if(b_flee != sd->flee)
		clif_updatestatus(sd,SP_FLEE1);
	if(b_aspd != sd->aspd)
		clif_updatestatus(sd,SP_ASPD);
#ifdef PRE_RENEWAL
	if(b_watk != sd->watk || b_watk_ != sd->watk_ || b_base_atk != sd->base_atk)
		clif_updatestatus(sd,SP_ATK1);
	if(b_def != sd->def)
		clif_updatestatus(sd,SP_DEF1);
	if(b_watk2 != sd->watk2 || b_watk_2 != sd->watk_2)
		clif_updatestatus(sd,SP_ATK2);
	if(b_def2 != sd->def2)
		clif_updatestatus(sd,SP_DEF2);
#else
	if(b_base_atk != sd->base_atk)
		clif_updatestatus(sd,SP_ATK1);
	if(b_def2 != sd->def2)
		clif_updatestatus(sd,SP_DEF1);
	if(b_watk != sd->watk || b_watk_ != sd->watk_ || b_watk2 != sd->watk2 || b_watk_2 != sd->watk_2 || b_plus_atk != sd->plus_atk)
		clif_updatestatus(sd,SP_ATK2);
	if(b_def != sd->def)
		clif_updatestatus(sd,SP_DEF2);
#endif
	if(b_flee2 != sd->flee2)
		clif_updatestatus(sd,SP_FLEE2);
	if(b_critical != sd->critical)
		clif_updatestatus(sd,SP_CRITICAL);
#ifdef PRE_RENEWAL
	if(b_matk1 != sd->matk1)
		clif_updatestatus(sd,SP_MATK1);
#else
	if(b_matk1 != sd->matk1 || b_plus_matk != sd->plus_matk)
		clif_updatestatus(sd,SP_MATK1);
#endif
	if(b_matk2 != sd->matk2)
		clif_updatestatus(sd,SP_MATK2);
#ifdef PRE_RENEWAL
	if(b_mdef != sd->mdef)
		clif_updatestatus(sd,SP_MDEF1);
	if(b_mdef2 != sd->mdef2)
		clif_updatestatus(sd,SP_MDEF2);
#else
	if(b_mdef2 != sd->mdef2)
		clif_updatestatus(sd,SP_MDEF1);
	if(b_mdef != sd->mdef)
		clif_updatestatus(sd,SP_MDEF2);
#endif
	if(b_attackrange != sd->range.attackrange)
		clif_updatestatus(sd,SP_ATTACKRANGE);
	if(b_max_hp != sd->status.max_hp)
		clif_updatestatus(sd,SP_MAXHP);
	if(b_max_sp != sd->status.max_sp)
		clif_updatestatus(sd,SP_MAXSP);
	if(b_hp != sd->status.hp)
		clif_updatestatus(sd,SP_HP);
	if(b_sp != sd->status.sp)
		clif_updatestatus(sd,SP_SP);
	/*
	if(before.cart_num != before.cart_num || before.cart_max_num != before.cart_max_num ||
		before.cart_weight != before.cart_weight || before.cart_max_weight != before.cart_max_weight )
		clif_updatestatus(sd,SP_CARTINFO);
	*/

	if( sd->sc.data[SC_AUTOBERSERK].timer != -1 &&
	    sd->status.hp < sd->status.max_hp>>2 &&
	    (sd->sc.data[SC_PROVOKE].timer == -1 || sd->sc.data[SC_PROVOKE].val2 == 0) &&
	    !unit_isdead(&sd->bl) )
	{
		// �I�[�g�o�[�T�[�N����
		status_change_start(&sd->bl,SC_PROVOKE,10,1,0,0,0,0);
	}

	return 0;
}

/*==========================================
 * PC��amotion���v�Z���ĕԂ�
 * �߂�͐�����1�ȏ�
 *------------------------------------------
 */
static int status_calc_amotion_pc(struct map_session_data *sd)
{
	double base_amotion;
	double amotion   = 0;
	int ferver_bonus = 0;
	int comfort_bonus= 0;
	int haste_val1   = 0;
	int haste_val2   = 0;
	int slow_val     = 0;
	int bonus_rate   = 0;
	int bonus_add    = 0;
	int skilllv;
	int tmp;
	char berserk_flag  = 0;
#ifndef PRE_RENEWAL
	double penalty   = 100;
#endif

	nullpo_retr(0, sd);

	/* ASPD�Œ�{�[�i�X */
	if(sd->fix_status.aspd) {
		int fix_aspd = 2000 - sd->fix_status.aspd*10;
		return (fix_aspd < 100)? 100:fix_aspd;
	}

#ifdef PRE_RENEWAL
	/* ��{ASPD�̌v�Z */
	if(sd->status.weapon < WT_MAX)	// �Ў�̏ꍇ�͒l�����̂܂܎擾
		base_amotion = job_db[sd->s_class.job].aspd_base[sd->status.weapon];
	else	// 2���̏ꍇ��2���p�̌v�Z���s��
		base_amotion = (job_db[sd->s_class.job].aspd_base[sd->weapontype1] + job_db[sd->s_class.job].aspd_base[sd->weapontype2]) * 140 / 200;

	/* ��{ASPD�Ɋe�p�����[�^�̃{�[�i�X��K�p */
	if(pc_isriding(sd))	// �R���C��
		base_amotion = base_amotion - 1000 * (50+10*pc_checkskill(sd,KN_CAVALIERMASTERY)) / 100 - (base_amotion * sd->paramc[4] / 1000) - (base_amotion * sd->paramc[1] / 250) + 1000;
	else if(pc_isdragon(sd))	// �h���S���g���[�j���O
		base_amotion = base_amotion - 1000 * (50+10*pc_checkskill(sd,RK_DRAGONTRAINING)) / 100 - (base_amotion * sd->paramc[4] / 1000) - (base_amotion * sd->paramc[1] / 250) + 1000;
	else	// �R�悵�Ă��Ȃ�
		base_amotion = base_amotion - (base_amotion * sd->paramc[4] / 1000) - (base_amotion * sd->paramc[1] / 250);
#else
	/* ��{ASPD�̌v�Z */
	if(sd->status.weapon < WT_MAX) {	// �Ў�̏ꍇ�͒l�����̂܂܎擾
		base_amotion = (2000 - job_db[sd->s_class.job].aspd_base[sd->status.weapon]) / 10;
		if(base_amotion > 144)
			penalty = (100-(base_amotion-144)*2);
		else
			penalty = 100;
	} else {	// 2���̏ꍇ��2���p�̌v�Z���s��
		base_amotion = (2000 - job_db[sd->s_class.job].aspd_base[sd->weapontype1]) / 10;
		base_amotion = base_amotion + (((2000 - job_db[sd->s_class.job].aspd_base[sd->weapontype2]) / 10) - 194) / 4;
	}

	/* ��{ASPD�Ɋe�p�����[�^�̃{�[�i�X��K�p */
	if(sd->status.weapon < WT_MAX){
		if( sd->status.weapon == WT_BOW ||
		    sd->status.weapon == WT_MUSICAL ||
		    sd->status.weapon == WT_WHIP ||
		    (sd->status.weapon >= WT_HANDGUN && sd->status.weapon <= WT_GRENADE) )
			base_amotion = (int)(200-(200-(base_amotion + sqrt(sd->paramc[1]*(10-1/(float)400) + sd->paramc[4]*9/(float)49) * penalty/100)));
		else
			base_amotion = (int)(200-(200-(base_amotion + sqrt(sd->paramc[1]*(10+10/(float)111) + sd->paramc[4]*9/(float)49) * penalty/100)));
	} else {
		base_amotion = (int)(200-(200-(base_amotion + sqrt(sd->paramc[1]*(10-1/(float)400) + sd->paramc[4]*9/(float)49)*1.05)));
	}
	base_amotion = (2000-base_amotion*10);
#endif

	/* ���y�i���e�B�̉��Z */
	if(sd->equip_index[EQUIP_INDEX_LARM] >= 0 && sd->inventory_data[sd->equip_index[EQUIP_INDEX_LARM]] && itemdb_isarmor(sd->inventory_data[sd->equip_index[EQUIP_INDEX_LARM]]->nameid) && job_db[sd->s_class.job].aspd_base[WT_MAX] != 0)
		base_amotion += job_db[sd->s_class.job].aspd_base[WT_MAX];

#ifdef PRE_RENEWAL
	/* �{�[�i�XADD_ASPD�̌v�Z */
	amotion = base_amotion + sd->aspd_add;
#else
	amotion = base_amotion;
#endif

	/* amotion���ω������Ԉُ�̌v�Z */
	if(sd->sc.count > 0) {
		/* amotion����������X�e�[�^�X�̌v�Z */

		// ����Y��Ȃ���
		if(sd->sc.data[SC_DONTFORGETME].timer != -1) {
			slow_val = sd->sc.data[SC_DONTFORGETME].val1;
		}

		// ����
		if(sd->sc.data[SC_STEELBODY].timer != -1) {
			if(slow_val < 25)
				slow_val = 25;
		}

		// �W���C���g�r�[�g
		if(sd->sc.data[SC_JOINTBEAT].timer != -1) {
			switch (sd->sc.data[SC_JOINTBEAT].val4) {
				case 1:		// ���
					if(slow_val < 25)
						slow_val = 25;
					break;
				case 2:		// �G
					if(slow_val < 10)
						slow_val = 10;
					break;
			}
		}

		// �O���r�e�[�V�����t�B�[���h
		if(sd->sc.data[SC_GRAVITATION].timer != -1) {
			int penalty = sd->sc.data[SC_GRAVITATION].val1*5;
			if(slow_val < penalty)
				slow_val = penalty;
		}

		// ���𔛂�Ȃ���
		if(sd->sc.data[SC_DANCING].timer != -1 && sd->sc.data[SC_BARDDANCER].timer == -1) {
			if(sd->sc.data[SC_LONGINGFREEDOM].timer != -1) {
				if(sd->sc.data[SC_LONGINGFREEDOM].val1 < 5) {
					int penalty = 50 - 10 * sd->sc.data[SC_LONGINGFREEDOM].val1;
					if(slow_val < penalty)
						slow_val = penalty;
				}
			}
		}

		// �n���V�l�[�V�����E�H�[�N(�y�i���e�B)
		if(sd->sc.data[SC_HALLUCINATIONWALK2].timer != -1) {
			if(slow_val < 50)
				slow_val = 50;
		}

		// �p�����C�Y
		if(sd->sc.data[SC_PARALIZE].timer != -1) {
			if(slow_val < 10)
				slow_val = 10;
		}

		// �t���X�g�~�X�e�B
		if(sd->sc.data[SC_FROSTMISTY].timer != -1) {
			if(slow_val < 15)
				slow_val = 15;
		}

		// �{�f�B�y�C���e�B���O
		if(sd->sc.data[SC__BODYPAINT].timer != -1) {
			if(slow_val < 25)
				slow_val = 25;
		}

		// �C���r�W�r���e�B
		if(sd->sc.data[SC__INVISIBILITY].timer != -1) {
			int penalty = 50 - 10 * sd->sc.data[SC__INVISIBILITY].val1;
			if(slow_val < penalty)
				slow_val = penalty;
		}

		// �}�X�J���[�h �F �O���[�~�[
		if(sd->sc.data[SC__GROOMY].timer != -1) {
			int penalty = 20 + 10 * sd->sc.data[SC__GROOMY].val1;
			if(slow_val < penalty)
				slow_val = penalty;
		}

		// �A�[�X�h���C�u
		if(sd->sc.data[SC_EARTHDRIVE].timer != -1) {
			if(slow_val < 25)
				slow_val = 25;
		}

		// �������R���[
		if(sd->sc.data[SC_GLOOMYDAY].timer != -1) {
			int penalty = 3 * sd->sc.data[SC_GLOOMYDAY].val1;
			if(slow_val < penalty)
				slow_val = penalty;
		}

		// ���������e
		if(sd->sc.data[SC_MELON_BOMB].timer != -1) {
			int penalty = sd->sc.data[SC_MELON_BOMB].val1;
			if(slow_val < penalty)
				slow_val = penalty;
		}

		/* amotion����������X�e�[�^�X�̌v�Z1 */

		// �����|�[�V����
		if(sd->sc.data[tmp = SC_SPEEDPOTION2].timer != -1 || sd->sc.data[tmp = SC_SPEEDPOTION1].timer != -1 || sd->sc.data[tmp = SC_SPEEDPOTION0].timer != -1)
			haste_val1 = sd->sc.data[tmp].val2;

		// �Z�k�T���}�C���W���[�X
		if(sd->sc.data[SC_EXTRACT_SALAMINE_JUICE].timer != -1)
			haste_val1 += sd->sc.data[SC_EXTRACT_SALAMINE_JUICE].val1;

		/* amotion����������X�e�[�^�X�̌v�Z2 */

		// �c�[�n���h�N�B�b�P��
		if(sd->sc.data[SC_TWOHANDQUICKEN].timer != -1)
			haste_val2 = sd->sc.data[SC_TWOHANDQUICKEN].val2;

		// �X�s�A�N�B�b�P��
		if(sd->sc.data[SC_SPEARQUICKEN].timer != -1) {
			if(haste_val2 < sd->sc.data[SC_SPEARQUICKEN].val2)
				haste_val2 = sd->sc.data[SC_SPEARQUICKEN].val2;
		}

		// �����n���h�N�B�b�P��
		if(sd->sc.data[SC_ONEHAND].timer != -1) {
			if(haste_val2 < 30)
				haste_val2 = 30;
		}

		// �A�h���i�������b�V��
		if(sd->sc.data[SC_ADRENALINE].timer != -1) {
			int bonus;
			if(sd->sc.data[SC_ADRENALINE].val2 || !battle_config.party_skill_penalty)
				bonus = 30;
			else
				bonus = 25;
			if(haste_val2 < bonus)
				haste_val2 = bonus;
		}

		// �A�h���i�������b�V��2
		if(sd->sc.data[SC_ADRENALINE2].timer != -1) {
			int bonus;
			if(sd->sc.data[SC_ADRENALINE2].val2 || !battle_config.party_skill_penalty)
				bonus = 30;
			else
				bonus = 25;
			if(haste_val2 < bonus)
				haste_val2 = bonus;
		}

		// �[�z�̃A�T�V���N���X
		if(sd->sc.data[SC_ASSNCROS].timer != -1) {
			int bonus = sd->sc.data[SC_ASSNCROS].val2;
			if(haste_val2 < bonus)
				haste_val2 = bonus;
		}
		else if(sd->sc.data[SC_ASSNCROS_].timer != -1) {
			int bonus = sd->sc.data[SC_ASSNCROS_].val2;
			if(haste_val2 < bonus)
				haste_val2 = bonus;
		}

		// ���̈��y
		if(sd->sc.data[SC_STAR_COMFORT].timer != -1) {
			comfort_bonus = (sd->status.base_level + sd->status.dex + sd->status.luk)/10;
			if(haste_val2 < comfort_bonus)
				haste_val2 = comfort_bonus;
		}

		// �K�g�����O�t�B�[�o�[
		if(sd->sc.data[SC_GATLINGFEVER].timer != -1) {
			ferver_bonus = sd->sc.data[SC_GATLINGFEVER].val1*2;
			if(haste_val2 < ferver_bonus)
				haste_val2 = ferver_bonus;
		}

		// �}�b�h�l�X�L�����Z���[
		if(sd->sc.data[SC_MADNESSCANCEL].timer != -1) {
			int bonus = 20+ferver_bonus;
			if(haste_val2 < bonus)
				haste_val2 = bonus;
		}

		// �_�� -��-
		if(sd->sc.data[SC_GENTLETOUCH_CHANGE].timer != -1) {
			int bonus = sd->sc.data[SC_GENTLETOUCH_CHANGE].val4;
			if(haste_val2 < bonus)
				haste_val2 = bonus;
		}

		// �X�C���O�_���X
		if(sd->sc.data[SC_SWING].timer != -1) {
			int bonus = sd->sc.data[SC_SWING].val4;
			if(haste_val2 < bonus)
				haste_val2 = bonus;
		}

		// �_���X�E�B�Y�E�H�[�O
		if(sd->sc.data[SC_DANCE_WITH_WUG].timer != -1) {
			int bonus = 2 + sd->sc.data[SC_DANCE_WITH_WUG].val4;
			if(haste_val2 < bonus)
				haste_val2 = bonus;
		}

		/* ���̑� */

		// �o�[�T�[�N
#ifdef PRE_RENEWAL
		if(sd->sc.data[SC_BERSERK].timer != -1)
			berserk_flag = 1;
#else
		if(sd->sc.data[SC_BERSERK].timer != -1 && sd->sc.data[SC_TWOHANDQUICKEN].timer == -1)
			berserk_flag = 1;
#endif

		// �f�B�t�F���_�[
		if(sd->sc.data[SC_DEFENDER].timer != -1)
			bonus_add += sd->sc.data[SC_DEFENDER].val3;

		// �t�@�C�e�B���O�X�s���b�g
		if(sd->sc.data[SC_EISIR].timer != -1)
			bonus_add -= sd->sc.data[SC_EISIR].val3;

		// �K�X�g
		if(sd->sc.data[SC_GUST].timer != -1)
			bonus_add -= sd->sc.data[SC_GUST].val2;

		// �u���X�g
		if(sd->sc.data[SC_BLAST].timer != -1)
			bonus_add -= sd->sc.data[SC_BLAST].val2;

		// ���C���h�X�g�[��
		if(sd->sc.data[SC_WILD_STORM].timer != -1)
			bonus_add -= sd->sc.data[SC_WILD_STORM].val2;

		// �����X�^�[�ϐg�f�r���`
		if(sd->sc.data[SC_MONSTER_TRANSFORM].timer != -1 && sd->sc.data[SC_MONSTER_TRANSFORM].val1 == 1109)
			bonus_add -= -10;
	}

	/* ���z�ƌ��Ɛ��̈��� */
	if((skilllv = pc_checkskill(sd,SG_DEVIL)) > 0 && sd->status.job_level >= 50) {
		int bonus = skilllv*3 + comfort_bonus;
		if(haste_val2 < bonus)
			haste_val2 = bonus;
	}

	/* slow_val��haste_val1��haste_val2�����Z���� */
	bonus_rate = slow_val - haste_val1 - haste_val2;

#ifdef PRE_RENEWAL
	/* bonus_rate�ɃA�C�e���̃{�[�i�X�����Z���� */
	if(sd->aspd_add_rate != 0 || sd->aspd_rate != 0) {
		sd->aspd_rate += sd->aspd_add_rate;
		bonus_rate -= sd->aspd_rate;
	}
#endif

	/* �o�[�T�[�N */
	if(berserk_flag)
		bonus_rate -= 30;

	/* �t���[�L���X�g */
	if(sd->ud.skilltimer != -1 && pc_checkskill(sd,SA_FREECAST) > 0)
		amotion += 5 * amotion * (10 - skilllv) / 100;

	/* bonus_rate�̌v�Z */
	if(bonus_rate != 0)
		amotion = amotion * (bonus_rate+100) / 100;

	/* �A�h�o���X�h�u�b�N */
	if(sd->weapontype1 == WT_BOOK && (skilllv = pc_checkskill(sd,SA_ADVANCEDBOOK)) > 0)
		amotion -= skilllv / 2;

	/* �V���O���A�N�V���� */
	if(sd->status.weapon >= WT_HANDGUN && sd->status.weapon <= WT_GRENADE && (skilllv = pc_checkskill(sd,GS_SINGLEACTION)) > 0)
		amotion -= (skilllv+1) / 2 * 10;

	/* bonus_add�̉��Z */
	if(bonus_add != 0)
		amotion += bonus_add;

#ifndef PRE_RENEWAL
	if(pc_isriding(sd))	// �R���C��
		amotion = amotion * (150-10*pc_checkskill(sd,KN_CAVALIERMASTERY)) / 100;
	else if(pc_isdragon(sd))	// �h���S���g���[�j���O
		amotion = amotion * (125-5*pc_checkskill(sd,RK_DRAGONTRAINING)) / 100;

	/* �A�C�e���̃{�[�i�X�����Z���� */
	if(sd->aspd_add_rate != 0 || sd->aspd_rate != 0) {
		sd->aspd_rate += sd->aspd_add_rate;
		amotion = amotion * (100-sd->aspd_rate) / 100;
	}

	/* �{�[�i�XADD_ASPD�̌v�Z */
	amotion += sd->aspd_add;
#endif

	/* �����؂�グ */
	amotion = ceil(amotion);

	return (amotion < 1)? 1:(int)amotion;
}

/*==========================================
 * PC��speed���v�Z���ĕԂ�
 * �߂�͐�����1�ȏ�
 *------------------------------------------
 */
static int status_calc_speed_pc(struct map_session_data *sd, int speed)
{
	int bonus_rate;
	int speed_rate;
	int haste_val1 = 0;
	int haste_val2 = 0;
	int slow_val   = 0;
	int skilllv    = 0;
	char defender_flag = 0;
	char walkspeed_flag = 0;

	nullpo_retr(0, sd);

	if(sd->fix_status.speed > MIN_WALK_SPEED && sd->fix_status.speed <= MAX_WALK_SPEED)	// SPEED�Œ�
		return sd->fix_status.speed;
	if(sd->ud.skilltimer != -1 && pc_checkskill(sd,SA_FREECAST) > 0)	// �t���[�L���X�g��ԂȂ�ړ����x�Œ�
		return sd->speed * (175 - 5 * pc_checkskill(sd,SA_FREECAST)) / 100;
	if(sd->sc.data[SC_STEELBODY].timer != -1)	// �����͈ړ����x�Œ�
		return 200;
	if(sd->ud.skilltimer != -1 && sd->ud.skillid == LG_EXEEDBREAK)		// �C�N�V�[�h�u���C�N�̉r����
		return (150 - 10 * sd->ud.skilllv);

	/* speed���ω�����X�e�[�^�X�̌v�Z */
	if(sd->sc.count > 0) {
		/* speed����������X�e�[�^�X�̌v�Z */

		// �g���l���h���C�u
		if(sd->sc.data[SC_HIDING].timer != -1 && pc_checkskill(sd,RG_TUNNELDRIVE) > 0) {
			slow_val = 120 - 6 * pc_checkskill(sd,RG_TUNNELDRIVE);
		// �`�F�C�X�E�H�[�N(�����)
		} else if(sd->sc.data[SC_CHASEWALK].timer != -1 && sd->sc.data[SC_ROGUE].timer != -1) {
			slow_val = -40;
		} else {
			// ���x����
			if(sd->sc.data[SC_DECREASEAGI].timer != -1)
				slow_val = 25;

			// ���𔛂�Ȃ���
			if(sd->sc.data[SC_LONGINGFREEDOM].timer != -1) {
				int penalty = 50 - 10 * sd->sc.data[SC_LONGINGFREEDOM].val1;
				if(slow_val < penalty)
					slow_val = penalty;
			// �x��/���t
			} else if(sd->sc.data[SC_DANCING].timer != -1) {
				int penalty = 500 - (40 + 10 * (sd->sc.data[SC_BARDDANCER].timer != -1)) * pc_checkskill(sd,((sd->sex == SEX_MALE)? BA_MUSICALLESSON: DC_DANCINGLESSON));
				if(slow_val < penalty)
					slow_val = penalty;
			}

			// �N�@�O�}�C�A
			if(sd->sc.data[SC_QUAGMIRE].timer != -1) {
				if(slow_val < 50)
					slow_val = 50;
			}

			// ����Y��Ȃ���
			if(sd->sc.data[SC_DONTFORGETME].timer != -1) {
				if(slow_val < sd->sc.data[SC_DONTFORGETME].val2)
					slow_val = sd->sc.data[SC_DONTFORGETME].val2;
			}
			else if(sd->sc.data[SC_DONTFORGETME_].timer != -1) {
				if(slow_val < sd->sc.data[SC_DONTFORGETME_].val2)
					slow_val = sd->sc.data[SC_DONTFORGETME_].val2;
			}

			// ��
			if(sd->sc.data[SC_CURSE].timer != -1) {
				if(slow_val < 300)
					slow_val = 300;
			}

			// �`�F�C�X�E�H�[�N(�ʏ�)
			if(sd->sc.data[SC_CHASEWALK].timer != -1 && sd->sc.data[SC_ROGUE].timer == -1) {
				int penalty = 35 - 5 * sd->sc.data[SC_CHASEWALK].val1;
				if(slow_val < penalty)
					slow_val = penalty;
			}

			// �}�[�V���I�u�A�r�X
			if(sd->sc.data[SC_MARSHOFABYSS].timer != -1) {
				if(slow_val < sd->sc.data[SC_MARSHOFABYSS].val2)
					slow_val = sd->sc.data[SC_MARSHOFABYSS].val2;
			}

			// �n���V�l�[�V�����E�H�[�N(�y�i���e�B)
			if(sd->sc.data[SC_HALLUCINATIONWALK2].timer != -1) {
				if(slow_val < 100)
					slow_val = 100;
			}

			// �J���t���[�W��
			if(sd->sc.data[SC_CAMOUFLAGE].timer != -1 && sd->sc.data[SC_CAMOUFLAGE].val1 > 2) {
				int penalty = 25 * (6 - sd->sc.data[SC_CAMOUFLAGE].val1);
				if(slow_val < penalty)
					slow_val = penalty;
			}

			// �j���[�g�����o���A�[(�g�p��)
			if(sd->sc.data[SC_NEUTRALBARRIER_USER].timer != -1) {
				if(slow_val < 30)
					slow_val = 30;
			}

			// �X�e���X�t�B�[���h(�g�p��)
			if(sd->sc.data[SC_STEALTHFIELD_USER].timer != -1) {
				if(slow_val < 30)
					slow_val = 30;
			}

			// �O���r�e�[�V�����t�B�[���h
			if(battle_config.player_gravitation_type && sd->sc.data[SC_GRAVITATION].timer != -1) {
				int penalty = sd->sc.data[SC_GRAVITATION].val1 * 5;
				if(slow_val < penalty)
					slow_val = penalty;
			}

			// �����ߑ�
			if(sd->sc.data[SC_WEDDING].timer != -1) {
				if(slow_val < 100)
					slow_val = 100;
			}

			// �W���C���g�r�[�g
			if(sd->sc.data[SC_JOINTBEAT].timer != -1) {
				int penalty = 0;
				switch (sd->sc.data[SC_JOINTBEAT].val4) {
				case 0:	// ����
					penalty = 50;
					break;
				case 2:	// �G
					penalty = 30;
					break;
				}
				if(slow_val < penalty)
					slow_val = penalty;
			}

			// �N���[�L���O(���n�ړ�)
			if(sd->sc.data[SC_CLOAKING].timer != -1) {
				int i;
				int check = 1;
				for(i=0; i<8; i++) {
					if(map_getcell(sd->bl.m,sd->bl.x+dirx[i],sd->bl.y+diry[i],CELL_CHKNOPASS)) {
						check = 0;
						break;
					}
				}
				if(check) {
					int penalty = (sd->sc.data[SC_CLOAKING].val1 < 3)? 300:30 - 3 * sd->sc.data[SC_CLOAKING].val1;
					if(slow_val < penalty)
						slow_val = penalty;
				}
			}

			// �ړ����x�ቺ(�A�C�e��)
			if(sd->sc.data[SC_SLOWPOTION].timer != -1) {
				if(slow_val < 100)
					slow_val = 100;
			}

			// �K�g�����O�t�B�[�o�[
			if(sd->sc.data[SC_GATLINGFEVER].timer != -1) {
				if(slow_val < 100)
					slow_val = 100;
			}

			// ����
			if(sd->sc.data[SC_SUITON].timer != -1) {
				if(sd->sc.data[SC_SUITON].val4) {
					if(slow_val < 50)
						slow_val = 50;
				}
			}

			// �t���X�g�~�X�e�B
			if(sd->sc.data[SC_FROSTMISTY].timer != -1) {
				if(slow_val < 50)
					slow_val = 50;
			}

			// �}�X�J���[�h �F ���C�W�[�l�X
			if(sd->sc.data[SC__LAZINESS].timer != -1) {
				if(slow_val < 10)
					slow_val = 10;
			}

			// �}�X�J���[�h �F �O���[�~�[
			if(sd->sc.data[SC__GROOMY].timer != -1) {
				int penalty = 5 + 5 * sd->sc.data[SC__GROOMY].val1;
				if(slow_val < penalty)
					slow_val = penalty;
			}

			// ���������e
			if(sd->sc.data[SC_MELON_BOMB].timer != -1) {
				int penalty = sd->sc.data[SC_MELON_BOMB].val1;
				if(slow_val < penalty)
					slow_val = penalty;
			}
		}

		/* speed����������X�e�[�^�X�v�Z1 */

		// �X�s�[�h�|�[�V����
		if(sd->sc.data[SC_SPEEDUP1].timer != -1)
			haste_val1 = 50;

		// ���x����
		if(sd->sc.data[SC_INCREASEAGI].timer != -1) {
			if(haste_val1 < 25)
				haste_val1 = 25;
		}

		// �E�C���h�E�H�[�N
		if(sd->sc.data[SC_WINDWALK].timer != -1) {
			int bonus = 2 * sd->sc.data[SC_WINDWALK].val1;
			if(haste_val1 < bonus)
				haste_val1 = bonus;
		}

		// �J�[�g�u�[�X�g
		if(sd->sc.data[SC_CARTBOOST].timer != -1) {
			if(haste_val1 < 20)
				haste_val1 = 20;
		}

		// �N���[�L���O(�ǉ����ړ�)
		if(sd->sc.data[SC_CLOAKING].timer != -1) {
			int i;
			int check = 1;
			for(i=0; i<8; i++) {
				if(map_getcell(sd->bl.m,sd->bl.x+dirx[i],sd->bl.y+diry[i],CELL_CHKNOPASS)) {
					check = 0;
					break;
				}
			}
			if(!check) {
				int bonus = (sd->sc.data[SC_CLOAKING].val1 >= 10)? 25 : 3 * sd->sc.data[SC_CLOAKING].val1 - 3;
				if(haste_val1 < bonus)
					haste_val1 = bonus;
			}
		}

		// �N���[�L���O�G�N�V�[�h
		if(sd->sc.data[SC_CLOAKINGEXCEED].timer != -1) {
			int bonus = sd->sc.data[SC_CLOAKINGEXCEED].val1 * 10;
			if(haste_val1 < bonus)
				haste_val1 = bonus;
		}

		// �o�[�T�[�N
		if(sd->sc.data[SC_BERSERK].timer != -1) {
			if(haste_val1 < 25)
				haste_val1 = 25;
		}

		// �^�C���M
		if(sd->sc.data[SC_RUN].timer != -1) {
			if(haste_val1 < 55)
				haste_val1 = 55;
		}

		// �ً}���
		if(sd->sc.data[SC_AVOID].timer != -1) {
			int bonus = 10 * sd->sc.data[SC_AVOID].val1;
			if(haste_val1 < bonus)
				haste_val1 = bonus;
		}

		// �E���t�_�b�V��
		if(sd->sc.data[SC_WUGDASH].timer != -1) {
			if(haste_val2 < 50)
				haste_val2 = 50;
		}

		// �A�N�Z�����[�V����
		if(sd->sc.data[SC_ACCELERATION].timer != -1) {
			if(haste_val2 < 25)
				haste_val2 = 25;
		}

		// �X�C���O�_���X
		if(sd->sc.data[SC_SWING].timer != -1) {
			if(haste_val1 < 25)
				haste_val1 = 25;
		}

		// �J�[�g�u�[�X�g
		if(sd->sc.data[SC_GN_CARTBOOST].timer != -1) {
			int bonus = 25 + ((sd->sc.data[SC_GN_CARTBOOST].val1 + 1) / 2) * 25;
			if(haste_val1 < bonus)
				haste_val1 = bonus;
		}

		// �ړ����x����(�A�C�e��)
		if(sd->sc.data[SC_SPEEDUP0].timer != -1) {
			if(haste_val1 < 25)
				haste_val1 = 25;
		}

		// �E�B���h�X�e�b�v
		if(sd->sc.data[SC_WIND_STEP].timer != -1) {
			int bonus = sd->sc.data[SC_WIND_STEP].val3;
			if(haste_val1 < bonus)
				haste_val1 = bonus;
		}

		/* speed����������X�e�[�^�X�v�Z2 */

		// �Z��
		if(sd->sc.data[SC_FUSION].timer != -1)
			haste_val2 = 25;

		/* ���̑� */

		// �f�B�t�F���_�[
		if(sd->sc.data[SC_DEFENDER].timer != -1)
			defender_flag = 1;

		// WALKSPEED
		if(sd->sc.data[SC_WALKSPEED].timer != -1 && sd->sc.data[SC_WALKSPEED].val1 > 0)
			walkspeed_flag = 1;
	}

	// ��𗦑���
	if((sd->s_class.job == PC_JOB_AS || sd->s_class.job == PC_JOB_GC) && (skilllv = pc_checkskill(sd,TF_MISS)) > 0) {
		if(haste_val1 < skilllv)
			haste_val1 = skilllv;
	}

	// �A�C�e���{�[�i�X
	speed_rate = sd->speed_rate + sd->speed_add_rate;
	if(speed_rate != 0) {
		if(haste_val1 < speed_rate)
			haste_val1 = speed_rate;
	}

	/* �R�� */

	// �y�R
	if(pc_isriding(sd)) {
		if(haste_val2 < 25)
			haste_val2 = 25;
	}

	// �h���S��
	if(pc_isdragon(sd)) {
		if(haste_val2 < 25)
			haste_val2 = 25;
	}

	// �E���t
	if(pc_iswolfmount(sd)) {
		int bonus = 15 + 5 * pc_checkskill(sd,RA_WUGRIDER);
		if(haste_val2 < bonus)
			haste_val2 = bonus;
	}

	// �R��V�X�e��
	if(sd->sc.data[SC_ALL_RIDING].timer != -1) {
		if(haste_val2 < 25)
			haste_val2 = 25;
	}

	/* bonus_rate�̍Œ�l��ݒ� */
	bonus_rate = slow_val - haste_val1 - haste_val2;
	if(bonus_rate < -60)
		bonus_rate = -60;

	/* speed�̍ŏI�v�Z */
	if(pc_iscarton(sd))	// �J�[�g
		speed += speed * (50 - 5 * pc_checkskill(sd,MC_PUSHCART)) / 100;
	if(pc_isgear(sd))	// �����M�A����
		speed += speed * (5 - pc_checkskill(sd,NC_MADOLICENCE)) / 10;
	if(bonus_rate != 0)	// bonus_rate
		speed = speed * (bonus_rate+100) / 100;
	if(defender_flag && speed < 200)	// �f�B�t�F���_�[
		speed = 200;
	if(walkspeed_flag)	// �X�N���v�g�p�X�e�[�^�X
		speed = speed * 100 / sd->sc.data[SC_WALKSPEED].val1;

	/* �Œ�l�A�ő�l��ݒ肷�� */
	if(speed < MIN_WALK_SPEED)
		speed = MIN_WALK_SPEED;
	if(speed > MAX_WALK_SPEED)
		speed = MAX_WALK_SPEED;

	return speed;
}

/*==========================================
 * �Ώۂ�group��Ԃ�
 * �߂�͐�����0�ȏ�
 *------------------------------------------
 */
int status_get_group(struct block_list *bl)
{
	nullpo_retr(0, bl);

	if(bl->type == BL_MOB && (struct mob_data *)bl)
		return mob_db[((struct mob_data *)bl)->class_].group_id;
	// PC PET��0�i���ݒ�)

	return 0;
}

/*==========================================
 * �Ώۂ�Class��Ԃ�
 * �߂�͐�����0�ȏ�
 *------------------------------------------
 */
int status_get_class(struct block_list *bl)
{
	nullpo_retr(0, bl);

	if(bl->type == BL_MOB && (struct mob_data *)bl)
		return ((struct mob_data *)bl)->class_;
	else if(bl->type == BL_PC && (struct map_session_data *)bl)
		return ((struct map_session_data *)bl)->status.class_;
	else if(bl->type == BL_PET && (struct pet_data *)bl)
		return ((struct pet_data *)bl)->class_;
	else if(bl->type == BL_HOM && (struct homun_data *)bl)
		return ((struct homun_data *)bl)->status.class_;
	else if(bl->type == BL_MERC && (struct merc_data *)bl)
		return ((struct merc_data *)bl)->status.class_;
	else if(bl->type == BL_ELEM && (struct elem_data *)bl)
		return ((struct elem_data *)bl)->status.class_;

	return 0;
}

/*==========================================
 * �Ώۂ̕�����Ԃ�
 * �߂�͐�����0�ȏ�
 *------------------------------------------
 */
int status_get_dir(struct block_list *bl)
{
	nullpo_retr(0, bl);

	if(bl->type == BL_MOB && (struct mob_data *)bl)
		return ((struct mob_data *)bl)->dir;
	else if(bl->type == BL_PC && (struct map_session_data *)bl)
		return ((struct map_session_data *)bl)->dir;
	else if(bl->type == BL_PET && (struct pet_data *)bl)
		return ((struct pet_data *)bl)->dir;
	else if(bl->type == BL_HOM && (struct homun_data *)bl)
		return ((struct homun_data *)bl)->dir;
	else if(bl->type == BL_MERC && (struct merc_data *)bl)
		return ((struct merc_data *)bl)->dir;
	else if(bl->type == BL_ELEM && (struct elem_data *)bl)
		return ((struct elem_data *)bl)->dir;

	return 0;
}

/*==========================================
 * �Ώۂ̃��x����Ԃ�
 * �߂�͐�����0�ȏ�
 *------------------------------------------
 */
int status_get_lv(struct block_list *bl)
{
	nullpo_retr(0, bl);

	if(bl->type == BL_MOB && (struct mob_data *)bl)
		return mob_db[((struct mob_data *)bl)->class_].lv;
	else if(bl->type == BL_PC && (struct map_session_data *)bl)
		return ((struct map_session_data *)bl)->status.base_level;
	else if(bl->type == BL_PET && (struct pet_data *)bl)
		return ((struct pet_data *)bl)->msd->pet.level;
	else if(bl->type == BL_HOM && (struct homun_data *)bl)
		return ((struct homun_data *)bl)->status.base_level;
	else if(bl->type == BL_MERC && (struct merc_data *)bl)
		return ((struct merc_data *)bl)->base_level;
	else if(bl->type == BL_ELEM && (struct elem_data *)bl)
		return ((struct elem_data *)bl)->base_level;

	return 0;
}

/*==========================================
 * �Ώۂ̎˒���Ԃ�
 * �߂�͐�����0�ȏ�
 *------------------------------------------
 */
int status_get_range(struct block_list *bl)
{
	nullpo_retr(0, bl);

	if(bl->type == BL_MOB && (struct mob_data *)bl)
		return mob_db[((struct mob_data *)bl)->class_].range;
	else if(bl->type == BL_PC && (struct map_session_data *)bl)
		return ((struct map_session_data *)bl)->range.attackrange;
	else if(bl->type == BL_PET && (struct pet_data *)bl)
		return mob_db[((struct pet_data *)bl)->class_].range;
	else if(bl->type == BL_HOM && (struct homun_data *)bl)
		return 2;//((struct homun_data *)bl)->attackrange;
	else if(bl->type == BL_MERC && (struct merc_data *)bl)
		return ((struct merc_data *)bl)->attackrange;
	else if(bl->type == BL_ELEM && (struct elem_data *)bl)
		return ((struct elem_data *)bl)->attackrange;

	return 0;
}

/*==========================================
 * �Ώۂ�HP��Ԃ�
 * �߂�͐�����0�ȏ�
 *------------------------------------------
 */
int status_get_hp(struct block_list *bl)
{
	nullpo_retr(1, bl);

	if(bl->type == BL_MOB && (struct mob_data *)bl)
		return ((struct mob_data *)bl)->hp;
	else if(bl->type == BL_PC && (struct map_session_data *)bl)
		return ((struct map_session_data *)bl)->status.hp;
	else if(bl->type == BL_HOM && (struct homun_data *)bl)
		return ((struct homun_data *)bl)->status.hp;
	else if(bl->type == BL_MERC && (struct merc_data *)bl)
		return ((struct merc_data *)bl)->status.hp;
	else if(bl->type == BL_ELEM && (struct elem_data *)bl)
		return ((struct elem_data *)bl)->status.hp;

	return 1;
}

/*==========================================
 * �Ώۂ�SP��Ԃ�
 * �߂�͐�����0�ȏ�
 *------------------------------------------
 */
int status_get_sp(struct block_list *bl)
{
	nullpo_retr(1, bl);

	if(bl->type == BL_PC && (struct map_session_data *)bl)
		return ((struct map_session_data *)bl)->status.sp;
	else if(bl->type == BL_HOM && (struct homun_data *)bl)
		return ((struct homun_data *)bl)->status.sp;
	else if(bl->type == BL_MERC && (struct merc_data *)bl)
		return ((struct merc_data *)bl)->status.sp;
	else if(bl->type == BL_ELEM && (struct elem_data *)bl)
		return ((struct elem_data *)bl)->status.sp;

	return 0;
}

/*==========================================
 * �Ώۂ�MHP��Ԃ�
 * �߂�͐�����0�ȏ�
 *------------------------------------------
 */
int status_get_max_hp(struct block_list *bl)
{
	int max_hp = 1;

	nullpo_retr(1, bl);

	if(bl->type == BL_PC && ((struct map_session_data *)bl)) {
		max_hp = ((struct map_session_data *)bl)->status.max_hp;
	} else if(bl->type == BL_HOM && ((struct homun_data *)bl)) {
		max_hp = ((struct homun_data *)bl)->max_hp;
	} else if(bl->type == BL_MERC && ((struct merc_data *)bl)) {
		max_hp = ((struct merc_data *)bl)->max_hp;
	} else if(bl->type == BL_ELEM && ((struct elem_data *)bl)) {
		max_hp = ((struct elem_data *)bl)->max_hp;
	} else {
		struct status_change *sc = status_get_sc(bl);

		if(bl->type == BL_MOB && ((struct mob_data*)bl)) {
			atn_bignumber hp = mob_db[((struct mob_data*)bl)->class_].max_hp;
			struct guild_castle *gc = guild_mapid2gc(bl->m);
			if(mob_db[((struct mob_data*)bl)->class_].mexp > 0) {
				if(battle_config.mvp_hp_rate != 100) {
					hp = hp * battle_config.mvp_hp_rate / 100;
				}
			} else {
				if(battle_config.monster_hp_rate != 100)
					hp = hp * battle_config.monster_hp_rate / 100;
			}
			if(gc && ((struct mob_data*)bl)->guild_id == gc->guild_id) {
				hp += gc->defense * 1000;
			}
			max_hp = (hp > 0x7FFFFFFF ? 0x7FFFFFFF : (int)hp);
		} else if(bl->type == BL_PET && ((struct pet_data*)bl)) {
			max_hp = mob_db[((struct pet_data*)bl)->class_].max_hp;
			if(mob_db[((struct pet_data*)bl)->class_].mexp > 0) {
				if(battle_config.mvp_hp_rate != 100) {
					atn_bignumber hp = (atn_bignumber)max_hp * battle_config.mvp_hp_rate / 100;
					max_hp = (hp > 0x7FFFFFFF ? 0x7FFFFFFF : (int)hp);
				}
			} else {
				if(battle_config.monster_hp_rate != 100)
					max_hp = (max_hp * battle_config.monster_hp_rate)/100;
			}
		}
		if(sc) {
			if(sc->data[SC_APPLEIDUN].timer != -1)
				max_hp += ((5+sc->data[SC_APPLEIDUN].val1*2+((sc->data[SC_APPLEIDUN].val2+1)>>1)
						+sc->data[SC_APPLEIDUN].val3/10) * max_hp)/100;
		}
		if(max_hp < 1) max_hp = 1;
	}

	return max_hp;
}

/*==========================================
 * �Ώۂ�MSP��Ԃ�
 * �߂�͐�����0�ȏ�
 *------------------------------------------
 */
int status_get_max_sp(struct block_list *bl)
{
	int max_sp = 0;

	nullpo_retr(0, bl);

	if(bl->type == BL_PC && ((struct map_session_data *)bl)) {
		max_sp = ((struct map_session_data *)bl)->status.max_sp;
	} else if(bl->type == BL_HOM && ((struct homun_data *)bl)) {
		max_sp = ((struct homun_data *)bl)->max_sp;
	} else if(bl->type == BL_MERC && ((struct merc_data *)bl)) {
		max_sp = ((struct merc_data *)bl)->max_sp;
	} else if(bl->type == BL_ELEM && ((struct elem_data *)bl)) {
		max_sp = ((struct elem_data *)bl)->max_sp;
	}

	return max_sp;
}

/*==========================================
 * �Ώۂ�Str��Ԃ�
 * �߂�͐�����0�ȏ�
 *------------------------------------------
 */
int status_get_str(struct block_list *bl)
{
	int str = 0;
	struct status_change *sc;

	nullpo_retr(0, bl);

	sc = status_get_sc(bl);
	if(bl->type == BL_MOB && ((struct mob_data *)bl))
		str = mob_db[((struct mob_data *)bl)->class_].str;
	else if(bl->type == BL_PC && ((struct map_session_data *)bl))
		return ((struct map_session_data *)bl)->paramc[0];
	else if(bl->type == BL_PET && ((struct pet_data *)bl))
		str = mob_db[((struct pet_data *)bl)->class_].str;
	else if(bl->type == BL_HOM && ((struct homun_data *)bl))
		str = ((struct homun_data *)bl)->status.str;
	else if(bl->type == BL_MERC && ((struct merc_data *)bl))
		str = ((struct merc_data *)bl)->str;
	else if(bl->type == BL_ELEM && ((struct elem_data *)bl))
		str = ((struct elem_data *)bl)->str;

	if(sc && bl->type != BL_HOM) {
		if(sc->data[SC_LOUD].timer != -1 && sc->data[SC_QUAGMIRE].timer == -1 && bl->type != BL_PC)
			str += 4;
		if( sc->data[SC_BLESSING].timer != -1 && bl->type != BL_PC) {	// �u���b�V���O
			int race = status_get_race(bl);
			if(battle_check_undead(race,status_get_elem_type(bl)) || race == RCT_DEMON)
				str >>= 1;				// ����/�s��
			else
				str += sc->data[SC_BLESSING].val1;	// ���̑�
		}
		if(sc->data[SC_TRUESIGHT].timer != -1 && bl->type != BL_PC)	// �g�D���[�T�C�g
			str += 5;
		if(sc->data[SC_CHASEWALK_STR].timer != -1)
			str += sc->data[SC_CHASEWALK_STR].val1;
	}
	if(str < 0) str = 0;
	return str;
}

/*==========================================
 * �Ώۂ�Agi��Ԃ�
 * �߂�͐�����0�ȏ�
 *------------------------------------------
 */
int status_get_agi(struct block_list *bl)
{
	int agi = 0;
	struct status_change *sc;

	nullpo_retr(0, bl);

	sc = status_get_sc(bl);
	if(bl->type == BL_MOB && (struct mob_data *)bl)
		agi = mob_db[((struct mob_data *)bl)->class_].agi;
	else if(bl->type == BL_PC && (struct map_session_data *)bl)
		agi = ((struct map_session_data *)bl)->paramc[1];
	else if(bl->type == BL_PET && (struct pet_data *)bl)
		agi = mob_db[((struct pet_data *)bl)->class_].agi;
	else if(bl->type == BL_HOM && ((struct homun_data *)bl))
		agi = ((struct homun_data *)bl)->agi;
	else if(bl->type == BL_MERC && ((struct merc_data *)bl))
		agi = ((struct merc_data *)bl)->agi;
	else if(bl->type == BL_ELEM && ((struct elem_data *)bl))
		agi = ((struct elem_data *)bl)->agi;

	if(sc && bl->type != BL_HOM) {
		if(sc->data[SC_SPEEDUP1].timer != -1 && bl->type != BL_PC)	// ���x����
			agi *= 3;
		if(sc->data[SC_INCREASEAGI].timer != -1 && sc->data[SC_QUAGMIRE].timer == -1 && sc->data[SC_DONTFORGETME].timer == -1 && bl->type != BL_PC)	// ���x����(PC��pc.c��)
			agi += 2+sc->data[SC_INCREASEAGI].val1;
		if(sc->data[SC_SUITON].timer != -1 && sc->data[SC_SUITON].val3 != 0 && bl->type != BL_PC)	// ����
			agi += sc->data[SC_SUITON].val3;
		if(sc->data[SC_CONCENTRATE].timer != -1 && sc->data[SC_QUAGMIRE].timer == -1 && bl->type != BL_PC)
			agi += agi*(2+sc->data[SC_CONCENTRATE].val1)/100;
		if(sc->data[SC_DECREASEAGI].timer != -1)	// ���x�����i�I�[�o�[�X�L���d�l��AGI-50�j
			agi -= (sc->data[SC_DECREASEAGI].val2)? 50: 2+sc->data[SC_DECREASEAGI].val1;
		if(sc->data[SC_QUAGMIRE].timer != -1)	// �N�@�O�}�C�A
#ifdef PRE_RENEWAL
			agi -= agi * sc->data[SC_QUAGMIRE].val1*10 / 100;
#else
			agi -= (agi/2 < sc->data[SC_QUAGMIRE].val1*10) ? agi/2 : sc->data[SC_QUAGMIRE].val1*10;
#endif
		if(sc->data[SC_MARSHOFABYSS].timer != -1)	// �}�[�V���I�u�A�r�X
			agi -= agi * sc->data[SC_MARSHOFABYSS].val3 / 100;
		if(sc->data[SC_TRUESIGHT].timer != -1 && bl->type != BL_PC)	// �g�D���[�T�C�g
			agi += 5;
	}
	if(agi < 0) agi = 0;
	return agi;
}

/*==========================================
 * �Ώۂ�Vit��Ԃ�
 * �߂�͐�����0�ȏ�
 *------------------------------------------
 */
int status_get_vit(struct block_list *bl)
{
	int vit = 0;
	struct status_change *sc;

	nullpo_retr(0, bl);

	sc = status_get_sc(bl);
	if(bl->type == BL_MOB && (struct mob_data *)bl)
		vit = mob_db[((struct mob_data *)bl)->class_].vit;
	else if(bl->type == BL_PC && (struct map_session_data *)bl)
		vit = ((struct map_session_data *)bl)->paramc[2];
	else if(bl->type == BL_PET && (struct pet_data *)bl)
		vit = mob_db[((struct pet_data *)bl)->class_].vit;
	else if(bl->type == BL_HOM && ((struct homun_data *)bl))
		vit = ((struct homun_data *)bl)->vit;
	else if(bl->type == BL_MERC && ((struct merc_data *)bl))
		vit = ((struct merc_data *)bl)->vit;
	else if(bl->type == BL_ELEM && ((struct elem_data *)bl))
		vit = ((struct elem_data *)bl)->vit;

	if(sc && bl->type != BL_HOM) {
		if(sc->data[SC_STRIPARMOR].timer != -1 && bl->type != BL_PC)
			vit = vit*60/100;
		if(sc->data[SC_TRUESIGHT].timer != -1 && bl->type != BL_PC)	// �g�D���[�T�C�g
			vit += 5;
	}

	if(vit < 0) vit = 0;
	return vit;
}

/*==========================================
 * �Ώۂ�Int��Ԃ�
 * �߂�͐�����0�ȏ�
 *------------------------------------------
 */
int status_get_int(struct block_list *bl)
{
	int int_ = 0;
	struct status_change *sc;

	nullpo_retr(0, bl);

	sc = status_get_sc(bl);
	if(bl->type == BL_MOB && (struct mob_data *)bl)
		int_ = mob_db[((struct mob_data *)bl)->class_].int_;
	else if(bl->type == BL_PC && (struct map_session_data *)bl)
		int_ = ((struct map_session_data *)bl)->paramc[3];
	else if(bl->type == BL_PET && (struct pet_data *)bl)
		int_ = mob_db[((struct pet_data *)bl)->class_].int_;
	else if(bl->type == BL_HOM && ((struct homun_data *)bl))
		int_ = ((struct homun_data *)bl)->int_;
	else if(bl->type == BL_MERC && ((struct merc_data *)bl))
		int_ = ((struct merc_data *)bl)->int_;
	else if(bl->type == BL_ELEM && ((struct elem_data *)bl))
		int_ = ((struct elem_data *)bl)->int_;

	if(sc && bl->type != BL_HOM) {
		if(sc->data[SC_BLESSING].timer != -1 && bl->type != BL_PC) {	// �u���b�V���O
			int race = status_get_race(bl);
			if(battle_check_undead(race,status_get_elem_type(bl)) || race == RCT_DEMON)
				int_ >>= 1;	// ����/�s��
			else
				int_ += sc->data[SC_BLESSING].val1;	// ���̑�
		}
		if(sc->data[SC_STRIPHELM].timer != -1 && bl->type != BL_PC)
			int_ = int_*60/100;
		if(sc->data[SC_TRUESIGHT].timer != -1 && bl->type != BL_PC)	// �g�D���[�T�C�g
			int_ += 5;
		if(sc->data[SC__STRIPACCESSARY].timer != -1 && bl->type != BL_PC)	// �X�g���b�v�A�N�Z�T���[
			int_ = int_ * 80 / 100;
	}
	if(int_ < 0) int_ = 0;
	return int_;
}

/*==========================================
 * �Ώۂ�Dex��Ԃ�
 * �߂�͐�����0�ȏ�
 *------------------------------------------
 */
int status_get_dex(struct block_list *bl)
{
	int dex = 0;
	struct status_change *sc;

	nullpo_retr(0, bl);

	sc = status_get_sc(bl);
	if(bl->type == BL_MOB && (struct mob_data *)bl)
		dex = mob_db[((struct mob_data *)bl)->class_].dex;
	else if(bl->type == BL_PC && (struct map_session_data *)bl)
		dex = ((struct map_session_data *)bl)->paramc[4];
	else if(bl->type == BL_PET && (struct pet_data *)bl)
		dex = mob_db[((struct pet_data *)bl)->class_].dex;
	else if(bl->type == BL_HOM && ((struct homun_data *)bl))
		dex = ((struct homun_data *)bl)->dex;
	else if(bl->type == BL_MERC && ((struct merc_data *)bl))
		dex = ((struct merc_data *)bl)->dex;
	else if(bl->type == BL_ELEM && ((struct elem_data *)bl))
		dex = ((struct elem_data *)bl)->dex;

	if(sc && bl->type != BL_HOM) {
		if(sc->data[SC_EXPLOSIONSPIRITS].timer != -1 && bl->type != BL_PC)
			dex *= 3;	// NPC�����g��
		if(sc->data[SC_CONCENTRATE].timer != -1 && sc->data[SC_QUAGMIRE].timer == -1 && bl->type != BL_PC)
			dex += dex*(2+sc->data[SC_CONCENTRATE].val1)/100;

		if(sc->data[SC_BLESSING].timer != -1 && bl->type != BL_PC) {	// �u���b�V���O
			int race = status_get_race(bl);
			if(battle_check_undead(race,status_get_elem_type(bl)) || race == RCT_DEMON)
				dex >>= 1;	// ����/�s��
			else
				dex += sc->data[SC_BLESSING].val1;	// ���̑�
		}
		if(sc->data[SC_QUAGMIRE].timer != -1)	// �N�@�O�}�C�A
#ifdef PRE_RENEWAL
			dex -= dex * sc->data[SC_QUAGMIRE].val1*10 / 100;
#else
			dex -= (dex/2 < sc->data[SC_QUAGMIRE].val1*10) ? dex/2 : sc->data[SC_QUAGMIRE].val1*10;
#endif
		if(sc->data[SC_MARSHOFABYSS].timer != -1)	// �}�[�V���I�u�A�r�X
			dex -= dex * sc->data[SC_MARSHOFABYSS].val3 / 100;
		if(sc->data[SC_TRUESIGHT].timer != -1 && bl->type != BL_PC)	// �g�D���[�T�C�g
			dex += 5;
		if(sc->data[SC__STRIPACCESSARY].timer != -1 && bl->type != BL_PC)	// �X�g���b�v�A�N�Z�T���[
			dex = dex * 80 / 100;
	}
	if(dex < 0) dex = 0;
	return dex;
}

/*==========================================
 * �Ώۂ�Luk��Ԃ�
 * �߂�͐�����0�ȏ�
 *------------------------------------------
 */
int status_get_luk(struct block_list *bl)
{
	int luk = 0;
	struct status_change *sc;

	nullpo_retr(0, bl);

	sc = status_get_sc(bl);
	if(bl->type == BL_MOB && (struct mob_data *)bl)
		luk = mob_db[((struct mob_data *)bl)->class_].luk;
	else if(bl->type == BL_PC && (struct map_session_data *)bl)
		luk = ((struct map_session_data *)bl)->paramc[5];
	else if(bl->type == BL_PET && (struct pet_data *)bl)
		luk = mob_db[((struct pet_data *)bl)->class_].luk;
	else if(bl->type == BL_HOM && ((struct homun_data *)bl))
		luk = ((struct homun_data *)bl)->luk;
	else if(bl->type == BL_MERC && ((struct merc_data *)bl))
		luk = ((struct merc_data *)bl)->luk;
	else if(bl->type == BL_ELEM && ((struct elem_data *)bl))
		luk = ((struct elem_data *)bl)->luk;

	if(sc && bl->type != BL_HOM) {
		if(sc->data[SC_GLORIA].timer != -1 && bl->type != BL_PC)	// �O�����A(PC��pc.c��)
			luk += 30;
		if(sc->data[SC_CURSE].timer != -1 )		// ��
			luk = 0;
		if(sc->data[SC_TRUESIGHT].timer != -1 && bl->type != BL_PC)	// �g�D���[�T�C�g
			luk += 5;
		if(sc->data[SC__STRIPACCESSARY].timer != -1 && bl->type != BL_PC)	// �X�g���b�v�A�N�Z�T���[
			luk = luk * 80 / 100;
		if(sc->data[SC_BANANA_BOMB].timer != -1 && bl->type != BL_PC)	// �o�i�i���e
			luk -= luk * sc->data[SC_BANANA_BOMB].val1 / 100;
	}
	if(luk < 0) luk = 0;
	return luk;
}

/*==========================================
 * �Ώۂ�Flee��Ԃ�
 * �߂�͐�����1�ȏ�
 *------------------------------------------
 */
int status_get_flee(struct block_list *bl)
{
	int flee = 1, target_count = 1;
	struct status_change *sc;

	nullpo_retr(1, bl);

	sc = status_get_sc(bl);
	if(bl->type == BL_PC && (struct map_session_data *)bl)
		flee = ((struct map_session_data *)bl)->flee;
	else if(bl->type == BL_HOM && ((struct homun_data *)bl))
		flee = ((struct homun_data *)bl)->flee;
	else if(bl->type == BL_MERC && ((struct merc_data *)bl))
		flee = ((struct merc_data *)bl)->flee;
	else if(bl->type == BL_ELEM && ((struct elem_data *)bl))
		flee = ((struct elem_data *)bl)->flee;
	else
		flee = status_get_agi(bl) + status_get_lv(bl);

#ifndef PRE_RENEWAL
	if(bl->type != BL_PC)
		flee += 100;
#endif

	if(sc && bl->type != BL_HOM) {
		if(sc->data[SC_WHISTLE].timer != -1 && bl->type != BL_PC)
			flee += sc->data[SC_WHISTLE].val1 + sc->data[SC_WHISTLE].val2 + sc->data[SC_WHISTLE].val3;
		if(sc->data[SC_BLIND].timer != -1 && bl->type != BL_PC)
			flee -= flee*25/100;
		if(sc->data[SC_WINDWALK].timer != -1 && bl->type != BL_PC)		// �E�B���h�E�H�[�N
			flee += sc->data[SC_WINDWALK].val2;
		if(sc->data[SC_SPIDERWEB].timer != -1 && bl->type != BL_PC)	// �X�p�C�_�[�E�F�u
			flee -= 50;
		if(sc->data[SC_THE_SUN].timer != -1 && bl->type != BL_PC)	// THE SUN
			flee = flee*80/100;
		if(sc->data[SC_GATLINGFEVER].timer != -1 && bl->type != BL_PC)	// �K�g�����O�t�B�[�o�[
			flee -= sc->data[SC_GATLINGFEVER].val1*5;
		if(sc->data[SC_ADJUSTMENT].timer != -1 && bl->type != BL_PC)	// �A�W���X�g�����g
			flee += 30;
		if(sc->data[SC_FEAR].timer != -1 && bl->type != BL_PC)		// ���|
			flee -= flee*20/100;
		if(sc->data[SC_HALLUCINATIONWALK].timer != -1 && bl->type != BL_PC)		// �n���V�l�[�V�����E�H�[�N
			flee += sc->data[SC_HALLUCINATIONWALK].val1 * 50;
		if(sc->data[SC_INFRAREDSCAN].timer != -1 && bl->type != BL_PC)	// �C���t�����b�h�X�L����
			flee  -= flee*30/100;
		if(sc->data[SC__LAZINESS].timer != -1 && bl->type != BL_PC)		// �}�X�J���[�h �F ���C�W�[�l�X
			flee  -= flee*10/100;
		if(sc->data[SC_GLOOMYDAY].timer != -1 && bl->type != BL_PC)		// �������R���[
			flee -= sc->data[SC_GLOOMYDAY].val1 * 5;
		if(sc->data[SC_SATURDAY_NIGHT_FEVER].timer != -1 && bl->type != BL_PC)		// �t���C�f�[�i�C�g�t�B�[�o�[
			flee -= flee * (40 + sc->data[SC_SATURDAY_NIGHT_FEVER].val1 * 10) / 100;
		if(sc->data[SC_FIRE_EXPANSION_SMOKE_POWDER].timer != -1 && bl->type != BL_PC)	// �t�@�C�A�[�G�N�X�p���V����(����)
			flee += flee * sc->data[SC_FIRE_EXPANSION_SMOKE_POWDER].val2 / 100;
		if(sc->data[SC_FIRE_EXPANSION_TEAR_GAS].timer != -1 && bl->type != BL_PC)	// �t�@�C�A�[�G�N�X�p���V����(�×�)
			flee -= flee * sc->data[SC_FIRE_EXPANSION_TEAR_GAS].val2 / 100;
#ifndef PRE_RENEWAL
		if(sc->data[SC_SPEARQUICKEN].timer != -1 && bl->type != BL_PC)      // �X�s�A�N�C�b�P��
			flee += sc->data[SC_SPEARQUICKEN].val1 * 2;
#endif
	}

	// ��𗦕␳
	if(bl->type != BL_HOM)
		target_count = unit_counttargeted(bl,battle_config.agi_penalty_count_lv);

	if(battle_config.agi_penalty_type > 0 && target_count >= battle_config.agi_penalty_count) {
		// �y�i���e�B�ݒ���Ώۂ�����
		if(battle_config.agi_penalty_type == 1) {
			// ��𗦂�agi_penalty_num%������
			int flee_rate = (target_count-(battle_config.agi_penalty_count-1)) * battle_config.agi_penalty_num;
			flee = flee * (100 - flee_rate) / 100;
		} else if(battle_config.agi_penalty_type == 2) {
			// ��𗦂�agi_penalty_num������
			flee -= (target_count - (battle_config.agi_penalty_count - 1))*battle_config.agi_penalty_num;
		}
	}
	// �ΐlMAP�ł̌�������
	if(battle_config.gvg_flee_penalty & 1 && map[bl->m].flag.gvg) {
		flee = flee*(200 - battle_config.gvg_flee_rate)/100;	// ���ۂ�GvG��Flee������
	}
	if(battle_config.gvg_flee_penalty & 2 && map[bl->m].flag.pvp) {
		flee = flee*(200 - battle_config.gvg_flee_rate)/100;	// ���ۂ�PvP��Flee������
	}
	if(flee < 1) flee = 1;

	return flee;
}

/*==========================================
 * �Ώۂ�Hit��Ԃ�
 * �߂�͐�����1�ȏ�
 *------------------------------------------
 */
int status_get_hit(struct block_list *bl)
{
	int hit = 0;

	nullpo_retr(1, bl);

#ifndef PRE_RENEWAL
	if(!(bl->type & (BL_PC | BL_HOM)))
		hit = 150;
#endif
	if(bl->type == BL_PC) {
		hit = ((struct map_session_data *)bl)->hit;
	} else if(bl->type == BL_HOM && ((struct homun_data *)bl)) {
		hit += ((struct homun_data *)bl)->hit;
	} else if(bl->type == BL_MERC && ((struct merc_data *)bl)) {
		hit += ((struct merc_data *)bl)->hit;
	} else if(bl->type == BL_ELEM && ((struct elem_data *)bl)) {
		hit += ((struct elem_data *)bl)->hit;
	} else {
		struct status_change *sc = status_get_sc(bl);

		hit += status_get_dex(bl) + status_get_lv(bl);
		if(sc) {
			if(sc->data[SC_HUMMING].timer != -1)
				hit += hit*(sc->data[SC_HUMMING].val1*2+sc->data[SC_HUMMING].val2 + sc->data[SC_HUMMING].val3)/100;
			if(sc->data[SC_TRUESIGHT].timer != -1)		// �g�D���[�T�C�g
				hit += 3*(sc->data[SC_TRUESIGHT].val1);
			if(sc->data[SC_CONCENTRATION].timer != -1)	// �R���Z���g���[�V����
				hit += 10*(sc->data[SC_CONCENTRATION].val1);
			if(sc->data[SC_THE_SUN].timer != -1 && bl->type != BL_PC)
				hit = hit*80/100;
			if(sc->data[SC_ADJUSTMENT].timer != -1 && bl->type != BL_PC) // �A�W���X�g�����g
				hit -= 30;
			if(sc->data[SC_INCREASING].timer != -1 && bl->type != BL_PC) // �C���N���[�V���O�A�L���A���V�[
				hit += 20;
			if(sc->data[SC_INCHIT].timer!=-1 && bl->type != BL_PC)	// �K�C�f�b�h�A�^�b�N
				hit += sc->data[SC_INCHIT].val1;
			if(sc->data[SC_FEAR].timer != -1 && bl->type != BL_PC)	// ���|
				hit -= hit*20/100;
			if(sc->data[SC__GROOMY].timer != -1 && bl->type != BL_PC)	// �}�X�J���[�h �F �O���[�~�[
				hit -= hit*(20*sc->data[SC__GROOMY].val1)/100;
			if(sc->data[SC_FIRE_EXPANSION_TEAR_GAS].timer != -1 && bl->type != BL_PC)	// �t�@�C�A�[�G�N�X�p���V����(�×�)
				hit -= hit*(sc->data[SC_FIRE_EXPANSION_TEAR_GAS].val2)/100;
		}
	}
	if(hit < 1) hit = 1;

	return hit;
}

/*==========================================
 * �Ώۂ̊��S�����Ԃ�
 * �߂�͐�����1�ȏ�
 *------------------------------------------
 */
int status_get_flee2(struct block_list *bl)
{
	int flee2 = 1;
	struct status_change *sc;

	nullpo_retr(1, bl);

	sc = status_get_sc(bl);
	if(bl->type == BL_PC && (struct map_session_data *)bl) {
		flee2 = status_get_luk(bl) + 10;
		flee2 += ((struct map_session_data *)bl)->flee2 - (((struct map_session_data *)bl)->paramc[5] + 10);
	} else {
		flee2 = status_get_luk(bl)+1;
	}

	if(sc) {
		if(sc->data[SC__UNLUCKY].timer != -1 && bl->type != BL_PC)	// �}�X�J���[�h �F �A�����b�L�[
			flee2 -= sc->data[SC__UNLUCKY].val1 * 10;
	}

	if(flee2 < 1) flee2 = 1;
	return flee2;
}

/*==========================================
 * �Ώۂ̃N���e�B�J����Ԃ�
 * �߂�͐�����1�ȏ�
 *------------------------------------------
 */
int status_get_critical(struct block_list *bl)
{
	int critical = 1;
	struct status_change *sc;

	nullpo_retr(1, bl);

	sc = status_get_sc(bl);
	if(bl->type == BL_PC && (struct map_session_data *)bl) {
#ifdef PRE_RENEWAL
		critical = status_get_luk(bl)*3 + 10;
		critical += ((struct map_session_data *)bl)->critical - ((((struct map_session_data *)bl)->paramc[5]*3) + 10);
#else
		critical = status_get_luk(bl)*33/10 + 10;
		critical += ((struct map_session_data *)bl)->critical - ((((struct map_session_data *)bl)->paramc[5]*33/10) + 10);
#endif
	} else if(bl->type == BL_HOM && ((struct homun_data *)bl)) {
		critical = ((struct homun_data *)bl)->critical;
	} else if(bl->type == BL_MERC && ((struct merc_data *)bl)) {
		critical = ((struct merc_data *)bl)->critical;
	} else if(bl->type == BL_ELEM && ((struct elem_data *)bl)) {
		critical = ((struct elem_data *)bl)->critical;
	} else {
		critical = status_get_luk(bl)*3 + 1;
	}

	if(sc) {
		if(sc->data[SC_FORTUNE].timer != -1 && bl->type != BL_PC)
			critical += (10+sc->data[SC_FORTUNE].val1+sc->data[SC_FORTUNE].val2+sc->data[SC_FORTUNE].val3)*10;
		if(sc->data[SC_EXPLOSIONSPIRITS].timer != -1 && bl->type != BL_PC)
			critical += sc->data[SC_EXPLOSIONSPIRITS].val2;
		if(sc->data[SC_TRUESIGHT].timer != -1 && bl->type != BL_PC)	// �g�D���[�T�C�g
			critical += 10*sc->data[SC_TRUESIGHT].val1;
		if(sc->data[SC__INVISIBILITY].timer != -1 && bl->type != BL_PC)	// �C���r�W�r���e�B
			critical += critical * (sc->data[SC__INVISIBILITY].val1 * 20) / 100;
		if(sc->data[SC__UNLUCKY].timer != -1 && bl->type != BL_PC)	// �}�X�J���[�h �F �A�����b�L�[
			critical -= critical * (sc->data[SC__UNLUCKY].val1 * 10) / 100;
		if(sc->data[SC_STRIKING].timer != -1 && bl->type != BL_PC)	// �X�g���C�L���O
			critical += 10 * sc->data[SC_STRIKING].val1;
#ifndef PRE_RENEWAL
		if(sc->data[SC_SPEARQUICKEN].timer != -1 && bl->type != BL_PC)   // �X�s�A�N�C�b�P��
			critical += 30*sc->data[SC_SPEARQUICKEN].val1;
#endif
	}
	if(critical < 1) critical = 1;
	return critical;
}

/*==========================================
 * base_atk�̎擾
 * �߂�͐�����1�ȏ�
 *------------------------------------------
 */
int status_get_baseatk(struct block_list *bl)
{
	struct status_change *sc;
	int batk = 1;

	nullpo_retr(1, bl);

	sc = status_get_sc(bl);

	if(bl->type == BL_PC) {
		struct map_session_data *sd = (struct map_session_data *)bl;
		if(sd) {
			batk = sd->base_atk;	// �ݒ肳��Ă���base_atk
#ifdef PRE_RENEWAL
			if(sd->status.weapon < WT_MAX) {
				batk += sd->weapon_atk[sd->status.weapon];
			}
#endif
		}
	} else if(bl->type == BL_HOM && ((struct homun_data *)bl)) {
#ifdef PRE_RENEWAL
		batk = 1;
#else
		batk = status_get_str(bl) + status_get_lv(bl);
#endif
	} else if(bl->type == BL_MERC && ((struct merc_data *)bl)) {
		batk = 1;
	} else if(bl->type == BL_ELEM && ((struct elem_data *)bl)) {
		batk = 1;
	} else {	// ����ȊO�Ȃ�
#ifdef PRE_RENEWAL
		int str, dstr;
		str  = status_get_str(bl);	// STR
		dstr = str/10;
		batk = dstr*dstr + str;	// base_atk���v�Z����
#else
		batk = status_get_str(bl) + status_get_lv(bl);	// base_atk���v�Z����
#endif
	}
	if(sc) {	// ��Ԉُ킠��
#ifdef PRE_RENEWAL
		if(sc->data[SC__BLOODYLUST].timer != -1 && bl->type != BL_PC)	// �u���b�f�B���X�g
			batk = batk*(100+32)/100;	// base_atk����
		else if(sc->data[SC_PROVOKE].timer != -1 && bl->type != BL_PC)	// PC�Ńv���{�b�N(SM_PROVOKE)���
			batk = batk*(100+2+3*sc->data[SC_PROVOKE].val1)/100;	// base_atk����
		if(sc->data[SC_CURSE].timer != -1)	// ����Ă�����
			batk -= batk*25/100;	// base_atk��25%����
		if(sc->data[SC_CONCENTRATION].timer != -1 && bl->type != BL_PC)	// �R���Z���g���[�V����
			batk += batk*(5*sc->data[SC_CONCENTRATION].val1)/100;
#endif
		if(sc->data[SC_JOINTBEAT].timer != -1 && sc->data[SC_JOINTBEAT].val4 == 4)	// �W���C���g�r�[�g�ō�
			batk -= batk*25/100;
		if(sc->data[SC_MADNESSCANCEL].timer != -1 && bl->type != BL_PC)	// �}�b�h�l�X�L�����Z���[
			batk += 100;
		if(sc->data[SC_THE_MAGICIAN].timer != -1)
			batk = batk*50/100;
		if(sc->data[SC_THE_DEVIL].timer != -1)
			batk = batk*50/100;
		if(sc->data[SC_THE_SUN].timer != -1)
			batk = batk*80/100;
		if(sc->data[SC_SKE].timer != -1 && bl->type == BL_MOB)	// �G�X�N
			batk *= 4;
		if(sc->data[SC__ENERVATION].timer != -1 && bl->type == BL_MOB)	// �}�X�J���[�h �F �G�i�[�x�[�V����
			batk -= batk * (20 + sc->data[SC__ENERVATION].val1 * 10) / 100;
		if(sc->data[SC_SHIELDSPELL_DEF].timer != -1 && sc->data[SC_SHIELDSPELL_DEF].val2 == 2 && bl->type == BL_MOB)	// �V�[���h�X�y��(DEF)
			batk += sc->data[SC_SHIELDSPELL_DEF].val3;
		if(sc->data[SC_SATURDAY_NIGHT_FEVER].timer != -1 && bl->type == BL_MOB)	// �t���C�f�[�i�C�g�t�B�[�o�[
			batk += 100 * sc->data[SC_SATURDAY_NIGHT_FEVER].val1;
		if(sc->data[SC_ODINS_POWER].timer != -1 && bl->type == BL_MOB)	// �I�[�f�B���̗�
			batk += 60 + 10 * sc->data[SC_ODINS_POWER].val1;
	}
	if(batk < 1) batk = 1;	// base_atk�͍Œ�ł�1
	return batk;
}

/*==========================================
 * �Ώۂ�Atk��Ԃ�
 * �߂�͐�����0�ȏ�
 *------------------------------------------
 */
int status_get_atk(struct block_list *bl)
{
	struct status_change *sc;
	int atk = 0;

	nullpo_retr(0, bl);

	sc = status_get_sc(bl);
	if(bl->type == BL_PC && (struct map_session_data *)bl) {
		atk = ((struct map_session_data*)bl)->watk;
	} else if(bl->type == BL_HOM && ((struct homun_data *)bl)) {
		atk = ((struct homun_data *)bl)->atk-((struct homun_data *)bl)->atk/10;
	} else if(bl->type == BL_MERC && ((struct merc_data *)bl)) {
		atk = ((struct merc_data *)bl)->atk1;
	} else if(bl->type == BL_ELEM && ((struct elem_data *)bl)) {
		atk = ((struct elem_data *)bl)->atk1;
	} else if(bl->type == BL_MOB) {
		struct mob_data *md = (struct mob_data *)bl;
		if(md) {
			int guardup_lv = md->guardup_lv;
#ifndef PRE_RENEWAL
			if(battle_config.monster_atk2_to_matk)
				atk = mob_db[md->class_].atk1 * 8 / 10;
			else
#endif
				atk = mob_db[md->class_].atk1;
			if(guardup_lv > 0)
				atk += 1000*guardup_lv;
		}
	} else if(bl->type == BL_PET && (struct pet_data *)bl) {
		atk = mob_db[((struct pet_data*)bl)->class_].atk1;
	}

	if(sc) {
		int rate = 100;
#ifdef PRE_RENEWAL
		if(sc->data[SC__BLOODYLUST].timer != -1 && bl->type != BL_PC)
			atk = atk*(100+32)/100;
		else if(sc->data[SC_PROVOKE].timer != -1 && bl->type != BL_PC)
			atk = atk*(100+2+3*sc->data[SC_PROVOKE].val1)/100;
		if(sc->data[SC_CURSE].timer != -1)
			atk -= atk*25/100;
		if(sc->data[SC_CONCENTRATION].timer != -1 && bl->type != BL_PC)	// �R���Z���g���[�V����
			atk += atk*(5*sc->data[SC_CONCENTRATION].val1)/100;
#endif
		if(sc->data[SC_EXPLOSIONSPIRITS].timer != -1 && bl->type != BL_PC)	// NPC�����g��
			rate += 200;
		if(sc->data[SC_STRIPWEAPON].timer != -1 && bl->type != BL_PC)
			atk -= atk*25/100;
		if(sc->data[SC_DISARM].timer != -1 && bl->type != BL_PC)		// �f�B�X�A�[��
			atk -= atk*25/100;
		if(sc->data[SC_MADNESSCANCEL].timer != -1 && bl->type != BL_PC)	// �}�b�h�l�X�L�����Z���[
			atk += 100;
		if(sc->data[SC_THE_MAGICIAN].timer != -1 && bl->type != BL_PC)
			atk = atk*50/100;
		if(sc->data[SC_THE_DEVIL].timer != -1 && bl->type != BL_PC)
			atk = atk*50/100;
		if(sc->data[SC_THE_SUN].timer != -1 && bl->type != BL_PC)
			atk = atk*80/100;
		if(sc->data[SC_SKE].timer != -1 && bl->type == BL_MOB)		// �G�X�N
			rate += 300;
		if(sc->data[SC__ENERVATION].timer != -1 && bl->type == BL_MOB)	// �}�X�J���[�h �F �G�i�[�x�[�V����
			atk -= atk * (20 + sc->data[SC__ENERVATION].val1 * 10) / 100;
		if(sc->data[SC_SHIELDSPELL_DEF].timer != -1 && sc->data[SC_SHIELDSPELL_DEF].val2 == 2 && bl->type == BL_MOB)	// �V�[���h�X�y��(DEF)
			atk += sc->data[SC_SHIELDSPELL_DEF].val3;
		if(sc->data[SC_SATURDAY_NIGHT_FEVER].timer != -1 && bl->type == BL_MOB)	// �t���C�f�[�i�C�g�t�B�[�o�[
			atk += 100 * sc->data[SC_SATURDAY_NIGHT_FEVER].val1;
		if(sc->data[SC_ODINS_POWER].timer != -1 && bl->type == BL_MOB)	// �I�[�f�B���̗�
			atk += 60 + 10 * sc->data[SC_ODINS_POWER].val1;
		if(rate != 100)	// NPC�����g���ƃG�X�N��{�����Z������
			atk = atk * rate / 100;
	}
	if(atk < 0) atk = 0;
	return atk;
}

/*==========================================
 * �Ώۂ̍���Atk��Ԃ�
 * �߂�͐�����0�ȏ�
 *------------------------------------------
 */
int status_get_atk_(struct block_list *bl)
{
	struct map_session_data *sd = NULL;

	nullpo_retr(0, bl);

	if(bl->type == BL_PC && (sd = (struct map_session_data *)bl)) {
		int atk = sd->watk_;

#ifdef PRE_RENEWAL
		if(sd->sc.data[SC_CURSE].timer != -1)
			atk -= atk*25/100;
#endif
		return atk;
	}
	return 0;
}

/*==========================================
 * �Ώۂ�Atk2��Ԃ�
 * �߂�͐�����0�ȏ�
 *------------------------------------------
 */
int status_get_atk2(struct block_list *bl)
{
	int atk2 = 0;

	nullpo_retr(0, bl);

	if(bl->type == BL_PC && (struct map_session_data *)bl) {
		atk2 = ((struct map_session_data*)bl)->watk2;
	} else if(bl->type == BL_HOM && (struct homun_data *)bl) {
		atk2 = ((struct homun_data *)bl)->atk;
	} else if(bl->type == BL_MERC && (struct merc_data *)bl) {
		atk2 = ((struct merc_data *)bl)->atk2;
	} else if(bl->type == BL_ELEM && (struct elem_data *)bl) {
		atk2 = ((struct elem_data *)bl)->atk2;
	} else {
		struct status_change *sc = status_get_sc(bl);

		if(bl->type == BL_MOB) {
			struct mob_data *md = (struct mob_data *)bl;
			if(md) {
				int guardup_lv = md->guardup_lv;
#ifndef PRE_RENEWAL
				if(battle_config.monster_atk2_to_matk)
					atk2 = mob_db[md->class_].atk1 * 12 / 10;
				else
#endif
					atk2 = mob_db[md->class_].atk2;
				if(guardup_lv > 0)
					atk2 += 1000*guardup_lv;
			}
		} else if(bl->type == BL_PET && (struct pet_data *)bl) {
			atk2 = mob_db[((struct pet_data*)bl)->class_].atk2;
		}
		if(sc) {
			int rate = 100;
#ifdef PRE_RENEWAL
			if(sc->data[SC_IMPOSITIO].timer != -1)
				atk2 += sc->data[SC_IMPOSITIO].val1*5;
			if(sc->data[SC__BLOODYLUST].timer != -1)
				atk2 = atk2*(100+32)/100;
			else if(sc->data[SC_PROVOKE].timer != -1)
				atk2 = atk2*(100+2+3*sc->data[SC_PROVOKE].val1)/100;
			if(sc->data[SC_CURSE].timer != -1)
				atk2 -= atk2*25/100;
			if(sc->data[SC_DRUMBATTLE].timer != -1)
				atk2 += sc->data[SC_DRUMBATTLE].val2;
			if(sc->data[SC_NIBELUNGEN].timer != -1 && (status_get_element(bl)/10) >= 8 )
				atk2 += sc->data[SC_NIBELUNGEN].val2;
#endif
			if(sc->data[SC_STRIPWEAPON].timer != -1)
				atk2 -= atk2*10/100;
#ifdef PRE_RENEWAL
			if(sc->data[SC_CONCENTRATION].timer != -1)	// �R���Z���g���[�V����
				atk2 += atk2*(5*sc->data[SC_CONCENTRATION].val1)/100;
#endif
			if(sc->data[SC_EXPLOSIONSPIRITS].timer != -1 && bl->type != BL_PC)	// NPC�����g��
				rate += 200;
			if(sc->data[SC_DISARM].timer != -1 && bl->type != BL_PC)		// �f�B�X�A�[��
				atk2 -= atk2*25/100;
			if(sc->data[SC_MADNESSCANCEL].timer != -1 && bl->type != BL_PC)	// �}�b�h�l�X�L�����Z���[
				atk2 += 100;
			if(sc->data[SC_THE_MAGICIAN].timer != -1 && bl->type != BL_PC)
				atk2 = atk2*50/100;
			if(sc->data[SC_THE_DEVIL].timer != -1 && bl->type != BL_PC)
				atk2 = atk2*50/100;
			if(sc->data[SC_THE_SUN].timer != -1 && bl->type != BL_PC)
				atk2 = atk2*80/100;
			if(sc->data[SC_SKE].timer != -1 && bl->type == BL_MOB)		// �G�X�N
				rate += 300;
			if(sc->data[SC__ENERVATION].timer != -1 && bl->type == BL_MOB)	// �}�X�J���[�h �F �G�i�[�x�[�V����
				atk2 -= atk2 * (20 + sc->data[SC__ENERVATION].val1 * 10) / 100;
			if(sc->data[SC_SHIELDSPELL_DEF].timer != -1 && sc->data[SC_SHIELDSPELL_DEF].val2 == 2 && bl->type == BL_MOB)	// �V�[���h�X�y��(DEF)
				atk2 += sc->data[SC_SHIELDSPELL_DEF].val3;
			if(sc->data[SC_SATURDAY_NIGHT_FEVER].timer != -1 && bl->type == BL_MOB)	// �t���C�f�[�i�C�g�t�B�[�o�[
				atk2 += 100 * sc->data[SC_SATURDAY_NIGHT_FEVER].val1;
			if(sc->data[SC_ODINS_POWER].timer != -1 && bl->type == BL_MOB)	// �I�[�f�B���̗�
				atk2 += 60 + 10 * sc->data[SC_ODINS_POWER].val1;
			if(rate != 100)	// NPC�����g���ƃG�X�N��{�����Z������
				atk2 = atk2 * rate / 100;
		}
		if(atk2 < 0) atk2 = 0;
	}
	return atk2;
}

/*==========================================
 * �Ώۂ̍���Atk2��Ԃ�
 * �߂�͐�����0�ȏ�
 *------------------------------------------
 */
int status_get_atk_2(struct block_list *bl)
{
	nullpo_retr(0, bl);

	if(bl->type == BL_PC && (struct map_session_data *)bl)
		return ((struct map_session_data*)bl)->watk_2;

	return 0;
}

/*==========================================
 * �Ώۂ�MAtk1��Ԃ�
 * �߂�͐�����0�ȏ�
 *------------------------------------------
 */
int status_get_matk1(struct block_list *bl)
{
	int matk1 = 0;

	nullpo_retr(0, bl);

	if (bl->type == BL_PC) {
		matk1 = ((struct map_session_data *)bl)->matk1;
	} else if (bl->type == BL_HOM) {
#ifdef PRE_RENEWAL
		matk1 = ((struct homun_data *)bl)->matk-((struct homun_data *)bl)->matk/10;
#else
		matk1 = ((struct homun_data *)bl)->matk;
#endif
	} else if (bl->type == BL_MERC) {
		matk1 = ((struct merc_data *)bl)->matk1;
	} else if (bl->type == BL_ELEM) {
		matk1 = ((struct elem_data *)bl)->matk1;
	} else if (!(bl->type & (BL_PET | BL_MOB))) {
		matk1 = 0;
	} else {
		struct status_change *sc = status_get_sc(bl);
		int int_ = status_get_int(bl);

#ifndef PRE_RENEWAL
		if(battle_config.monster_atk2_to_matk && bl->type == BL_MOB)
			matk1 = mob_db[status_get_class(bl)].atk2 * 13 / 10;
		else
#endif
			matk1 = int_+(int_/5)*(int_/5);

		// MOB��max_sp�l��MATK�␳�l�ŏ����鎞
		if(battle_config.mob_take_over_sp == 1) {
			if(bl->type == BL_MOB) {
				int b_class = status_get_class(bl);		// ����maxsp�擾�����̈׃�������
				if(mob_db[b_class].max_sp > 0) {		// 1�ȏ�̎��̂�
					matk1 = matk1 * (mob_db[b_class].max_sp/100);
				}
			}
		}

		if(sc) {
#ifdef PRE_RENEWAL
			if(sc->data[SC_MINDBREAKER].timer != -1)
				matk1 += (matk1*20*sc->data[SC_MINDBREAKER].val1)/100;
#endif
			if(sc->data[SC_STRENGTH].timer != -1)
				matk1 = matk1*50/100;
			if(sc->data[SC_THE_DEVIL].timer != -1)
				matk1 = matk1*50/100;
			if(sc->data[SC_THE_SUN].timer != -1)
				matk1 = matk1*80/100;
			if(sc->data[SC_ODINS_POWER].timer != -1)	// �I�[�f�B���̗�
				matk1 += 60 + 10 * sc->data[SC_ODINS_POWER].val1;
		}
	}
	return matk1;
}

/*==========================================
 * �Ώۂ�MAtk2��Ԃ�
 * �߂�͐�����0�ȏ�
 *------------------------------------------
 */
int status_get_matk2(struct block_list *bl)
{
	int matk2 = 0;

	nullpo_retr(0, bl);

	if (bl->type == BL_PC) {
		matk2 = ((struct map_session_data *)bl)->matk2;
	} else if (bl->type == BL_HOM) {
#ifdef PRE_RENEWAL
		matk2 = ((struct homun_data *)bl)->matk;
#else
		matk2 = ((struct homun_data *)bl)->matk-((struct homun_data *)bl)->matk/10;
#endif
	} else if (bl->type == BL_MERC) {
		matk2 = ((struct merc_data *)bl)->matk2;
	} else if (bl->type == BL_ELEM) {
		matk2 = ((struct elem_data *)bl)->matk2;
	} else if (!(bl->type & (BL_PET | BL_MOB))) {
		matk2 = 0;
	} else {
		struct status_change *sc = status_get_sc(bl);
		int int_ = status_get_int(bl);

#ifndef PRE_RENEWAL
		if(battle_config.monster_atk2_to_matk && bl->type == BL_MOB)
			matk2 = mob_db[status_get_class(bl)].atk2 * 7 / 10;
		else
#endif
			matk2 = int_+(int_/7)*(int_/7);

		// MOB��max_sp�l��MATK�␳�l�ŏ����鎞
		if(battle_config.mob_take_over_sp == 1) {
			if(bl->type == BL_MOB) {
				int b_class = status_get_class(bl);		// ����maxsp�擾�����̈׃�������
				if(mob_db[b_class].max_sp > 0) {		// 1�ȏ�̎��̂�
					matk2 = matk2 * (mob_db[b_class].max_sp/100);
				}
			}
		}

		if(sc) {
#ifdef PRE_RENEWAL
			if(sc->data[SC_MINDBREAKER].timer != -1)
				matk2 += (matk2*20*sc->data[SC_MINDBREAKER].val1)/100;
#endif
			if(sc->data[SC_STRENGTH].timer != -1)
				matk2 = matk2*50/100;
			if(sc->data[SC_THE_DEVIL].timer != -1)
				matk2 = matk2*50/100;
			if(sc->data[SC_THE_SUN].timer != -1)
				matk2 = matk2*80/100;
			if(sc->data[SC_ODINS_POWER].timer != -1)	// �I�[�f�B���̗�
				matk2 += 60 + 10 * sc->data[SC_ODINS_POWER].val1;
		}
	}
	return matk2;
}

/*==========================================
 * �Ώۂ�Def��Ԃ�
 * �߂�͐�����0�ȏ�
 *------------------------------------------
 */
int status_get_def(struct block_list *bl)
{
	struct unit_data *ud= NULL;
	struct status_change *sc;
	int def;

	nullpo_retr(0, bl);

	ud = unit_bl2ud(bl);
	sc = status_get_sc(bl);

	if(bl->type == BL_PC && (struct map_session_data *)bl) {
		def = ((struct map_session_data *)bl)->def;
	} else if(bl->type == BL_MOB && (struct mob_data *)bl) {
		def = mob_db[((struct mob_data *)bl)->class_].def;
	} else if(bl->type == BL_PET && (struct pet_data *)bl) {
		def = mob_db[((struct pet_data *)bl)->class_].def;
	} else if(bl->type == BL_MERC && (struct merc_data *)bl) {
		def = ((struct merc_data *)bl)->def;
	} else if(bl->type == BL_ELEM && (struct elem_data *)bl) {
		def = ((struct elem_data *)bl)->def;
	} else {
		def = 0;
	}

	if(def < 1000000) {
		if(sc) {
			// �L�[�s���O����DEF100
			if(sc->data[SC_KEEPING].timer != -1)
				def *= 2;
			// �v���{�b�N���͌��Z
			if(sc->data[SC__BLOODYLUST].timer != -1 && bl->type != BL_PC)
				def = def * (100 - 55) / 100;
			else if(sc->data[SC_PROVOKE].timer != -1 && bl->type != BL_PC)
				def = def * (100 - 5 - 5 * sc->data[SC_PROVOKE].val1) / 100;
			// �푾�ۂ̋������͉��Z
			if(sc->data[SC_DRUMBATTLE].timer != -1 && bl->type != BL_PC)
				def += sc->data[SC_DRUMBATTLE].val3;
			// �łɂ������Ă��鎞�͌��Z
			if(sc->data[SC_POISON].timer != -1 && bl->type != BL_PC)
				def = def*75/100;
			// �X�g���b�v�V�[���h���͌��Z
			if(sc->data[SC_STRIPSHIELD].timer != -1 && bl->type != BL_PC)
				def = def*85/100;
			// �V�O�i���N���V�X���͌��Z
			if(sc->data[SC_SIGNUMCRUCIS].timer != -1 && bl->type != BL_PC)
				def = def * (100 - sc->data[SC_SIGNUMCRUCIS].val2)/100;
			// �i���̍��׎���PC�ȊODEF��0�ɂȂ�
			if(sc->data[SC_ETERNALCHAOS].timer != -1 && bl->type != BL_PC)
				def = 0;
			// �����A�Ή����͉E�V�t�g
			if(sc->data[SC_FREEZE].timer != -1 || (sc->data[SC_STONE].timer != -1 && sc->data[SC_STONE].val2 == 0))
				def >>= 1;
			// �R���Z���g���[�V�������͌��Z
			if( sc->data[SC_CONCENTRATION].timer != -1 && bl->type != BL_PC)
				def = def * (100 - 5*sc->data[SC_CONCENTRATION].val1) / 100;
			// NPC�f�B�t�F���_�[
			if(sc->data[SC_NPC_DEFENDER].timer != -1 && bl->type != BL_PC)
				def += 100;
			// THE SUN
			if(sc->data[SC_THE_SUN].timer != -1 && bl->type != BL_PC)
				def = def*80/100;
			// �t���C���O
			if(sc->data[SC_FLING].timer != -1 && bl->type != BL_PC)
				def = def * (100 - 5*sc->data[SC_FLING].val1)/100;
			// �G�X�N
			if(sc->data[SC_SKE].timer != -1 && bl->type == BL_MOB)
				def = def/2;
#ifdef PRE_RENEWAL
			// �X�g�[���X�L��
			if(sc->data[SC_STONESKIN].timer != -1 && bl->type != BL_PC)
				def = def * (100 + 20 * sc->data[SC_STONESKIN].val1) / 100;
			// �A���`�}�W�b�N
			if(sc->data[SC_ANTIMAGIC].timer != -1 && bl->type != BL_PC)
				def = def * (100 - 20 * sc->data[SC_ANTIMAGIC].val1) / 100;
#endif
			// �t���X�g�~�X�e�B
			if(sc->data[SC_FROSTMISTY].timer != -1 && bl->type != BL_PC)
				def = def * 90 / 100;
			// �A�i���C�Y
			if(sc->data[SC_ANALYZE].timer != -1 && bl->type != BL_PC)
				def = def * (100 - 14 * sc->data[SC_ANALYZE].val1) / 100;
			// �j���[�g�����o���A�[
			if(sc->data[SC_NEUTRALBARRIER].timer != -1 && bl->type != BL_PC)
				def = def * (110 + 5 * sc->data[SC_NEUTRALBARRIER].val1) / 100;
			// �A�[�X�h���C�u
			if(sc->data[SC_EARTHDRIVE].timer != -1 && bl->type != BL_PC)
				def = def * 85 / 100;
			// �t���C�f�[�i�C�g�t�B�[�o�[
			if(sc->data[SC_SATURDAY_NIGHT_FEVER].timer != -1 && bl->type != BL_PC)
				def = def * (90 - 10 * sc->data[SC_SATURDAY_NIGHT_FEVER].val1) / 100;
			// �I�[�f�B���̗�
			if(sc->data[SC_ODINS_POWER].timer != -1 && bl->type != BL_PC)
				def -= 10 + 10 * sc->data[SC_ODINS_POWER].val1;
		}
		// �r�����͉r�������Z���Ɋ�Â��Č��Z
		if(ud && ud->skilltimer != -1) {
			int def_rate = skill_get_castdef(ud->skillid);
			if(def_rate != 0)
				def = (def * (100 - def_rate))/100;
		}
	}
	if(def < 0) def = 0;
	return def;
}

/*==========================================
 * �Ώۂ�MDef��Ԃ�
 * �߂�͐�����0�ȏ�
 *------------------------------------------
 */
int status_get_mdef(struct block_list *bl)
{
	struct status_change *sc;
	int mdef = 0;

	nullpo_retr(0, bl);

	sc = status_get_sc(bl);
	if(bl->type == BL_PC && (struct map_session_data *)bl)
		mdef = ((struct map_session_data *)bl)->mdef;
	else if(bl->type == BL_MOB && (struct mob_data *)bl)
		mdef = mob_db[((struct mob_data *)bl)->class_].mdef;
	else if(bl->type == BL_HOM && (struct homun_data *)bl)
		mdef = ((struct homun_data *)bl)->mdef;
	else if(bl->type == BL_MERC && (struct merc_data *)bl)
		mdef = ((struct merc_data *)bl)->mdef;
	else if(bl->type == BL_ELEM && (struct elem_data *)bl)
		mdef = ((struct elem_data *)bl)->mdef;
	else if(bl->type == BL_PET && (struct pet_data *)bl)
		mdef = mob_db[((struct pet_data *)bl)->class_].mdef;

	if(mdef < 1000000) {
		if(sc) {
			// �o���A�[��Ԏ���MDEF100
			if(sc->data[SC_BARRIER].timer != -1)
				mdef += 100;
			// �����A�Ή�����1.25�{
			if(sc->data[SC_FREEZE].timer != -1 || (sc->data[SC_STONE].timer != -1 && sc->data[SC_STONE].val2 == 0))
				mdef = mdef*125/100;
#ifdef PRE_RENEWAL
			// �X�g�[���X�L��
			if(sc->data[SC_STONESKIN].timer != -1 && bl->type != BL_PC)
				mdef = mdef * (100 - 20 * sc->data[SC_STONESKIN].val1) / 100;
			// �A���`�}�W�b�N
			if(sc->data[SC_ANTIMAGIC].timer != -1 && bl->type != BL_PC)
				mdef = mdef * (100 + 20 * sc->data[SC_ANTIMAGIC].val1) / 100;
#endif
			// �A�i���C�Y
			if(sc->data[SC_ANALYZE].timer != -1 && bl->type != BL_PC)
				mdef = mdef * (100 - 14 * sc->data[SC_ANALYZE].val1) / 100;
			// �j���[�g�����o���A�[
			if(sc->data[SC_NEUTRALBARRIER].timer != -1 && bl->type != BL_PC)
				mdef = mdef * (110 + 5 * sc->data[SC_NEUTRALBARRIER].val1) / 100;
			// �I�[�f�B���̗�
			if(sc->data[SC_ODINS_POWER].timer != -1 && bl->type != BL_PC)
				mdef -= 10 + 10 * sc->data[SC_ODINS_POWER].val1;
			// �_�� -��-
			if(sc->data[SC_GENTLETOUCH_CHANGE].timer != -1)
				mdef -= sc->data[SC_GENTLETOUCH_CHANGE].val3;
		}
	}
	if(mdef < 0) mdef = 0;
	return mdef;
}

/*==========================================
 * �Ώۂ�Def2��Ԃ�
 * �߂�͐�����1�ȏ�
 *------------------------------------------
 */
int status_get_def2(struct block_list *bl)
{
	struct status_change *sc;
	int def2 = 1;

	nullpo_retr(1, bl);

	sc = status_get_sc(bl);
	if(bl->type == BL_PC && (struct map_session_data *)bl)
		def2 = ((struct map_session_data *)bl)->def2;
	else if(bl->type == BL_MOB && (struct mob_data *)bl)
#ifdef PRE_RENEWAL
		def2 = mob_db[((struct mob_data *)bl)->class_].vit;
#else
		def2 = (int)(mob_db[((struct mob_data *)bl)->class_].vit/(float)2 + mob_db[((struct mob_data *)bl)->class_].lv/(float)2);
#endif
	else if(bl->type == BL_PET && (struct pet_data *)bl)
		def2 = mob_db[((struct pet_data *)bl)->class_].vit;
	else if(bl->type == BL_HOM && (struct homun_data *)bl)
		def2 = ((struct homun_data *)bl)->def;
	else if(bl->type == BL_MERC && (struct merc_data *)bl)
		def2 = ((struct merc_data *)bl)->vit;
	else if(bl->type == BL_ELEM && (struct elem_data *)bl)
		def2 = ((struct elem_data *)bl)->vit;

	if(sc) {
		if(sc->data[SC_ANGELUS].timer != -1 && bl->type != BL_PC) {
#ifdef PRE_RENEWAL
			def2 = def2*(110+5*sc->data[SC_ANGELUS].val1)/100;
#else
			def2 = def2*(100+5*sc->data[SC_ANGELUS].val1)/100;
#endif
		}
		if(sc->data[SC__BLOODYLUST].timer != -1 && bl->type != BL_PC)
			def2 = def2 * (100 - 55) / 100;
		else if(sc->data[SC_PROVOKE].timer != -1 && bl->type != BL_PC)
			def2 = def2 * (100 - 5 - 5 * sc->data[SC_PROVOKE].val1) / 100;
		if(sc->data[SC_POISON].timer != -1 && bl->type != BL_PC)
			def2 = def2*75/100;
		// �R���Z���g���[�V�������͌��Z
		if( sc->data[SC_CONCENTRATION].timer != -1 && bl->type != BL_PC)
			def2 = def2*(100 - 5*sc->data[SC_CONCENTRATION].val1)/100;
		// �W���C���g�r�[�g���Ȃ猸�Z
		if(sc->data[SC_JOINTBEAT].timer != -1) {
			if(sc->data[SC_JOINTBEAT].val4 == 3)	// ��
				def2 -= def2*50/100;
			if(sc->data[SC_JOINTBEAT].val4 == 4)	// ��
				def2 -= def2*25/100;
		}
		// �i���̍��׎���DEF2��0�ɂȂ�
		if(sc->data[SC_ETERNALCHAOS].timer != -1)
			def2 = 0;
		// THE SUN
		if(sc->data[SC_THE_SUN].timer != -1 && bl->type != BL_PC)
			def2 = def2*80/100;
		// �G�X�J
		if(sc->data[SC_SKA].timer != -1 && bl->type == BL_MOB)
			def2 += 90;
		// �V�[���h�X�y��
		if(sc->data[SC_SHIELDSPELL_REF].timer != -1 && sc->data[SC_SHIELDSPELL_REF].val2 == 2 && bl->type == BL_MOB)
			def2 += sc->data[SC_SHIELDSPELL_REF].val2;
		// �v���X�e�B�[�W
		if(sc->data[SC_PRESTIGE].timer != -1 && bl->type == BL_MOB)
			def2 += sc->data[SC_PRESTIGE].val2;
	}
	if(def2 < 1) def2 = 1;
	return def2;
}

/*==========================================
 * �Ώۂ�MDef2��Ԃ�
 * �߂�͐�����0�ȏ�
 *------------------------------------------
 */
int status_get_mdef2(struct block_list *bl)
{
	struct status_change *sc;
	int mdef2 = 0;

	nullpo_retr(0, bl);

	sc = status_get_sc(bl);

	if(bl->type == BL_MOB && (struct mob_data *)bl)
#ifdef PRE_RENEWAL
		mdef2 = mob_db[((struct mob_data *)bl)->class_].int_ + (mob_db[((struct mob_data *)bl)->class_].vit>>1);
#else
		mdef2 = (int)(mob_db[((struct mob_data *)bl)->class_].int_/(float)4 + mob_db[((struct mob_data *)bl)->class_].lv/(float)4);
#endif
	else if(bl->type == BL_PC && (struct map_session_data *)bl)
#ifdef PRE_RENEWAL
		mdef2 = ((struct map_session_data *)bl)->mdef2 + (((struct map_session_data *)bl)->paramc[2]>>1);
#else
		mdef2 = ((struct map_session_data *)bl)->mdef2;
#endif
	else if(bl->type == BL_PET && (struct pet_data *)bl)
		mdef2 = mob_db[((struct pet_data *)bl)->class_].int_ + (mob_db[((struct pet_data *)bl)->class_].vit>>1);
	else if (bl->type == BL_HOM && (struct homun_data *)bl)
		mdef2 = ((struct homun_data *)bl)->mdef;
	else if (bl->type == BL_MERC && (struct merc_data *)bl)
		mdef2 = ((struct merc_data *)bl)->int_ + (((struct merc_data *)bl)->vit>>1);
	else if (bl->type == BL_ELEM && (struct elem_data *)bl)
		mdef2 = ((struct elem_data *)bl)->int_ + (((struct elem_data *)bl)->vit>>1);

	if(sc) {
		// �G�X�J
		if(sc->data[SC_SKA].timer != -1 && bl->type == BL_MOB)
			mdef2 = 90;
		// �}�C���h�u���C�J�[
		if(sc->data[SC_MINDBREAKER].timer != -1 && bl->type != BL_PC)
			mdef2 -= (mdef2*12*sc->data[SC_MINDBREAKER].val1)/100;
	}
	return mdef2;
}

/*==========================================
 * �Ώۂ�Speed(�ړ����x)��Ԃ�
 * �߂�͐�����1�ȏ�
 * Speed�͏������ق����ړ����x������
 *------------------------------------------
 */
int status_get_speed(struct block_list *bl)
{
	int speed = 1000;

	nullpo_retr(1000, bl);

	if(bl->type == BL_PC && (struct map_session_data *)bl) {
		speed = ((struct map_session_data *)bl)->speed;
	} else if(bl->type == BL_HOM && (struct homun_data *)bl) {
		if(battle_config.homun_speed_is_same_as_pc)
			speed = ((struct homun_data *)bl)->msd->speed;
		else
			speed = ((struct homun_data *)bl)->speed;
	} else if(bl->type == BL_MERC && (struct merc_data *)bl) {
		if(battle_config.merc_speed_is_same_as_pc)
			speed = ((struct merc_data *)bl)->msd->speed;
		else
			speed = ((struct merc_data *)bl)->speed;
	} else if(bl->type == BL_ELEM && (struct elem_data *)bl) {
		speed = ((struct elem_data *)bl)->speed;
	} else {
		int bonus_rate;
		int haste_val  = 0;
		int slow_val   = 0;
		char defender_flag = 0;
		char walkspeed_flag = 0;
		struct status_change *sc = status_get_sc(bl);

		if(bl->type == BL_MOB && (struct mob_data *)bl)
			speed = ((struct mob_data *)bl)->speed;
		else if(bl->type == BL_PET && (struct pet_data *)bl)
			speed = ((struct pet_data *)bl)->speed;

		/* speed���ω�����X�e�[�^�X�̌v�Z */
		if(sc) {
			/* speed����������X�e�[�^�X�̌v�Z */

			// �����͈ړ����x�Œ�
			if(sc->data[SC_STEELBODY].timer != -1)
				return 200;

			// ���x����
			if(sc->data[SC_DECREASEAGI].timer != -1)
				slow_val = 25;

			// �N�@�O�}�C�A
			if(sc->data[SC_QUAGMIRE].timer != -1) {
				if(slow_val < 50)
					slow_val = 50;
			}

			// ����Y��Ȃ���
			if(sc->data[SC_DONTFORGETME].timer != -1) {
				if(slow_val < sc->data[SC_DONTFORGETME].val2)
					slow_val = sc->data[SC_DONTFORGETME].val2;
			}

			// ��
			if(sc->data[SC_CURSE].timer != -1) {
				if(slow_val < 300)
					slow_val = 300;
			}

			// �}�[�V���I�u�A�r�X
			if(sc->data[SC_MARSHOFABYSS].timer != -1) {
				if(slow_val < sc->data[SC_MARSHOFABYSS].val2)
					slow_val = sc->data[SC_MARSHOFABYSS].val2;
			}

			// �n���V�l�[�V�����E�H�[�N(�y�i���e�B)
			if(sc->data[SC_HALLUCINATIONWALK2].timer != -1) {
				if(slow_val < 100)
					slow_val = 100;
			}

			// �W���C���g�r�[�g
			if(sc->data[SC_JOINTBEAT].timer != -1) {
				int penalty = 0;
				switch (sc->data[SC_JOINTBEAT].val4) {
				case 0:	// ����
					penalty = 50;
					break;
				case 2:	// �G
					penalty = 30;
					break;
				}
				if(slow_val < penalty)
					slow_val = penalty;
			}

			// �K�g�����O�t�B�[�o�[
			if(sc->data[SC_GATLINGFEVER].timer != -1) {
				if(slow_val < 100)
					slow_val = 100;
			}

			// �G�X�E
			if(sc->data[SC_SWOO].timer != -1) {
				if(slow_val < 300)
					slow_val = 300;
			}

			// �p�����C�Y
			if(sc->data[SC_PARALIZE].timer != -1) {
				if(slow_val < 100)
					slow_val = 100;
			}

			// �t���X�g�~�X�e�B
			if(sc->data[SC_FROSTMISTY].timer != -1) {
				if(slow_val < 50)
					slow_val = 50;
			}

			// �}�X�J���[�h �F ���C�W�[�l�X
			if(sc->data[SC__LAZINESS].timer != -1) {
				if(slow_val < 10)
					slow_val = 10;
			}

			// �}�X�J���[�h �F �O���[�~�[
			if(sc->data[SC__GROOMY].timer != -1) {
				int penalty = 5 + 5 * sc->data[SC__GROOMY].val1;
				if(slow_val < penalty)
					slow_val = penalty;
			}

			// ���������e
			if(sc->data[SC_MELON_BOMB].timer != -1) {
				int penalty = sc->data[SC_MELON_BOMB].val1;
				if(slow_val < penalty)
					slow_val = penalty;
			}

			// �O���r�e�[�V�����t�B�[���h
			if(battle_config.enemy_gravitation_type && sc->data[SC_GRAVITATION].timer != -1) {
				int penalty = sc->data[SC_GRAVITATION].val1 * 5;
				if(slow_val < penalty)
					slow_val = penalty;
			}

			/* speed����������X�e�[�^�X�v�Z */

			// ���x����
			if(sc->data[SC_SPEEDUP1].timer != -1)
				haste_val = 50;

			// ���x����
			if(sc->data[SC_INCREASEAGI].timer != -1) {
				if(haste_val < 25)
					haste_val = 25;
			}

			// �E�C���h�E�H�[�N
			if(sc->data[SC_WINDWALK].timer != -1) {
				int bonus = 2 * sc->data[SC_WINDWALK].val1;
				if(haste_val < bonus)
					haste_val = bonus;
			}

			// �C���r���V�u��
			if(sc->data[SC_INVINCIBLE].timer != -1 && sc->data[SC_INVINCIBLEOFF].timer == -1) {
				if(haste_val < 75)
					haste_val = 75;
			}

			/* ���̑� */

			// �f�B�t�F���_�[
			if(sc->data[SC_DEFENDER].timer != -1)
				defender_flag = 1;

			// WALKSPEED
			if(sc->data[SC_WALKSPEED].timer != -1 && sc->data[SC_WALKSPEED].val1 > 0)
				walkspeed_flag = 1;
		}

		/* bonus_rate�̍Œ�l��ݒ� */
		bonus_rate = slow_val - haste_val;
		if(bonus_rate < -60)
			bonus_rate = -60;

		/* speed�̍ŏI�v�Z */
		if(bonus_rate != 0)	// bonus_rate
			speed = speed * (bonus_rate+100) / 100;
		if(defender_flag && speed < 200)	// �f�B�t�F���_�[
			speed = 200;
		if(walkspeed_flag)	// �X�N���v�g�p�X�e�[�^�X
			speed = speed * 100 / sc->data[SC_WALKSPEED].val1;

		/* �Œ�l�A�ő�l��ݒ肷�� */
		if(speed < MIN_WALK_SPEED)
			speed = MIN_WALK_SPEED;
		if(speed > MAX_WALK_SPEED)
			speed = MAX_WALK_SPEED;
	}

	return speed;
}

/*==========================================
 * �Ώۂ�aDelay(�U�����f�B���C)��Ԃ�
 * aDelay�͏������ق����U�����x������
 *------------------------------------------
 */
int status_get_adelay(struct block_list *bl)
{
	int adelay = 4000;

	nullpo_retr(4000, bl);

	if(bl->type == BL_PC && (struct map_session_data *)bl) {
		adelay = (((struct map_session_data *)bl)->aspd);
	} else {
		double calc_adelay = 0;
		int haste_val1     = 0;
		int haste_val2     = 0;
		int slow_val       = 0;
		int bonus_rate     = 0;
		int ferver_bonus   = 0;
		int tmp            = 0;
		char defender_flag = 0;
		struct status_change *sc = status_get_sc(bl);

		if(bl->type == BL_MOB && (struct mob_data *)bl) {
			int guardup_lv = ((struct mob_data*)bl)->guardup_lv;
			if(mob_db[((struct mob_data *)bl)->class_].adelay < mob_db[((struct mob_data *)bl)->class_].amotion)
				calc_adelay = mob_db[((struct mob_data *)bl)->class_].amotion;
			else
				calc_adelay = mob_db[((struct mob_data *)bl)->class_].adelay;

			if(guardup_lv > 0)
				bonus_rate -= 5 + 5*guardup_lv;
		} else if(bl->type == BL_PET && (struct pet_data *)bl) {
			if(mob_db[((struct pet_data *)bl)->class_].adelay < mob_db[((struct pet_data *)bl)->class_].amotion)
				calc_adelay = mob_db[((struct pet_data *)bl)->class_].amotion;
			else
				calc_adelay = mob_db[((struct pet_data *)bl)->class_].adelay;
		} else if(bl->type == BL_HOM && (struct homun_data *)bl) {
			calc_adelay = (((struct homun_data *)bl)->aspd);
		} else if(bl->type == BL_MERC && (struct merc_data *)bl) {
			calc_adelay = ((struct merc_data *)bl)->adelay;
		} else if(bl->type == BL_ELEM && (struct elem_data *)bl) {
			calc_adelay = ((struct elem_data *)bl)->adelay;
		}

		/* amotion���ω������Ԉُ�̌v�Z */
		if(sc) {

			/* amotion����������X�e�[�^�X�̌v�Z */

			// ����Y��Ȃ���
			if(sc->data[SC_DONTFORGETME].timer != -1) {
				slow_val = sc->data[SC_DONTFORGETME].val1;
			}

			// ����
			if(sc->data[SC_STEELBODY].timer != -1) {
				if(slow_val < 25)
					slow_val = 25;
			}

			// �W���C���g�r�[�g
			if(sc->data[SC_JOINTBEAT].timer != -1) {
				switch (sc->data[SC_JOINTBEAT].val4) {
					case 1:		// ���
						if(slow_val < 25)
							slow_val = 25;
						break;
					case 2:		// �G
						if(slow_val < 10)
							slow_val = 10;
						break;
				}
			}

			// �O���r�e�[�V�����t�B�[���h
			if(sc->data[SC_GRAVITATION].timer != -1) {
				int penalty = sc->data[SC_GRAVITATION].val1*5;
				if(slow_val < penalty)
					slow_val = penalty;
			}

			// �t���X�g�~�X�e�B
			if(sc->data[SC_FROSTMISTY].timer != -1) {
				if(slow_val < 15)
					slow_val = 15;
			}

			// �{�f�B�y�C���e�B���O
			if(sc->data[SC__BODYPAINT].timer != -1) {
				if(slow_val < 25)
					slow_val = 25;
			}

			// �C���r�W�r���e�B
			if(sc->data[SC__INVISIBILITY].timer != -1) {
				int penalty = 50 - 10 * sc->data[SC__INVISIBILITY].val1;
				if(slow_val < penalty)
					slow_val = penalty;
			}

			// �}�X�J���[�h �F �O���[�~�[
			if(sc->data[SC__GROOMY].timer != -1) {
				int penalty = 20 + 10 * sc->data[SC__GROOMY].val1;
				if(slow_val < penalty)
					slow_val = penalty;
			}

			// �A�[�X�h���C�u
			if(sc->data[SC_EARTHDRIVE].timer != -1) {
				if(slow_val < 25)
					slow_val = 25;
			}

			// �������R���[
			if(sc->data[SC_GLOOMYDAY].timer != -1) {
				int penalty = 3 * sc->data[SC_GLOOMYDAY].val1;
				if(slow_val < penalty)
					slow_val = penalty;
			}

			// ���������e
			if(sc->data[SC_MELON_BOMB].timer != -1) {
				int penalty = sc->data[SC_MELON_BOMB].val1;
				if(slow_val < penalty)
					slow_val = penalty;
			}

			/* amotion����������X�e�[�^�X�̌v�Z1 */

			// �����|�[�V����
			if(sc->data[tmp = SC_SPEEDPOTION2].timer != -1 || sc->data[tmp = SC_SPEEDPOTION1].timer != -1 || sc->data[tmp = SC_SPEEDPOTION0].timer != -1)
				haste_val1 = sc->data[tmp].val2;

			/* amotion����������X�e�[�^�X�̌v�Z2 */

			// �c�[�n���h�N�B�b�P��
			if(sc->data[SC_TWOHANDQUICKEN].timer != -1)
				haste_val2 = sc->data[SC_TWOHANDQUICKEN].val2;

			// �X�s�A�N�B�b�P��
			if(sc->data[SC_SPEARQUICKEN].timer != -1) {
				if(haste_val2 < sc->data[SC_SPEARQUICKEN].val2)
					haste_val2 = sc->data[SC_SPEARQUICKEN].val2;
			}

			// �����n���h�N�B�b�P��
			if(sc->data[SC_ONEHAND].timer != -1) {
				if(haste_val2 < 30)
					haste_val2 = 30;
			}

			// �A�h���i�������b�V��
			if(sc->data[SC_ADRENALINE].timer != -1) {
				int bonus;
				if(sc->data[SC_ADRENALINE].val2 || !battle_config.party_skill_penalty)
					bonus = 30;
				else
					bonus = 25;
				if(haste_val2 < bonus)
					haste_val2 = bonus;
			}

			// �A�h���i�������b�V��2
			if(sc->data[SC_ADRENALINE2].timer != -1) {
				int bonus;
				if(sc->data[SC_ADRENALINE2].val2 || !battle_config.party_skill_penalty)
					bonus = 30;
				else
					bonus = 25;
				if(haste_val2 < bonus)
					haste_val2 = bonus;
			}

			// �[�z�̃A�T�V���N���X
			if(sc->data[SC_ASSNCROS].timer != -1) {
				int bonus = sc->data[SC_ASSNCROS].val2;
				if(haste_val2 < bonus)
					haste_val2 = bonus;
			}

			// �K�g�����O�t�B�[�o�[
			if(sc->data[SC_GATLINGFEVER].timer != -1) {
				ferver_bonus = sc->data[SC_GATLINGFEVER].val1*2;
				if(haste_val2 < ferver_bonus)
					haste_val2 = ferver_bonus;
			}

			// �}�b�h�l�X�L�����Z���[
			if(sc->data[SC_MADNESSCANCEL].timer != -1) {
				int bonus = 20+ferver_bonus;
				if(haste_val2 < bonus)
					haste_val2 = bonus;
			}

			// �_�� -��-
			if(sc->data[SC_GENTLETOUCH_CHANGE].timer != -1) {
				int bonus = sc->data[SC_GENTLETOUCH_CHANGE].val4;
				if(haste_val2 < bonus)
					haste_val2 = bonus;
			}

			/* ���̑� */

			// �f�B�t�F���_�[
			if(sc->data[SC_DEFENDER].timer != -1)
				defender_flag = 1;
		}

		/* slow_val��haste_val1��haste_val2�����Z���� */
		bonus_rate = slow_val - haste_val1 - haste_val2;

		/* bonus_rate�̌v�Z */
		if(bonus_rate != 0)
			calc_adelay = calc_adelay * (bonus_rate+100) / 100;

		/* �f�B�t�F���_�[ */
		if(defender_flag)
			calc_adelay += sc->data[SC_DEFENDER].val3;

		/* �����؂�グ */
		adelay = (int)ceil(calc_adelay);

		/* �Œ�l�̐ݒ� */
		switch(bl->type)
		{
			case BL_MOB:
			case BL_PET:
				if(adelay < (battle_config.monster_max_aspd>>1) )
					adelay = battle_config.monster_max_aspd>>1;
				break;
			case BL_HOM:
				if(adelay < battle_config.homun_max_aspd)
					adelay = battle_config.homun_max_aspd;
				break;
			case BL_MERC:
				if(adelay < (battle_config.merc_max_aspd>>1) )
					adelay = battle_config.merc_max_aspd>>1;
			case BL_ELEM:
				if(adelay < (battle_config.elem_max_aspd>>1) )
					adelay = battle_config.elem_max_aspd>>1;
				break;
		}
	}

	return adelay;
}

/*==========================================
 * �Ώۂ�amotion��Ԃ�
 *------------------------------------------
 */
int status_get_amotion(struct block_list *bl)
{
	int amotion = 2000;

	nullpo_retr(2000, bl);

	if(bl->type == BL_PC && (struct map_session_data *)bl) {
		amotion = ((struct map_session_data *)bl)->amotion;
	} else {
		double calc_amotion = 0;
		int haste_val1      = 0;
		int haste_val2      = 0;
		int slow_val        = 0;
		int bonus_rate      = 0;
		int ferver_bonus    = 0;
		int tmp             = 0;
		char defender_flag  = 0;
		struct status_change *sc = status_get_sc(bl);

		if(bl->type == BL_MOB && (struct mob_data *)bl) {
			int guardup_lv = ((struct mob_data*)bl)->guardup_lv;
			calc_amotion = mob_db[((struct mob_data *)bl)->class_].amotion;
			if(guardup_lv > 0)
				bonus_rate -= 5 + 5*guardup_lv;
		} else if(bl->type == BL_PET && (struct pet_data *)bl) {
			calc_amotion = mob_db[((struct pet_data *)bl)->class_].amotion;
		} else if(bl->type == BL_HOM && (struct homun_data *)bl && ((struct homun_data *)bl)->msd) {
			calc_amotion = ((struct homun_data *)bl)->aspd;
		} else if(bl->type == BL_MERC && (struct merc_data *)bl && ((struct merc_data *)bl)->msd) {
			calc_amotion = ((struct merc_data *)bl)->amotion;
		} else if(bl->type == BL_ELEM && (struct elem_data *)bl && ((struct elem_data *)bl)->msd) {
			calc_amotion = ((struct elem_data *)bl)->amotion;
		}

		/* amotion���ω������Ԉُ�̌v�Z */
		if(sc) {

			/* amotion����������X�e�[�^�X�̌v�Z */

			// ����Y��Ȃ���
			if(sc->data[SC_DONTFORGETME].timer != -1) {
				slow_val = sc->data[SC_DONTFORGETME].val1;
			}

			// ����
			if(sc->data[SC_STEELBODY].timer != -1) {
				if(slow_val < 25)
					slow_val = 25;
			}

			// �W���C���g�r�[�g
			if(sc->data[SC_JOINTBEAT].timer != -1) {
				switch (sc->data[SC_JOINTBEAT].val4) {
					case 1:		// ���
						if(slow_val < 25)
							slow_val = 25;
						break;
					case 2:		// �G
						if(slow_val < 10)
							slow_val = 10;
						break;
				}
			}

			// �O���r�e�[�V�����t�B�[���h
			if(sc->data[SC_GRAVITATION].timer != -1) {
				int penalty = sc->data[SC_GRAVITATION].val1*5;
				if(slow_val < penalty)
					slow_val = penalty;
			}

			// �t���X�g�~�X�e�B
			if(sc->data[SC_FROSTMISTY].timer != -1) {
				if(slow_val < 15)
					slow_val = 15;
			}

			// �{�f�B�y�C���e�B���O
			if(sc->data[SC__BODYPAINT].timer != -1) {
				if(slow_val < 25)
					slow_val = 25;
			}

			// �C���r�W�r���e�B
			if(sc->data[SC__INVISIBILITY].timer != -1) {
				int penalty = 50 - 10 * sc->data[SC__INVISIBILITY].val1;
				if(slow_val < penalty)
					slow_val = penalty;
			}

			// �}�X�J���[�h �F �O���[�~�[
			if(sc->data[SC__GROOMY].timer != -1) {
				int penalty = 20 + 10 * sc->data[SC__GROOMY].val1;
				if(slow_val < penalty)
					slow_val = penalty;
			}

			// �A�[�X�h���C�u
			if(sc->data[SC_EARTHDRIVE].timer != -1) {
				if(slow_val < 25)
					slow_val = 25;
			}

			// �������R���[
			if(sc->data[SC_GLOOMYDAY].timer != -1) {
				int penalty = 3 * sc->data[SC_GLOOMYDAY].val1;
				if(slow_val < penalty)
					slow_val = penalty;
			}

			// ���������e
			if(sc->data[SC_MELON_BOMB].timer != -1) {
				int penalty = sc->data[SC_MELON_BOMB].val1;
				if(slow_val < penalty)
					slow_val = penalty;
			}

			/* amotion����������X�e�[�^�X�̌v�Z1 */

			// �����|�[�V����
			if(sc->data[tmp = SC_SPEEDPOTION2].timer != -1 || sc->data[tmp = SC_SPEEDPOTION1].timer != -1 || sc->data[tmp = SC_SPEEDPOTION0].timer != -1)
				haste_val1 = sc->data[tmp].val2;

			/* amotion����������X�e�[�^�X�̌v�Z2 */

			// �c�[�n���h�N�B�b�P��
			if(sc->data[SC_TWOHANDQUICKEN].timer != -1)
				haste_val2 = sc->data[SC_TWOHANDQUICKEN].val2;

			// �X�s�A�N�B�b�P��
			if(sc->data[SC_SPEARQUICKEN].timer != -1) {
				if(haste_val2 < sc->data[SC_SPEARQUICKEN].val2)
					haste_val2 = sc->data[SC_SPEARQUICKEN].val2;
			}

			// �����n���h�N�B�b�P��
			if(sc->data[SC_ONEHAND].timer != -1) {
				if(haste_val2 < 30)
					haste_val2 = 30;
			}

			// �A�h���i�������b�V��
			if(sc->data[SC_ADRENALINE].timer != -1) {
				int bonus;
				if(sc->data[SC_ADRENALINE].val2 || !battle_config.party_skill_penalty)
					bonus = 30;
				else
					bonus = 25;
				if(haste_val2 < bonus)
					haste_val2 = bonus;
			}

			// �A�h���i�������b�V��2
			if(sc->data[SC_ADRENALINE2].timer != -1) {
				int bonus;
				if(sc->data[SC_ADRENALINE2].val2 || !battle_config.party_skill_penalty)
					bonus = 30;
				else
					bonus = 25;
				if(haste_val2 < bonus)
					haste_val2 = bonus;
			}

			// �[�z�̃A�T�V���N���X
			if(sc->data[SC_ASSNCROS].timer != -1) {
				int bonus = sc->data[SC_ASSNCROS].val2;
				if(haste_val2 < bonus)
					haste_val2 = bonus;
			}

			// �K�g�����O�t�B�[�o�[
			if(sc->data[SC_GATLINGFEVER].timer != -1) {
				ferver_bonus = sc->data[SC_GATLINGFEVER].val1*2;
				if(haste_val2 < ferver_bonus)
					haste_val2 = ferver_bonus;
			}

			// �}�b�h�l�X�L�����Z���[
			if(sc->data[SC_MADNESSCANCEL].timer != -1) {
				int bonus = 20+ferver_bonus;
				if(haste_val2 < bonus)
					haste_val2 = bonus;
			}

			// �_�� -��-
			if(sc->data[SC_GENTLETOUCH_CHANGE].timer != -1) {
				int bonus = sc->data[SC_GENTLETOUCH_CHANGE].val4;
				if(haste_val2 < bonus)
					haste_val2 = bonus;
			}

			/* ���̑� */

			// �f�B�t�F���_�[
			if(sc->data[SC_DEFENDER].timer != -1)
				defender_flag = 1;
		}

		/* slow_val��haste_val1��haste_val2�����Z���� */
		bonus_rate = slow_val - haste_val1 - haste_val2;

		/* bonus_rate�̌v�Z */
		if(bonus_rate != 0)
			calc_amotion = calc_amotion * (bonus_rate+100) / 100;

		/* �f�B�t�F���_�[ */
		if(defender_flag)
			calc_amotion += sc->data[SC_DEFENDER].val3;

		/* �����؂�グ */
		amotion = (int)ceil(calc_amotion);

		/* �Œ�l�̐ݒ� */
		switch(bl->type)
		{
			case BL_MOB:
			case BL_PET:
				if(amotion < battle_config.monster_max_aspd)
					amotion = battle_config.monster_max_aspd;
				break;
			case BL_HOM:
				if(amotion < battle_config.homun_max_aspd)
					amotion = battle_config.homun_max_aspd;
				break;
			case BL_MERC:
				if(amotion < battle_config.merc_max_aspd)
					amotion = battle_config.merc_max_aspd;
				break;
			case BL_ELEM:
				if(amotion < battle_config.elem_max_aspd)
					amotion = battle_config.elem_max_aspd;
				break;
		}
	}

	return amotion;
}

/*==========================================
 * �Ώۂ�dmotion��Ԃ�
 *------------------------------------------
 */
int status_get_dmotion(struct block_list *bl)
{
	int dmotion = 2000;

	nullpo_retr(2000, bl);

	if(bl->type == BL_MOB && (struct mob_data *)bl) {
		dmotion = mob_db[((struct mob_data *)bl)->class_].dmotion;
		if(battle_config.monster_damage_delay_rate != 100)
			dmotion = dmotion*battle_config.monster_damage_delay_rate/100;
	}
	else if(bl->type == BL_PC) {
		struct map_session_data *sd = (struct map_session_data *)bl;
		if(sd) {
			if((sd->sc.data[SC_ENDURE].timer != -1 || sd->special_state.infinite_endure) && !map[sd->bl.m].flag.gvg) {
				dmotion = 0;
			} else {
				dmotion = sd->dmotion;
				if(battle_config.pc_damage_delay_rate != 100)
					dmotion = dmotion*battle_config.pc_damage_delay_rate/100;
			}
		}
	}
	else if(bl->type == BL_HOM && (struct homun_data *)bl && ((struct homun_data *)bl)->msd) {
		dmotion = 800 - ((struct homun_data *)bl)->status.agi*4;
		if(dmotion < 400)
			dmotion = 400;
	}
	else if(bl->type == BL_MERC && (struct merc_data *)bl && ((struct merc_data *)bl)->msd) {
		dmotion = ((struct merc_data *)bl)->dmotion;
	}
	else if(bl->type == BL_ELEM && (struct elem_data *)bl && ((struct elem_data *)bl)->msd) {
		dmotion = ((struct elem_data *)bl)->dmotion;
	}
	else if(bl->type == BL_PET && (struct pet_data *)bl) {
		dmotion = mob_db[((struct pet_data *)bl)->class_].dmotion;
	}

	return dmotion;
}

/*==========================================
 * �Ώۂ̑�����Ԃ�
 *------------------------------------------
 */
int status_get_element(struct block_list *bl)
{
	int ret = 20;
	struct status_change *sc;

	nullpo_retr(ret, bl);

	sc = status_get_sc(bl);
	if(sc) {
		if(sc->data[SC_BENEDICTIO].timer != -1)		// ���̍~��
			ret = 20 + ELE_HOLY;
		if(sc->data[SC_ELEMENTWATER].timer != -1)	// ��
			ret = 20*sc->data[SC_ELEMENTWATER].val1 + ELE_WATER;
		if(sc->data[SC_ELEMENTGROUND].timer != -1)	// �y
			ret = 20*sc->data[SC_ELEMENTGROUND].val1 + ELE_EARTH;
		if(sc->data[SC_ELEMENTFIRE].timer != -1)		// ��
			ret = 20*sc->data[SC_ELEMENTFIRE].val1 + ELE_FIRE;
		if(sc->data[SC_ELEMENTWIND].timer != -1)		// ��
			ret = 20*sc->data[SC_ELEMENTWIND].val1 + ELE_WIND;
		if(sc->data[SC_ELEMENTPOISON].timer != -1)	// ��
			ret = 20*sc->data[SC_ELEMENTPOISON].val1 + ELE_POISON;
		if(sc->data[SC_ELEMENTHOLY].timer != -1)		// ��
			ret = 20*sc->data[SC_ELEMENTHOLY].val1 + ELE_HOLY;
		if(sc->data[SC_ELEMENTDARK].timer != -1)		// ��
			ret = 20*sc->data[SC_ELEMENTDARK].val1 + ELE_DARK;
		if(sc->data[SC_ELEMENTELEKINESIS].timer != -1)	// �O
			ret = 20*sc->data[SC_ELEMENTELEKINESIS].val1 + ELE_GHOST;
		if(sc->data[SC_ELEMENTUNDEAD].timer != -1)	// �s��
			ret = 20*sc->data[SC_ELEMENTUNDEAD].val1 + ELE_UNDEAD;
		if(sc->data[SC_FREEZE].timer != -1)		// ����
			ret = 20 + ELE_WATER;
		if(sc->data[SC_STONE].timer != -1 && sc->data[SC_STONE].val2 == 0)
			ret = 20 + ELE_EARTH;
		if(sc->data[SC_SHAPESHIFT].timer != -1)		// �V�F�C�v�V�t�g
			ret = 20 + sc->data[SC_SHAPESHIFT].val2;

		if(ret != 20)
			return ret;
	}

	if(bl->type == BL_MOB && (struct mob_data *)bl) {
		// 10�̈ʁ�Lv*2�A�P�̈ʁ�����
		ret = ((struct mob_data *)bl)->def_ele;
	}
	else if(bl->type == BL_PC && (struct map_session_data *)bl) {
		// �h�䑮��Lv1
		ret = 20+((struct map_session_data *)bl)->def_ele;
	}
	else if(bl->type == BL_PET && (struct pet_data *)bl) {
		ret = mob_db[((struct pet_data *)bl)->class_].element;
	}
	else if(bl->type == BL_HOM && (struct homun_data *)bl) {
		ret = homun_db[((struct homun_data *)bl)->status.class_-HOM_ID].element;
	}
	else if(bl->type == BL_MERC && (struct merc_data *)bl) {
		struct merc_db *db = merc_search_data(((struct merc_data *)bl)->status.class_);
		if(db)
			ret = db->element;
	}
	else if(bl->type == BL_ELEM && (struct elem_data *)bl) {
		struct elem_db *db = elem_search_data(((struct elem_data *)bl)->status.class_);
		if(db)
			ret = db->element;
	}

	return ret;
}

/*==========================================
 * �Ώۂ̍U��������Ԃ�
 *------------------------------------------
 */
int status_get_attack_element(struct block_list *bl)
{
	int ret = ELE_NEUTRAL;
	struct status_change *sc = NULL;

	nullpo_retr(ELE_NEUTRAL, bl);

	sc = status_get_sc(bl);
	if(bl->type == BL_MOB && (struct mob_data *)bl)
		ret = ELE_NEUTRAL;
	else if(bl->type == BL_PC && (struct map_session_data *)bl) {
		if(((struct map_session_data *)bl)->elementball.num >= MAX_ELEMENTBALL)
			ret = ((struct map_session_data *)bl)->elementball.ele;
		else
			ret = ((struct map_session_data *)bl)->atk_ele;
	}
	else if(bl->type == BL_PET && (struct pet_data *)bl)
		ret = ELE_NEUTRAL;
	else if(bl->type == BL_HOM && (struct homun_data *)bl)
		ret = ELE_NONE;	// ������
	else if(bl->type == BL_MERC && (struct merc_data *)bl)
		ret = ELE_NONE;	// ������
	else if(bl->type == BL_ELEM && (struct elem_data *)bl)
		ret = ELE_NONE;	// ������

	if(sc) {
		if(sc->data[SC_FROSTWEAPON].timer != -1)		// �t���X�g�E�F�|��
			ret = ELE_WATER;
		if(sc->data[SC_SEISMICWEAPON].timer != -1)	// �T�C�Y�~�b�N�E�F�|��
			ret = ELE_EARTH;
		if(sc->data[SC_FLAMELAUNCHER].timer != -1)	// �t���[�������`���[
			ret = ELE_FIRE;
		if(sc->data[SC_LIGHTNINGLOADER].timer != -1)	// ���C�g�j���O���[�_�[
			ret = ELE_WIND;
		if(sc->data[SC_ENCPOISON].timer != -1)		// �G���`�����g�|�C�Y��
			ret = ELE_POISON;
		if(sc->data[SC_ASPERSIO].timer != -1)		// �A�X�y���V�I
			ret = ELE_HOLY;
		if(sc->data[SC_DARKELEMENT].timer != -1)		// �ő���
			ret = ELE_DARK;
		if(sc->data[SC_ATTENELEMENT].timer != -1)	// �O����
			ret = ELE_GHOST;
		if(sc->data[SC_UNDEADELEMENT].timer != -1)	// �s������
			ret = ELE_UNDEAD;
		if(sc->data[SC_SEVENWIND].timer != -1)		// �g������
			ret = sc->data[SC_SEVENWIND].val3;
		if(sc->data[SC_TIDAL_WEAPON].timer != -1 || sc->data[SC_TIDAL_WEAPON_OPTION].timer != -1)	// �^�C�_���E�F�|��
			ret = ELE_WATER;
	}
	return ret;
}

/*==========================================
 * �Ώۂ̍U�������i����j��Ԃ�
 *------------------------------------------
 */
int status_get_attack_element2(struct block_list *bl)
{
	nullpo_retr(ELE_NEUTRAL, bl);

	if(bl->type == BL_PC && (struct map_session_data *)bl) {
		int ret;
		struct status_change *sc = status_get_sc(bl);
		if(((struct map_session_data *)bl)->elementball.num >= MAX_ELEMENTBALL)
			ret = ((struct map_session_data *)bl)->elementball.ele;
		else
			ret = ((struct map_session_data *)bl)->atk_ele_;

		if(sc) {
			if(sc->data[SC_FROSTWEAPON].timer != -1)		// �t���X�g�E�F�|��
				ret = ELE_WATER;
			if(sc->data[SC_SEISMICWEAPON].timer != -1)	// �T�C�Y�~�b�N�E�F�|��
				ret = ELE_EARTH;
			if(sc->data[SC_FLAMELAUNCHER].timer != -1)	// �t���[�������`���[
				ret = ELE_FIRE;
			if(sc->data[SC_LIGHTNINGLOADER].timer != -1)	// ���C�g�j���O���[�_�[
				ret = ELE_WIND;
			if(sc->data[SC_ENCPOISON].timer != -1)		// �G���`�����g�|�C�Y��
				ret = ELE_POISON;
			if(sc->data[SC_ASPERSIO].timer != -1)		// �A�X�y���V�I
				ret = ELE_HOLY;
			if(sc->data[SC_DARKELEMENT].timer != -1)		// �ő���
				ret = ELE_DARK;
			if(sc->data[SC_ATTENELEMENT].timer != -1)	// �O����
				ret = ELE_GHOST;
			if(sc->data[SC_UNDEADELEMENT].timer != -1)	// �s������
				ret = ELE_UNDEAD;
			if(sc->data[SC_SEVENWIND].timer != -1)		// �g������
				ret = sc->data[SC_SEVENWIND].val3;
		}
		return ret;
	}
	return ELE_NEUTRAL;
}

/*==========================================
 * �Ώۂ̕��푮�����������U��������Ԃ�
 *------------------------------------------
 */
int status_get_attack_element_nw(struct block_list *bl)
{
	int ret = ELE_NEUTRAL;
	struct status_change *sc = NULL;

	nullpo_retr(ELE_NEUTRAL, bl);

	sc = status_get_sc(bl);

	if(bl->type == BL_PC && (struct map_session_data *)bl) {
		if(((struct map_session_data *)bl)->elementball.num >= MAX_ELEMENTBALL)
			ret = ((struct map_session_data *)bl)->elementball.ele;
	}
	else if(bl->type == BL_HOM && (struct homun_data *)bl)
		ret = ELE_NONE;	// ������
	else if(bl->type == BL_MERC && (struct merc_data *)bl)
		ret = ELE_NONE;	// ������
	else if(bl->type == BL_ELEM && (struct elem_data *)bl)
		ret = ELE_NONE;	// ������

	if(sc) {
		if(sc->data[SC_FROSTWEAPON].timer != -1)		// �t���X�g�E�F�|��
			ret = ELE_WATER;
		if(sc->data[SC_SEISMICWEAPON].timer != -1)	// �T�C�Y�~�b�N�E�F�|��
			ret = ELE_EARTH;
		if(sc->data[SC_FLAMELAUNCHER].timer != -1)	// �t���[�������`���[
			ret = ELE_FIRE;
		if(sc->data[SC_LIGHTNINGLOADER].timer != -1)	// ���C�g�j���O���[�_�[
			ret = ELE_WIND;
		if(sc->data[SC_ENCPOISON].timer != -1)		// �G���`�����g�|�C�Y��
			ret = ELE_POISON;
		if(sc->data[SC_ASPERSIO].timer != -1)		// �A�X�y���V�I
			ret = ELE_HOLY;
		if(sc->data[SC_DARKELEMENT].timer != -1)		// �ő���
			ret = ELE_DARK;
		if(sc->data[SC_ATTENELEMENT].timer != -1)	// �O����
			ret = ELE_GHOST;
		if(sc->data[SC_UNDEADELEMENT].timer != -1)	// �s������
			ret = ELE_UNDEAD;
		if(sc->data[SC_SEVENWIND].timer != -1)		// �g������
			ret = sc->data[SC_SEVENWIND].val3;
		if(sc->data[SC__INVISIBILITY].timer != -1)	// �C���r�W�r���e�B
			ret = ELE_GHOST;
	}
	return ret;
}

/*==========================================
 * �Ώۂ̃p�[�e�BID��Ԃ�
 *------------------------------------------
 */
int status_get_party_id(struct block_list *bl)
{
	nullpo_retr(0, bl);

	if(bl->type == BL_PC && (struct map_session_data *)bl) {
		return ((struct map_session_data *)bl)->status.party_id;
	}
	else if(bl->type == BL_MOB && (struct mob_data *)bl) {
		struct mob_data *md = (struct mob_data *)bl;
		return (md->master_id > 0)? -md->master_id: -md->bl.id;
	}
	else if(bl->type == BL_HOM && (struct homun_data *)bl) {
		//struct homun_data *hd = (struct homun_data *)bl;
		//return status_get_party_id(&hd->msd->bl);
		return 0;
	}
	else if(bl->type == BL_MERC && (struct merc_data *)bl) {
		//struct merc_data *mcd = (struct merc_data *)bl;
		//return status_get_party_id(&mcd->msd->bl);
		return 0;
	}
	else if(bl->type == BL_ELEM && (struct elem_data *)bl) {
		//struct elem_data *eld = (struct elem_data *)bl;
		//return status_get_party_id(&eld->msd->bl);
		return 0;
	}
	else if(bl->type == BL_SKILL && (struct skill_unit *)bl) {
		return ((struct skill_unit *)bl)->group->party_id;
	}

	return 0;
}

/*==========================================
 * �Ώۂ̃M���hID��Ԃ�
 *------------------------------------------
 */
int status_get_guild_id(struct block_list *bl)
{
	nullpo_retr(0, bl);

	if(bl->type == BL_PC && (struct map_session_data *)bl) {
		return ((struct map_session_data *)bl)->status.guild_id;
	} else if(bl->type == BL_MOB && (struct mob_data *)bl) {
		return ((struct mob_data *)bl)->class_;
	} else if(bl->type == BL_HOM && (struct homun_data *)bl) {
		//struct homun_data *hd = (struct homun_data *)bl;
		//return status_get_guild_id(&hd->msd->bl);
		return 0;
	} else if(bl->type == BL_MERC && (struct merc_data *)bl) {
		//struct merc_data *mcd = (struct merc_data *)bl;
		//return status_get_guild_id(&mcd->msd->bl);
		return 0;
	} else if(bl->type == BL_ELEM && (struct elem_data *)bl) {
		//struct elem_data *eld = (struct elem_data *)bl;
		//return status_get_guild_id(&eld->msd->bl);
		return 0;
	} else if(bl->type == BL_SKILL && (struct skill_unit *)bl) {
		return ((struct skill_unit *)bl)->group->guild_id;
	}

	return 0;
}

/*==========================================
 * �Ώۂ̎푰��Ԃ�
 *------------------------------------------
 */
int status_get_race(struct block_list *bl)
{
	int race = RCT_FORMLESS;
	struct status_change *sc;

	nullpo_retr(RCT_FORMLESS, bl);

	if(bl->type == BL_MOB && (struct mob_data *)bl) {
		race = mob_db[((struct mob_data *)bl)->class_].race;
	}
	else if(bl->type == BL_PC && (struct map_session_data *)bl) {
		race = ((struct map_session_data *)bl)->race;
	}
	else if(bl->type == BL_PET && (struct pet_data *)bl) {
		return mob_db[((struct pet_data *)bl)->class_].race;
	}
	else if(bl->type == BL_HOM && (struct homun_data *)bl) {
		return homun_db[((struct homun_data *)bl)->status.class_-HOM_ID].race;
	}
	else if(bl->type == BL_MERC && (struct merc_data *)bl) {
		struct merc_db *db = merc_search_data(((struct merc_data *)bl)->status.class_);
		if(db)
			return db->race;
	}
	else if(bl->type == BL_ELEM && (struct elem_data *)bl) {
		struct elem_db *db = elem_search_data(((struct elem_data *)bl)->status.class_);
		if(db)
			return db->race;
	}
	else {
		return RCT_FORMLESS;
	}

	sc = status_get_sc(bl);

	if(sc) {
		if(sc->data[SC_RACEUNKNOWN].timer != -1)	// ���`
			race = RCT_FORMLESS;
		if(sc->data[SC_RACEUNDEAD].timer != -1)	// �s��
			race = RCT_UNDEAD;
		if(sc->data[SC_RACEBEAST].timer != -1)	// ����
			race = RCT_BRUTE;
		if(sc->data[SC_RACEPLANT].timer != -1)	// �A��
			race = RCT_PLANT;
		if(sc->data[SC_RACEINSECT].timer != -1)	// ����
			race = RCT_INSECT;
		if(sc->data[SC_RACEFISH].timer != -1)	// ���L
			race = RCT_FISH;
		if(sc->data[SC_RACEDEVIL].timer != -1)	// ����
			race = RCT_DEMON;
		if(sc->data[SC_RACEHUMAN].timer != -1)	// �l��
			race = RCT_HUMAN;
		if(sc->data[SC_RACEANGEL].timer != -1)	// �V�g
			race = RCT_ANGEL;
		if(sc->data[SC_RACEDRAGON].timer != -1)	// ��
			race = RCT_DRAGON;
	}

	return race;
}

/*==========================================
 * �Ώۂ̃T�C�Y��Ԃ�
 *------------------------------------------
 */
int status_get_size(struct block_list *bl)
{
	nullpo_retr(1, bl);

	if(bl->type == BL_MOB && (struct mob_data *)bl) {
		return mob_db[((struct mob_data *)bl)->class_].size;
	} else if(bl->type == BL_PC && (struct map_session_data *)bl) {
		if(pc_isbaby((struct map_session_data *)bl))
			return 0;
		else
			return 1;
	} else if(bl->type == BL_PET && (struct pet_data *)bl) {
		return mob_db[((struct pet_data *)bl)->class_].size;
	} else if(bl->type == BL_HOM && (struct homun_data *)bl) {
		return homun_db[((struct homun_data *)bl)->status.class_-HOM_ID].size;
	} else if(bl->type == BL_MERC && (struct merc_data *)bl) {
		struct merc_db *db = merc_search_data(((struct merc_data *)bl)->status.class_);
		if(db)
			return db->size;
	} else if(bl->type == BL_ELEM && (struct elem_data *)bl) {
		struct elem_db *db = elem_search_data(((struct elem_data *)bl)->status.class_);
		if(db)
			return db->size;
	}

	return 1;
}

/*==========================================
 * �Ώۂ̃��[�h��Ԃ�
 *------------------------------------------
 */
int status_get_mode(struct block_list *bl)
{
	nullpo_retr(0x01, bl);

	if(bl->type == BL_MOB) {
		struct mob_data* md = (struct mob_data*)bl;
		return (md->mode ? md->mode : mob_db[md->class_].mode);
	}
	else if(bl->type == BL_PET) {
		return mob_db[((struct pet_data *)bl)->class_].mode;
	}

	return MD_CANMOVE;	// �Ƃ肠���������Ƃ������Ƃ�1
}

/*==========================================
 * �Ώۂ�MVPExp��Ԃ�
 *------------------------------------------
 */
int status_get_mexp(struct block_list *bl)
{
	nullpo_retr(0, bl);

	if(bl->type == BL_MOB && (struct mob_data *)bl)
		return mob_db[((struct mob_data *)bl)->class_].mexp;
	else if(bl->type == BL_PET && (struct pet_data *)bl)
		return mob_db[((struct pet_data *)bl)->class_].mexp;

	return 0;
}

/*==========================================
 * �Ώۂ̓G�^�C�v��Ԃ�
 *------------------------------------------
 */
int status_get_enemy_type(struct block_list *bl)
{
	nullpo_retr(EMY_ETC, bl);

	if( bl->type == BL_PC )
		return EMY_PC;
	else if( bl->type == BL_MOB && !(status_get_mode(bl)&MD_BOSS) )
		return EMY_MOB;
	else if( bl->type == BL_HOM )
		return EMY_HOM;
	else if( bl->type == BL_MERC )
		return EMY_MERC;

	return EMY_ETC;
}

/*==========================================
 * �Ώۂ̕��F��Ԃ�
 *------------------------------------------
 */
short status_get_clothes_color(struct block_list *bl)
{
	short color = -1;

	nullpo_retr(-1, bl);

	if(bl->type == BL_PC) {
		color = ((struct map_session_data *)bl)->status.clothes_color;
	} else if(bl->type & (BL_MOB | BL_PET)) {
		int id = status_get_class(bl);
		if(id >= 0 && mob_is_pcview(id))
			color = mob_get_clothes_color(id);
	}
	if(color >= 0 && color < MAX_CLOTH_COLOR)
		return color;

	return -1;
}

/*==========================================
 * StatusChange�̎擾
 *------------------------------------------
 */
struct status_change *status_get_sc(struct block_list *bl)
{
	nullpo_retr(NULL, bl);

	if(bl->type == BL_MOB && (struct mob_data *)bl)
		return &((struct mob_data*)bl)->sc;
	if(bl->type == BL_PC && (struct map_session_data *)bl)
		return &((struct map_session_data*)bl)->sc;
	if(bl->type == BL_HOM && (struct homun_data *)bl)
		return &((struct homun_data*)bl)->sc;
	if(bl->type == BL_MERC && (struct merc_data *)bl)
		return &((struct merc_data*)bl)->sc;
	if(bl->type == BL_ELEM && (struct elem_data *)bl)
		return &((struct elem_data*)bl)->sc;

	return NULL;
}

/*==========================================
 * ���@�������ǂ���
 *------------------------------------------
 */
int status_check_no_magic_damage(struct block_list *bl)
{
	nullpo_retr(0, bl);

	if(bl->type == BL_PC)
	{
		if(((struct map_session_data*)bl)->special_state.no_magic_damage)
			return 1;
	}
	return 0;
}

/*==========================================
 * ��Ԉُ�̑ϐ��v�Z
 *------------------------------------------
 */
int status_change_rate(struct block_list *bl,int type,int rate,int src_level)
{
	int sc_flag = 0;
#ifndef PRE_RENEWAL
	int cap;
#endif

	nullpo_retr(0, bl);

	if(type < 0)	// type��0�����̏ꍇ���s
		return 0;

	if(rate <= 0)	// �m����0�ȉ��̂��͎̂��s
		return 0;

#ifndef PRE_RENEWAL
	cap = src_level - status_get_lv(bl);
	if(cap <= 0)	// �Ώۂ̂ق������x���������ꍇ��0
		cap = 0;
	else
		cap = cap * cap / 5;
#endif

	switch(type) {	// ��Ԉُ�ϐ��X�e�[�^�X rate�͖�����
#ifdef PRE_RENEWAL
		case SC_STONE:	// �Ή�
		case SC_FREEZE:	// ����
			rate += src_level*10 - rate * status_get_mdef(bl)*10 / 1000 - status_get_luk(bl)*10 - status_get_lv(bl)*10;
			sc_flag = 1;
			break;
		case SC_STUN:	// �X�^��
		case SC_SILENCE:	// ����
		case SC_POISON:	// ��
		case SC_DPOISON:	// �ғ�
		case SC_BLEED:	// �o��
			rate += src_level*10 - rate * status_get_vit(bl)*10 / 1000 - status_get_luk(bl)*10 - status_get_lv(bl)*10;
			sc_flag = 1;
			break;
		case SC_SLEEP:	// ����
			rate += src_level*10 - rate * status_get_int(bl)*10 / 1000 - status_get_luk(bl)*10 - status_get_lv(bl)*10;
			sc_flag = 1;
			break;
		case SC_BLIND:	// �Í�
			rate += src_level*10 - rate * (status_get_vit(bl)*10 + status_get_int(bl)*10) / 2000 - status_get_luk(bl)*10 - status_get_lv(bl)*10;
			sc_flag = 1;
			break;
		case SC_CURSE:	// ��
			rate += src_level*10 - rate * status_get_luk(bl)*10 / 1000 - status_get_luk(bl)*10;
			sc_flag = 1;
			break;
		case SC_CONFUSION:	// ����
			rate += status_get_luk(bl)*10 + -(rate * (status_get_str(bl)*10 + status_get_int(bl)*10) / 2000) - src_level*10 + status_get_lv(bl)*10;
			sc_flag = 1;
			break;
#else
		case SC_STONE:	// �Ή�
		case SC_FREEZE:	// ����
			rate = rate * (100 - (status_get_mdef(bl) - cap)) / 100;
			sc_flag = 1;
			break;
		case SC_STUN:	// �X�^��
		case SC_POISON:	// ��
		case SC_DPOISON:	// �ғ�
			rate = rate * (100 - (status_get_vit(bl) - cap)) / 100;
			sc_flag = 1;
			break;
		case SC_SLEEP:	// ����
		case SC_BLEED:	// �o��
			rate = rate * (100 - (status_get_agi(bl) - cap)) / 100;
			sc_flag = 1;
			break;
		case SC_BLIND:	// �Í�
		case SC_SILENCE:	// ����
			rate = rate * (100 - (status_get_int(bl) - cap)) / 100;
			sc_flag = 1;
			break;
		case SC_CONFUSION:	// ����
		case SC_CURSE:	// ��
			rate = rate * (100 - (status_get_luk(bl) - cap)) / 100;
			sc_flag = 1;
			break;
#endif
		case SC_TOXIN:
		case SC_PARALIZE:
		case SC_VENOMBLEED:
		case SC_MAGICMUSHROOM:
		case SC_DEATHHURT:
		case SC_PYREXIA:
		case SC_OBLIVIONCURSE:
		case SC_LEECHEND:
			if(bl->type == BL_PC)
				rate -= rate * pc_checkskill((struct map_session_data *)bl,GC_RESEARCHNEWPOISON) * 5 / 100;
			break;
		default:
			break;
	}
	if(sc_flag) {
		struct status_change *sc = status_get_sc(bl);
		if(sc) {
			if(sc->data[SC_STATUS_UNCHANGE].timer != -1 && status_is_disable(type,0x10)) {
				rate = 0;	// �S�X�y���̑S��Ԉُ�ϐ����Ȃ疳��
			}
			if(sc->data[SC_NAUTHIZ].timer != -1 && status_is_disable(type,0x10)) {
				rate = 0;	// ���t���b�V�����ʒ��͖���
			}
			if(sc->data[SC_INSPIRATION].timer != -1 && status_is_disable(type,0x10)) {
				rate = 0;	// �C���X�s���[�V�������ʒ��Ȃ疳��
			}
		}
	}

	return rate;
}

/*==========================================
 * ��Ԉُ�̕��ʁi�f�b�h���[�C���t�F�N�g�j
 *------------------------------------------
 */
int status_change_copy(struct block_list *src,struct block_list *bl)
{
	int type;
	unsigned int tick;
	struct status_change *sc;
	struct TimerData *td = NULL;

	nullpo_retr(0, src);
	nullpo_retr(0, bl);
	nullpo_retr(0, sc = status_get_sc(src));

	if(sc->count <= 0)
		return 0;
	if(sc->data[SC__DEADLYINFECT].timer != -1 && atn_rand()%100 > sc->data[SC__DEADLYINFECT].val1*20)
		return 0;

	tick = gettick();
	status_calc_pc_stop_begin(bl);

	for(type = 0; type < MAX_STATUSCHANGE; type++) {
		if(scdata_db[type].copyable) {
			if(sc->data[type].timer != -1)
			{
				td = get_timer(sc->data[type].timer);
				if (td == NULL || td->func != status_change_timer || DIFF_TICK(td->tick,tick) < 0)
					continue;
				status_change_start(bl,type,sc->data[type].val1,sc->data[type].val2,sc->data[type].val3,sc->data[type].val4,DIFF_TICK(td->tick,tick),0);
			}
		}
	}
	status_calc_pc_stop_end(bl);

	return 0;
}

/*==========================================
 * �X�e�[�^�X�ُ�f�[�^�̓��I�m��
 *------------------------------------------
 */
#ifdef DYNAMIC_SC_DATA

int status_calloc_sc_data(struct status_change *sc)
{
	if(sc && sc->data == dummy_sc_data)
	{
		int i;
		sc->data = (struct status_change_data *)aCalloc(MAX_STATUSCHANGE, sizeof(struct status_change_data));
		for(i=0; i<MAX_STATUSCHANGE; i++) {
			sc->data[i].timer = -1;
			sc->data[i].val1  = 0;
			sc->data[i].val2  = 0;
			sc->data[i].val3  = 0;
			sc->data[i].val4  = 0;
		}
		sc->count = 0;
		return 1;	// calloced
	}

	return 0;
}

int status_free_sc_data(struct status_change *sc)
{
	if(sc && sc->data != dummy_sc_data)
	{
		map_freeblock(sc->data);
		sc->data  = dummy_sc_data;
		sc->count = 0;
	}

	return 0;
}

#endif

/*==========================================
 * �X�e�[�^�X�ُ�J�n
 *------------------------------------------
 */
int status_change_start(struct block_list *bl,int type,int val1,int val2,int val3,int val4,int tick,int flag)
{
	struct map_session_data *sd  = NULL;
	struct mob_data         *md  = NULL;
	struct homun_data       *hd  = NULL;
	struct merc_data        *mcd = NULL;
	struct elem_data        *eld = NULL;
	struct status_change    *sc  = NULL;
	struct unit_data        *ud  = NULL;
	int icon_tick = tick, icon_val1 = 0, icon_val2 = 0, icon_val3 = 0, opt_flag = 0, calc_flag = 0, race, mode, elem;
	unsigned int current_tick = gettick();

	nullpo_retr(0, bl);

	if(type < 0)
		return 0;

	sc = status_get_sc(bl);
	if(sc == NULL)
		return 0;
	ud = unit_bl2ud(bl);
	if(ud == NULL)
		return 0;

	if(bl->type == BL_HOM && !battle_config.allow_homun_status_change)
	{
		if(type < SC_AVOID || type > SC_SPEED)
			return 0;
	}

	if(status_get_class(bl) == MOBID_EMPERIUM && type != SC_SAFETYWALL && type != SC_PNEUMA)
		return 0;

	race = status_get_race(bl);
	mode = status_get_mode(bl);
	elem = status_get_elem_type(bl);

	if(type == SC_AETERNA && (sc->data[SC_STONE].timer != -1 || sc->data[SC_FREEZE].timer != -1))
		return 0;

	// ����n
	if(type >= MAX_STATUSCHANGE) {
		switch(type) {
			case SC_SOUL:
				status_change_soulstart(bl,val1,val2,val3,val4,tick,flag);
				break;
			default:
				if(battle_config.error_log)
					printf("UnknownStatusChange [%d]\n", type);
				break;
		}
		return 0;
	}

	// ON/OFF
	switch(type) {
		case SC_AUTOBERSERK:
		case SC_READYSTORM:
		case SC_READYDOWN:
		case SC_READYTURN:
		case SC_READYCOUNTER:
		case SC_DODGE:
		case SC_FUSION:
		case SC_WEAPONBLOCKING:
		case SC_CLOAKINGEXCEED:
		//case SC_CAMOUFLAGE:
		case SC__REPRODUCE:
		case SC__INVISIBILITY:
		case SC_REFLECTDAMAGE:
		case SC_FORCEOFVANGUARD:
			if(sc->data[type].timer != -1) {
				status_change_end(bl,type,-1);
				return 0;
			}
			break;
		// 3���V�ŃX�L��
		case SC_TOXIN:
		case SC_PARALIZE:
		case SC_VENOMBLEED:
		case SC_MAGICMUSHROOM:
		case SC_DEATHHURT:
		case SC_PYREXIA:
		case SC_OBLIVIONCURSE:
		case SC_LEECHEND:
			if(sc->data[SC_TOXIN].timer != -1 ||
			   sc->data[SC_PARALIZE].timer != -1 ||
			   sc->data[SC_VENOMBLEED].timer != -1 ||
			   sc->data[SC_MAGICMUSHROOM].timer != -1 ||
			   sc->data[SC_DEATHHURT].timer != -1 ||
			   sc->data[SC_PYREXIA].timer != -1 ||
			   sc->data[SC_OBLIVIONCURSE].timer != -1 ||
			   sc->data[SC_LEECHEND].timer != -1
			) {
				// �V�ł͏d�������Ȃ�
				return 0;
			}
			break;
		// 3���̃X�L��
		case SC_SWING:
		case SC_SYMPHONY_LOVE:
		case SC_MOONLIT_SERENADE:
		case SC_RUSH_WINDMILL:
		case SC_ECHOSONG:
		case SC_HARMONIZE:
		case SC_SIREN:
		case SC_SIRCLEOFNATURE:
		case SC_SONG_OF_MANA:
		case SC_DANCE_WITH_WUG:
		case SC_SATURDAY_NIGHT_FEVER:
		case SC_LERADS_DEW:
		case SC_MELODYOFSINK:
		case SC_BEYOND_OF_WARCRY:
		case SC_UNLIMITED_HUMMING_VOICE:
			if(battle_config.third_song_overlap) {	// �퓬�ݒ�ŉ̃X�L���d���\�̏ꍇ
				// �����f�B�[�I�u�V���N�ƃr�����h�I�u�E�H�[�N���C�͏d�������Ȃ�
				if(type == SC_MELODYOFSINK && sc->data[SC_BEYOND_OF_WARCRY].timer != -1)
					status_change_end(bl,SC_BEYOND_OF_WARCRY,-1);
				if(type == SC_BEYOND_OF_WARCRY && sc->data[SC_MELODYOFSINK].timer != -1)
					status_change_end(bl,SC_MELODYOFSINK,-1);
				break;
			}
			if(sc->data[SC_SWING].timer != -1)
				status_change_end(bl,SC_SWING,-1);
			if(sc->data[SC_SYMPHONY_LOVE].timer != -1)
				status_change_end(bl,SC_SYMPHONY_LOVE,-1);
			if(sc->data[SC_MOONLIT_SERENADE].timer != -1)
				status_change_end(bl,SC_MOONLIT_SERENADE,-1);
			if(sc->data[SC_RUSH_WINDMILL].timer != -1)
				status_change_end(bl,SC_RUSH_WINDMILL,-1);
			if(sc->data[SC_ECHOSONG].timer != -1)
				status_change_end(bl,SC_ECHOSONG,-1);
			if(sc->data[SC_HARMONIZE].timer != -1)
				status_change_end(bl,SC_HARMONIZE,-1);
			if(sc->data[SC_SIREN].timer != -1)
				status_change_end(bl,SC_SIREN,-1);
			if(sc->data[SC_SIRCLEOFNATURE].timer != -1)
				status_change_end(bl,SC_SIRCLEOFNATURE,-1);
			if(sc->data[SC_SONG_OF_MANA].timer != -1)
				status_change_end(bl,SC_SONG_OF_MANA,-1);
			if(sc->data[SC_DANCE_WITH_WUG].timer != -1)
				status_change_end(bl,SC_DANCE_WITH_WUG,-1);
			if(sc->data[SC_SATURDAY_NIGHT_FEVER].timer != -1)
				status_change_end(bl,SC_SATURDAY_NIGHT_FEVER,-1);
			if(sc->data[SC_LERADS_DEW].timer != -1)
				status_change_end(bl,SC_LERADS_DEW,-1);
			if(sc->data[SC_MELODYOFSINK].timer != -1)
				status_change_end(bl,SC_MELODYOFSINK,-1);
			if(sc->data[SC_BEYOND_OF_WARCRY].timer != -1)
				status_change_end(bl,SC_BEYOND_OF_WARCRY,-1);
			if(sc->data[SC_UNLIMITED_HUMMING_VOICE].timer != -1)
				status_change_end(bl,SC_UNLIMITED_HUMMING_VOICE,-1);
			break;
	}

	sd  = BL_DOWNCAST( BL_PC,   bl );
	md  = BL_DOWNCAST( BL_MOB,  bl );
	hd  = BL_DOWNCAST( BL_HOM,  bl );
	mcd = BL_DOWNCAST( BL_MERC, bl );
	eld = BL_DOWNCAST( BL_ELEM, bl );

	if( sd ) {
		int weapon = sd->status.weapon;
		if(weapon >= WT_MAX)
			weapon -= WT_DOUBLE_DD + WT_MAX;

		// �A�h���i�������b�V���̕��픻��
		if( type == SC_ADRENALINE && !(skill_get_weapontype(BS_ADRENALINE)&(1<<weapon)) )
			return 0;
		// �t���A�h���i�������b�V���̕��픻��
		if( type == SC_ADRENALINE2 && !(skill_get_weapontype(BS_ADRENALINE2)&(1<<weapon)) )
			return 0;
		if( !(flag&8) && SC_STONE <= type && type <= SC_BLEED ) {	/* �J�[�h�ɂ��ϐ� */
			int scdef = sd->reseff[type-SC_STONE];
			if(sc->data[SC_SIEGFRIED].timer != -1) {	// �W�[�N�t���[�h�̏�Ԉُ�ϐ�
				scdef += 5000;
			}
			if(scdef > 0 && atn_rand()%10000 < scdef) {
				if(battle_config.battle_log)
					printf("PC %d skill_sc_start: card�ɂ��ُ�ϐ�����\n", sd->bl.id);
				return 0;
			}
#ifndef PRE_RENEWAL
			tick = tick * (10000 - scdef) / 10000;
			if(tick <= 0)
				return 0;
#endif
		}
	}

	// �A���f�b�h�͓����E�Ή��E�o������
	if((race == RCT_UNDEAD || elem == ELE_UNDEAD) && !(flag&1) && (type == SC_STONE || type == SC_FREEZE || type == SC_BLEED))
		return 0;

	// �E�H�[�O�o�C�g��Ԓ��̓n�C�f�B���O�A�N���[�L���O����
	if(sc->data[SC_WUGBITE].timer != -1 && (type == SC_HIDING || type == SC_CLOAKING || type == SC_CLOAKINGEXCEED))
		return 0;

	// �E�H�[�}�[��Ԓ��͓����A�X���A�Ⓚ����
	if(sc->data[SC_WARMER].timer != -1 && (type == SC_FREEZE || type == SC_FROSTMISTY || type == SC_DIAMONDDUST))
		return 0;

	// �s���s�\��Ԉُ�̗D�揇��
	if(type >= SC_STONE && type < SC_SLEEP) {
		int i;
		for(i = type; i < SC_SLEEP; i++) {
			if(sc->data[i+1].timer != -1)
				return 0;
		}
	}

	if((type == SC_ADRENALINE || type == SC_ADRENALINE2 || type == SC_WEAPONPERFECTION || type == SC_OVERTHRUST) &&
		sc->data[type].timer != -1 && sc->data[type].val2 && !val2)
		return 0;

	// �{�X�����ɂ͖���(�������J�[�h�ɂ����ʂ͓K�p�����)
	if( mode&MD_BOSS && !(flag&1) && status_is_disable(type,0x01) ) {
		if(type == SC_BLESSING && !battle_check_undead(race,elem) && race != RCT_DEMON) {
			// �u���X�͕s��/�����łȂ��Ȃ����
			;
		} else {
			return 0;
		}
	}

	// �N�@�O�}�C�A���͖���
	if(sc->data[SC_QUAGMIRE].timer != -1 && status_is_disable(type,0x02))
		return 0;
	// ����Y��Ȃ��Œ��͖���
	if(sc->data[SC_DONTFORGETME].timer != -1 && status_is_disable(type,0x04))
		return 0;
	// ���x�������͖���
	if(sc->data[SC_DECREASEAGI].timer != -1 && status_is_disable(type,0x08))
		return 0;

	if(type == SC_STUN || type == SC_SLEEP)
		unit_stop_walking(bl,1);

	// �u���b�V���O�ɂ��􂢁A�Ή��̉���
	if(type == SC_BLESSING && (sd || (!battle_check_undead(race,elem) && race != RCT_DEMON))) {
		bool flag = false;
		if(sc->data[SC_CURSE].timer != -1) {
			status_change_end(bl,SC_CURSE,-1);
			flag = true;
		}
		if(sc->data[SC_STONE].timer != -1 && sc->data[SC_STONE].val2 == 0) {
			status_change_end(bl,SC_STONE,-1);
			flag = true;
		}

		// �􂢁A�Ή��������̓u���b�V���O��ԂɂȂ�Ȃ��̂ŏI���
		if(flag)
			return 0;
	}

	if(sc->data[type].timer != -1) {	/* ���łɓ����ُ�ɂȂ��Ă���ꍇ�^�C�}���� */
		if(type == SC_ALL_RIDING || type == SC_HAT_EFFECT) {
			status_change_end(bl,type,-1);
			return 0;
		}
		if(sc->data[type].val1 > val1 && type != SC_COMBO && type != SC_DANCING && type != SC_DEVOTION &&
			type != SC_SPEEDPOTION0 && type != SC_SPEEDPOTION1 && type != SC_SPEEDPOTION2 &&
			type != SC_DOUBLE && type != SC_TKCOMBO && type != SC_DODGE && type != SC_SPURT && type != SC_SEVENWIND &&
			type != SC_SHAPESHIFT && type != SC_ON_PUSH_CART)
			return 0;
		if((type >= SC_STUN && type <= SC_BLIND) || type == SC_DPOISON || type == SC_FOGWALLPENALTY || type == SC_FORCEWALKING || type == SC_ORATIO)
			return 0;	/* �p���������ł��Ȃ���Ԉُ�ł��鎞�͏�Ԉُ���s��Ȃ� */
		if(type == SC_GRAFFITI || type == SC_SEVENWIND || type == SC_SHAPESHIFT || type == SC__AUTOSHADOWSPELL || type == SC_PROPERTYWALK) {
			// �ُ풆�ɂ�����x��Ԉُ�ɂȂ������ɉ������Ă���ēx������
			status_change_end(bl,type,-1);
		} else {
			sc->count--;
			delete_timer(sc->data[type].timer, status_change_timer);
			sc->data[type].timer = -1;
#ifdef DYNAMIC_SC_DATA
			if(sc->count <= 0 && battle_config.free_sc_data_dynamically)
				status_free_sc_data(sc);
#endif
		}
	}

	switch(type) {	/* �ُ�̎�ނ��Ƃ̏��� */
		case SC_DOUBLE:				/* �_�u���X�g���C�t�B���O */
		case SC_SUFFRAGIUM:			/* �T�t���M�� */
		case SC_MAGNIFICAT:			/* �}�O�j�t�B�J�[�g */
		case SC_AETERNA:			/* �G�[�e���i */
		case SC_BASILICA:			/* �o�W���J */
		case SC_TRICKDEAD:			/* ���񂾂ӂ� */
		case SC_STRIPWEAPON:
		case SC_STRIPSHIELD:
		case SC_STRIPARMOR:
		case SC_STRIPHELM:
		case SC_CP_WEAPON:
		case SC_CP_SHIELD:
		case SC_CP_ARMOR:
		case SC_CP_HELM:
		case SC_DEVOTION:			/* �f�B�{�[�V���� */
		case SC_COMBO:
		case SC_EXTREMITYFIST:			/* ���C���e���� */
		case SC_RICHMANKIM:
		case SC_ROKISWEIL:			/* ���L�̋��� */
		case SC_INTOABYSS:			/* �[���̒��� */
		case SC_POEMBRAGI:			/* �u���M�̎� */
		case SC_ANKLE:				/* �A���N�� */
		case SC_MAGNUM:				/* �}�O�i���u���C�N */
		case SC_TIGERFIST:			/* ���Ռ� */
		case SC_ENERGYCOAT:			/* �G�i�W�[�R�[�g */
		case SC_POEMBRAGI_:			/* �u���M�̎� */
		case SC_FOGWALLPENALTY:
		case SC_FOGWALL:
		case SC_REVERSEORCISH:
		case SC_GRAVITATION_USER:
		case SC_BLADESTOP_WAIT:			/* ���n���(�҂�) */
		case SC_SAFETYWALL:			/* �Z�[�t�e�B�E�H�[�� */
		case SC_PNEUMA:				/* �j���[�} */
		case SC_KEEPING:
		case SC_BARRIER:
		case SC_AURABLADE:			/* �I�[���u���[�h */
		case SC_HEADCRUSH:			/* �w�b�h�N���b�V�� */
		case SC_MELTDOWN:			/* �����g�_�E�� */
		case SC_SPLASHER:			/* �x�i���X�v���b�V���[ */
		case SC_GOSPEL:				/* �S�X�y�� */
		case SC_STATUS_UNCHANGE:		/* �S��Ԉُ�ϐ� */
		case SC_INCDAMAGE:			/* ��_���[�W%�㏸ */
		case SC_PRESERVE:			/* �v���U�[�u */
		case SC_REGENERATION:			/* ���� */
		case SC_BATTLEORDER_DELAY:
		case SC_REGENERATION_DELAY:
		case SC_RESTORE_DELAY:
		case SC_EMERGENCYCALL_DELAY:
		case SC_MEAL_INCEXP:
		case SC_MEAL_INCJOB:
		case SC_COMBATHAN:			/* �퓬���� */
		case SC_LIFEINSURANCE:			/* �����ی��� */
		case SC_JOB_COMBATHAN:		/* JOB���� */
		case SC_FORCEWALKING:
		case SC_TKCOMBO:			/* �e�R���R���{ */
		case SC_TRIPLEATTACK_RATE_UP:
		case SC_COUNTER_RATE_UP:
		case SC_WARM:				/* ������ */
		case SC_KAIZEL:				/* �J�C�[�� */
		case SC_KAAHI:				/* �J�A�q */
		case SC_SMA:				/* �G�X�} */
		case SC_MIRACLE:			/* ���z�ƌ��Ɛ��̊�� */
		case SC_ANGEL:				/* ���z�ƌ��Ɛ��̓V�g */
		case SC_BABY:				/* �p�p�A�}�}�A��D�� */
		case SC_DODGE:
		case SC_DODGE_DELAY:
		case SC_DOUBLECASTING:			/* �_�u���L���X�e�B���O */
		case SC_SHRINK:				/* �V�������N */
		case SC_TIGEREYE:
		case SC_PK_PENALTY:
		case SC_HERMODE:
		case SC_TATAMIGAESHI:			/* ���Ԃ� */
		case SC_NPC_DEFENDER:
		case SC_SLOWCAST:			/* �X���E�L���X�g */
		case SC_CRITICALWOUND:			/* �v���� */
		case SC_MAGICMIRROR:			/* �}�W�b�N�~���[ */
		case SC_ITEMDROPRATE:			/* �o�u���K�� */
		case SC_HAPPY:				/* �y������� */
		case SC_NATURAL_HEAL_STOP:		/* ���R�񕜒�~ */
		case SC_REBIRTH:			/* ���o�[�X */
		case SC_HELLPOWER:			/* �w���p���[ */
		case SC_MANU_ATK:			/* �}�k�N�t�B�[���hMOB(�����_���[�W�㏸) */
		case SC_MANU_DEF:			/* �}�k�N�t�B�[���hMOB(�_���[�W����) */
		case SC_MANU_MATK:			/* �}�k�N�t�B�[���hMOB(���@�_���[�W�㏸) */
		case SC_SPL_ATK:			/* �X�v�����e�B�b�h�t�B�[���hMOB(�����_���[�W�㏸) */
		case SC_SPL_DEF:			/* �X�v�����e�B�b�h�t�B�[���hMOB(�_���[�W����) */
		case SC_SPL_MATK:			/* �X�v�����e�B�b�h�t�B�[���hMOB(���@�_���[�W����) */
		case SC_ENCHANTBLADE:		/* �G���`�����g�u���C�h */
		case SC_NAUTHIZ:			/* ���t���b�V�� */
		case SC_ISHA:				/* �o�C�^���e�B�A�N�e�B�x�[�V���� */
		case SC_WEAPONBLOCKING2:	/* �E�F�|���u���b�L���O�i�u���b�N�j */
		case SC_WHITEIMPRISON:		/* �z���C�g�C���v���Y�� */
		case SC_RECOGNIZEDSPELL:	/* ���S�O�i�C�Y�h�X�y�� */
		case SC_STASIS:				/* �X�e�C�V�X */
		case SC_SPELLBOOK:			/* �X�y���u�b�N */
		case SC_WUGBITE:			/* �E�H�[�O�o�C�g */
		case SC_HOVERING:			/* �z�o�[�����O */
		case SC_STEALTHFIELD:		/* �X�e���X�t�B�[���h */
		case SC__DEADLYINFECT:		/* �f�b�h���[�C���t�F�N�g */
		case SC__IGNORANCE:			/* �}�X�J���[�h �F �C�O�m�A�����X */
		case SC__MANHOLE:			/* �}���z�[�� */
		case SC_FALLENEMPIRE:		/* ��Z���x */
		case SC_CRESCENTELBOW:		/* �j�ꒌ */
		case SC_CURSEDCIRCLE_USER:	/* �����w(�g�p��) */
		case SC_CURSEDCIRCLE:		/* �����w */
		case SC_LIGHTNINGWALK:		/* �M�d�� */
		case SC_NETHERWORLD:		/* �n���̉� */
		case SC_MELODYOFSINK:		/* �����f�B�[�I�u�V���N */
		case SC_BEYOND_OF_WARCRY:	/* �r�����h�I�u�E�H�[�N���C */
		case SC_UNLIMITED_HUMMING_VOICE:	/* �G���h���X�n�~���O�{�C�X */
		case SC_WARMER:				/* �E�H�[�}�[ */
		case SC_VACUUM_EXTREME:		/* �o�L���[���G�N�X�g���[�� */
		case SC_THORNS_TRAP:		/* �\�[���g���b�v */
		case SC_SPORE_EXPLOSION:	/* �X�|�A�G�N�X�v���[�W���� */
		case SC_KG_KAGEHUMI:		/* ���p -�e����- */
		case SC_KYOMU:				/* ���p -�����̉e- */
		case SC_AKAITSUKI:			/* ���p -�g��- */
		case SC_KO_ZENKAI:			/* �p�� -�W�J- */
		case SC_KO_JYUMONJIKIRI:	/* �\�����a�� */
		case SC_CIRCLE_OF_FIRE:		/* �T�[�N���I�u�t�@�C�A */
		case SC_TIDAL_WEAPON:		/* �^�C�_���E�F�|�� */
		case SC_JP_EVENT01:
		case SC_JP_EVENT02:
		case SC_JP_EVENT03:
		case SC_JP_EVENT04:
		case SC_RAID:
		case SC_PHI_DEMON:			/* �Ñ㐸��̂���� */
			break;

		case SC_CONCENTRATE:			/* �W���͌��� */
		case SC_BLESSING:			/* �u���b�V���O */
		case SC_ANGELUS:			/* �A���[���X */
		case SC_RESISTWATER:
		case SC_RESISTGROUND:
		case SC_RESISTFIRE:
		case SC_RESISTWIND:
		case SC_RESISTPOISON:
		case SC_RESISTHOLY:
		case SC_RESISTDARK:
		case SC_RESISTTELEKINESIS:
		case SC_RESISTUNDEAD:
		case SC_RESISTALL:
		case SC_IMPOSITIO:			/* �C���|�V�e�B�I�}�k�X */
		case SC_GLORIA:				/* �O�����A */
		case SC_LOUD:				/* ���E�h�{�C�X */
		case SC_MINDBREAKER:			/* �}�C���h�u���[�J�[ */
		case SC_ETERNALCHAOS:			/* �G�^�[�i���J�I�X */
		case SC_WHISTLE:			/* ���J */
		case SC_ASSNCROS:			/* �[�z�̃A�T�V���N���X */
		case SC_APPLEIDUN:			/* �C�h�D���̗ь� */
		case SC_SANTA:
		case SC_SUMMER:
		case SC_TRUESIGHT:			/* �g�D���[�T�C�g */
		case SC_SPIDERWEB:			/* �X�p�C�_�[�E�F�b�u */
		case SC_CONCENTRATION:			/* �R���Z���g���[�V���� */
		case SC_MARIONETTE:			/* �}���I�l�b�g�R���g���[�� */
		case SC_MARIONETTE2:			/* �}���I�l�b�g�R���g���[�� */
		case SC_WEDDING:			/* �����p(�����ߏւɂȂ��ĕ����̂��x���Ƃ�) */
		case SC_HUMMING:			/* �n�~���O */
		case SC_FORTUNE:			/* �K�^�̃L�X */
		case SC_SERVICE4U:			/* �T�[�r�X�t�H�[���[ */
		case SC_WHISTLE_:			/* ���J */
		case SC_ASSNCROS_:			/* �[�z�̃A�T�V���N���X */
		case SC_APPLEIDUN_:			/* �C�h�D���̗ь� */
		case SC_HUMMING_:			/* �n�~���O */
		case SC_DONTFORGETME_:			/* ����Y��Ȃ��� */
		case SC_FORTUNE_:			/* �K�^�̃L�X */
		case SC_SERVICE4U_:			/* �T�[�r�X�t�H�[���[ */
		case SC_INCATK:				/* ATK�㏸ (�_��p) */
		case SC_INCMATK:			/* MATK�㏸ (�_��̑��p) */
		case SC_INCHIT:				/* HIT�㏸ */
		case SC_INCMHP2:			/* MHP%�㏸ */
		case SC_INCMSP2:			/* MSP%�㏸ */
		case SC_INCATK2:			/* ATK%�㏸ */
		case SC_INCHIT2:			/* HIT%�㏸ */
		case SC_INCFLEE2:			/* FLEE%�㏸ */
		case SC_INCALLSTATUS:			/* �S�X�e�[�^�X�{20 */
		case SC_CHASEWALK_STR:			/* STR�㏸ */
		case SC_BATTLEORDER:			/* �Ր�Ԑ� */
		case SC_THE_MAGICIAN:
		case SC_STRENGTH:
		case SC_THE_DEVIL:
		case SC_THE_SUN:
		case SC_SPURT:				/* �삯���pSTR */
		case SC_SUN_COMFORT:			/* ���z�̈��y */
		case SC_MOON_COMFORT:			/* ���̈��y */
		case SC_STAR_COMFORT:			/* ���̈��y */
		case SC_FUSION:				/* ���z�ƌ��Ɛ��̗Z�� */
		case SC_MEAL_INCHIT:	// �H���p
		case SC_MEAL_INCFLEE:
		case SC_MEAL_INCFLEE2:
		case SC_MEAL_INCCRITICAL:
		case SC_MEAL_INCDEF:
		case SC_MEAL_INCMDEF:
		case SC_MEAL_INCATK:
		case SC_MEAL_INCMATK:
		case SC_SKE:				/* �G�X�N */
		case SC_SKA:				/* �G�X�J */
		case SC_CLOSECONFINE:			/* �N���[�Y�R���t�@�C�� */
		case SC_STOP:				/* �z�[���h�E�F�u */
		case SC_DISARM:				/* �f�B�X�A�[�� */
		case SC_FLING:				/* �t���C���O */
		case SC_MADNESSCANCEL:			/* �}�b�h�l�X�L�����Z���[ */
		case SC_ADJUSTMENT:			/* �A�W���X�g�����g */
		case SC_INCREASING:			/* �C���N���[�W���O�A�L���A���V�[ */
		case SC_FULLBUSTER:			/* �t���o�X�^�[ */
		case SC_NEN:				/* �O */
		case SC_AVOID:				/* �ً}��� */
		case SC_CHANGE:				/* �����^���`�F���W */
		case SC_DEFENCE:			/* �f�B�t�F���X */
		case SC_BLOODLUST:			/* �u���b�h���X�g */
		case SC_FLEET:				/* �t���[�g���[�u */
		case SC_SPEED:				/* �I�[�o�[�h�X�s�[�h */
		case SC_STONESKIN:			/* �X�g�[���X�L�� */
		case SC_ANTIMAGIC:			/* �A���`�}�W�b�N */
		case SC_WEAPONQUICKEN:			/* �E�F�|���N�C�b�P�� */
		case SC_WE_FEMALE:			/* ���Ȃ��ɐs�����܂� */
		case SC_TURISUSS:			/* �W���C�A���g�O���[�X */
		case SC_HALLUCINATIONWALK2:	/* �n���V�l�[�V�����E�H�[�N(�y�i���e�B) */
		case SC_INFRAREDSCAN:		/* �C���t�����b�h�X�L���� */
		case SC_ANALYZE:			/* �A�i���C�Y */
		case SC_NEUTRALBARRIER:		/* �j���[�g�����o���A�[ */
		case SC__BODYPAINT:			/* �{�f�B�y�C���e�B���O */
		case SC__ENERVATION:		/* �}�X�J���[�h �F �G�i�[�x�[�V���� */
		case SC__UNLUCKY:			/* �}�X�J���[�h �F �A�����b�L�[ */
		case SC__WEAKNESS:			/* �}�X�J���[�h �F �E�B�[�N�l�X */
		case SC__STRIPACCESSARY:	/* �X�g���b�v�A�N�Z�T���[ */
		case SC__BLOODYLUST:		/* �u���b�f�B���X�g */
		case SC_EARTHDRIVE:			/* �A�[�X�h���C�u */
		case SC_HARMONIZE:			/* �n�[���i�C�Y */
		case SC_GLOOMYDAY:			/* �������R���[ */
		case SC_LERADS_DEW:			/* ���[���Y�̘I */
		case SC_DANCE_WITH_WUG:		/* �_���X�E�B�Y�E�H�[�O */
		case SC_MYSTERIOUS_POWDER:	/* �s�v�c�ȕ� */
		case SC_BOOST500:			/* �u�[�X�g500 */
		case SC_FULL_SWING_K:		/* �t���X�C���OK */
		case SC_MANA_PLUS:			/* �}�i�v���X */
		case SC_MUSTLE_M:			/* �}�b�X��M */
		case SC_LIFE_FORCE_F:		/* ���C�t�t�H�[�XF */
		case SC_PROMOTE_HEALTH_RESERCH:	/* HP�����|�[�V���� */
		case SC_ENERGY_DRINK_RESERCH:	/* SP�����|�[�V���� */
		case SC_EXTRACT_WHITE_POTION_Z:	/* �Z�k�z���C�g�|�[�V����Z */
		case SC_VITATA_500:			/* �r�^�^500 */
		case SC_EXTRACT_SALAMINE_JUICE:	/* �Z�k�T���}�C���W���[�X */
		case SC_SAVAGE_STEAK:		/* �T�x�[�W�̊ۏĂ� */
		case SC_COCKTAIL_WARG_BLOOD:	/* �J�N�e���E�H�[�O�u���b�h */
		case SC_MINOR_BBQ:			/* �~�m�^�E���X�̋��J���r */
		case SC_SIROMA_ICE_TEA:		/* �V���}�A�C�X�e�B�[ */
		case SC_DROCERA_HERB_STEAMED:	/* �h���Z���̃n�[�u�� */
		case SC_PUTTI_TAILS_NOODLES:	/* �v�e�B�b�g�̂����ۖ� */
		case SC_STOMACHACHE:		/* ���� */
		case SC_ODINS_POWER:		/* �I�[�f�B���̗� */
		case SC_ZEPHYR:				/* �[�t�@�[ */
		case SC_INVINCIBLE:			/* �C���r���V�u�� */
		case SC_INVINCIBLEOFF:		/* �C���r���V�u���I�t */
		case SC_ATKPOTION:
		case SC_MATKPOTION:
		case SC_ALMIGHTY:
			calc_flag = 1;
			break;

		case SC_MONSTER_TRANSFORM:	/* �����X�^�[�ϐg */
			icon_val1 = val1;	// val1�̓����X�^�[ID
			calc_flag = 1;
			break;
		case SC_ALL_RIDING:			/* �R��V�X�e�� */
			if(sd) {
				// ���Ɋ����̏�蕨�ɓ��撆�ł���ꍇ�͉������Ȃ�
				if(pc_isriding(sd) || pc_isdragon(sd) || pc_iswolfmount(sd) || pc_isgear(sd))
					return 0;
				icon_val1 = 1;	// val1��1
				icon_val2 = 25;	// val2��25(�ړ����x����)
				clif_status_load_id(sd,SI_RIDING,1);
			}
			calc_flag = 1;
			ud->state.change_speed = 1;
			break;
		case SC_KYOUGAKU:			/* ���p -����- */
			icon_val1 = 1002;
			calc_flag = 1;
			break;
		case SC_HAT_EFFECT:			/* �������G�t�F�N�g */
			icon_val1 = val1;
			break;
		case SC_ACTIVE_MONSTER_TRANSFORM:	/* �A�N�e�B�u�����X�^�[�ϐg */
			icon_val1 = val1;	// val1�̓����X�^�[ID
			break;

		case SC_SPEEDUP0:			/* �ړ����x����(�A�C�e��) */
		case SC_SPEEDUP1:			/* �X�s�[�h�|�[�V���� */
		case SC_WALKSPEED:			/* �ړ����x����(�X�N���v�g) */
		case SC_SLOWPOTION:			/* �ړ����x�ቺ(�A�C�e��) */
		case SC_STEELBODY:			/* ���� */
		case SC_INCFLEE:			/* FLEE�㏸ */
		case SC_GRAVITATION:			/* �O���r�e�[�V�����t�B�[���h */
		case SC_SUITON:				/* ���� */
		case SC_GATLINGFEVER:			/* �K�g�����O�t�B�[�o�[ */
		case SC_HALLUCINATIONWALK:	/* �n���V�l�[�V�����E�H�[�N */
		case SC_PARALIZE:			/* �p�����C�Y */
		case SC_FROSTMISTY:			/* �t���X�g�~�X�e�B */
		case SC_WUGDASH:			/* �E�H�[�O�_�b�V�� */
		case SC_ACCELERATION:		/* �A�N�Z�����[�V���� */
		case SC_NEUTRALBARRIER_USER:	/* �j���[�g�����o���A�[(�g�p��) */
		case SC__GROOMY:			/* �}�X�J���[�h �F �O���[�~�[ */
		case SC__LAZINESS:			/* �}�X�J���[�h �F ���C�W�[�l�X */
		case SC_GN_CARTBOOST:		/* �J�[�g�u�[�X�g */
		case SC_MELON_BOMB:			/* ���������e */
		case SC_POWER_OF_GAIA:		/* �p���[�I�u�K�C�A */
			calc_flag = 1;
			ud->state.change_speed = 1;
			break;

		case SC_PROVOKE:			/* �v���{�b�N */
			calc_flag = 1;
			if(tick <= 0) tick = 1000;	/* (�I�[�g�o�[�T�[�N) */
			break;
		case SC_ENDURE:				/* �C���f���A */
			if(tick <= 0) tick = 1000 * 60;
			calc_flag = 1;
			val2 = 7;	// 7��U�����ꂽ�����
			if(sd && !map[bl->m].flag.gvg) {
				// ��f�B�{�[�V�����҂��C���f���A�ɂ���
				struct map_session_data *tsd;
				int i;
				for(i = 0; i < 5; i++) {
					if(sd->dev.val1[i] && (tsd = map_id2sd(sd->dev.val1[i])) != NULL)
						status_change_start(&tsd->bl,type,val1,0,0,0,skill_get_time(SM_ENDURE,val1),0);
				}
			}
			break;
		case SC_INCREASEAGI:		/* ���x���� */
			calc_flag = 1;
			ud->state.change_speed = 1;
			if(sc->data[SC_DECREASEAGI].timer != -1)
				status_change_end(bl,SC_DECREASEAGI,-1);
			break;
		case SC_DECREASEAGI:		/* ���x���� */
			calc_flag = 1;
			ud->state.change_speed = 1;
			break;
		case SC_SIGNUMCRUCIS:		/* �V�O�i���N���V�X */
			calc_flag = 1;
			val2 = 10 + val1*4;
			tick = 600*1000;
			clif_emotion(bl,4);
			break;
		case SC_SLOWPOISON:
			if (sc->data[SC_POISON].timer == -1 && sc->data[SC_DPOISON].timer == -1)
				return 0;
			break;
		case SC_ONEHAND:			/* 1HQ */
			if(sc->data[SC_SPEEDPOTION0].timer != -1)
				status_change_end(bl,SC_SPEEDPOTION0,-1);
			if(sc->data[SC_SPEEDPOTION1].timer != -1)
				status_change_end(bl,SC_SPEEDPOTION1,-1);
			if(sc->data[SC_SPEEDPOTION2].timer != -1)
				status_change_end(bl,SC_SPEEDPOTION2,-1);
			calc_flag = 1;
			break;
		case SC_ADRENALINE:			/* �A�h���i�������b�V�� */
			calc_flag = 1;
			if(sc->data[SC_ADRENALINE2].timer != -1)
				status_change_end(bl,SC_ADRENALINE2,-1);
			if(!(flag&2) && sd && pc_checkskill(sd,BS_HILTBINDING) > 0)
				icon_tick = tick += tick / 10;
			break;
		case SC_ADRENALINE2:			/* �t���A�h���i�������b�V�� */
			calc_flag = 1;
			if(sc->data[SC_ADRENALINE].timer != -1)
				status_change_end(bl,SC_ADRENALINE,-1);
			if(!(flag&2) && sd && pc_checkskill(sd,BS_HILTBINDING) > 0)
				icon_tick = tick += tick / 10;
			break;
		case SC_WEAPONPERFECTION:		/* �E�F�|���p�[�t�F�N�V���� */
			if(!(flag&2) && sd && pc_checkskill(sd,BS_HILTBINDING) > 0)
				icon_tick = tick += tick / 10;
			break;
		case SC_OVERTHRUST:			/* �I�[�o�[�g���X�g */
			if(sc->data[SC_OVERTHRUSTMAX].timer != -1)
				return 0;
			val3 = (val2)? val1*5 : val1;
			if(!(flag&2) && sd && pc_checkskill(sd,BS_HILTBINDING) > 0)
				icon_tick = tick += tick / 10;
			break;
		case SC_MAXIMIZEPOWER:		/* �}�L�V�}�C�Y�p���[ */
			if(sd)
				tick = val2;	// SP��1���鎞��
			else
				tick = 5000*val1;
			break;
		case SC_ENCPOISON:			/* �G���`�����g�|�C�Y�� */
			calc_flag = 1;
			val2 = (((val1 - 1) / 2) + 3)*100;	// �ŕt�^�m��
			status_enchant_elemental_end(bl,SC_ENCPOISON);
			break;
		case SC_EDP:			/* �G���`�����g�f�b�h���[�|�C�Y�� */
			val2 = 250 + val1 * 50;	// �ғŕt�^�m��(%)
			break;
		case SC_POISONREACT:	/* �|�C�Y�����A�N�g */
			val2 = (val1 + 1) / 2;
			break;
		case SC_ASPERSIO:			/* �A�X�y���V�I */
			status_enchant_elemental_end(bl,SC_ASPERSIO);
			break;
		case SC_BENEDICTIO:			/* ���� */
			status_enchant_armor_elemental_end(bl,SC_BENEDICTIO);
			break;
		case SC_ELEMENTWATER:		// ��
			status_enchant_armor_elemental_end(bl,SC_ELEMENTWATER);
			if(sd) {
				// �h��ɐ��������t�^����܂����B
				clif_displaymessage(sd->fd, msg_txt(195));
			}
			break;
		case SC_ELEMENTGROUND:		// �y
			status_enchant_armor_elemental_end(bl,SC_ELEMENTGROUND);
			if(sd) {
				// �h��ɓy�������t�^����܂����B
				clif_displaymessage(sd->fd, msg_txt(196));
			}
			break;
		case SC_ELEMENTFIRE:		// ��
			status_enchant_armor_elemental_end(bl,SC_ELEMENTFIRE);
			if(sd) {
				// �h��ɉΑ������t�^����܂����B
				clif_displaymessage(sd->fd, msg_txt(197));
			}
			break;
		case SC_ELEMENTWIND:		// ��
			status_enchant_armor_elemental_end(bl,SC_ELEMENTWIND);
			if(sd) {
				// �h��ɕ��������t�^����܂����B
				clif_displaymessage(sd->fd, msg_txt(198));
			}
			break;
		case SC_ELEMENTHOLY:		// ��
			status_enchant_armor_elemental_end(bl,SC_ELEMENTHOLY);
			if(sd) {
				// �h��ɐ��������t�^����܂����B
				clif_displaymessage(sd->fd, msg_txt(199));
			}
			break;
		case SC_ELEMENTDARK:		// ��
			status_enchant_armor_elemental_end(bl,SC_ELEMENTDARK);
			if(sd) {
				// �h��Ɉő������t�^����܂����B
				clif_displaymessage(sd->fd, msg_txt(200));
			}
			break;
		case SC_ELEMENTELEKINESIS:	// �O
			status_enchant_armor_elemental_end(bl,SC_ELEMENTELEKINESIS);
			if(sd) {
				// �h��ɔO�������t�^����܂����B
				clif_displaymessage(sd->fd, msg_txt(201));
			}
			break;
		case SC_ELEMENTPOISON:		// ��
			status_enchant_armor_elemental_end(bl,SC_ELEMENTPOISON);
			if(sd) {
				// �h��ɓő������t�^����܂����B
				clif_displaymessage(sd->fd, msg_txt(202));
			}
			break;
		case SC_ELEMENTUNDEAD:		// �s��
			status_enchant_armor_elemental_end(bl,SC_ELEMENTUNDEAD);
			if(sc->data[SC_BLESSING].timer != -1)
				status_change_end(bl,SC_BLESSING,-1);
			if(sc->data[SC_INCREASEAGI].timer != -1)
				status_change_end(bl,SC_INCREASEAGI,-1);
			break;
		case SC_RACEUNKNOWN:
		case SC_RACEUNDEAD:
		case SC_RACEBEAST:
		case SC_RACEPLANT:
		case SC_RACEINSECT:
		case SC_RACEFISH:
		case SC_RACEDEVIL:
		case SC_RACEHUMAN:
		case SC_RACEANGEL:
		case SC_RACEDRAGON:
			status_change_race_end(bl,type);
			if(sd) {
				const char *race_name[] = { "���`", "�s��", "����", "�A��", "����", "", "���L", "����", "�l��", "�V�g", "��" };
				msg_output(sd->fd, msg_txt(204), race_name[type-SC_RACEUNKNOWN]);	// �푰��%s�ɂȂ�܂���
			}
			break;
		case SC_MAGICROD:
			val2 = val1*20;
			break;
		case SC_KYRIE:				/* �L���G�G���C�\�� */
			// �A�X�����|�����Ă������������
			if(sc->data[SC_ASSUMPTIO].timer != -1)
				status_change_end(bl,SC_ASSUMPTIO,-1);
			if(sc->data[SC_ASSUMPTIO2].timer != -1)
				status_change_end(bl,SC_ASSUMPTIO2,-1);
			// �L���G���|����
			if(val4 == AB_PRAEFATIO) {		// �v���G�t�@�e�B�I�̏ꍇ
				val2 = (int)((atn_bignumber)status_get_max_hp(bl) * (val1 * 2 + 16) / 100);	// �ϋv�x
				val3 = val1 + 6;	// ��
			} else {
				val2 = (int)((atn_bignumber)status_get_max_hp(bl) * (val1 * 2 + 10) / 100);	// �ϋv�x
				val3 = val1 / 2 + 5;	// ��
			}
			break;
		case SC_QUAGMIRE:			/* �N�@�O�}�C�A */
			calc_flag = 1;
			ud->state.change_speed = 1;
			if(sc->data[SC_CONCENTRATE].timer != -1)
				status_change_end(bl,SC_CONCENTRATE,-1);
			if(sc->data[SC_INCREASEAGI].timer != -1)
				status_change_end(bl,SC_INCREASEAGI,-1);
			if(sc->data[SC_TWOHANDQUICKEN].timer != -1)
				status_change_end(bl,SC_TWOHANDQUICKEN,-1);
			if(sc->data[SC_SPEARQUICKEN].timer != -1)
				status_change_end(bl,SC_SPEARQUICKEN,-1);
			if(sc->data[SC_ADRENALINE].timer != -1)
				status_change_end(bl,SC_ADRENALINE,-1);
			if(sc->data[SC_ADRENALINE2].timer != -1)
				status_change_end(bl,SC_ADRENALINE2,-1);
			if(sc->data[SC_LOUD].timer != -1)
				status_change_end(bl,SC_LOUD,-1);
			if(sc->data[SC_WINDWALK].timer != -1)
				status_change_end(bl,SC_WINDWALK,-1);
			if(sc->data[SC_CARTBOOST].timer != -1)
				status_change_end(bl,SC_CARTBOOST,-1);
			if(sc->data[SC_ONEHAND].timer != -1)
				status_change_end(bl,SC_ONEHAND,-1);
			if(sc->data[SC_SPEEDUP1].timer != -1)
				status_change_end(bl,SC_SPEEDUP1,-1);
			break;
		case SC_MAGICPOWER:			/* ���@�͑��� */
			val2 = 1;	// ��x��������
			break;
		case SC_SACRIFICE:			/* �T�N���t�@�C�X */
			val2 = 5;	// 5��̍U���ŗL��
			break;
		case SC_FLAMELAUNCHER:		/* �t���[�������`���[ */
		case SC_FROSTWEAPON:		/* �t���X�g�E�F�|�� */
		case SC_LIGHTNINGLOADER:	/* ���C�g�j���O���[�_�[ */
		case SC_SEISMICWEAPON:		/* �T�C�Y�~�b�N�E�F�|�� */
		case SC_DARKELEMENT:		/* �ő��� */
		case SC_ATTENELEMENT:		/* �O���� */
		case SC_UNDEADELEMENT:		/* �s������ */
			status_enchant_elemental_end(bl,type);
			break;
		case SC_SEVENWIND:			/* �g������ */
			status_enchant_elemental_end(bl,SC_ENCPOISON);	// �G���`�����g�|�C�Y���͏d�����Ă��悢�H
#ifdef PRE_RENEWAL
			switch(val1) {
				case 1:  val2 = SI_SEISMICWEAPON;   val3 = ELE_EARTH; break;
				case 2:  val2 = SI_LIGHTNINGLOADER; val3 = ELE_WIND;  break;
				case 3:  val2 = SI_FROSTWEAPON;     val3 = ELE_WATER; break;
				case 4:  val2 = SI_FLAMELAUNCHER;   val3 = ELE_FIRE;  break;
				case 5:  val2 = SI_ATTENELEMENT;    val3 = ELE_GHOST; break;
				default: val2 = SI_DARKELEMENT;     val3 = ELE_DARK;  break;
			}
#else
			switch(val1) {
				case 1:  val2 = SI_SEISMICWEAPON;   val3 = ELE_EARTH; break;
				case 2:  val2 = SI_LIGHTNINGLOADER; val3 = ELE_WIND;  break;
				case 3:  val2 = SI_FROSTWEAPON;     val3 = ELE_WATER; break;
				case 4:  val2 = SI_FLAMELAUNCHER;   val3 = ELE_FIRE;  break;
				case 5:  val2 = SI_ATTENELEMENT;    val3 = ELE_GHOST; break;
				case 6:  val2 = SI_DARKELEMENT;     val3 = ELE_DARK;  break;
				default: val2 = SI_ASPERSIO;        val3 = ELE_HOLY;  break;
			}
#endif
			if(sd) {
				clif_status_change(bl,val2,1,icon_tick,0,0,0);
			}
			break;
		case SC_PROVIDENCE:			/* �v�����B�f���X */
			calc_flag = 1;
			val2 = val1*5;
			break;
		case SC_REFLECTSHIELD:
			// ���t���N�g�_���[�W���|�����Ă��������
			if(sc->data[SC_REFLECTDAMAGE].timer != -1)
				status_change_end(bl,SC_REFLECTDAMAGE,-1);
			val2 = 10+val1*3;
			if(sd) {
				// ��f�B�{�[�V�����҂����t���N�g�V�[���h�ɂ���
				struct map_session_data *tsd;
				int i;
				for(i = 0; i < 5; i++) {
					if(sd->dev.val1[i] && (tsd = map_id2sd(sd->dev.val1[i])) != NULL) {
						status_change_start(&tsd->bl,type,val1,0,0,0,skill_get_time(CR_REFLECTSHIELD,val1),0);
					}
				}
			}
			break;
		case SC_AUTOSPELL:			/* �I�[�g�X�y�� */
			val4 = 5 + val1*2;
			break;
		case SC_VOLCANO:
			calc_flag = 1;
			val3 = val1*10;
			val4 = (val1 > 5)? 20: val1*(11-val1)/2 + 5;
			break;
		case SC_DELUGE:
			calc_flag = 1;
			val3 = (val1 > 5)? 15: val1*(11-val1)/2;
			val4 = (val1 > 5)? 20: val1*(11-val1)/2 + 5;
			break;
		case SC_VIOLENTGALE:
			calc_flag = 1;
			val3 = val1*3;
			val4 = (val1 > 5)? 20: val1*(11-val1)/2 + 5;
			break;
		case SC_TWOHANDQUICKEN:			/* 2HQ */
			calc_flag = 1;
			if(bl->type == BL_MOB && battle_config.monster_skill_over && val1 >= battle_config.monster_skill_over)
				val2 = 50;
			else
				val2 = 30;
			break;
		case SC_SPEARQUICKEN:		/* �X�s�A�N�C�b�P�� */
			calc_flag = 1;
#ifdef PRE_RENEWAL
			val2 = 20+val1;
#else
			val2 = 30;
#endif
			break;
		case SC_BLADESTOP:		/* ���n��� */
			if(val2 == 2)
				clif_bladestop(map_id2bl(val3),val4,1);
			break;
		case SC_LULLABY:			/* �q��S */
			val2 = 11;
			break;
		case SC_DRUMBATTLE:			/* �푾�ۂ̋��� */
			calc_flag = 1;
#ifdef PRE_RENEWAL
			val2 = (val1+1)*25;
			val3 = (val1+1)*2;
#else
			val2 = 100+val1*30;
			val3 = val1*10;
#endif
			break;
		case SC_NIBELUNGEN:			/* �j�[�x�����O�̎w�� */
			calc_flag = 1;
#ifdef PRE_RENEWAL
			val2 = (val1+2)*25;
#else
			val2 = val1*40;
#endif
			break;
		case SC_SIEGFRIED:			/* �s���g�̃W�[�N�t���[�h */
			calc_flag = 1;
			val2 = 5 + val1*15;
			break;
		case SC_DISSONANCE:			/* �s���a�� */
			val2 = 10;
			break;
		case SC_UGLYDANCE:			/* ��������ȃ_���X */
			val2 = 10;
			break;
		case SC_DONTFORGETME:		/* ����Y��Ȃ��� */
			calc_flag = 1;
			ud->state.change_speed = 1;
			if(sc->data[SC_INCREASEAGI].timer != -1)
				status_change_end(bl,SC_INCREASEAGI,-1);
			if(sc->data[SC_TWOHANDQUICKEN].timer != -1)
				status_change_end(bl,SC_TWOHANDQUICKEN,-1);
			if(sc->data[SC_SPEARQUICKEN].timer != -1)
				status_change_end(bl,SC_SPEARQUICKEN,-1);
			if(sc->data[SC_ADRENALINE].timer != -1)
				status_change_end(bl,SC_ADRENALINE,-1);
			if(sc->data[SC_ADRENALINE2].timer != -1)
				status_change_end(bl,SC_ADRENALINE2,-1);
			if(sc->data[SC_ASSNCROS].timer != -1)
				status_change_end(bl,SC_ASSNCROS,-1);
			if(sc->data[SC_WINDWALK].timer != -1)
				status_change_end(bl,SC_WINDWALK,-1);
			if(sc->data[SC_CARTBOOST].timer != -1)
				status_change_end(bl,SC_CARTBOOST,-1);
			if(sc->data[SC_ONEHAND].timer != -1)
				status_change_end(bl,SC_ONEHAND,-1);
			break;
		case SC_LONGINGFREEDOM:		/* �����S�����Ȃ��� */
			calc_flag = 1;
			val3 = 1;
			tick = 1000;
			break;
		case SC_DANCING:			/* �_���X/���t�� */
			calc_flag = 1;
			ud->state.change_speed = 1;
			val3 = tick / 1000;
			tick = 1000;
			break;
		case SC_EXPLOSIONSPIRITS:	/* �����g�� */
			calc_flag = 1;
			val2 = 75 + 25*val1;
			break;
		case SC_AUTOCOUNTER:
			val3 = 0;
			val4 = 0;
			break;
		case SC_SPEEDPOTION0:		/* �����|�[�V���� */
		case SC_SPEEDPOTION1:
		case SC_SPEEDPOTION2:
			calc_flag = 1;
			val2 = 5*(2+type-SC_SPEEDPOTION0);
			break;
		case SC_NOCHAT:		/* �`���b�g�֎~��� */
			{
				time_t timer;
				tick = 60000;
				if(!val2)
					val2 = (int)time(&timer);
				if(sd)
					clif_updatestatus(sd,SP_MANNER);	// �X�e�[�^�X���N���C�A���g�ɑ���
			}
			break;
		case SC_SELFDESTRUCTION:	/* ���� */
			tick = 100;
			break;

		/* option1 */
		case SC_STONE:				/* �Ή� */
			if(!(flag&2)) {
#ifdef PRE_RENEWAL
				tick += 100 * status_get_luk(bl) - status_get_mdef(bl) * tick / 100;
#else
				tick += 3000 - tick * status_get_mdef(bl) / 100;
#endif
			}
			val3 = tick / 1000;
			if(val3 < 1)
				val3 = 1;
			tick = val4;
			if(tick < 1000)
				tick = 1000;
			val2 = 1;
			break;
		case SC_SLEEP:				/* ���� */
			if(!(flag&2)) {
#ifdef PRE_RENEWAL
				tick += -10 * status_get_luk(bl) - tick * status_get_int(bl) / 100;
#else
				tick += 1000 - tick * status_get_agi(bl) / 100;
#endif
			}
			break;
		case SC_FREEZE:				/* ���� */
			if(!(flag&2)) {
#ifdef PRE_RENEWAL
				tick += 10 * status_get_luk(bl) - tick * status_get_mdef(bl) / 100;
#else
				tick += 3000 - tick * status_get_mdef(bl) / 100;
#endif
			}
			break;
		case SC_STUN:				/* �X�^���ival2�Ƀ~���b�Z�b�g�j */
			if(!(flag&2)) {
#ifdef PRE_RENEWAL
				tick += -10 * status_get_luk(bl) - tick * status_get_vit(bl) / 100;
#else
				tick += 1000 - tick * status_get_vit(bl) / 100;
#endif
			}
			break;
		/* option2 */
		case SC_DPOISON:			/* �ғ� */
			{
				int mhp = status_get_max_hp(bl);
				int hp  = status_get_hp(bl);
				if(!(flag&2)) {
					if(bl->type == BL_PC)	// PC�̏ꍇ�̂݌v�Z�����Ⴄ
						tick = 5000 + -10000 * status_get_luk(bl) / 100 - 45000 * status_get_vit(bl) / 100 + 55000;
					else
						tick = 30000 - 20000 * status_get_vit(bl) / 100;
				}
				// MHP��1/4�ȉ��ɂ͂Ȃ�Ȃ�
				if(hp > mhp>>2) {
					int diff = 0;
					if(sd)
						diff = mhp * 10 / 100;
					else if(md)
						diff = mhp * 15 / 100;
					if(hp - diff < mhp>>2)
						diff = hp - (mhp>>2);
					unit_heal(bl, -diff, 0);
				}
			}
			// fall through
		case SC_POISON:				/* �� */
			calc_flag = 1;
			if(!(flag&2)) {
				if(bl->type == BL_PC)	// PC�̏ꍇ�̂݌v�Z�����Ⴄ
					tick = 5000 + -10000 * status_get_luk(bl) / 100 - 45000 * status_get_vit(bl) / 100 + 55000;
				else
					tick = 30000 - 20000 * status_get_vit(bl) / 100;
			}
			val3 = tick/1000;
			if(val3 < 1) val3 = 1;
			tick = 1000;
			break;
		case SC_SILENCE:			/* ���� */
			skill_stop_dancing(bl,0);
			if (sc->data[SC_GOSPEL].timer != -1) {
				status_change_end(bl,SC_GOSPEL,-1);
				break;
			}
			if(!(flag&2)) {
#ifdef PRE_RENEWAL
				tick += -10 * status_get_luk(bl) - tick * status_get_vit(bl) / 100;
#else
				tick += 1000 - tick * status_get_int(bl) / 100;
#endif
			}
			break;
		case SC_BLIND:				/* �Í� */
			calc_flag = 1;
			if(!(flag&2)) {
#ifdef PRE_RENEWAL
				tick += -10 * status_get_luk(bl) - tick * (status_get_vit(bl) + status_get_int(bl)) / 200;
#else
				tick += 1000 - tick * status_get_int(bl) / 100;
#endif
			}
			break;
		case SC_CURSE:				/* �� */
			calc_flag = 1;
			ud->state.change_speed = 1;
			if(!(flag&2)) {
#ifdef PRE_RENEWAL
				tick += -10 * status_get_luk(bl) - tick * status_get_vit(bl) / 100;
#else
				tick += 1000 - tick * status_get_luk(bl) / 100;
#endif
			}
			break;
		case SC_CONFUSION:			/* ���� */
			if(!(flag&2)) {
#ifdef PRE_RENEWAL
				tick += -10 * status_get_luk(bl) - tick * (status_get_str(bl) + status_get_int(bl)) / 200;
#else
				tick += 1000 - tick * status_get_luk(bl) / 100;
#endif
			}
			break;
		case SC_BLEED:				/* �o�� */
			if(!(flag&2)) {
#ifdef PRE_RENEWAL
				icon_tick = tick = -10 * status_get_luk(bl) - tick * status_get_vit(bl) / 100 + tick;
#else
				icon_tick = tick = 1000 - tick * status_get_agi(bl) / 100 + tick;
#endif
			}
			val3 = (tick < 10000)? 1: tick / 10000;
			tick = 10000;	// �_���[�W������10sec��
			break;

		/* option */
		case SC_HIDING:		/* �n�C�f�B���O */
			calc_flag = 1;
			if(sd) {
				val2 = tick / 1000;		/* �������� */
				tick = 1000;
			}
			break;
		case SC_CHASEWALK:		/* �`�F�C�X�E�H�[�N */
		case SC_CLOAKING:		/* �N���[�L���O */
		case SC_INVISIBLE:		/* �C���r�W�u�� */
			if(sd) {
				calc_flag = 1;
				tick = val2;
			} else {
				tick = 5000*val1;
			}
			break;
		case SC_SIGHTBLASTER:		/* �T�C�g�u���X�^�[ */
		case SC_SIGHT:			/* �T�C�g */
		case SC_RUWACH:			/* ���A�t */
			val2 = tick/250;
			tick = 10;
			break;

		case SC_MODECHANGE:
			tick = 1200;
			break;
		case SC_AUTOGUARD:
			val2 = (val1 > 10)? 30: (22 - val1) * val1 / 4;
			if(sd) {
				// ��f�B�{�[�V�����҂��I�[�g�K�[�h�ɂ���
				struct map_session_data *tsd;
				int i;
				for(i = 0; i < 5; i++) {
					if(sd->dev.val1[i] && (tsd = map_id2sd(sd->dev.val1[i])) != NULL)
						status_change_start(&tsd->bl,type,val1,0,0,0,skill_get_time(CR_AUTOGUARD,val1),0);
				}
			}
			break;
		case SC_DEFENDER:
			calc_flag = 1;
			ud->state.change_speed = 1;
			val2 = 5 + val1*15;
			val3 = 250 - val1*50;
			if(sd) {
				// ��f�B�{�[�V�����҂��f�B�t�F���_�[�ɂ���
				struct map_session_data *tsd;
				int i;
				for(i = 0; i < 5; i++) {
					if(sd->dev.val1[i] && (tsd = map_id2sd(sd->dev.val1[i])) != NULL)
						status_change_start(&tsd->bl,type,val1,0,0,0,skill_get_time(CR_DEFENDER,val1),0);
				}
			}
			break;
		case SC_HALLUCINATION:
			if(sd && !battle_config.hallucianation_off) {
				// on�Ȃ̂ŃA�C�R���\��
				clif_status_change(bl, SI_HALLUCINATION, 1, icon_tick, 0, 0, 0);
			}
			break;
		case SC_TENSIONRELAX:	/* �e���V���������b�N�X */
			if(sd == NULL)
				return 0;
			tick = 10000;
			break;
		case SC_PARRYING:		/* �p���C���O */
			val2 = 20 + val1*3;
			break;
		case SC_JOINTBEAT:		/* �W���C���g�r�[�g */
			calc_flag = 1;
			ud->state.change_speed = 1;
			val4 = atn_rand()%6;
			if(val4 == 5) {
				// ��͋����I�ɏo���t�� �E �g�p�҂̃��x�����擾�ł��Ȃ��̂łƂ肠����0��������
				if(atn_rand() % 10000 < status_change_rate(bl,SC_BLEED,10000,0))
					status_change_start(bl,SC_BLEED,val1,0,0,0,skill_get_time2(LK_JOINTBEAT,val1),10);
			}
			if(!(flag&2)) {
				tick = tick - (status_get_agi(bl)/10 + status_get_luk(bl)/4)*1000;
			}
			break;
		case SC_WINDWALK:		/* �E�C���h�E�H�[�N */
			calc_flag = 1;
			ud->state.change_speed = 1;
			val2 = val1 / 2;	// Flee�㏸��
			break;
		case SC_BERSERK:		/* �o�[�T�[�N */
			unit_heal(bl,0,-status_get_sp(bl));
			if(sd) {
				clif_status_change(bl,SI_INCREASEAGI,1,icon_tick, 0, 0, 0);	// �A�C�R���\��
			}
			tick = 1000;
			calc_flag = 1;
			break;
		case SC_ASSUMPTIO:		/* �A�X���v�e�B�I */
		case SC_ASSUMPTIO2:		/* �L���b�V���A�X���v�e�B�I */
			// �L���G���|�����Ă��������
			if(sc->data[SC_KYRIE].timer != -1)
				status_change_end(bl,SC_KYRIE,-1);
			// �J�C�g���|�����Ă��������
			if(sc->data[SC_KAITE].timer != -1)
				status_change_end(bl,SC_KAITE,-1);
			break;
		case SC_CARTBOOST:		/* �J�[�g�u�[�X�g */
			calc_flag = 1;
			if(sc->data[SC_DECREASEAGI].timer != -1)
				status_change_end(bl,SC_DECREASEAGI,-1);
			break;
		case SC_REJECTSWORD:	/* ���W�F�N�g�\�[�h */
			val2 = 3; // 3��U���𒵂˕Ԃ�
			break;
		case SC_MEMORIZE:		/* �������C�Y */
			val2 = 5; // 5��r����1/2�ɂ���
			break;
		case SC_GRAFFITI:		/* �O���t�B�e�B */
			{
				struct skill_unit_group *sg = skill_unitsetting(bl,RG_GRAFFITI,val1,val2,val3,0);
				if(sg)
					val4 = sg->bl.id;
			}
			break;
		case SC_OVERTHRUSTMAX:		/* �I�[�o�[�g���X�g�}�b�N�X */
			if(sc->data[SC_OVERTHRUST].timer != -1)
				status_change_end(bl,SC_OVERTHRUST,-1);
			calc_flag = 1;
			break;

		case SC_MEAL_INCSTR:	// �H���p
		case SC_MEAL_INCAGI:
		case SC_MEAL_INCVIT:
		case SC_MEAL_INCINT:
		case SC_MEAL_INCDEX:
		case SC_MEAL_INCLUK:
			if(sc->data[type - SC_MEAL_INCSTR + SC_MEAL_INCSTR2].timer != -1) {
				// ���ʂ��Ⴂ�ꍇ�͌��ʂȂ�
				if(val1 < sc->data[type - SC_MEAL_INCSTR + SC_MEAL_INCSTR2].val1)
					return 0;
				// �������������ʂȂ�g�p���̌��ʂ�����
				else
					status_change_end(bl, type - SC_MEAL_INCSTR + SC_MEAL_INCSTR2, -1);
			}
			calc_flag = 1;
			break;
		case SC_MEAL_INCSTR2:	// �ۋ������p
		case SC_MEAL_INCAGI2:
		case SC_MEAL_INCVIT2:
		case SC_MEAL_INCINT2:
		case SC_MEAL_INCDEX2:
		case SC_MEAL_INCLUK2:
			if(sc->data[type - SC_MEAL_INCSTR2 + SC_MEAL_INCSTR].timer != -1) {
				// ���ʂ��Ⴂ�ꍇ�͌��ʂȂ�
				if(val1 < sc->data[type - SC_MEAL_INCSTR2 + SC_MEAL_INCSTR].val1)
					return 0;
				// �������������ʂȂ�g�p���̌��ʂ�����
				else
					status_change_end(bl, type - SC_MEAL_INCSTR2 + SC_MEAL_INCSTR, -1);
			}
			calc_flag = 1;
			break;
		case SC_ELEMENTFIELD:		/* ������ */
			tick = val2;
			break;
		case SC_RUN:			/* �^�C���M */
			val4 = 0;
			calc_flag = 1;
			break;
		case SC_KAUPE:			/* �J�E�v */
			val2 = val1*33;
			if(val1 >= 3)
				val2 = 100;
			break;
		case SC_KAITE:			/* �J�C�g */
			// �A�X�����|�����Ă������������
			if(sc->data[SC_ASSUMPTIO].timer != -1)
				status_change_end(bl,SC_ASSUMPTIO,-1);
			if(sc->data[SC_ASSUMPTIO2].timer != -1)
				status_change_end(bl,SC_ASSUMPTIO2,-1);
			// ���ˉ�
			val2 = val1 / 5 + 1;
			break;
		case SC_SWOO:			/* �G�X�E */
			calc_flag = 1;
			ud->state.change_speed = 1;
			if(status_get_mode(bl)&MD_BOSS)
				tick /= 5;
			break;
		case SC_MONK:			/* �����N�̍� */
		case SC_STAR:			/* �P���Z�C�̍� */
		case SC_SAGE:			/* �Z�[�W�̍� */
		case SC_CRUSADER:		/* �N���Z�C�_�[�̍� */
		case SC_WIZARD:			/* �E�B�U�[�h�̍� */
		case SC_PRIEST:			/* �v���[�X�g�̍� */
		case SC_ROGUE:			/* ���[�O�̍� */
		case SC_ASSASIN:		/* �A�T�V���̍� */
		case SC_SOULLINKER:		/* �\�E�������J�[�̍� */
		case SC_GUNNER:			/* �K���X�����K�[�̍� */
		case SC_NINJA:			/* �E�҂̍� */
		case SC_DEATHKINGHT:		/* �f�X�i�C�g�̍� */
		case SC_COLLECTOR:		/* �R���N�^�[�̍� */
			if(sd && battle_config.disp_job_soul_state_change)
				clif_disp_onlyself(sd->fd, msg_txt(206));	// ����ԂɂȂ�܂���
			break;
		case SC_KNIGHT:			/* �i�C�g�̍� */
		case SC_ALCHEMIST:		/* �A���P�~�X�g�̍� */
		case SC_BARDDANCER:		/* �o�[�h�ƃ_���T�[�̍� */
		case SC_BLACKSMITH:		/* �u���b�N�X�~�X�̍� */
		case SC_HUNTER:			/* �n���^�[�̍� */
		case SC_HIGH:			/* �ꎟ��ʐE�Ƃ̍� */
			if(sd && battle_config.disp_job_soul_state_change)
				clif_disp_onlyself(sd->fd, msg_txt(206));	// ����ԂɂȂ�܂���
			calc_flag = 1;
			break;
		case SC_SUPERNOVICE:		/* �X�[�p�[�m�[�r�X�̍� */
			if(sd) {
				// 1%�Ŏ��S�t���O����
				if(atn_rand()%10000 < battle_config.repeal_die_counter_rate) {
					sd->status.die_counter = 0;	// ���ɃJ�E���^�[���Z�b�g
					if(sd->status.job_level >= 70 || sd->s_class.job == PC_JOB_ESNV)
						clif_misceffect(&sd->bl,7);	// �X�p�m�r�V�g
				}
				if(battle_config.disp_job_soul_state_change)
					clif_disp_onlyself(sd->fd, msg_txt(206));	// ����ԂɂȂ�܂���
			}
			calc_flag = 1;
			break;
		case SC_AUTOBERSERK:
			if( status_get_hp(bl) < status_get_max_hp(bl) >> 2 &&
			    (sc->data[SC_PROVOKE].timer == -1 || sc->data[SC_PROVOKE].val2 == 0) )
			{
				// �I�[�g�o�[�T�[�N����
				status_change_start(&sd->bl,SC_PROVOKE,10,1,0,0,0,0);
			}
			tick = 600*1000;
			break;
		case SC_READYSTORM:
		case SC_READYDOWN:
		case SC_READYTURN:
		case SC_READYCOUNTER:
			tick = 600*1000;
			break;
		case SC_UTSUSEMI:		/* ��� */
			val3 = (val1+1)/2;
			break;
		case SC_BUNSINJYUTSU:		/* ���g�̏p */
			val3 = (val1+1)/2;
			if(sd) {
				val4 = sd->status.clothes_color;
				pc_changelook(sd, LOOK_CLOTHES_COLOR, 0);
			}
			break;
		case SC_BOSSMAPINFO:		/* �ʖʋ� */
			if(map[bl->m].mvpboss == NULL) {
				if(sd) {
					// ���Ȃ��̂Ń��b�Z�[�W���o���ďI��
					clif_bossmapinfo(sd, "", 0, 0, 0, 0);
				}
				tick = 0;
			}
			break;
		case SC_HAGALAZ:			/* �X�g�[���n�[�h�X�L�� */
			{
				int hp = status_get_hp(bl);
				val3 = (int)((atn_bignumber)status_get_max_hp(bl) * 25 / 100);
				if(val3 >= hp)
					val3 = hp - 1;
				unit_heal(bl, -val3, 0);
			}
			break;
		case SC_BERKANA:			/* �~���j�A���V�[���h */
			val2 = 2 + atn_rand()%3;
			val3 = 1000;
			if(sd)
				clif_mshield(sd, val2);
			break;
		case SC_EISIR:				/* �t�@�C�e�B���O�X�s���b�g */
			calc_flag = 1;
			val3 = (val3>9)? 4: (val3>7)? 3: (val3>4)? 2: (val3>2)? 1: 0;
			break;
		case SC_URUZ:				/* �A�o���_���X */
			val3 = tick / 10000;
			if(val3 < 1)
				val3 = 1;
			tick = 10000;
			break;
		case SC_DEATHBOUND:			/* �f�X�o�E���h */
			val2 = 500+val1*100;
			break;
		case SC_FEAR:				/* ���| */
			calc_flag = 1;
			clif_emotion(bl,79);
			val3 = (tick < 2000) ? 2000: tick - 2000;
			tick = 2000;
			break;
		case SC_VENOMIMPRESS:		/* �x�i���C���v���X */
			val2 = val1 * 10;
			break;
		case SC_POISONINGWEAPON:	/* �|�C�Y�j���O�E�F�|�� */
			val3 = 200 + val1 * 200;
			break;
		case SC_WEAPONBLOCKING:		/* �E�F�|���u���b�L���O */
			val2 = 10 + val1 * 2;
			val3 = tick / 5000;
			tick = 5000;
			break;
		case SC_CLOAKINGEXCEED:		/* �N���[�L���O�G�N�V�[�h */
			calc_flag = 1;
			val2 = (val1 + 1) / 2;	// �_���[�W�ϐ�
			tick = 1000;
			break;
		case SC_ROLLINGCUTTER:		/* ���[�����O�J�b�^�[ */
			tick = 600*1000;
			icon_val1 = val1;	// val2(��]��)��n���ăA�C�R���\������
			break;
		case SC_TOXIN:				/* �g�L�V�� */
			val2 = tick / 10000;
			tick = 10000;
			break;
		case SC_VENOMBLEED:			/* �x�i���u���[�h */
			calc_flag = 1;
			val2 = 15;
			break;
		case SC_MAGICMUSHROOM:		/* �}�W�b�N�}�b�V�����[�� */
		case SC_OBLIVIONCURSE:		/* �I�u���r�I���J�[�X */
			val2 = tick / 2000;
			tick = 2000;
			break;
		case SC_DEATHHURT:			/* �f�X�n�[�g */
			val2 = 20;
			break;
		case SC_PYREXIA:			/* �p�C���b�N�V�A */
			val2 = tick / 3000;
			tick = 3000;
			break;
		case SC_LEECHEND:			/* ���[�`�G���h */
			val2 = tick / 1000;
			tick = 1000;
			break;
		case SC_EPICLESIS:			/* �G�s�N���V�X */
			calc_flag = 1;
			val2 = val1 * 5;
			break;
		case SC_ORATIO:				/* �I���e�B�I */
			val2 = val1 * 2;
			break;
		case SC_LAUDAAGNUS:			/* ���E�_�A�O�k�X */
		case SC_LAUDARAMUS:			/* ���E�_�����X */
			calc_flag = 1;
			val2 = val1 + 4;
			break;
		case SC_RENOVATIO:			/* ���m���@�e�B�I */
			val2 = 3;
			val3 = tick / 5000;
			tick = 5000;
			break;
		case SC_EXPIATIO:			/* �G�N�X�s�A�e�B�I */
			val2 = val1 * 5;
			break;
		case SC_DUPLELIGHT:			/* �f���v�����C�g */
			val2 = 10 + val1 * 2;	// �����U�������m��
			val3 = 10 + val1 * 2;	// ���@�U�������m��
			break;
		case SC_SACRAMENT:			/* �T�N�������g */
			val2 = val1 * 10;
			break;
		case SC_MARSHOFABYSS:		/* �}�[�V���I�u�A�r�X */
			calc_flag = 1;
			ud->state.change_speed = 1;
			val2 = val1 * 10;	// �ړ����x������
			val3 = val1 * 6;	// AGI/DEX������
			break;
		case SC_SUMMONBALL1:		/* �T�����{�[��(1��) */
		case SC_SUMMONBALL2:		/* �T�����{�[��(2��) */
		case SC_SUMMONBALL3:		/* �T�����{�[��(3��) */
		case SC_SUMMONBALL4:		/* �T�����{�[��(4��) */
		case SC_SUMMONBALL5:		/* �T�����{�[��(5��) */
			icon_val1 = val2;	// val2(�����̎��)��n���ăA�C�R���\������
			break;
		case SC_HELLINFERNO:		/* �w���C���t�F���m */
			calc_flag = 1;
			val2 = tick / 1000;
			val3 = 0;
			tick = 1000;
			break;
		case SC_FEARBREEZE:			/* �t�B�A�[�u���[�Y */
			val2 = (val1 > 1)? val1 * 2 + (val1 - 1) / 2 % 2 : 4;	// �����m��
			val3 = (val1 > 1)? val1: 2;	// �ő�U����
			break;
		case SC_ELECTRICSHOCKER:	/* �G���N�g���b�N�V���b�J�[ */
			val2 = tick / 1000;
			tick = 1000;
			break;
		case SC_CAMOUFLAGE:			/* �J���t���[�W�� */
			calc_flag = 1;
			ud->state.change_speed = 1;
			val2 = tick / 1000;
			tick = 1000;
			break;
		case SC_OVERHEAT:			/* ���M */
			val2 = tick / 1000;
			tick = 1000;
			break;
		case SC_SHAPESHIFT:			/* �V�F�C�v�V�t�g */
			switch(val1) {
				case 1:  val2 = ELE_FIRE;  break;
				case 2:  val2 = ELE_EARTH; break;
				case 3:  val2 = ELE_WIND;  break;
				default: val2 = ELE_WATER; break;
			}
			break;
		case SC_MAGNETICFIELD:		/* �}�O�l�e�B�b�N�t�B�[���h */
			val2 = tick / 3000;
			tick = 3000;
			break;
		case SC_STEALTHFIELD_USER:	/* �X�e���X�t�B�[���h(�g�p��) */
			val3 = (6000 - 1000 * val1);
			val3 = (val3 < 1000)? 1000: val3;
			val2 = tick / val3;
			tick = val3;
			calc_flag = 1;
			ud->state.change_speed = 1;
			break;
		case SC__REPRODUCE:		/* ���v���f���[�X */
			tick = 1000;
			break;
		case SC__AUTOSHADOWSPELL:	/* �V���h�E�I�[�g�X�y�� */
			val4 = 32 - val1*2;		// �����m��
			val4 = (val4 < 1)? 1: val4;
			break;
		case SC__SHADOWFORM:		/* �V���h�E�t�H�[�� */
			val3 = 4 + val1;	// �c���
			val4 = tick / 1000;
			tick = 1000;
			break;
		case SC__INVISIBILITY:		/* �C���r�W�r���e�B */
			if(sd) {
				tick = 1000;
				calc_flag = 1;
			} else {
				tick = 5000*val1;
			}
			break;
		case SC_REFLECTDAMAGE:		/* ���t���N�g�_���[�W */
			// ���t���N�g�V�[���h���|�����Ă��������
			if(sc->data[SC_REFLECTSHIELD].timer != -1)
				status_change_end(bl,SC_REFLECTSHIELD,-1);
			val2 = tick / 10000;
			val3 = val1 * 5 + 15;	// ���˗�
			tick = 10000;
			break;
		case SC_FORCEOFVANGUARD:	/* �t�H�[�X�I�u�o���K�[�h */
			val2 = tick / 10000;
			val3 = val1 * 12 + 8;	// �{��J�E���^�[������
			tick = 10000;
			icon_tick = -1;
			calc_flag = 1;
			break;
		case SC_SHIELDSPELL_DEF:	/* �V�[���h�X�y��(DEF) */
		case SC_SHIELDSPELL_MDEF:	/* �V�[���h�X�y��(MDEF) */
		case SC_SHIELDSPELL_REF:	/* �V�[���h�X�y��(���B) */
			if(sc->data[SC_SHIELDSPELL_DEF].timer != -1)
				status_change_end(bl,SC_SHIELDSPELL_DEF,-1);
			if(sc->data[SC_SHIELDSPELL_MDEF].timer != -1)
				status_change_end(bl,SC_SHIELDSPELL_MDEF,-1);
			if(sc->data[SC_SHIELDSPELL_REF].timer != -1)
				status_change_end(bl,SC_SHIELDSPELL_REF,-1);
			calc_flag = 1;
			break;
		case SC_EXEEDBREAK:			/* �C�N�V�[�h�u���C�N */
			if(sd) {
				int idx = sd->equip_index[EQUIP_INDEX_RARM];
				val2 = 100 + val1 * 150 + sd->status.job_level * 15;
				if(idx >= 0 && sd->inventory_data[idx])
					val2 += sd->inventory_data[idx]->weight/10 * sd->inventory_data[idx]->wlv * sd->status.base_level / 100;
			}
			else {
				val2 = val1 * 150;
			}
			tick = 600*1000;
			break;
		case SC_PRESTIGE:	/* �v���X�e�B�[�W */
			val2 = val1 * 15 * status_get_lv(bl) / 100;		// Def�����l
			if(sd) {
				val2 += pc_checkskill(sd,CR_DEFENDER) / 5 * status_get_lv(bl) / 2;
			}
			val3 = (1 + status_get_agi(bl) / 20 + status_get_luk(bl) / 40) * val1 / 2;		// ���@���
			calc_flag = 1;
			break;
		case SC_BANDING:	/* �o���f�B���O */
			tick = 5000;
			icon_tick = 9999;
			calc_flag = 1;
			break;
		case SC_SITDOWN_FORCE:		/* �]�| */
			if(sd){
				pc_setsit(sd);
				clif_sitting(&sd->bl, 1);
			}
			break;
		case SC_INSPIRATION:	/* �C���X�s���[�V���� */
			val2 = tick / 6000;
			tick = 6000;
			calc_flag = 1;
			break;
		case SC_RAISINGDRAGON:	/* �������V */
			val2 = tick / 5000;
			val3 = val1 + 2;	// MaxHP,MaxSP������
			tick = 5000;
			calc_flag = 1;
			break;
		case SC_GENTLETOUCH_ENERGYGAIN:	/* �_�� -��- */
			// �_�� -��-���|�����Ă��������
			if(sc->data[SC_GENTLETOUCH_CHANGE].timer != -1)
				status_change_end(bl,SC_GENTLETOUCH_CHANGE,-1);
			// �_�� -��-���|�����Ă��������
			if(sc->data[SC_GENTLETOUCH_REVITALIZE].timer != -1)
				status_change_end(bl,SC_GENTLETOUCH_REVITALIZE,-1);
			val2 = val1 * 5 + 10;	// �C�e������
			break;
		case SC_GENTLETOUCH_CHANGE:	/* �_�� -��- */
			// �_�� -��-���|�����Ă��������
			if(sc->data[SC_GENTLETOUCH_ENERGYGAIN].timer != -1)
				status_change_end(bl,SC_GENTLETOUCH_ENERGYGAIN,-1);
			// �_�� -��-���|�����Ă��������
			if(sc->data[SC_GENTLETOUCH_REVITALIZE].timer != -1)
				status_change_end(bl,SC_GENTLETOUCH_REVITALIZE,-1);
			val4 = status_get_agi(bl) / 15;		// ASPD�㏸�l
			calc_flag = 1;
			break;
		case SC_GENTLETOUCH_REVITALIZE:	/* �_�� -��- */
			// �_�� -��-���|�����Ă��������
			if(sc->data[SC_GENTLETOUCH_ENERGYGAIN].timer != -1)
				status_change_end(bl,SC_GENTLETOUCH_ENERGYGAIN,-1);
			// �_�� -��-���|�����Ă��������
			if(sc->data[SC_GENTLETOUCH_CHANGE].timer != -1)
				status_change_end(bl,SC_GENTLETOUCH_CHANGE,-1);
			calc_flag = 1;
			break;
		case SC_SWING:				/* �X�C���O�_���X */
			val4 = (val1 * 4) + (val2 * 2 / 10);
			calc_flag = 1;
			ud->state.change_speed = 1;
			break;
		case SC_RUSH_WINDMILL:		/* ���ԂɌ������ēˌ� */
			val4 = (val1 * 6) + (val2 / 5) + val3;
			break;
		case SC_MOONLIT_SERENADE:	/* ��������̃Z���i�[�f */
			val4 = (val1 * 5) + (val2 / 4) + val3;
			break;
		case SC_ECHOSONG:			/* �G�R�[�̉� */
			calc_flag = 1;
			val4 = (val1 * 6) + (val2 / 4) + val3;
			break;
		case SC_SYMPHONY_LOVE:		/* ���l�����ׂ̈̃V���t�H�j�[ */
			calc_flag = 1;
			val4 = (val1 * 12) + (val2 / 4) + val3;
			break;
		case SC_WINKCHARM:	/* ���f�̃E�B���N */
		case SC_SIREN:		/* �Z�C���[���̐� */
			val3 = tick / 3000;
			tick = 3000;
			break;
		case SC_DEEP_SLEEP:		/* ���炬�̎q��S */
			val2 = tick / 2000;
			tick = 2000;
			break;
		case SC_SIRCLEOFNATURE:		/* �z���鎩�R�̉� */
			val2 = tick / 1000;
			tick = 1000;
			break;
		case SC_SONG_OF_MANA:		/* �}�i�̉� */
			val2 = tick / 5000;
			tick = 5000;
			break;
		case SC_SATURDAY_NIGHT_FEVER:	/* �t���C�f�[�i�C�g�t�B�[�o�[ */
			val2 = tick / 3000;
			tick = 3000;
			calc_flag = 1;
			break;
		case SC_PROPERTYWALK:		/* �t�@�C�A�[/�G���N�g���b�N�E�H�[�N */
			val3 = val1 * 2 + 6;	// ������
			break;
		case SC_DIAMONDDUST:		/* �_�C�������h�_�X�g */
			val2 = tick / 1000;
			tick = 1000;
			break;
		case SC_SPELLFIST:		/* �X�y���t�B�X�g */
			val4 = val1 + 1;	// ��
			break;
		case SC_STRIKING:			/* �X�g���C�L���O */
			val2 = tick / 1000;
			tick = 1000;
			calc_flag = 1;
			break;
		case SC_BLOOD_SUCKER:		/* �u���b�h�T�b�J�[ */
			val4 = tick / 1000;
			tick = 1000;	// �_���[�W�����Ԋu
			break;
		case SC_FIRE_EXPANSION_SMOKE_POWDER:	/* �t�@�C�A�[�G�N�X�p���V����(����) */
			val2 = 25;		// �ߋ����E�������_���[�W������
			val3 = 25;		// Flee�㏸��
			calc_flag = 1;
			break;
		case SC_FIRE_EXPANSION_TEAR_GAS:	/* �t�@�C�A�[�G�N�X�p���V����(�×܃K�X) */
			val2 = 25;		// Hit,Flee������
			val3 = tick / 3000;
			tick = 3000;
			calc_flag = 1;
			break;
		case SC_MANDRAGORA:			/* �n�E�����O�I�u�}���h���S�� */
			val2 = val1 * 5 + 5;		// Int�����l
			val3 = 3000;		// �Œ�r�������l
			calc_flag = 1;
			break;
		case SC_BANANA_BOMB:		/* �o�i�i���e */
			if(sd){
				pc_setsit(sd);
				clif_sitting(&sd->bl, 1);
			}
			calc_flag = 1;
			break;
		case SC_MEIKYOUSISUI:		/* �����~�� */
			val2 = tick / 1000;
			tick = 1000;
			calc_flag = 1;
			break;
		case SC_IZAYOI:			/* �\�Z�� */
			if(sd) {
				val2 = sd->status.job_level/2 * val1;
			} else {
				val2 = 50 * val1;
			}
			val3 = tick / 1000;
			tick = 1000;
			calc_flag = 1;
			break;
		case SC_KAGEMUSYA:		/* ���p -���g- */
			tick = 1000;
			calc_flag = 1;
			break;
		case SC_ZANGETSU:		/* ���p -�c��- */
			if(status_get_hp(bl)%2 == 0)
				val2 += status_get_lv(bl) / 3 + val1 * 20; 	// HP�������̏ꍇ
			else
				val2 -= status_get_lv(bl) / 3 + val1 * 30; 	// HP����̏ꍇ
			if(status_get_sp(bl)%2 == 0)
				val3 += status_get_lv(bl) / 3 + val1 * 20; 	// SP�������̏ꍇ
			else
				val3 -= status_get_lv(bl) / 3 + val1 * 30; 	// SP����̏ꍇ
			calc_flag = 1;
			break;
		case SC_GENSOU:		/* ���p -�O���z- */
			val2 = val1 * 10; 	// �_���[�W���˗�
			break;
		case SC_MER_FLEE:		/* �b���{�[�i�X(FLEE) */
		case SC_MER_ATK:		/* �b���{�[�i�X(ATK) */
		case SC_MER_HIT:		/* �b���{�[�i�X(HIT) */
			val2 = val1 * 15;
			calc_flag = 1;
			break;
		case SC_MER_HP:			/* �b���{�[�i�X(HP) */
		case SC_MER_SP:			/* �b���{�[�i�X(SP) */
			val2 = val1 * 5;
			calc_flag = 1;
			break;
		case SC_ON_PUSH_CART:	/* �J�[�g */
			icon_val1 = val1;	// �J�[�g�^�C�v��n��
			ud->state.change_speed = 1;
			calc_flag = 1;
			if(sd && val2) {
				clif_cart_itemlist(sd);
				clif_cart_equiplist(sd);
				clif_updatestatus(sd,SP_CARTINFO);
			}
			val2 = 1;
			break;
		case SC_SUMMON_ELEM:	/* �T�����G�������^�� */
			val2 = tick / 10000;
			tick = 10000;
			val3 = val1 * 3 + 2;		// SP�����
			break;
		case SC_FIRE_CLOAK:		/* �t�@�C�A�[�N���[�N */
		case SC_WATER_DROP:		/* �E�H�[�^�[�h���b�v */
		case SC_WIND_CURTAIN:	/* �E�B���h�J�[�e�� */
		case SC_STONE_SHIELD:	/* �X�g�[���V�[���h */
			val2 = 100;		// �����ϐ������l
			val3 = 100;		// �����ϐ������l
			calc_flag = 1;
			break;
		case SC_WATER_SCREEN:	/* �E�H�[�^�[�X�N���[�� */
			val2 = tick / 5000;
			tick = 5000;
			break;
		case SC_WIND_STEP:			/* �E�B���h�X�e�b�v */
			val2 = 10;	// Flee�����l
			val3 = 50;	// �ړ����x������
			calc_flag = 1;
			ud->state.change_speed = 1;
			break;
		case SC_SOLID_SKIN:			/* �\���b�h�X�L�� */
			val2 = 10;	// Def������
			val3 = 25;	// MaxHP������
			calc_flag = 1;
			break;
		case SC_PYROTECHNIC:	/* �p�C���e�N�j�b�N */
			val2 = 40;	// Atk�����l
			val3 = 10;	// �_���[�W������
			calc_flag = 1;
			break;
		case SC_HEATER:	/* �q�[�^�[ */
			val2 = 80;	// Atk�����l
			val3 = 20;	// �_���[�W������
			calc_flag = 1;
			break;
		case SC_TROPIC:	/* �g���s�b�N */
			val2 = 120;	// Atk�����l
			calc_flag = 1;
			break;
		case SC_AQUAPLAY:	/* �A�N�A�v���C */
			val2 = 40;	// Matk�����l
			val3 = 10;	// �_���[�W������
			calc_flag = 1;
			break;
		case SC_COOLER:	/* �N�[���[ */
			val2 = 80;	// Matk�����l
			val3 = 20;	// �_���[�W������
			calc_flag = 1;
			break;
		case SC_CHILLY_AIR:	/* �N�[���G�A�[ */
			val2 = 120;	// Matk�����l
			calc_flag = 1;
			break;
		case SC_GUST:	/* �K�X�g */
			val2 = 50;	// Aspd�����l
			val3 = 10;	// �_���[�W������
			calc_flag = 1;
			break;
		case SC_BLAST:	/* �u���X�g */
			val2 = 50;	// Aspd�����l
			val3 = 10;	// �_���[�W������
			calc_flag = 1;
			break;
		case SC_WILD_STORM:	/* ���C���h�X�g�[�� */
			val2 = 50;	// Aspd�����l
			calc_flag = 1;
			break;
		case SC_PETROLOGY:	/* �y�g���W�[ */
			val2 = 5;	// MaxHP������
			val3 = 10;	// �_���[�W������
			calc_flag = 1;
			break;
		case SC_CURSED_SOIL:	/* �J�[�X�h�\�C�� */
			val2 = 10;	// MaxHP������
			val3 = 20;	// �_���[�W������
			calc_flag = 1;
			break;
		case SC_UPHEAVAL:	/* �A�b�v�w�C�o�� */
			val2 = 15;	// MaxHP������
			calc_flag = 1;
			break;
		case SC_TIDAL_WEAPON_OPTION:	/* �^�C�_���E�F�|��(����) */
			val2 = 20;	// Atk������
			calc_flag = 1;
			break;
		case SC_CIRCLE_OF_FIRE_OPTION:	/* �T�[�N���I�u�t�@�C�A(����) */
		case SC_FIRE_CLOAK_OPTION:		/* �t�@�C�A�[�N���[�N(����) */
		case SC_WATER_SCREEN_OPTION:	/* �E�H�[�^�[�X�N���[��(����) */
		case SC_WATER_DROP_OPTION:		/* �E�H�[�^�[�h���b�v(����) */
		case SC_WIND_STEP_OPTION:		/* �E�B���h�X�e�b�v(����) */
		case SC_WIND_CURTAIN_OPTION:	/* �E�B���h�J�[�e��(����) */
		case SC_SOLID_SKIN_OPTION:		/* �\���b�h�X�L��(����) */
		case SC_STONE_SHIELD_OPTION:	/* �X�g�[���V�[���h(����) */
			val2 = tick / 1000;
			tick = 1000;
			val3 = 5;		// SP�����
			break;
		case SC_PYROTECHNIC_OPTION:	/* �p�C���e�N�j�b�N(����) */
		case SC_AQUAPLAY_OPTION:	/* �A�N�A�v���C(����) */
		case SC_GUST_OPTION:		/* �K�X�g(����) */
		case SC_PETROLOGY_OPTION:	/* �y�g���W�[(����) */
			val2 = tick / 10000;
			tick = 10000;
			val3 = 10;		// SP�����
			break;
		case SC_HEATER_OPTION:		/* �q�[�^�[(����) */
		case SC_COOLER_OPTION:		/* �N�[���[(����) */
		case SC_BLAST_OPTION:		/* �u���X�g(����) */
		case SC_CURSED_SOIL_OPTION:	/* �J�[�X�h�\�C��(����) */
			val2 = tick / 10000;
			tick = 10000;
			val3 = 20;		// SP�����
			break;
		case SC_TROPIC_OPTION:		/* �g���s�b�N(����) */
		case SC_CHILLY_AIR_OPTION:	/* �N�[���G�A�[(����) */
		case SC_WILD_STORM_OPTION:	/* ���C���h�X�g�[��(����) */
		case SC_UPHEAVAL_OPTION:	/* �A�b�v�w�C�o��(����) */
			val2 = tick / 10000;
			tick = 10000;
			val3 = 30;		// SP�����
			break;
		default:
			if(battle_config.error_log)
				printf("UnknownStatusChange [%d]\n", type);
			return 0;
	}

	if(tick <= 0)
		return 0;

	if(StatusIconChangeTable[type] != SI_BLANK)
		clif_status_change(bl,StatusIconChangeTable[type],1,icon_tick,icon_val1,icon_val2,icon_val3);	// �A�C�R���\��

	/* �ʖʋ��̓A�C�R�����M��ɏ������� */
	if(type == SC_BOSSMAPINFO) {
		struct mob_data *mmd = map[bl->m].mvpboss;
		if(sd && mmd) {
			if(mmd->bl.prev == NULL) {	// �ĕ����҂���
				int diff = DIFF_TICK(mmd->last_spawntime, current_tick);
				if(diff < 0)
					diff = 0;
				clif_bossmapinfo(sd, mmd->name, 0, 0, diff, 3);
				val3 = -1;
				val4 = -1;
			} else {			// �o����
				clif_bossmapinfo(sd, mmd->name, 0, 0, 0, 2);
				if(mmd->bl.m == bl->m) {
					clif_bossmapinfo(sd, "", mmd->bl.x, mmd->bl.y, 0, 1);
				}
				val3 = mmd->bl.m;
				val4 = mmd->bl.x + (mmd->bl.y << 16);
			}
		}
		val2 = tick / 1000;
		tick = 1000;
	}

	/* option�̕ύX */
	switch(type) {
		// opt1
		case SC_STONE:
		case SC_FREEZE:
		case SC_STUN:
		case SC_SLEEP:
			unit_stopattack(bl);	// �U����~
			skill_stop_dancing(bl,0);	// ���t/�_���X�̒��f
			{
				// �����Ɋ|����Ȃ��X�e�[�^�X�ُ������
				int i;
				for(i = SC_STONE; i <= SC_SLEEP; i++) {
					if(sc->data[i].timer != -1) {
						sc->count--;
						delete_timer(sc->data[i].timer, status_change_timer);
						sc->data[i].timer = -1;
					}
				}
			}
			if(type == SC_STONE)
				sc->opt1 = OPT1_STONECURSE_ING;
			else
				sc->opt1 = type - SC_STONE + 1;

			if(md) {
				mob_unlocktarget(md, current_tick);
				md->attacked_id = 0;
				md->attacked_players = 0;
			}
			opt_flag = 1;
			break;
		case SC_HELLINFERNO:		/* �w���C���t�F���m */
			sc->opt1 = OPT1_BURNNING;
			opt_flag = 1;
			break;
		case SC_WHITEIMPRISON:		/* �z���C�g�C���v���Y�� */
			sc->opt1 = OPT1_IMPRISON;
			opt_flag = 1;
			break;
		// opt2
		case SC_POISON:
		case SC_CURSE:
		case SC_SILENCE:
		case SC_CONFUSION:
			sc->opt2 |= OPT2_POISON<<(type-SC_POISON);
			opt_flag = 1;
			break;
		case SC_FOGWALLPENALTY:
		case SC_BLIND:
			if(sc->data[SC_FOGWALLPENALTY].timer == -1) {
				sc->opt2 |= OPT2_BLIND;
				opt_flag = 1;
				if(md && !(flag&2))
					md->target_id = 0;
			}
			break;
		case SC_ANGELUS:			/* �A���[���X */
			sc->opt2 |= OPT2_ANGELUS;
			opt_flag = 1;
			break;
		case SC_DPOISON:
			sc->opt2 |= OPT2_HEAVYPOISON;
			opt_flag = 1;
			break;
		case SC_BLEED:
			sc->opt2 |= OPT2_BLEEDING;
			opt_flag = 1;
			break;
		case SC_FEAR:
			sc->opt2 |= OPT2_FEAR;
			opt_flag = 1;
			break;
		// opt3
		case SC_ONEHAND:		/* 1HQ */
		case SC_TWOHANDQUICKEN:		/* 2HQ */
		case SC_SPEARQUICKEN:		/* �X�s�A�N�C�b�P�� */
		case SC_CONCENTRATION:		/* �R���Z���g���[�V���� */
		case SC_WEAPONQUICKEN:		/* �E�F�|���N�C�b�P�� */
			sc->opt3 |= OPT3_QUICKEN;
			opt_flag = 2;
			break;
		case SC_OVERTHRUST:		/* �I�[�o�[�g���X�g */
		case SC_SWOO:			/* �G�X�E */
			sc->opt3 |= OPT3_OVERTHRUST;
			opt_flag = 2;
			break;
		case SC_ENERGYCOAT:		/* �G�i�W�[�R�[�g */
		case SC_SKE:			/* �G�X�N */
			sc->opt3 |= OPT3_ENERGYCOAT;
			opt_flag = 2;
			break;
		case SC_EXPLOSIONSPIRITS:	/* �����g�� */
			sc->opt3 |= OPT3_EXPLOSIONSPIRITS;
			opt_flag = 2;
			break;
		case SC_STEELBODY:		/* ���� */
		case SC_SKA:			/* �G�X�J */
			sc->opt3 |= OPT3_STEELBODY;
			opt_flag = 2;
			break;
		case SC_BLADESTOP:		/* ���n��� */
			sc->opt3 |= OPT3_BLADESTOP;
			opt_flag = 2;
			break;
		case SC_AURABLADE:			/* �I�[���u���[�h */
			sc->opt3 |= OPT3_AURABLADE;
			opt_flag = 2;
			break;
		case SC_BERSERK:		/* �o�[�T�[�N */
			sc->opt3 |= OPT3_BERSERK;
			opt_flag = 2;
			break;
		case SC_DANCING:			/* �_���X/���t�� */
			if(sc->data[SC_DANCING].val1 != CG_MOONLIT)
				break;
			sc->opt3 |= OPT3_MOON;
			opt_flag = 2;
			break;
		case SC_MARIONETTE:		/* �}���I�l�b�g�R���g���[�� */
		case SC_MARIONETTE2:		/* �}���I�l�b�g�R���g���[�� */
			sc->opt3 |= OPT3_MARIONETTE;
			opt_flag = 2;
			break;
		case SC_ASSUMPTIO:		/* �A�X���v�e�B�I */
		case SC_ASSUMPTIO2:		/* �L���b�V���A�X���v�e�B�I */
			sc->opt3 |= OPT3_ASSUMPTIO;
			clif_misceffect2(bl,375);
			opt_flag = 2;
			break;
		case SC_WARM:			/* ������ */
			sc->opt3 |= OPT3_SUN_WARM;
			opt_flag = 2;
			break;
		case SC_KAITE:
			sc->opt3 |= OPT3_KAITE;
			opt_flag = 2;
			break;
		case SC_BUNSINJYUTSU:		/* ���g�̏p */
			sc->opt3 |= OPT3_BUNSIN;
			opt_flag = 2;
			break;
		case SC_MONK:			/* �����N�̍� */
		case SC_STAR:			/* �P���Z�C�̍� */
		case SC_SAGE:			/* �Z�[�W�̍� */
		case SC_CRUSADER:		/* �N���Z�C�_�[�̍� */
		case SC_WIZARD:			/* �E�B�U�[�h�̍� */
		case SC_PRIEST:			/* �v���[�X�g�̍� */
		case SC_ROGUE:			/* ���[�O�̍� */
		case SC_ASSASIN:		/* �A�T�V���̍� */
		case SC_SOULLINKER:		/* �\�E�������J�[�̍� */
		case SC_KNIGHT:			/* �i�C�g�̍� */
		case SC_ALCHEMIST:		/* �A���P�~�X�g�̍� */
		case SC_BARDDANCER:		/* �o�[�h�ƃ_���T�[�̍� */
		case SC_BLACKSMITH:		/* �u���b�N�X�~�X�̍� */
		case SC_HUNTER:			/* �n���^�[�̍� */
		case SC_HIGH:			/* �ꎟ��ʐE�Ƃ̍� */
		case SC_SUPERNOVICE:		/* �X�[�p�[�m�[�r�X�̍� */
		case SC_GUNNER:			/* �K���X�����K�[�̍� */
		case SC_NINJA:			/* �E�҂̍� */
		case SC_DEATHKINGHT:		/* �f�X�i�C�g�̍� */
		case SC_COLLECTOR:		/* �R���N�^�[�̍� */
			sc->opt3 |= OPT3_SOULLINK;
			clif_misceffect2(bl,424);
			opt_flag = 2;
			break;
		case SC_ELEMENTUNDEAD:		// �s��
			sc->opt3 |= OPT3_UNDEAD;
			opt_flag = 2;
			break;
		// option
		case SC_SIGHT:
			sc->option |= OPTION_SIGHT;
			opt_flag = 1;
			break;
		case SC_HIDING:
			if(sd && val3 == 0)	// ���a��łȂ��ʏ�̃n�C�h�Ȃ�A�C�R���\��
				clif_status_change(bl,SI_HIDING,1,icon_tick,0,0,0);
			unit_stopattack(bl);
			sc->option |= OPTION_HIDE;
			opt_flag = 1;
			break;
		case SC_CLOAKING:
		case SC_CLOAKINGEXCEED:		/* �N���[�L���O�G�N�V�[�h */
		case SC__INVISIBILITY:		/* �C���r�W�r���e�B */
			unit_stopattack(bl);
			sc->option |= OPTION_CLOAKING;
			opt_flag = 1;
			break;
		case SC_INVISIBLE:
			unit_stopattack(bl);
			sc->option |= OPTION_SPECIALHIDING;
			opt_flag = 1;
			break;
		case SC_REVERSEORCISH:
			sc->option |= OPTION_ORCFACE;
			opt_flag = 1;
			break;
		case SC_WEDDING:
			sc->option |= OPTION_MARRIED;
			opt_flag = 1;
			break;
		case SC_RUWACH:
			sc->option |= OPTION_RUWACH;
			opt_flag = 1;
			break;
		case SC_CHASEWALK:
			unit_stopattack(bl);
			sc->option |= (OPTION_CLOAKING | OPTION_FOOTPRINT);
			opt_flag = 1;
			break;
		case SC_FUSION:
			sc->option |= OPTION_STAR2;
			opt_flag = 1;
			break;
		case SC_SANTA:
			sc->option |= OPTION_SANTA;
			opt_flag = 1;
			break;
		case SC_SUMMER:
			sc->option |= OPTION_SUMMER;
			opt_flag = 1;
			break;
		case SC_ON_PUSH_CART:	/* �J�[�g */
#if PACKETVER < 20120201
			// �Â��N���C�A���g�̓I�v�V�������X�V����
			sc->option = (sc->option & ~OPTION_CARTMASK) | val1;
			opt_flag = 1;
#endif
			break;
	}

	/* option�̕ύX */
	if(opt_flag == 1) {
		clif_changeoption(bl);
	} else if(opt_flag == 2) {
		clif_changeoption2(bl);
	}

#ifdef DYNAMIC_SC_DATA
	status_calloc_sc_data(sc);
#endif

	sc->count++;	/* �X�e�[�^�X�ُ�̐� */

	sc->data[type].val1 = val1;
	sc->data[type].val2 = val2;
	sc->data[type].val3 = val3;
	sc->data[type].val4 = val4;
	/* �^�C�}�[�ݒ� */
	sc->data[type].timer = add_timer(current_tick + tick, status_change_timer, bl->id, INT2PTR(type));

	if(calc_flag) {
		// �X�e�[�^�X�Čv�Z
		if(sd) {
			if(!(flag&4))
				status_calc_pc(sd,0);
		} else if(hd) {
			homun_calc_status(hd);
			clif_send_homstatus(hd->msd,0);
		} else if(mcd) {
			merc_calc_status(mcd);
			clif_send_mercstatus(mcd->msd);
			clif_mercskillinfoblock(mcd->msd);
		} else if(eld) {
			elem_calc_status(eld);
			clif_send_elemstatus(eld->msd);
		}
	}
	// �v�Z��ɑ��点��
	switch(type) {
		case SC_RUN:
		case SC_WUGDASH:
			// clif_skill_nodamage() �͕K�� clif_status_change() �� clif_walkok() �̊ԂɌĂяo��
			if(type == SC_RUN)
				clif_skill_nodamage(bl,bl,TK_RUN,val1,1);
			else if(type == SC_WUGDASH)
				clif_skill_nodamage(bl,bl,RA_WUGDASH,val1,1);
			if(sd) {
				pc_runtodir(sd);
			}
			break;
		case SC_FORCEWALKING:
			unit_forcewalktodir(bl,val4);
			break;
	}

	return 0;
}

/*==========================================
 * �X�e�[�^�X�ُ�I��
 *------------------------------------------
 */
int status_change_end(struct block_list* bl, int type, int tid)
{
	struct map_session_data *sd  = NULL;
	struct mob_data         *md  = NULL;
	struct homun_data       *hd  = NULL;
	struct merc_data        *mcd = NULL;
	struct elem_data        *eld = NULL;
	struct status_change    *sc  = NULL;
	struct unit_data        *ud  = NULL;
	int opt_flag = 0, calc_flag = 0;

	nullpo_retr(0, bl);
	nullpo_retr(0, sc = status_get_sc(bl));

	if(type < 0)
		return 0;
	ud = unit_bl2ud(bl);
	if(ud == NULL)
		return 0;

	if(type >= MAX_STATUSCHANGE) {
		switch(type) {
			case SC_RACECLEAR:
				status_change_race_end(bl,-1);
				break;
			case SC_RESISTCLEAR:
				status_change_resistclear(bl);
				break;
			case SC_SOUL:
			case SC_SOULCLEAR:
				status_change_soulclear(bl);
				break;
			default:
				if(battle_config.error_log)
					printf("UnknownStatusChangeEnd [%d]\n", type);
				break;
		}
		return 0;
	}

	if(sc->count <= 0 || sc->data[type].timer == -1)
		return 0;

	if(tid != -1 && sc->data[type].timer != tid)
		return 0;

	if(tid == -1)	/* �^�C�}����Ă΂�Ă��Ȃ��Ȃ�^�C�}�폜������ */
		delete_timer(sc->data[type].timer,status_change_timer);

	/* �Y���ُ̈�𐳏�ɖ߂� */
	sc->data[type].timer = -1;
	sc->count--;

	sd  = BL_DOWNCAST( BL_PC,   bl );
	md  = BL_DOWNCAST( BL_MOB,  bl );
	hd  = BL_DOWNCAST( BL_HOM,  bl );
	mcd = BL_DOWNCAST( BL_MERC, bl );
	eld = BL_DOWNCAST( BL_ELEM, bl );

	switch(type) {	/* �ُ�̎�ނ��Ƃ̏��� */
		case SC_PROVOKE:			/* �v���{�b�N */
		case SC_CONCENTRATE:			/* �W���͌��� */
		case SC_BLESSING:			/* �u���b�V���O */
		case SC_ANGELUS:			/* �A���[���X */
		case SC_SIGNUMCRUCIS:			/* �V�O�i���N���V�X */
		case SC_HIDING:
		case SC_CLOAKING:
		case SC_TWOHANDQUICKEN:			/* 2HQ */
		case SC_ONEHAND:			/* 1HQ */
		case SC_ADRENALINE:			/* �A�h���i�������b�V�� */
		case SC_ENCPOISON:			/* �G���`�����g�|�C�Y�� */
		case SC_IMPOSITIO:			/* �C���|�V�e�B�I�}�k�X */
		case SC_GLORIA:				/* �O�����A */
		case SC_LOUD:				/* ���E�h�{�C�X */
		case SC_MINDBREAKER:			/* �}�C���h�u���[�J�[ */
		case SC_PROVIDENCE:			/* �v�����B�f���X */
		case SC_SPEARQUICKEN:			/* �X�s�A�N�C�b�P�� */
		case SC_VOLCANO:
		case SC_DELUGE:
		case SC_VIOLENTGALE:
		case SC_ETERNALCHAOS:			/* �G�^�[�i���J�I�X */
		case SC_DRUMBATTLE:			/* �푾�ۂ̋��� */
		case SC_NIBELUNGEN:			/* �j�[�x�����O�̎w�� */
		case SC_SIEGFRIED:			/* �s���g�̃W�[�N�t���[�h */
		case SC_EXPLOSIONSPIRITS:		/* �����g�� */
		case SC_SPEEDPOTION0:			/* �����|�[�V���� */
		case SC_SPEEDPOTION1:
		case SC_SPEEDPOTION2:
		case SC_BLADESTOP_WAIT:
		case SC_CONCENTRATION:			/* �R���Z���g���[�V���� */
		case SC_TRUESIGHT:			/* �g�D���[�T�C�g */
		case SC_SPIDERWEB:			/* �X�p�C�_�[�E�F�b�u */
		case SC_CARTBOOST:			/* �J�[�g�u�[�X�g */
		case SC_INCATK:				/* ATK�㏸ (�_��p) */
		case SC_INCMATK:			/* MATK�㏸ (�_��̑��p) */
		case SC_WEDDING:			/* �E�F�f�B���O */
		case SC_SANTA:
		case SC_SUMMER:
		case SC_INCALLSTATUS:
		case SC_INCHIT:
		case SC_INCMHP2:
		case SC_INCMSP2:
		case SC_INCATK2:
		case SC_INCHIT2:
		case SC_INCFLEE2:
		case SC_OVERTHRUSTMAX:
		case SC_CHASEWALK:			/* �`�F�C�X�E�H�[�N */
		case SC_CHASEWALK_STR:
		case SC_BATTLEORDER:
		case SC_MEAL_INCSTR:	// �H���p
		case SC_MEAL_INCAGI:
		case SC_MEAL_INCVIT:
		case SC_MEAL_INCINT:
		case SC_MEAL_INCDEX:
		case SC_MEAL_INCLUK:
		case SC_MEAL_INCHIT:
		case SC_MEAL_INCFLEE:
		case SC_MEAL_INCFLEE2:
		case SC_MEAL_INCCRITICAL:
		case SC_MEAL_INCDEF:
		case SC_MEAL_INCMDEF:
		case SC_MEAL_INCATK:
		case SC_MEAL_INCMATK:
		case SC_MEAL_INCSTR2:	// �ۋ������p
		case SC_MEAL_INCAGI2:
		case SC_MEAL_INCVIT2:
		case SC_MEAL_INCINT2:
		case SC_MEAL_INCDEX2:
		case SC_MEAL_INCLUK2:
		case SC_SPURT:
		case SC_SUN_COMFORT:			/* ���z�̈��y */
		case SC_MOON_COMFORT:			/* ���̈��y */
		case SC_STAR_COMFORT:			/* ���̈��y */
		case SC_FUSION:				/* ���z�ƌ��Ɛ��̗Z�� */
		case SC_ADRENALINE2:			/* �t���A�h���i�������b�V�� */
		case SC_RESISTWATER:
		case SC_RESISTGROUND:
		case SC_RESISTFIRE:
		case SC_RESISTWIND:
		case SC_RESISTPOISON:
		case SC_RESISTHOLY:
		case SC_RESISTDARK:
		case SC_RESISTTELEKINESIS:
		case SC_RESISTUNDEAD:
		case SC_RESISTALL:
		case SC_INVISIBLE:
		case SC_TIGEREYE:
		case SC_THE_MAGICIAN:
		case SC_STRENGTH:
		case SC_THE_DEVIL:
		case SC_THE_SUN:
		case SC_DISARM:				/* �f�B�X�A�[�� */
		case SC_FLING:				/* �t���C���O */
		case SC_MADNESSCANCEL:			/* �}�b�h�l�X�L�����Z���[ */
		case SC_ADJUSTMENT:			/* �A�W���X�g�����g */
		case SC_INCREASING:			/* �C���N���[�W���O�A�L���A���V�[ */
		case SC_FULLBUSTER:			/* �t���o�X�^�[ */
		case SC_NEN:				/* �O */
		case SC_AVOID:				/* �ً}��� */
		case SC_CHANGE:				/* �����^���`�F���W */
		case SC_DEFENCE:			/* �f�B�t�F���X */
		case SC_BLOODLUST:			/* �u���b�h���X�g */
		case SC_FLEET:				/* �t���[�g���[�u */
		case SC_SPEED:				/* �I�[�o�[�h�X�s�[�h */
		case SC_STONESKIN:			/* �X�g�[���X�L�� */
		case SC_ANTIMAGIC:			/* �A���`�}�W�b�N */
		case SC_WEAPONQUICKEN:			/* �E�F�|���N�C�b�P�� */
		case SC_WE_FEMALE:			/* ���Ȃ��ɐs�����܂� */
		case SC_TURISUSS:			/* �W���C�A���g�O���[�X */
		case SC_EISIR:				/* �t�@�C�e�B���O�X�s���b�g */
		case SC_FEAR:				/* ���| */
		case SC_EPICLESIS:			/* �G�s�N���V�X */
		case SC_LAUDAAGNUS:			/* ���E�_�A�O�k�X */
		case SC_LAUDARAMUS:			/* ���E�_�����X */
		case SC_VENOMIMPRESS:		/* �x�i���C���v���X */
		case SC_CLOAKINGEXCEED:		/* �N���[�L���O�G�N�V�[�h */
		case SC_HALLUCINATIONWALK2:	/* �n���V�l�[�V�����E�H�[�N(�y�i���e�B) */
		case SC_VENOMBLEED:			/* �x�i���u���[�h */
		case SC_INFRAREDSCAN:		/* �C���t�����b�h�X�L���� */
		case SC_ANALYZE:			/* �A�i���C�Y */
		case SC_NEUTRALBARRIER:		/* �j���[�g�����o���A�[ */
		case SC__BODYPAINT:			/* �{�f�B�y�C���e�B���O */
		case SC__INVISIBILITY:		/* �C���r�W�r���e�B */
		case SC__ENERVATION:		/* �}�X�J���[�h �F �G�i�[�x�[�V���� */
		case SC__UNLUCKY:			/* �}�X�J���[�h �F �A�����b�L�[ */
		case SC__STRIPACCESSARY:	/* �X�g���b�v�A�N�Z�T���[ */
		case SC__BLOODYLUST:		/* �u���b�f�B���X�g */
		case SC_SHIELDSPELL_DEF:	/* �V�[���h�X�y��(DEF) */
		case SC_SHIELDSPELL_MDEF:	/* �V�[���h�X�y��(MDEF) */
		case SC_SHIELDSPELL_REF:	/* �V�[���h�X�y��(���B) */
		case SC_PRESTIGE:			/* �v���X�e�B�[�W */
		case SC_EARTHDRIVE:			/* �A�[�X�h���C�u */
		case SC_INSPIRATION:		/* �C���X�s���[�V���� */
		case SC_GENTLETOUCH_CHANGE:	/* �_�� -��- */
		case SC_GENTLETOUCH_REVITALIZE:	/* �_�� -��- */
		case SC_SYMPHONY_LOVE:		/* ���l�����ׂ̈̃V���t�H�j�[ */
		case SC_ECHOSONG:			/* �G�R�[�̉� */
		case SC_HARMONIZE:			/* �n�[���i�C�Y */
		case SC_GLOOMYDAY:			/* �������R���[ */
		case SC_LERADS_DEW:			/* ���[���Y�̘I */
		case SC_DANCE_WITH_WUG:		/* �_���X�E�B�Y�E�H�[�O */
		case SC_STRIKING:			/* �X�g���C�L���O */
		case SC_FIRE_EXPANSION_SMOKE_POWDER:	/* �t�@�C�A�[�G�N�X�p���V����(����) */
		case SC_FIRE_EXPANSION_TEAR_GAS:	/* �t�@�C�A�[�G�N�X�p���V����(�×܃K�X) */
		case SC_MANDRAGORA:			/* �n�E�����O�I�u�}���h���S�� */
		case SC_BANANA_BOMB:		/* �o�i�i���e */
		case SC_MYSTERIOUS_POWDER:	/* �s�v�c�ȕ� */
		case SC_BOOST500:			/* �u�[�X�g500 */
		case SC_FULL_SWING_K:		/* �t���X�C���OK */
		case SC_MANA_PLUS:			/* �}�i�v���X */
		case SC_MUSTLE_M:			/* �}�b�X��M */
		case SC_LIFE_FORCE_F:		/* ���C�t�t�H�[�XF */
		case SC_PROMOTE_HEALTH_RESERCH:	/* HP�����|�[�V���� */
		case SC_ENERGY_DRINK_RESERCH:	/* SP�����|�[�V���� */
		case SC_EXTRACT_WHITE_POTION_Z:	/* �Z�k�z���C�g�|�[�V����Z */
		case SC_VITATA_500:			/* �r�^�^500 */
		case SC_EXTRACT_SALAMINE_JUICE:	/* �Z�k�T���}�C���W���[�X */
		case SC_SAVAGE_STEAK:		/* �T�x�[�W�̊ۏĂ� */
		case SC_COCKTAIL_WARG_BLOOD:	/* �J�N�e���E�H�[�O�u���b�h */
		case SC_MINOR_BBQ:			/* �~�m�^�E���X�̋��J���r */
		case SC_SIROMA_ICE_TEA:		/* �V���}�A�C�X�e�B�[ */
		case SC_DROCERA_HERB_STEAMED:	/* �h���Z���̃n�[�u�� */
		case SC_PUTTI_TAILS_NOODLES:	/* �v�e�B�b�g�̂����ۖ� */
		case SC_STOMACHACHE:		/* ���� */
		case SC_MONSTER_TRANSFORM:	/* �����X�^�[�ϐg */
		case SC_MEIKYOUSISUI:		/* �����~�� */
		case SC_IZAYOI:				/* �\�Z�� */
		case SC_KG_KAGEHUMI:		/* ���p -�e����- */
		case SC_KYOMU:				/* ���p -�����̉e- */
		case SC_KAGEMUSYA:			/* ���p -���g- */
		case SC_ZANGETSU:			/* ���p -�c��- */
		case SC_AKAITSUKI:			/* ���p -�g��- */
		case SC_KYOUGAKU:			/* ���p -����- */
		case SC_ODINS_POWER:		/* �I�[�f�B���̗� */
		case SC_MER_FLEE:			/* �b���{�[�i�X(FLEE) */
		case SC_MER_ATK:			/* �b���{�[�i�X(ATK) */
		case SC_MER_HP:				/* �b���{�[�i�X(HP) */
		case SC_MER_SP:				/* �b���{�[�i�X(SP) */
		case SC_MER_HIT:			/* �b���{�[�i�X(HIT) */
		case SC_FIRE_CLOAK:			/* �t�@�C�A�[�N���[�N */
		case SC_WATER_DROP:			/* �E�H�[�^�[�h���b�v */
		case SC_WIND_CURTAIN:		/* �E�B���h�J�[�e�� */
		case SC_SOLID_SKIN:			/* �\���b�h�X�L�� */
		case SC_STONE_SHIELD:		/* �X�g�[���V�[���h */
		case SC_PYROTECHNIC:		/* �p�C���e�N�j�b�N */
		case SC_HEATER:				/* �q�[�^�[ */
		case SC_TROPIC:				/* �g���s�b�N */
		case SC_AQUAPLAY:			/* �A�N�A�v���C */
		case SC_COOLER:				/* �N�[���[ */
		case SC_CHILLY_AIR:			/* �N�[���G�A�[ */
		case SC_GUST:				/* �K�X�g */
		case SC_BLAST:				/* �u���X�g */
		case SC_WILD_STORM:			/* ���C���h�X�g�[�� */
		case SC_PETROLOGY:			/* �y�g���W�[ */
		case SC_CURSED_SOIL:		/* �J�[�X�h�\�C�� */
		case SC_UPHEAVAL:			/* �A�b�v�w�C�o�� */
		case SC_TIDAL_WEAPON_OPTION:	/* �^�C�_���E�F�|��(����) */
		case SC_INVINCIBLE:			/* �C���r���V�u�� */
		case SC_INVINCIBLEOFF:		/* �C���r���V�u���I�t */
		case SC_ATKPOTION:
		case SC_MATKPOTION:
		case SC_ALMIGHTY:
			calc_flag = 1;
			break;
		case SC_SPEEDUP0:			/* �ړ����x����(�A�C�e��) */
		case SC_SPEEDUP1:			/* �X�s�[�h�|�[�V���� */
		case SC_WALKSPEED:			/* �ړ����x����(�X�N���v�g) */
		case SC_SLOWPOTION:			/* �ړ����x�ቺ(�A�C�e��) */
		case SC_STEELBODY:			/* ���� */
		case SC_INCREASEAGI:			/* ���x�㏸ */
		case SC_WINDWALK:			/* �E�C���h�E�H�[�N */
		case SC_INCFLEE:
		case SC_DECREASEAGI:			/* ���x���� */
		case SC_QUAGMIRE:			/* �N�@�O�}�C�A */
		case SC_MARSHOFABYSS:		/* �}�[�V���I�u�A�r�X */
		case SC_DEFENDER:			/* �f�B�t�F���_�[ */
		case SC_GRAVITATION:
		case SC_SUITON:				/* ���� */
		case SC_GATLINGFEVER:			/* �K�g�����O�t�B�[�o�[ */
		case SC_PARALIZE:			/* �p�����C�Y */
		case SC_FROSTMISTY:			/* �t���X�g�~�X�e�B */
		case SC_CAMOUFLAGE:			/* �J���t���[�W�� */
		case SC__GROOMY:			/* �}�X�J���[�h �F �O���[�~�[ */
		case SC__LAZINESS:			/* �}�X�J���[�h �F ���C�W�[�l�X */
		case SC_SWING:				/* �X�C���O�_���X */
		case SC_GN_CARTBOOST:		/* �J�[�g�u�[�X�g */
		case SC_MELON_BOMB:			/* ���������e */
		case SC_WIND_STEP:			/* �E�B���h�X�e�b�v */
			calc_flag = 1;
			ud->state.change_speed = 1;
			break;
		case SC_ALL_RIDING:			/* �R��V�X�e�� */
			if(sd)
				clif_status_load_id(sd,SI_RIDING,0);
			calc_flag = 1;
			ud->state.change_speed = 1;
			break;
		case SC_ON_PUSH_CART:		/* �J�[�g */
			calc_flag = 1;
			ud->state.change_speed = 1;
			if(sc->data[SC_CARTBOOST].timer != -1)
				status_change_end(bl, SC_CARTBOOST, -1);
			if(sc->data[SC_GN_CARTBOOST].timer != -1)
				status_change_end(bl, SC_GN_CARTBOOST, -1);
			if(sd)
				clif_cart_clear(sd);
			break;
		case SC_ELEMENTWATER:		// ��
		case SC_ELEMENTGROUND:		// �y
		case SC_ELEMENTFIRE:		// ��
		case SC_ELEMENTWIND:		// ��
		case SC_ELEMENTHOLY:		// ��
		case SC_ELEMENTDARK:		// ��
		case SC_ELEMENTELEKINESIS:	// �O
		case SC_ELEMENTPOISON:		// ��
		//case SC_ELEMENTUNDEAD:	// �s��
			if(sd)
				clif_displaymessage(sd->fd, msg_txt(203));	// �h��̑��������ɖ߂�܂���
			break;
		case SC_RACEUNKNOWN:
		case SC_RACEUNDEAD:
		case SC_RACEBEAST:
		case SC_RACEPLANT:
		case SC_RACEINSECT:
		case SC_RACEFISH:
		case SC_RACEDEVIL:
		case SC_RACEHUMAN:
		case SC_RACEANGEL:
		case SC_RACEDRAGON:
			if(sd)
				clif_displaymessage(sd->fd, msg_txt(205));	// �푰�����ɖ߂�܂���
			break;
		case SC_RUN:			/* �^�C���M */
		case SC_WUGDASH:		/* �E�H�[�O�_�b�V�� */
			unit_stop_walking(bl,0);
			calc_flag = 1;
			break;
		case SC_MONK:			/* �����N�̍� */
		case SC_STAR:			/* �P���Z�C�̍� */
		case SC_SAGE:			/* �Z�[�W�̍� */
		case SC_CRUSADER:		/* �N���Z�C�_�[�̍� */
		case SC_WIZARD:			/* �E�B�U�[�h�̍� */
		case SC_PRIEST:			/* �v���[�X�g�̍� */
		case SC_ROGUE:			/* ���[�O�̍� */
		case SC_ASSASIN:		/* �A�T�V���̍� */
		case SC_SOULLINKER:		/* �\�E�������J�[�̍� */
		case SC_SUPERNOVICE:		/* �X�[�p�[�m�[�r�X�̍� */
		case SC_GUNNER:			/* �K���X�����K�[�̍� */
		case SC_NINJA:			/* �E�҂̍� */
		case SC_DEATHKINGHT:		/* �f�X�i�C�g�̍� */
		case SC_COLLECTOR:		/* �R���N�^�[�̍� */
			if(sd && battle_config.disp_job_soul_state_change)
				clif_disp_onlyself(sd->fd, msg_txt(207));	// ����Ԃ��I�����܂���
			break;
		case SC_KNIGHT:			/* �i�C�g�̍� */
		case SC_ALCHEMIST:		/* �A���P�~�X�g�̍� */
		case SC_BARDDANCER:		/* �o�[�h�ƃ_���T�[�̍� */
		case SC_BLACKSMITH:		/* �u���b�N�X�~�X�̍� */
		case SC_HUNTER:			/* �n���^�[�̍� */
		case SC_HIGH:			/* �ꎟ��ʐE�Ƃ̍� */
			if(sd && battle_config.disp_job_soul_state_change)
				clif_disp_onlyself(sd->fd, msg_txt(207));	// ����Ԃ��I�����܂���
			calc_flag = 1;
			break;
		case SC_POEMBRAGI:			/* �u���M */
		case SC_WHISTLE:			/* ���J */
		case SC_ASSNCROS:			/* �[�z�̃A�T�V���N���X */
		case SC_APPLEIDUN:			/* �C�h�D���̗ь� */
		case SC_HUMMING:			/* �n�~���O */
		case SC_DONTFORGETME:			/* ����Y��Ȃ��� */
		case SC_FORTUNE:			/* �K�^�̃L�X */
		case SC_SERVICE4U:			/* �T�[�r�X�t�H�[���[ */
			calc_flag = 1;
			if(type == SC_DONTFORGETME)
				ud->state.change_speed = 1;
			// �x�艉�t�����Z�b�g
			if(sc->data[type + SC_WHISTLE_ - SC_WHISTLE].timer == -1)
				status_change_start(bl,type + SC_WHISTLE_ - SC_WHISTLE,sc->data[type].val1,
					sc->data[type].val2,sc->data[type].val3,sc->data[type].val4,battle_config.dance_and_play_duration,0);
			break;
		case SC_WHISTLE_:			/* ���J */
		case SC_ASSNCROS_:			/* �[�z�̃A�T�V���N���X */
		case SC_APPLEIDUN_:			/* �C�h�D���̗ь� */
		case SC_HUMMING_:			/* �n�~���O */
		case SC_DONTFORGETME_:			/* ����Y��Ȃ��� */
		case SC_FORTUNE_:			/* �K�^�̃L�X */
		case SC_SERVICE4U_:			/* �T�[�r�X�t�H�[���[ */
			calc_flag = 1;
			break;
		case SC_MARIONETTE:			/* �}���I�l�b�g�R���g���[�� (����) */
		case SC_MARIONETTE2:			/* �}���I�l�b�g�R���g���[�� (����) */
			{
				struct block_list *tbl = map_id2bl(sc->data[type].val2);
				if(tbl) {
					struct status_change *tsc = status_get_sc(tbl);
					int tmp = (type == SC_MARIONETTE)? SC_MARIONETTE2: SC_MARIONETTE;
					// �������}���I�l�b�g��ԂȂ炢������ɉ���
					if(tsc && tsc->data[tmp].timer != -1)
						status_change_end(tbl, tmp, -1);
				}
			}
			calc_flag = 1;
			break;
		case SC_BERSERK:			/* �o�[�T�[�N */
			calc_flag = 1;
			if(sd) {
				clif_status_load_id(sd,SI_INCREASEAGI,0);	// �A�C�R������
				status_change_start(bl,SC_NATURAL_HEAL_STOP,0,0,0,0,skill_get_time2(LK_BERSERK,sc->data[type].val1),0);
			}
			break;
		case SC_HALLUCINATION:
			if(sd)
				clif_status_load_id(sd,SI_HALLUCINATION,0);	// �A�C�R������
			break;

		case SC_ENDURE:				/* �C���f���A */
			calc_flag = 1;
			// fall through
		case SC_AUTOGUARD:			/* �I�[�g�K�[�h */
		case SC_REFLECTSHIELD:			/* ���t���N�g�V�[���h */
			if(sd) {
				struct map_session_data *tsd;
				int i;
				// ��f�B�{�[�V�����҂���������
				for(i = 0; i < 5; i++) {
					if(sd->dev.val1[i] && (tsd = map_id2sd(sd->dev.val1[i])) != NULL && tsd->sc.data[type].timer != -1)
						status_change_end(&tsd->bl, type, -1);
				}
			}
			break;

		case SC_DEVOTION:		/* �f�B�{�[�V���� */
			{
				struct map_session_data *dsd = map_id2sd(sc->data[type].val1);

				sc->data[type].val1 = 0;
				sc->data[type].val2 = 0;

				if(sc->data[SC_AUTOGUARD].timer != -1)
					status_change_end(bl, SC_AUTOGUARD, -1);
				if(sc->data[SC_DEFENDER].timer != -1)
					status_change_end(bl, SC_DEFENDER, -1);
				if(sc->data[SC_REFLECTSHIELD].timer != -1)
					status_change_end(bl, SC_REFLECTSHIELD, -1);
				if(sc->data[SC_ENDURE].timer != -1)
					status_change_end(bl, SC_ENDURE, -1);
				if(dsd)
					skill_devotion(dsd);
			}
			break;
		case SC_BLADESTOP:
			{
				struct block_list *tbl = map_id2bl(sc->data[type].val4);
				if(tbl) {
					struct status_change *tsc = status_get_sc(tbl);
					// �Е����؂ꂽ�̂ő���̔��n��Ԃ��؂�ĂȂ��̂Ȃ����
					if(tsc && tsc->data[SC_BLADESTOP].timer != -1)
						status_change_end(tbl,SC_BLADESTOP,-1);
					if(sc->data[type].val2 == 2)
						clif_bladestop(map_id2bl(sc->data[type].val3),tbl->id,0);
				}
			}
			break;
		case SC_CLOSECONFINE:
		case SC_STOP:
			{
				struct block_list *tbl = map_id2bl(sc->data[type].val4);
				if(tbl) {
					struct status_change *tsc = status_get_sc(tbl);
					// �Е����؂ꂽ�̂ő��肪�؂�ĂȂ��̂Ȃ����
					if(tsc && tsc->data[type].timer != -1)
						status_change_end(tbl,type,-1);
				}
				calc_flag = 1;
			}
			break;
		case SC_DANCING:
			{
				struct map_session_data *dsd;
				// �����肾�������ŃA�C�R������
				if(sc->data[type].val1 == CG_MOONLIT)
					clif_status_change(bl,SI_MOONLIT,0,0,0,0,0);	// �A�C�R������

				if(sc->data[type].val4 && (dsd = map_id2sd(sc->data[type].val4))) {
					// ���t�ő��肪����ꍇ�����val4��0�ɂ���
					if(dsd->sc.data[type].timer != -1)
						dsd->sc.data[type].val4 = 0;
				}
				if(sc->data[SC_LONGINGFREEDOM].timer != -1)
					status_change_end(bl,SC_LONGINGFREEDOM,-1);
			}
			calc_flag = 1;
			ud->state.change_speed = 1;
			break;
		case SC_GOSPEL:
		case SC_GRAFFITI:
		case SC_WARM:
		case SC_GRAVITATION_USER:
			{
				struct skill_unit_group *sg = map_id2sg(sc->data[type].val4);	// val4��group_id
				sc->data[type].val4 = 0;
				if(sg)
					skill_delunitgroup(sg);
			}
			break;
		case SC_NOCHAT:			/* �`���b�g�֎~��� */
			if(sd)
				clif_updatestatus(sd,SP_MANNER);
			break;
		case SC_SPLASHER:		/* �x�i���X�v���b�V���[ */
			{
				struct block_list *src = map_id2bl(sc->data[type].val3);
				if(src && tid != -1) {
					// �����Ƀ_���[�W������3*3�Ƀ_���[�W
					skill_castend_damage_id(src,bl,sc->data[type].val2,sc->data[type].val1,gettick(),0);
				}
			}
			break;
		case SC_ANKLE:
		case SC_THORNS_TRAP:	/* �\�[���g���b�v */
			{
				struct skill_unit_group *sg = map_id2sg(sc->data[type].val2);
				// skill_delunitgroup����status_change_end ���Ă΂�Ȃ��ׂɁA
				// ��[�������Ă��Ȃ����ɂ��Ă���O���[�v�폜����B
				if(sg) {
					sg->val2 = 0;
					skill_delunitgroup(sg);
				}
			}
			break;
		case SC_SELFDESTRUCTION:	/* ���� */
			unit_stop_walking(bl,5);
			if(md) {
				md->mode &= ~MD_CANMOVE;
				md->state.special_mob_ai = 2;
			}
			break;
		case SC_BUNSINJYUTSU:	/* ���g�̏p */
			if(sd) {
				int color = sc->data[SC_BUNSINJYUTSU].val4;
				if(color > 0)
					pc_changelook(sd, LOOK_CLOTHES_COLOR, color);
			}
			break;
		case SC_SEVENWIND:	/* �g������ */
			if(sd)
				clif_status_load_id(sd,sc->data[type].val2,0);
			break;
		case SC_AUTOBERSERK:
			if(sc->data[SC_PROVOKE].timer != -1 && sc->data[SC_PROVOKE].val2 == 1) {
				status_change_end(bl,SC_PROVOKE,-1);
			}
			break;
		case SC_HALLUCINATIONWALK:	/* �n���V�l�[�V�����E�H�[�N */
			// �y�i���e�B�J�n
			status_change_start(bl,SC_HALLUCINATIONWALK2,0,0,0,0,skill_get_time2(GC_HALLUCINATIONWALK,sc->data[type].val1),0);
			calc_flag = 1;
			ud->state.change_speed = 1;
			break;
		case SC_WHITEIMPRISON:	/* �z���C�g�C���v���Y�� */
			{
				int dmg = 400 * sc->data[type].val1;
				clif_damage(bl,bl,gettick(),0,0,dmg,0,9,0,0);
				battle_damage(bl,bl,dmg,0,0,0);
			}
			break;
		case SC_SPELLBOOK:			/* �X�y���u�b�N */
			if(sd) {
				// �ۑ��X�L������������
				memset(sd->freeze_sp_skill, 0, sizeof(sd->freeze_sp_skill[0])*MAX_FREEZE_SPELL);
				sd->freeze_sp_slot = 0;		// �X���b�g����������
			}
			break;
		case SC_NEUTRALBARRIER_USER:	/* �j���[�g�����o���A�[(�g�p��) */
		case SC_STEALTHFIELD_USER:		/* �X�e���X�t�B�[���h(�g�p��) */
			{
				struct skill_unit_group *sg = map_id2sg(sc->data[type].val4);	// val4��group_id
				sc->data[type].val4 = 0;
				if(sg)
					skill_delunitgroup(sg);
				calc_flag = 1;
				ud->state.change_speed = 1;
			}
			break;
		case SC__SHADOWFORM:		/* �V���h�E�t�H�[�� */
			{
				struct map_session_data *dsd;
				if((dsd = map_id2sd(sc->data[type].val2)) != NULL)
					dsd->shadowform_id = 0;
			}
			break;
		case SC_FORCEOFVANGUARD:	/* �t�H�[�X�I�u�o���K�[�h */
			clif_mshield(sd, 0);
			calc_flag = 1;
			break;
		case SC_BANDING:		/* �o���f�B���O */
			{
				struct skill_unit_group *sg = map_id2sg(sc->data[type].val4);
				sc->data[type].val4 = 0;
				if(sg)
					skill_delunitgroup(sg);
				calc_flag = 1;
			}
			break;
		case SC_RAISINGDRAGON:		/* �������V */
			if(sd) {
				int max = pc_checkskill(sd,MO_CALLSPIRITS);
				if(sd->spiritball.num > max)
					pc_delspiritball(sd,sd->spiritball.num-max,0);
			}
			calc_flag = 1;
			break;
		case SC_SATURDAY_NIGHT_FEVER:	/* �t���C�f�[�i�C�g�t�B�[�o�[ */
			status_change_start(bl,SC_SITDOWN_FORCE,0,0,0,0,3000,0);
			calc_flag = 1;
			break;
		case SC_SPORE_EXPLOSION:	/* �X�|�A�G�N�X�v���[�W���� */
			{
				struct block_list *src = map_id2bl(sc->data[type].val3);
				if(src && tid != -1) {
					skill_castend_damage_id(src,bl,sc->data[type].val2,sc->data[type].val1,gettick(),0);
				}
			}
			break;
		case SC_SUMMON_ELEM:	/* �T�����G�������^�� */
			if(sd && sd->eld)
				elem_delete_data(sd);
			break;
		/* option1 */
		case SC_FREEZE:
			sc->data[type].val3 = 0;
			break;

		/* option2 */
		case SC_POISON:				/* �� */
		case SC_BLIND:				/* �Í� */
		case SC_CURSE:
			calc_flag = 1;
			break;
	}

	if(StatusIconChangeTable[type] != SI_BLANK)	// �A�C�R������
		clif_status_load(bl,StatusIconChangeTable[type],0);

	switch(type) {	/* ����ɖ߂�Ƃ��Ȃɂ��������K�v */
		// opt1
		case SC_STONE:
		case SC_FREEZE:
		case SC_STUN:
		case SC_SLEEP:
		case SC_HELLINFERNO:
		case SC_WHITEIMPRISON:
			sc->opt1 = OPT1_NORMAL;
			opt_flag = 1;
			break;
		// opt2
		case SC_POISON:
			sc->opt2 &= ~OPT2_POISON;
			opt_flag = 1;
			break;
		case SC_CURSE:
		case SC_SILENCE:
		case SC_CONFUSION:
			sc->opt2 &= ~(OPT2_POISON<<(type-SC_POISON));
			opt_flag = 1;
			if(type == SC_CURSE)
				ud->state.change_speed = 1;
			break;
		case SC_FOGWALLPENALTY:
			if(sc->data[SC_BLIND].timer == -1) {
				sc->opt2 &= ~OPT2_BLIND;
				opt_flag = 1;
			}
			break;
		case SC_BLIND:
			if(sc->data[SC_FOGWALLPENALTY].timer == -1) {
				sc->opt2 &= ~OPT2_BLIND;
				opt_flag = 1;
			}
			break;
		case SC_ANGELUS:			/* �A���[���X */
			sc->opt2 &= ~OPT2_ANGELUS;
			opt_flag = 1;
			break;
		case SC_DPOISON:
			sc->opt2 &= ~OPT2_HEAVYPOISON;
			opt_flag = 1;
			break;
		case SC_BLEED:
			sc->opt2 &= ~OPT2_BLEEDING;
			opt_flag = 1;
			break;
		case SC_FEAR:
			sc->opt2 &= ~OPT2_FEAR;
			opt_flag = 1;
			break;
		// opt3
		case SC_ONEHAND:		/* 1HQ */
		case SC_TWOHANDQUICKEN:		/* 2HQ */
		case SC_SPEARQUICKEN:		/* �X�s�A�N�C�b�P�� */
		case SC_CONCENTRATION:		/* �R���Z���g���[�V���� */
		case SC_WEAPONQUICKEN:		/* �E�F�|���N�C�b�P�� */
			sc->opt3 &= ~OPT3_QUICKEN;
			opt_flag = 2;
			break;
		case SC_OVERTHRUST:		/* �I�[�o�[�g���X�g */
		case SC_SWOO:			/* �G�X�E */
			sc->opt3 &= ~OPT3_OVERTHRUST;
			opt_flag = 2;
			if(type == SC_SWOO)
				ud->state.change_speed = 1;
			break;
		case SC_ENERGYCOAT:		/* �G�i�W�[�R�[�g */
		case SC_SKE:			/* �G�X�N */
			sc->opt3 &= ~OPT3_ENERGYCOAT;
			opt_flag = 2;
			break;
		case SC_EXPLOSIONSPIRITS:	/* �����g�� */
			sc->opt3 &= ~OPT3_EXPLOSIONSPIRITS;
			opt_flag = 2;
			break;
		case SC_STEELBODY:		/* ���� */
		case SC_SKA:			/* �G�X�J */
			sc->opt3 &= ~OPT3_STEELBODY;
			opt_flag = 2;
			break;
		case SC_BLADESTOP:		/* ���n��� */
			sc->opt3 &= ~OPT3_BLADESTOP;
			opt_flag = 2;
			break;
		case SC_AURABLADE:			/* �I�[���u���[�h */
			sc->opt3 &= ~OPT3_AURABLADE;
			opt_flag = 2;
			break;
		case SC_BERSERK:		/* �o�[�T�[�N */
			sc->opt3 &= ~OPT3_BERSERK;
			opt_flag = 2;
			break;
		case SC_DANCING:			/* �_���X/���t�� */
			if(sc->data[SC_DANCING].val1 != CG_MOONLIT)
				break;
			sc->opt3 &= ~OPT3_MOON;
			opt_flag = 2;
			break;
		case SC_MARIONETTE:		/* �}���I�l�b�g�R���g���[�� */
		case SC_MARIONETTE2:		/* �}���I�l�b�g�R���g���[�� */
			sc->opt3 &= ~OPT3_MARIONETTE;
			opt_flag = 2;
			break;
		case SC_ASSUMPTIO:		/* �A�X���v�e�B�I */
		case SC_ASSUMPTIO2:		/* �L���b�V���A�X���v�e�B�I */
			sc->opt3 &= ~OPT3_ASSUMPTIO;
			opt_flag = 2;
			break;
		case SC_WARM:			/* ������ */
			sc->opt3 &= ~OPT3_SUN_WARM;
			opt_flag = 2;
			break;
		case SC_KAITE:			/* �J�C�g */
			sc->opt3 &= ~OPT3_KAITE;
			opt_flag = 2;
			break;
		case SC_BUNSINJYUTSU:		/* ���g�̏p */
			sc->opt3 &= ~OPT3_BUNSIN;
			opt_flag = 2;
			break;
		case SC_MONK:			/* �����N�̍� */
		case SC_STAR:			/* �P���Z�C�̍� */
		case SC_SAGE:			/* �Z�[�W�̍� */
		case SC_CRUSADER:		/* �N���Z�C�_�[�̍� */
		case SC_WIZARD:			/* �E�B�U�[�h�̍� */
		case SC_PRIEST:			/* �v���[�X�g�̍� */
		case SC_ROGUE:			/* ���[�O�̍� */
		case SC_ASSASIN:		/* �A�T�V���̍� */
		case SC_SOULLINKER:		/* �\�E�������J�[�̍� */
		case SC_KNIGHT:			/* �i�C�g�̍� */
		case SC_ALCHEMIST:		/* �A���P�~�X�g�̍� */
		case SC_BARDDANCER:		/* �o�[�h�ƃ_���T�[�̍� */
		case SC_BLACKSMITH:		/* �u���b�N�X�~�X�̍� */
		case SC_HUNTER:			/* �n���^�[�̍� */
		case SC_HIGH:			/* �ꎟ��ʐE�Ƃ̍� */
		case SC_SUPERNOVICE:		/* �X�[�p�[�m�[�r�X�̍� */
		case SC_GUNNER:			/* �K���X�����K�[�̍� */
		case SC_NINJA:			/* �E�҂̍� */
		case SC_DEATHKINGHT:		/* �f�X�i�C�g�̍� */
		case SC_COLLECTOR:		/* �R���N�^�[�̍� */
			sc->opt3 &= ~OPT3_SOULLINK;
			opt_flag = 2;
			break;
		case SC_ELEMENTUNDEAD:		// �s��
			sc->opt3 &= ~OPT3_UNDEAD;
			opt_flag = 2;
			break;
		// option
		case SC_SIGHT:
			sc->option &= ~OPTION_SIGHT;
			opt_flag = 1;
			break;
		case SC_HIDING:
			// ���a��łȂ��ʏ�̃n�C�h�Ȃ�A�C�R������
			if(sd && sc->data[type].val3 == 0)
				clif_status_load_id(sd,SI_HIDING,0);
			sc->option &= ~OPTION_HIDE;
			opt_flag = 1;
			break;
		case SC_CLOAKING:
		case SC_CLOAKINGEXCEED:		/* �N���[�L���O�G�N�V�[�h */
		case SC__INVISIBILITY:		/* �C���r�W�r���e�B */
			sc->option &= ~OPTION_CLOAKING;
			opt_flag = 1;
			break;
		case SC_INVISIBLE:
			sc->option &= ~OPTION_SPECIALHIDING;
			opt_flag = 1;
			break;
		case SC_REVERSEORCISH:
			sc->option &= ~OPTION_ORCFACE;
			opt_flag = 1;
			break;
		case SC_WEDDING:		/* �E�F�f�B���O */
			sc->option &= ~OPTION_MARRIED;
			opt_flag = 1;
			break;
		case SC_RUWACH:
			sc->option &= ~OPTION_RUWACH;
			opt_flag = 1;
			break;
		case SC_CHASEWALK:		/* �`�F�C�X�E�H�[�N */
			sc->option &= ~(OPTION_CLOAKING | OPTION_FOOTPRINT);
			opt_flag = 1;
			break;
		case SC_FUSION:
			sc->option &= ~OPTION_STAR2;
			opt_flag = 1;
			break;
		case SC_SANTA:
			sc->option &= ~OPTION_SANTA;
			opt_flag = 1;
			break;
		case SC_SUMMER:
			sc->option &= ~OPTION_SUMMER;
			opt_flag = 1;
			break;
		case SC_ON_PUSH_CART:
#if PACKETVER < 20120201
			// �Â��N���C�A���g�̓I�v�V�������X�V����
			sc->option &= ~sc->data[type].val1;
			opt_flag = 1;
#endif
			break;
	}

	/* option�̕ύX */
	if(opt_flag == 1) {
		clif_changeoption(bl);
	} else if(opt_flag == 2) {
		clif_changeoption2(bl);
	}

#ifdef DYNAMIC_SC_DATA
	if(sc->count <= 0 && battle_config.free_sc_data_dynamically)
		status_free_sc_data(sc);
#endif

	/* �X�e�[�^�X�Čv�Z */
	if(sd) {
		if(calc_flag || sd->auto_status_calc_pc[type] == 1) {
			status_calc_pc(sd,0);
		}
	} else if(hd) {
		if(calc_flag) {
			homun_calc_status(hd);
			clif_send_homstatus(hd->msd,0);
		}
	} else if(mcd) {
		if(calc_flag) {
			merc_calc_status(mcd);
			clif_send_mercstatus(mcd->msd);
			clif_mercskillinfoblock(mcd->msd);
		}
	} else if(eld) {
		if(calc_flag) {
			elem_calc_status(eld);
			clif_send_elemstatus(eld->msd);
		}
	}

	return 0;
}

/*==========================================
 * �X�e�[�^�X�Čv�Z���ꎞ��~����
 *------------------------------------------
 */
int status_calc_pc_stop_begin(struct block_list *bl)
{
	nullpo_retr(0, bl);

	if(bl->type == BL_PC)
		((struct map_session_data *)bl)->stop_status_calc_pc++;
	return 0;
}

/*==========================================
 * �X�e�[�^�X�Čv�Z���ĊJ����
 *------------------------------------------
 */
int status_calc_pc_stop_end(struct block_list *bl)
{
	nullpo_retr(0, bl);

	if(bl->type == BL_PC) {
		struct map_session_data *sd = (struct map_session_data *)bl;
		sd->stop_status_calc_pc--;
		if(sd->stop_status_calc_pc == 0 && sd->call_status_calc_pc_while_stopping > 0)
			status_calc_pc(sd,0);
		if(sd->stop_status_calc_pc < 0) {
			printf("status_calc_pc_stop_end���s���ɌĂяo����Ă��܂�\n");
		}
	}
	return 0;
}

/*==========================================
 * �X�e�[�^�X�ُ�J�n�^�C�}�[
 *------------------------------------------
 */
static int status_pretimer_timer(int tid, unsigned int tick, int id, void *data)
{
	struct block_list *bl = map_id2bl(id);
	struct status_pretimer *stpt = NULL;
	struct unit_data *ud;

	if(bl == NULL)
		return 0;	// �Y��ID�����łɏ��ł��Ă���

	nullpo_retr(0, ud = unit_bl2ud(bl));

	do {
		if(bl->prev == NULL)
			break;

		stpt = (struct status_pretimer*)data;
		stpt->timer = -1;

		if(linkdb_erase(&ud->statuspretimer, stpt) == NULL) {
			printf("status_pretimer: data lost !!\n");
			break;
		}

		if(stpt->target_id) {
			struct block_list *target = map_id2bl(stpt->target_id);
			if( target == NULL || bl->m != target->m || unit_isdead(bl) || unit_isdead(target) )
				break;
		} else {
			if(bl->m != stpt->m)
				break;
		}

		status_change_start(bl, stpt->type, stpt->val1, stpt->val2, stpt->val3, stpt->val4, stpt->tick, stpt->flag);
	} while(0);

	aFree(stpt);

	return 0;
}

/*==========================================
 * �X�e�[�^�X�ُ�J�n�^�C�}�[�̍폜
 *------------------------------------------
 */
int status_clearpretimer(struct block_list *bl)
{
	struct unit_data *ud;
	struct linkdb_node *node1, *node2;

	nullpo_retr(0, bl);
	nullpo_retr(0, ud = unit_bl2ud(bl));

	node1 = ud->statuspretimer;
	while(node1) {
		struct status_pretimer *stpt = (struct status_pretimer *)node1->data;
		if(stpt->timer != -1) {
			delete_timer(stpt->timer, status_pretimer_timer);
		}
		node2 = node1->next;
		aFree(stpt);
		node1 = node2;
	}
	linkdb_final(&ud->statuspretimer);

	return 0;
}

/*==========================================
 * �X�e�[�^�X�ُ�J�n�^�C�}�[�̃Z�b�g
 *------------------------------------------
 */
int status_change_pretimer(struct block_list *bl,int type,int val1,int val2,int val3,int val4,int tick,int flag,int pre_tick)
{
	struct unit_data *ud;
	struct status_pretimer *stpt;

	nullpo_retr(1, bl);
	ud = unit_bl2ud(bl);
	if(ud == NULL)
		return 0;

	stpt = (struct status_pretimer *)aCalloc(1, sizeof(struct status_pretimer));
	stpt->timer     = add_timer(pre_tick, status_pretimer_timer, bl->id, stpt);
	stpt->target_id = bl->id;
	stpt->m         = bl->m;
	stpt->type      = type;
	stpt->val1      = val1;
	stpt->val2      = val2;
	stpt->val3      = val3;
	stpt->val4      = val4;
	stpt->tick      = tick;
	stpt->flag      = flag;

	linkdb_insert(&ud->statuspretimer, stpt, stpt);

	return 0;
}

/*==========================================
 * �X�e�[�^�X�ُ�I���^�C�}�[
 *
 * �\�[�X���C��������ւ̒���
 *
 * �E��Ԉُ�p�����ɂ́Aadd_timer() ��������� break ���邱��
 * �E��Ԉُ�I�����ɂ́A�֐��̍Ō�ɂ���status_change_end() ��
 *   �Ăяo���O�� return ���Ȃ�����
 *
 * ���̂Q�_������Ă��Ȃ��ƁA���l�̏�Ԉُ킪����ɉ������ꂽ��A
 * delete_timer error���o�Ă���Ȃǂ̃o�O���������܂��B
 *------------------------------------------
 */
int status_change_timer(int tid, unsigned int tick, int id, void *data)
{
	int type  = PTR2INT(data);
	int timer = -1;
	struct block_list *bl;
	struct map_session_data *sd  = NULL;
	struct mob_data         *md  = NULL;
	struct homun_data       *hd  = NULL;
	struct merc_data        *mcd = NULL;
	struct elem_data        *eld = NULL;
	struct status_change    *sc  = NULL;

	if(type < 0 || type >= MAX_STATUSCHANGE)
		return 0;

	if((bl = map_id2bl(id)) == NULL)
		return 0;	// �Y��ID�����łɏ��ł��Ă���Ƃ����̂͂����ɂ����肻���Ȃ̂ŃX���[���Ă݂�

	nullpo_retr(0, sc = status_get_sc(bl));

	sd  = BL_DOWNCAST( BL_PC,   bl );
	md  = BL_DOWNCAST( BL_MOB,  bl );
	hd  = BL_DOWNCAST( BL_HOM,  bl );
	mcd = BL_DOWNCAST( BL_MERC, bl );
	eld = BL_DOWNCAST( BL_ELEM, bl );

	if(sc->data[type].timer != tid) {
		if(battle_config.error_log)
			printf("status_change_timer %d != %d (type = %d)\n",tid,sc->data[type].timer,type);
		return 0;
	}

	map_freeblock_lock();

	switch(type) {	/* ����ȏ����ɂȂ�ꍇ */
	case SC_MAXIMIZEPOWER:	/* �}�L�V�}�C�Y�p���[ */
	case SC_CLOAKING:	/* �N���[�L���O */
	case SC_INVISIBLE:	/* �C���r�W�u�� */
		if(sd) {
			if(sd->status.sp > 0) {	/* SP�؂��܂Ŏ��� */
				sd->status.sp--;
				clif_updatestatus(sd,SP_SP);
				timer = add_timer(	/* �^�C�}�[�Đݒ� */
					sc->data[type].val2+tick, status_change_timer,
					bl->id, data);
			}
		}
		break;

	case SC_CHASEWALK:	/* �`�F�C�X�E�H�[�N */
		if(sd) {
			int sp = 10+sc->data[SC_CHASEWALK].val1*2;
			if(map[sd->bl.m].flag.gvg)
				sp *= 5;
			if(sd->status.sp > sp) {
				sd->status.sp -= sp;
				clif_updatestatus(sd,SP_SP);
				if((++sc->data[SC_CHASEWALK].val4) == 1) {
					// ���[�O�̍��Ȃ���ʎ���10�{
					status_change_start(
						bl, SC_CHASEWALK_STR, 1<<(sc->data[SC_CHASEWALK].val1-1), 0, 0, 0,
						((sc->data[SC_ROGUE].timer != -1)? 300000: 30000), 0
					);
				}
				timer = add_timer( /* �^�C�}�[�Đݒ� */
					sc->data[type].val2+tick, status_change_timer, bl->id, data);
			}
		}
		break;

	case SC_HIDING:		/* �n�C�f�B���O */
		if(sd) {		/* SP�������āA���Ԑ����̊Ԃ͎��� */
			if(sd->status.sp > 0 && (--sc->data[type].val2) > 0) {
#ifdef PRE_RENEWAL
				if(sc->data[type].val2 % (sc->data[type].val1+3) == 0) {
#else
				if(sc->data[type].val2 % (sc->data[type].val1+4) == 0) {
#endif
					sd->status.sp--;
					clif_updatestatus(sd,SP_SP);
				}
				timer = add_timer(	/* �^�C�}�[�Đݒ� */
					1000+tick, status_change_timer,
					bl->id, data);
			}
		}
		break;

	case SC_SIGHTBLASTER:
	case SC_SIGHT:		/* �T�C�g */
	case SC_RUWACH:		/* ���A�t */
		{
			int range;
			if(type == SC_RUWACH)
				range = 2;
			else if(type == SC_SIGHTBLASTER)
				range = 1;
			else
				range = (sc->data[type].val4 == NPC_WIDESIGHT)? 14: 3;

			map_foreachinarea(status_change_timer_sub,
				bl->m, bl->x-range, bl->y-range, bl->x+range,bl->y+range, BL_CHAR,
				bl, type, sc->data[type].val1, tick);

			if((--sc->data[type].val2) > 0) {
				timer = add_timer(	/* �^�C�}�[�Đݒ� */
					250+tick, status_change_timer,
					bl->id, data);
			}
		}
		break;

	case SC_SIGNUMCRUCIS:		/* �V�O�i���N���V�X */
		{
			int race = status_get_race(bl);
			if(race == RCT_DEMON || battle_check_undead(race,status_get_elem_type(bl))) {
				timer = add_timer(1000*600+tick, status_change_timer, bl->id, data);
			}
		}
		break;

	case SC_PROVOKE:	/* �v���{�b�N/�I�[�g�o�[�T�[�N */
		if(sc->data[type].val2 != 0) {	/* �I�[�g�o�[�T�[�N�i�P�b���Ƃ�HP�`�F�b�N�j */
			if(status_get_hp(bl) > status_get_max_hp(bl)>>2)	/* ��~ */
				break;
			timer = add_timer(1000+tick, status_change_timer, bl->id, data);
		}
		break;

	case SC_DISSONANCE:	/* �s���a�� */
		if((--sc->data[type].val2) > 0) {
			struct skill_unit *unit = map_id2su(sc->data[type].val4);
			struct block_list *src;
			if(!unit || !unit->group)
				break;
			src = map_id2bl(unit->group->src_id);
			if(!src)
				break;
			battle_skill_attack(BF_MISC,src,&unit->bl,bl,unit->group->skill_id,sc->data[type].val1,tick,0);
			timer = add_timer(
				skill_get_time2(unit->group->skill_id,unit->group->skill_lv)+tick, status_change_timer,
				bl->id, data);
		}
		break;
	case SC_UGLYDANCE:	/* ��������ȃ_���X */
		if((--sc->data[type].val2) > 0) {
			struct skill_unit *unit = map_id2su(sc->data[type].val4);
			struct block_list *src;
			if(!unit || !unit->group)
				break;
			src = map_id2bl(unit->group->src_id);
			if(!src)
				break;
			skill_additional_effect(src,bl,unit->group->skill_id,sc->data[type].val1,0,tick);
			timer = add_timer(
				skill_get_time2(unit->group->skill_id,unit->group->skill_lv)+tick, status_change_timer,
				bl->id, data);
		}
		break;

	case SC_LULLABY:	/* �q��S */
		if((--sc->data[type].val2) > 0) {
			struct skill_unit *unit = map_id2su(sc->data[type].val4);
			int interval;

			if(!unit || !unit->group || unit->group->src_id == bl->id)
				break;
#ifdef PRE_RENEWAL
			interval = skill_get_time(unit->group->skill_id,unit->group->skill_lv)/10;
#else
			interval = skill_get_time(unit->group->skill_id,unit->group->skill_lv)/15;
#endif
			skill_additional_effect(bl,bl,unit->group->skill_id,sc->data[type].val1,BF_LONG|BF_SKILL|BF_MISC,tick);
			timer = add_timer(interval+tick, status_change_timer, bl->id, data);
		}
		break;

	case SC_STONE:
		if(sc->data[type].val2 != 0) {
			sc->data[type].val2 = 0;
			sc->data[type].val4 = 0;
			unit_stop_walking(bl,1);
			sc->opt1 = OPT1_STONECURSE;
			clif_changeoption(bl);
			timer = add_timer(1000+tick, status_change_timer, bl->id, data);
		}
		else if((--sc->data[type].val3) > 0) {
			int hp = status_get_max_hp(bl);
			if((++sc->data[type].val4)%5 == 0 && status_get_hp(bl) > hp>>2) {
				hp = (hp < 100)? 1: hp/100;
				unit_heal(bl,-hp,0);
			}
			timer = add_timer(1000+tick, status_change_timer, bl->id, data);
		}
		break;
	case SC_POISON:
		if(sc->data[SC_SLOWPOISON].timer == -1 && (--sc->data[type].val3) > 0) {
			int hp    = status_get_hp(bl);
			int p_dmg = status_get_max_hp(bl);
			if(hp > p_dmg>>2) {		// �ő�HP��25%�ȏ�
				p_dmg = 3 + p_dmg*3/200;
				if(p_dmg >= hp)
					p_dmg = hp-1;	// �łł͎��ȂȂ�
				unit_heal(bl, -p_dmg, 0);
			}
		}
		if(sc->data[type].val3 > 0) {
			timer = add_timer(1000+tick, status_change_timer, bl->id, data);
		}
		break;
	case SC_DPOISON:
		if(sc->data[SC_SLOWPOISON].timer == -1 && (--sc->data[type].val3) > 0) {
			int hp = status_get_max_hp(bl);
			if(status_get_hp(bl) > hp>>2) {
				hp = 3 + hp/50;
				unit_heal(bl, -hp, 0);
			}
		}
		if(sc->data[type].val3 > 0 && !unit_isdead(bl) && sc->data[type].timer != -1) {
			// �����Ă��ĉ����ς݂łȂ��Ȃ�p��
			timer = add_timer(1000+tick, status_change_timer, bl->id, data);
		}
		break;
	case SC_BLEED:
		if(--sc->data[type].val3 > 0) {
			int dmg = atn_rand()%600 + 200;
			if(md) {
				// mob��HP50�ȉ��ɂȂ�Ȃ�
				md->hp = (md->hp - dmg < 50)? 50: md->hp - dmg;
			} else {
				unit_heal(bl, -dmg, 0);
			}
			if(!unit_isdead(bl) && sc->data[type].timer != -1) {
				// �����Ă��ĉ����ς݂łȂ��Ȃ�p��
				timer = add_timer(10000+tick, status_change_timer, bl->id, data);
			}
		}
		break;
	case SC_TENSIONRELAX:	/* �e���V���������b�N�X */
		if(sd) {		/* HP�����^���łȂ���Όp�� */
			if(sd->status.max_hp > sd->status.hp) {
				if(sc->data[type].val2 % (sc->data[type].val1+3) == 0)
					timer = add_timer(	/* �^�C�}�[�Đݒ� */
						10000+tick, status_change_timer,
						bl->id, data);
			} else if(sd->status.max_hp <= sd->status.hp) {
				status_change_end(&sd->bl,SC_TENSIONRELAX,-1);
			}
		}
		break;

	/* ���Ԑ؂ꖳ���H�H */
	case SC_AETERNA:
	case SC_TRICKDEAD:
	case SC_REJECTSWORD:		/* ���W�F�N�g�\�[�h */
	case SC_MEMORIZE:		/* �������C�Y */
	case SC_SACRIFICE:		/* �T�N���t�@�C�X */
	case SC_READYSTORM:
	case SC_READYDOWN:
	case SC_READYTURN:
	case SC_READYCOUNTER:
	case SC_DODGE:
	case SC_AUTOBERSERK:
	case SC_RUN:
	case SC_MARIONETTE:
	case SC_MARIONETTE2:
	case SC_REBIRTH:
	case SC_BERKANA:
	case SC_ROLLINGCUTTER:
	case SC_WUGDASH:
	case SC_EXEEDBREAK:
	case SC_ALL_RIDING:	/* �R��V�X�e�� */
	case SC_ON_PUSH_CART:	/* �J�[�g */
	case SC_HAT_EFFECT:	/* �������G�t�F�N�g */
	case SC_ACTIVE_MONSTER_TRANSFORM:	/* �A�N�e�B�u�����X�^�[�ϐg */
	case SC_INVINCIBLE:	/* �C���r���V�u�� */
		timer = add_timer(1000 * 600 + tick, status_change_timer, bl->id, data);
		break;
	case SC_MODECHANGE:
		clif_emotion(bl,1);
		timer = add_timer(1500 + tick, status_change_timer, bl->id, data);
		break;
	case SC_LONGINGFREEDOM:
		if(sd && sd->status.sp >= 3) {
			if(--sc->data[type].val3 <= 0)
			{
				sd->status.sp -= 3;
				clif_updatestatus(sd, SP_SP);
				sc->data[type].val3 = 3;
			}
			timer = add_timer(1000 + tick, status_change_timer, bl->id, data);
		}
		break;
	case SC_DANCING:
		if(sd) {	// �_���X�X�L���̎���SP����
			int cost = 1;
			if(sc->data[type].val1 == CG_HERMODE)
				cost = 5;
			if(sd->status.sp < cost) {
				skill_stop_dancing(&sd->bl,0);
			} else if(--sc->data[type].val3 > 0) {
				int s = 0;

				switch(sc->data[type].val1) {
				case BD_RICHMANKIM:			/* �j�����h�̉� 3�b��SP1 */
				case BD_DRUMBATTLEFIELD:		/* �푾�ۂ̋��� 3�b��SP1 */
				case BD_RINGNIBELUNGEN:			/* �j�[�x�����O�̎w�� 3�b��SP1 */
				case BD_SIEGFRIED:			/* �s���g�̃W�[�N�t���[�h 3�b��SP1 */
				case BA_DISSONANCE:			/* �s���a�� 3�b��SP1 */
				case BA_ASSASSINCROSS:			/* �[�z�̃A�T�V���N���X 3�b��SP1 */
				case DC_UGLYDANCE:			/* ��������ȃ_���X 3�b��SP1 */
				case CG_LONGINGFREEDOM:
					s = 3;
					break;
				case BD_LULLABY:			/* �q��� 4�b��SP1 */
				case BD_ETERNALCHAOS:			/* �i���̍��� 4�b��SP1 */
				case BD_ROKISWEIL:			/* ���L�̋��� 4�b��SP1 */
				case DC_FORTUNEKISS:			/* �K�^�̃L�X 4�b��SP1 */
					s = 4;
					break;
				case BD_INTOABYSS:			/* �[���̒��� 5�b��SP1 */
				case BA_WHISTLE:			/* ���J 5�b��SP1 */
				case DC_HUMMING:			/* �n�~���O 5�b��SP1 */
				case BA_POEMBRAGI:			/* �u���M�̎� 5�b��SP1 */
				case DC_SERVICEFORYOU:			/* �T�[�r�X�t�H�[���[ 5�b��SP1 */
				case CG_HERMODE:			/* �w�����[�h�̏� */
					s = 5;
					break;
				case BA_APPLEIDUN:			/* �C�h�D���̗ь� 6�b��SP1 */
					s = 6;
					break;
				case DC_DONTFORGETME:			/* ����Y��Ȃ��Łc 10�b��SP1 */
				case CG_MOONLIT:			/* ������̉��� 10�b��SP1�H */
					s = 10;
					break;
				}
				if(s && (sc->data[type].val3 % s) == 0) {
					sd->status.sp -= cost;
					clif_updatestatus(sd,SP_SP);
				}
				timer = add_timer(	/* �^�C�}�[�Đݒ� */
					1000+tick, status_change_timer,
					bl->id, data);
			}
		} else if(md)
			timer = add_timer(1000+tick, status_change_timer, bl->id, data);
		break;
	case SC_BERSERK:		/* �o�[�T�[�N */
		{
			int dmg = (int)((atn_bignumber)status_get_max_hp(bl) * 5 / 100);
			if(status_get_hp(bl) - dmg > 100) {
				if(sd) {
					sd->status.hp -= dmg;
					clif_updatestatus(sd,SP_HP);
				} else if(hd) {
					hd->status.hp -= dmg;
					clif_send_homstatus(hd->msd,0);
				} else if(mcd) {
					mcd->status.hp -= dmg;
					clif_mercupdatestatus(mcd->msd,SP_HP);
				}
				timer = add_timer(	/* �^�C�}�[�Đݒ� */
					10000+tick, status_change_timer,
					bl->id, data);
			} else {
				if(sd) {
					sd->status.hp = 100;
					clif_updatestatus(sd,SP_HP);
				} else if(hd) {
					hd->status.hp = 100;
					clif_send_homstatus(hd->msd,0);
				} else if(mcd) {
					mcd->status.hp = 100;
					clif_mercupdatestatus(mcd->msd,SP_HP);
				}
			}
		}
		break;
	case SC_NOCHAT:			/* �`���b�g�֎~��� */
		if(sd) {
			if(++sd->status.manner && time(NULL) < (sc->data[type].val2 - 60*sd->status.manner)) {	// �J�n����status.manner���o���ĂȂ��̂Ōp��
				clif_updatestatus(sd,SP_MANNER);
				timer = add_timer(	/* �^�C�}�[�Đݒ�(60�b) */
					60000+tick, status_change_timer,
					bl->id, data);
			}
		}
		break;
	case SC_SELFDESTRUCTION:		/* ���� */
		if(md && unit_iscasting(&md->bl) && md->state.special_mob_ai > 2 && md->mode&MD_CANMOVE && md->speed > 0) {
			md->speed -= 5;
			if(md->speed <= 0)
				md->speed = 1;
			md->dir = sc->data[type].val4;
			unit_walktodir(&md->bl,1);	// ���x���ς��̂Ŗ���Ăяo��

			/* �^�C�}�[�Đݒ� */
			timer = add_timer(100+tick, status_change_timer, bl->id, data);
		}
		break;
	case SC_BOSSMAPINFO:			/* �ʖʋ� */
		if(sd && --sc->data[type].val2 > 0) {
			struct mob_data *mmd = map[sd->bl.m].mvpboss;
			if(mmd == NULL)
				break;
			if(mmd->bl.prev == NULL) {
				if(sc->data[type].val3 >= 0) {
					// �|���ꂽ�̂Ŏ���̏o�����Ԃ��A�i�E���X���ďI��
					int diff = DIFF_TICK(mmd->last_spawntime, gettick());
					if(diff < 0)
						diff = 0;
					clif_bossmapinfo(sd, mmd->name, 0, 0, diff, 3);
					break;
				}
			} else {
				if(sc->data[type].val3 < 0) {
					// �o�������̂ŃA�i�E���X
					clif_bossmapinfo(sd, mmd->name, 0, 0, 0, 2);
				}
				if(sc->data[type].val3 != mmd->bl.m || sc->data[type].val4 != mmd->bl.x + (mmd->bl.y << 16)) {
					// �o�����ō��W���ω����Ă���̂Ń~�jMAP�̃h�b�g���X�V
					if(mmd->bl.m == sd->bl.m) {
						clif_bossmapinfo(sd, "", mmd->bl.x, mmd->bl.y, 0, 1);
					} else {
						clif_bossmapinfo(sd, "", -1, -1, 0, 1);
					}
					sc->data[type].val3 = mmd->bl.m;
					sc->data[type].val4 = mmd->bl.x + (mmd->bl.y << 16);
				}
			}
			/* �^�C�}�[�Đݒ� */
			timer = add_timer(1000+tick, status_change_timer, bl->id, data);
		}
		break;
	case SC_CHANGE:		/* �����^���`�F���W */
		unit_heal(bl, -status_get_hp(bl)+10, -status_get_sp(bl)+10);	// ���Ԑ؂�̂Ƃ��̂�HP,SP��10�ɂȂ�
		break;
	case SC_URUZ:		/* �A�o���_���X */
		if((--sc->data[type].val3) > 0) {
			int sp = 60;

			if(sd) {
				if(sd->status.sp < sd->status.max_sp) {
					if(sd->status.sp + sp > sd->status.max_sp) {
						sp = sd->status.max_sp - sd->status.sp;
					}
					sd->status.sp += sp;
					clif_heal(sd->fd,SP_SP,sp);
				}
			} else {
				unit_heal(bl, 0, sp);
			}
			timer = add_timer(10000+tick, status_change_timer, bl->id, data);
		}
		break;
	case SC_FEAR:				/* ���| */
		if(sc->data[type].val3 > 0) {
			timer = add_timer(sc->data[type].val3+tick, status_change_timer, bl->id, data);
			sc->data[type].val3 = 0;
		}
		break;
	case SC_WEAPONBLOCKING:		/* �E�F�|���u���b�L���O */
		if((--sc->data[type].val3) > 0) {
			if(sd) {
				if(sd->status.sp < 3)
					break;
				sd->status.sp -= 3;
				clif_updatestatus(sd,SP_SP);
			}
			timer = add_timer(5000+tick, status_change_timer, bl->id, data);
		}
		break;
	case SC_CLOAKINGEXCEED:	/* �N���[�L���O�G�N�V�[�h */
		if(sd) {
			int sp = 10 - sc->data[type].val1;
			if(sp > 0 && sd->status.sp >= sp) {
				sd->status.sp -= sp;
				clif_updatestatus(sd,SP_SP);
				timer = add_timer(1000+tick, status_change_timer,bl->id, data);
			}
		}
		break;
	case SC_TOXIN:		/* �g�L�V�� */
		if((--sc->data[type].val2) > 0) {
			int damage = (int)((atn_bignumber)status_get_max_sp(bl) * 3 / 100);
			if(damage)
				unit_heal(bl, 0, -damage);
			timer = add_timer(10000+tick, status_change_timer, bl->id, data);
		}
		break;
	case SC_MAGICMUSHROOM:	/* �}�W�b�N�}�b�V�����[�� */
		if((--sc->data[type].val2) > 0) {
			clif_emotion(bl,18);
			if(sc->data[type].val2 % 2 == 0) {
				int damage = (int)((atn_bignumber)status_get_max_hp(bl) * 3 / 100);
				if(sd) {
					sd->skill_item.id = skill_searchrandomid(1);
					sd->skill_item.lv = 1;
					sd->skill_item.flag = 1;
					clif_item_skill(sd, sd->skill_item.id, sd->skill_item.lv, "");
				}
				if(damage)
					unit_heal(bl, -damage, 0);
			}
			if(!unit_isdead(bl) && sc->data[type].timer != -1) {
				// �����Ă��ĉ����ς݂łȂ��Ȃ�p��
				timer = add_timer(2000+tick, status_change_timer, bl->id, data);
			}
		}
		break;
	case SC_PYREXIA:	/* �p�C���b�N�V�A */
		if((--sc->data[type].val2) > 0) {
			unit_heal(bl, -100, 0);
			if(!unit_isdead(bl) && sc->data[type].timer != -1) {
				// �����Ă��ĉ����ς݂łȂ��Ȃ�p��
				timer = add_timer(3000+tick, status_change_timer, bl->id, data);
			}
		}
		break;
	case SC_OBLIVIONCURSE:	/* �I�u���r�I���J�[�X */
		if((--sc->data[type].val2) > 0) {
			clif_emotion(bl,1);
			timer = add_timer(2000+tick, status_change_timer, bl->id, data);
		}
		break;
	case SC_LEECHEND:		/* ���[�`�G���h */
		if((--sc->data[type].val2) > 0) {
			int damage = status_get_max_hp(bl) / 100;
			if(damage)
				unit_heal(bl, -damage, 0);
			if(!unit_isdead(bl) && sc->data[type].timer != -1) {
				// �����Ă��ĉ����ς݂łȂ��Ȃ�p��
				timer = add_timer(1000+tick, status_change_timer, bl->id, data);
			}
		}
		break;
	case SC_RENOVATIO:		/* ���m���@�e�B�I */
		if((--sc->data[type].val3) > 0) {
			int heal = (int)((atn_bignumber)status_get_max_hp(bl) * 3 / 100);
			if(heal) {
				if(sc->data[SC_AKAITSUKI].timer != -1)
					unit_fixdamage(bl,bl,gettick(),0,status_get_dmotion(bl),heal,0,0,0,0);
				else
					unit_heal(bl, heal, 0);
			}
			timer = add_timer(5000+tick, status_change_timer, bl->id, data);
		}
		break;
	case SC_HELLINFERNO:	/* �w���C���t�F���m */
		if((--sc->data[type].val2) > 0) {
			if(++sc->data[type].val3 % 3 == 0) {
				int damage = (int)((atn_bignumber)status_get_max_hp(bl) * 3 / 100);
				sc->data[type].val3 = 0;
				clif_damage(bl,bl,tick,0,0,1000 + damage,0,9,0,0);
				battle_damage(bl,bl,1000 + damage,0,0,0);
			}
			if(!unit_isdead(bl) && sc->data[type].timer != -1) {
				// �����Ă��ĉ����ς݂łȂ��Ȃ�p��
				timer = add_timer(1000+tick, status_change_timer, bl->id, data);
			}
		}
		break;
	case SC_SPELLBOOK:	/* �X�y���u�b�N */
		if(sd) {
			int sp = sd->freeze_sp_slot;
			if(sp > 0 && sd->status.sp >= sp) {
				sd->status.sp -= sp;
				clif_updatestatus(sd,SP_SP);
				timer = add_timer(10000 + tick, status_change_timer, bl->id, data);
			}
		}
		break;
	case SC_ELECTRICSHOCKER:	/* �G���N�g���b�N�V���b�J�[ */
		if((--sc->data[type].val2) > 0) {
			int damage = (int)((atn_bignumber)status_get_max_sp(bl) * sc->data[type].val1 / 100);
			if(damage)
				unit_heal(bl, 0, -damage);
			timer = add_timer(1000+tick, status_change_timer, bl->id, data);
		}
		break;
	case SC_CAMOUFLAGE:	/* �J���t���[�W�� */
		if((--sc->data[type].val2) > 0) {
			if((++sc->data[type].val3) > 10)
				sc->data[type].val3 = 10;
			if(sd) {
				int sp = 7 - sc->data[type].val1;
				if(sp > 0 && sd->status.sp >= sp) {
					sd->status.sp -= sp;
					clif_updatestatus(sd,SP_SP);
					timer = add_timer(1000+tick, status_change_timer,bl->id, data);
				}
			} else {
				timer = add_timer(1000+tick, status_change_timer,bl->id, data);
			}
		}
		break;
	case SC_OVERHEAT:			/* ���M */
		if(sd && pc_isgear(sd))	// �����M�A�񓋏�Ȃ�Έꉞ�~�߂�
			break;
		if((--sc->data[type].val2) > 0) {
			int damage = status_get_max_hp(bl) / 100;
			if(damage)
				unit_heal(bl, -damage, 0);
			timer = add_timer(1000+tick, status_change_timer, bl->id, data);
		}
		break;
	case SC_MAGNETICFIELD:		/* �}�O�l�e�B�b�N�t�B�[���h */
		if((--sc->data[type].val2) > 0) {
			int damage = 40 + sc->data[type].val1 * 10;
			if(damage)
				unit_heal(bl, 0, -damage);
			timer = add_timer(3000+tick, status_change_timer, bl->id, data);
		}
		break;
	case SC_STEALTHFIELD_USER:	/* �X�e���X�t�B�[���h(�g�p��) */
		if((--sc->data[type].val2) > 0) {
			int damage = (int)((atn_bignumber)status_get_max_sp(bl) * 3 / 100);
			if(damage)
				unit_heal(bl, 0, -damage);
			timer = add_timer(sc->data[type].val3+tick, status_change_timer, bl->id, data);
		}
		break;
	case SC__REPRODUCE:		/* ���v���f���[�X */
		if(sd) {
			int sp = 9 - (sc->data[type].val1 + 1) / 2;
			if(sp > 0 && sd->status.sp >= sp) {
				sd->status.sp -= sp;
				clif_updatestatus(sd,SP_SP);
				timer = add_timer(1000+tick, status_change_timer, bl->id, data);
			}
		} else {
			timer = add_timer(1000+tick, status_change_timer, bl->id, data);
		}
		break;
	case SC__SHADOWFORM:		/* �V���h�E�t�H�[�� */
		if((--sc->data[type].val4) > 0) {
			if(sd) {
				int sp = 11 - sc->data[type].val1;
				if(sp > 0 && sd->status.sp >= sp) {
					sd->status.sp -= sp;
					clif_updatestatus(sd,SP_SP);
					timer = add_timer(1000+tick, status_change_timer, bl->id, data);
				}
			} else {
				timer = add_timer(1000+tick, status_change_timer, bl->id, data);
			}
		}
		break;
	case SC__INVISIBILITY:		/* �C���r�W�r���e�B */
		if(sd) {
			int sp = sd->status.max_sp * 5 / 100;
			if(sp > 0 && sd->status.sp >= sp) {
				sd->status.sp -= sp;
				clif_updatestatus(sd,SP_SP);
				timer = add_timer(1000+tick, status_change_timer, bl->id, data);
			}
		}
		break;
	case SC_REFLECTDAMAGE:		/* ���t���N�g�_���[�W */
		if((--sc->data[type].val2) > 0) {
			if(sd) {
				int sp = 10 + sc->data[type].val1 * 2;
				if(sd->status.sp >= sp) {
					sd->status.sp -= sp;
					clif_updatestatus(sd,SP_SP);
					timer = add_timer(10000+tick, status_change_timer,bl->id, data);
				}
			} else {
				timer = add_timer(10000+tick, status_change_timer,bl->id, data);
			}
		}
		break;
	case SC_FORCEOFVANGUARD:	/* �t�H�[�X�I�u�o���K�[�h */
		if(sd) {
			int sp = 24 - sc->data[type].val1 * 4;
			if(sd->status.sp >= sp) {
				sd->status.sp -= sp;
				clif_updatestatus(sd,SP_SP);
				timer = add_timer(10000+tick, status_change_timer,bl->id, data);	
			}
		}
		break;
	case SC_BANDING:		/* �o���f�B���O */
		if(sd) {
			int sp = 7 - sc->data[type].val1;
			if(sd->status.sp >= sp) {
				sd->status.sp -= sp;
				clif_updatestatus(sd,SP_SP);
				timer = add_timer(5000+tick, status_change_timer,bl->id, data);	
			}
		}
		break;
	case SC_INSPIRATION:	/* �C���X�s���[�V���� */
		if((--sc->data[type].val2) > 0) {
			if(sd) {
				int hp = sd->status.max_hp / 100;
				int sp = sd->status.max_sp / 100;
				unit_heal(bl, -hp, -sp);
				if(sd->status.sp > 0 && !unit_isdead(bl) && sc->data[type].timer != -1) {
					// �����Ă��ĉ����ς݂łȂ��Ȃ�p��
					timer = add_timer(6000+tick, status_change_timer,bl->id, data);
				}
			} else {
				timer = add_timer(6000+tick, status_change_timer,bl->id, data);
			}
		}
		break;
	case SC_RAISINGDRAGON:		/* �������V */
		if((--sc->data[type].val2) > 0) {
			if(sd) {
				int hp = sd->status.max_hp / 100;
				if(hp > 0 && sd->status.hp >= hp) {
					sd->status.hp -= hp;
					clif_updatestatus(sd,SP_HP);
					if(sd->status.max_hp * 15 / 100 < sd->status.hp) {
						timer = add_timer(5000+tick, status_change_timer, bl->id, data);
					}
				}
			} else {
				timer = add_timer(5000+tick, status_change_timer, bl->id, data);
			}
		}
		break;
	case SC_WINKCHARM:	/* ���f�̃E�B���N */
	case SC_SIREN:		/* �Z�C���[���̐� */
		if((--sc->data[type].val3) > 0) {
			clif_emotion(bl,3);
			timer = add_timer(3000+tick, status_change_timer, bl->id, data);
		}
		break;
	case SC_DEEP_SLEEP:		/* ���炬�̎q��S */
		if((--sc->data[type].val2) > 0) {
			int hp, sp = 0;
			hp = (int)((atn_bignumber)status_get_max_hp(bl) * 3 / 100);
			if(sd)
				sp = (int)((atn_bignumber)status_get_max_sp(bl) * 3 / 100);
			if(hp || sp)
				unit_heal(bl, hp, sp);
			timer = add_timer(2000+tick, status_change_timer, bl->id, data);
		}
		break;
	case SC_SIRCLEOFNATURE:		/* �z���鎩�R�̉� */
		if((--sc->data[type].val2) > 0) {
			int hp = 40 * sc->data[type].val1;
			if(sd) {
				int sp = 4 * sc->data[type].val1;
				if(sd && sd->status.sp >= sp) {
					unit_heal(bl, hp, -sp);
					timer = add_timer(1000+tick, status_change_timer,bl->id, data);	
				}
			} else {
				unit_heal(bl, hp, 0);
				timer = add_timer(1000+tick, status_change_timer,bl->id, data);	
			}
		}
		break;
	case SC_SONG_OF_MANA:		/* �}�i�̉� */
		if((--sc->data[type].val2) > 0) {
			if(sd) {
				int sp = 10 + 2 * sc->data[type].val4;
				if(sd->status.sp + sp > sd->status.max_sp) {
					sp = sd->status.max_sp - sd->status.sp;
					sd->status.sp = sd->status.max_sp;
				} else {
					sd->status.sp += sp;
				}
				clif_updatestatus(sd,SP_SP);
			}
			timer = add_timer(5000+tick, status_change_timer,bl->id, data);	
		}
		break;
	case SC_SATURDAY_NIGHT_FEVER:		/* �t���C�f�[�i�C�g�t�B�[�o�[ */
		if(--sc->data[type].val2 > 0) {
			int hp = 0,sp = 0;
			hp = (int)((atn_bignumber)status_get_max_hp(bl) * (1 + sc->data[type].val1) / 100);
			if(sd)
				sp = (int)((atn_bignumber)status_get_max_sp(bl) * (1 + sc->data[type].val1) / 100);
			unit_heal(bl, -hp, -sp);
			if(!unit_isdead(bl) && sc->data[type].timer != -1)
				timer = add_timer(3000+tick, status_change_timer, bl->id, data);
		}
		break;
	case SC_DIAMONDDUST:		/* �_�C�������h�_�X�g */
		if(--sc->data[type].val2 > 0) {
			int hp = 0,sp = 0;
			hp = (int)((atn_bignumber)status_get_max_hp(bl) * 2 / 100);
			if(sd)
				sp = status_get_max_sp(bl) / 100;
			if(hp >= status_get_hp(bl))
				hp = status_get_hp(bl) - 1;
			unit_heal(bl, -hp, -sp);
			if(!unit_isdead(bl) && sc->data[type].timer != -1)
				timer = add_timer(1000+tick, status_change_timer, bl->id, data);
		}
		break;
	case SC_STRIKING:		/* �X�g���C�L���O */
		if((--sc->data[type].val2) > 0) {
			if(sd) {
				int sp = 6 - sc->data[type].val1;
				if(sp > 0 && sd->status.sp >= sp) {
					sd->status.sp -= sp;
					clif_updatestatus(sd,SP_SP);
					timer = add_timer(1000+tick, status_change_timer, bl->id, data);
				}
			} else {
				timer = add_timer(1000+tick, status_change_timer, bl->id, data);
			}
		}
		break;
	case SC_BLOOD_SUCKER:		/* �u���b�h�T�b�J�[ */
		if((--sc->data[type].val4) > 0) {
			struct block_list *src = map_id2bl(sc->data[type].val3);
			if(src && tid != -1) {
				skill_castend_damage_id(src,bl,sc->data[type].val2,sc->data[type].val1,gettick(),0);
			}
			if(!unit_isdead(bl) && sc->data[type].timer != -1) {
				// �����Ă��ĉ����ς݂łȂ��Ȃ�p��
				timer = add_timer(1000+tick, status_change_timer,bl->id, data);
			}
		}
		break;
	case SC_FIRE_EXPANSION_TEAR_GAS:		/* �t�@�C�A�[�G�N�X�p���V����(�×�) */
		if((--sc->data[type].val3) > 0) {
			int hp = (int)((atn_bignumber)status_get_max_hp(bl) / 100);
			clif_emotion(bl,28);
			// �t�@�C�A�[�G�N�X�p���V����(�×�)�ł͎��ȂȂ����Ƃɂ���i���j
			if(hp >= status_get_hp(bl))
				hp = status_get_hp(bl) - 1;
			unit_heal(bl, -hp, 0);
			if(!unit_isdead(bl) && sc->data[type].timer != -1)
				timer = add_timer(3000+tick, status_change_timer, bl->id, data);
		}
		break;
	case SC_MEIKYOUSISUI:		/* �����~�� */
		if((--sc->data[type].val2) > 0) {
			if(sd) {
				int hp = (int)((atn_bignumber)status_get_max_hp(&sd->bl) * (sc->data[type].val1 * 2) / 100);
				int sp = (int)((atn_bignumber)status_get_max_sp(&sd->bl) * sc->data[type].val1 / 100);
				if(sd->status.max_hp >= sd->status.hp + hp) {
					sd->status.hp += hp;
					clif_updatestatus(sd,SP_HP);
				} else {
					sd->status.hp = sd->status.max_hp;
					clif_updatestatus(sd,SP_HP);
				}
				if(sd->status.max_sp >= sd->status.sp + sp) {
					sd->status.sp += sp;
					clif_updatestatus(sd,SP_SP);
				} else {
					sd->status.sp = sd->status.max_sp;
					clif_updatestatus(sd,SP_SP);
				}
				timer = add_timer(1000+tick, status_change_timer, bl->id, data);
			} else {
				timer = add_timer(1000+tick, status_change_timer, bl->id, data);
			}
		}
		break;
	case SC_IZAYOI:		/* �\�Z�� */
		if((--sc->data[type].val3) > 0) {
			if(sd) {
				int sp = 1 * sc->data[type].val1;
				if(sd->status.sp >= sp) {
					sd->status.sp -= sp;
					clif_updatestatus(sd,SP_SP);
					timer = add_timer(1000+tick, status_change_timer, bl->id, data);
				}
			} else {
				timer = add_timer(1000+tick, status_change_timer, bl->id, data);
			}
		}
		break;
	case SC_KAGEMUSYA:	/* �e���� */
		if(sd) {
			if(sd->status.sp >= 1) {
				sd->status.sp -= 1;
				clif_updatestatus(sd,SP_SP);
				timer = add_timer(1000+tick, status_change_timer,bl->id, data);
			}
		}
		break;
	case SC_SUMMON_ELEM:	/* �T�����G�������^�� */
		if((--sc->data[type].val2) > 0) {
			if(sd) {
				// �������Ă��鐸�삪���Ȃ���ΏI��
				if(!sd->eld)
					break;
				if(sd->status.sp >= sc->data[type].val3) {
					sd->status.sp -= sc->data[type].val3;
					clif_updatestatus(sd,SP_SP);
					timer = add_timer(10000+tick, status_change_timer, bl->id, data);
				}
			} else {
				timer = add_timer(10000+tick, status_change_timer, bl->id, data);
			}
		}
		break;
	case SC_WATER_SCREEN:	/* �E�H�[�^�[�X�N���[�� */
		if((--sc->data[type].val2) > 0) {
			int heal = (int)((atn_bignumber)status_get_max_hp(bl) * 3 / 100);
			if(heal)
				unit_heal(bl, heal, 0);
			timer = add_timer(5000+tick, status_change_timer, bl->id, data);
		}
		break;
	case SC_CIRCLE_OF_FIRE_OPTION:	/* �T�[�N���I�u�t�@�C�A(����) */
	case SC_FIRE_CLOAK_OPTION:		/* �t�@�C�A�[�N���[�N(����) */
	case SC_WATER_SCREEN_OPTION:	/* �E�H�[�^�[�X�N���[��(����) */
	case SC_WATER_DROP_OPTION:		/* �E�H�[�^�[�h���b�v(����) */
	case SC_WIND_STEP_OPTION:		/* �E�B���h�X�e�b�v(����) */
	case SC_WIND_CURTAIN_OPTION:	/* �E�B���h�J�[�e��(����) */
	case SC_SOLID_SKIN_OPTION:		/* �\���b�h�X�L��(����) */
	case SC_STONE_SHIELD_OPTION:	/* �X�g�[���V�[���h(����) */
		if((--sc->data[type].val2) > 0) {
			if(eld) {
				if(eld->status.sp >= sc->data[type].val3) {
					eld->status.sp -= sc->data[type].val3;
					clif_elemupdatestatus(eld->msd,SP_SP);
					timer = add_timer(1000+tick, status_change_timer,bl->id, data);
				}
			} else {
				timer = add_timer(1000+tick, status_change_timer, bl->id, data);
			}
		}
		break;
	case SC_PYROTECHNIC_OPTION:	/* �p�C���e�N�j�b�N(����) */
	case SC_AQUAPLAY_OPTION:	/* �A�N�A�v���C(����) */
	case SC_GUST_OPTION:		/* �K�X�g(����) */
	case SC_PETROLOGY_OPTION:	/* �y�g���W�[(����) */
	case SC_HEATER_OPTION:		/* �q�[�^�[(����) */
	case SC_COOLER_OPTION:		/* �N�[���[(����) */
	case SC_BLAST_OPTION:		/* �u���X�g(����) */
	case SC_CURSED_SOIL_OPTION:	/* �J�[�X�h�\�C��(����) */
	case SC_TROPIC_OPTION:		/* �g���s�b�N(����) */
	case SC_CHILLY_AIR_OPTION:	/* �N�[���G�A�[(����) */
	case SC_WILD_STORM_OPTION:	/* ���C���h�X�g�[��(����) */
	case SC_UPHEAVAL_OPTION:	/* �A�b�v�w�C�o��(����) */
		if((--sc->data[type].val2) > 0) {
			if(eld) {
				if(eld->status.sp >= sc->data[type].val3) {
					eld->status.sp -= sc->data[type].val3;
					clif_elemupdatestatus(eld->msd,SP_SP);
					timer = add_timer(10000+tick, status_change_timer,bl->id, data);
				}
			} else {
				timer = add_timer(10000+tick, status_change_timer, bl->id, data);
			}
		}
		break;
	}

	if(timer == -1 && sd && sd->eternal_status_change[type] > 0 && !unit_isdead(&sd->bl))
	{
		timer = add_timer(	/* �^�C�}�[�Đݒ� */
			sd->eternal_status_change[type]+tick, status_change_timer,
			bl->id, data);
	}

	map_freeblock_unlock();

	if(timer != -1) {
		// �^�C�}�[ID��ۑ����Čp������
		sc->data[type].timer = timer;
		return 0;
	}

	return status_change_end(bl, type, tid);
}

/*==========================================
 * �X�e�[�^�X�ُ�^�C�}�[�͈͏���
 *------------------------------------------
 */
int status_change_timer_sub(struct block_list *bl, va_list ap)
{
	struct block_list *src;
	struct status_change *sc;
	int type, skilllv;
	unsigned int tick;

	nullpo_retr(0, bl);
	nullpo_retr(0, ap);
	nullpo_retr(0, src = va_arg(ap,struct block_list*));

	type    = va_arg(ap,int);
	skilllv = va_arg(ap,int);
	tick    = va_arg(ap,unsigned int);

	if(!(bl->type & (BL_PC | BL_MOB | BL_MERC)))
		return 0;

	nullpo_retr(0, sc = status_get_sc(bl));

	switch( type ) {
	case SC_SIGHT:	/* �T�C�g */
	case SC_CONCENTRATE:
		if(sc->option & (OPTION_HIDE | OPTION_CLOAKING)) {
			status_change_end(bl, SC_HIDING, -1);
			status_change_end(bl, SC_CLOAKING, -1);
			status_change_end(bl, SC_CLOAKINGEXCEED, -1);
			status_change_end(bl, SC__INVISIBILITY, -1);
		}
		if(sc->option & OPTION_SPECIALHIDING) {
			status_change_end(bl, SC_INVISIBLE, -1);
		}
		break;
	case SC_RUWACH:	/* ���A�t */
		if(sc->option & (OPTION_HIDE | OPTION_CLOAKING | OPTION_SPECIALHIDING)) {
			if(sc->option & (OPTION_HIDE | OPTION_CLOAKING)) {
				status_change_end(bl, SC_HIDING, -1);
				status_change_end(bl, SC_CLOAKING, -1);
				status_change_end(bl, SC_CLOAKINGEXCEED, -1);
				status_change_end(bl, SC__INVISIBILITY, -1);
			}
			if(sc->option & OPTION_SPECIALHIDING) {
				status_change_end(bl, SC_INVISIBLE, -1);
			}
			if(battle_check_target(src, bl, BCT_ENEMY) > 0)
				battle_skill_attack(BF_MAGIC,src,src,bl,AL_RUWACH,skilllv,tick,0);
		}
		break;
	case SC_SIGHTBLASTER:
		if(battle_check_target(src, bl, BCT_ENEMY) > 0 && !unit_isdead(bl)) {
			struct status_change *ssc;
			battle_skill_attack(BF_MAGIC,src,src,bl,WZ_SIGHTBLASTER,skilllv,tick,0);
			ssc = status_get_sc(src);
			if(ssc) {
				ssc->data[type].val2 = 0;
			}
			status_change_end(src,SC_SIGHTBLASTER,-1);
		}
		break;
	}
	return 0;
}

/*==========================================
 * �X�e�[�^�X�ُ�S����
 *------------------------------------------
 */
int status_change_clear(struct block_list *bl,int type)
{
	struct status_change *sc;
	int i;

	nullpo_retr(0, bl);
	nullpo_retr(0, sc = status_get_sc(bl));

#ifdef DYNAMIC_SC_DATA
	if(sc->data == dummy_sc_data)
		return 0;
#endif

	if(sc->count <= 0)
		return 0;

	status_calc_pc_stop_begin(bl);
	for(i = 0; i < MAX_STATUSCHANGE; i++) {
		if(i == SC_BABY && type == 0 && unit_isdead(bl))
			continue;

		/* �ُ킪����Ȃ�^�C�}�[���폜���� */
		if(i == SC_DANCING) {
			skill_stop_dancing(bl,0);
		} else {
			if(i == SC_BASILICA)
				skill_basilica_cancel(bl);
			status_change_end(bl,i,-1);
		}
	}
	status_calc_pc_stop_end(bl);

	sc->opt1 = OPT1_NORMAL;
	sc->opt2 = OPT2_NORMAL;
	sc->opt3 = OPT3_NORMAL;
	sc->option &= OPTION_MASK;

	if(type != 1) {
		clif_changeoption(bl);
	}

	return 0;
}

/*==========================================
 * ����������ɂ�����X�e�[�^�X�ُ����
 *------------------------------------------
 */
int status_change_release(struct block_list *bl,int mask)
{
	struct status_change *sc;
	int i;

	nullpo_retr(0, bl);
	nullpo_retr(0, sc = status_get_sc(bl));

#ifdef DYNAMIC_SC_DATA
	if(sc->data == dummy_sc_data)
		return 0;
#endif

	if(sc->count <= 0)
		return 0;

	status_calc_pc_stop_begin(bl);
	for(i = 0; i < MAX_STATUSCHANGE; i++) {
		// �ُ킪�����Ċ��t���O������Ȃ�^�C�}�[���폜
		if( (scdata_db[i].releasable & mask) && sc->data[i].timer != -1 ) {
			if(i == SC_DANCING) {
				skill_stop_dancing(bl,0);
			} else {
				if(i == SC_BASILICA)
					skill_basilica_cancel(bl);
				status_change_end(bl,i,-1);
			}
		}
	}
	status_calc_pc_stop_end(bl);

	return 0;
}

/*==========================================
 * �X�e�[�^�X�ُ�(����̑���)�I��
 *------------------------------------------
 */
int status_enchant_elemental_end(struct block_list *bl,int type)
{
	struct status_change *sc;

	nullpo_retr(0, bl);
	nullpo_retr(0, sc = status_get_sc(bl));

	if( type != SC_ENCPOISON && sc->data[SC_ENCPOISON].timer != -1 )	/* �G���`�����g�|�C�Y������ */
		status_change_end(bl,SC_ENCPOISON,-1);
	if( type != SC_ASPERSIO && sc->data[SC_ASPERSIO].timer != -1 )	/* �A�X�y���V�I���� */
		status_change_end(bl,SC_ASPERSIO,-1);
	if( type != SC_FLAMELAUNCHER && sc->data[SC_FLAMELAUNCHER].timer != -1 )	/* �t���C�������`������ */
		status_change_end(bl,SC_FLAMELAUNCHER,-1);
	if( type != SC_FROSTWEAPON && sc->data[SC_FROSTWEAPON].timer != -1 )	/* �t���X�g�E�F�|������ */
		status_change_end(bl,SC_FROSTWEAPON,-1);
	if( type != SC_LIGHTNINGLOADER && sc->data[SC_LIGHTNINGLOADER].timer != -1 )	/* ���C�g�j���O���[�_�[���� */
		status_change_end(bl,SC_LIGHTNINGLOADER,-1);
	if( type != SC_SEISMICWEAPON && sc->data[SC_SEISMICWEAPON].timer != -1 )	/* �T�C�X�~�b�N�E�F�|������ */
		status_change_end(bl,SC_SEISMICWEAPON,-1);
	if( type != SC_DARKELEMENT && sc->data[SC_DARKELEMENT].timer != -1 )		// ��
		status_change_end(bl,SC_DARKELEMENT,-1);
	if( type != SC_ATTENELEMENT && sc->data[SC_ATTENELEMENT].timer != -1 )	// �O
		status_change_end(bl,SC_ATTENELEMENT,-1);
	if( type != SC_UNDEADELEMENT && sc->data[SC_UNDEADELEMENT].timer != -1 )	// �s��
		status_change_end(bl,SC_UNDEADELEMENT,-1);
	if( type != SC_SEVENWIND && sc->data[SC_SEVENWIND].timer != -1 )		/* �g���������� */
		status_change_end(bl,SC_SEVENWIND,-1);

	return 0;
}

/*==========================================
 * �X�e�[�^�X�ُ�(�̂̑���)�I��
 *------------------------------------------
 */
int status_enchant_armor_elemental_end(struct block_list *bl,int type)
{
	struct status_change *sc;

	nullpo_retr(0, bl);
	nullpo_retr(0, sc = status_get_sc(bl));

	if( type != SC_BENEDICTIO && sc->data[SC_BENEDICTIO].timer != -1 )	// ����
		status_change_end(bl,SC_BENEDICTIO,-1);
	if( type != SC_ELEMENTWATER && sc->data[SC_ELEMENTWATER].timer != -1 )	// ��
		status_change_end(bl,SC_ELEMENTWATER,-1);
	if( type != SC_ELEMENTGROUND && sc->data[SC_ELEMENTGROUND].timer != -1 )	// �n
		status_change_end(bl,SC_ELEMENTGROUND,-1);
	if( type != SC_ELEMENTWIND && sc->data[SC_ELEMENTWIND].timer != -1 )		// ��
		status_change_end(bl,SC_ELEMENTWIND,-1);
	if( type != SC_ELEMENTFIRE && sc->data[SC_ELEMENTFIRE].timer != -1 )		// ��
		status_change_end(bl,SC_ELEMENTFIRE,-1);
	if( type != SC_ELEMENTHOLY && sc->data[SC_ELEMENTHOLY].timer != -1 )	// ��
		status_change_end(bl,SC_ELEMENTHOLY,-1);
	if( type != SC_ELEMENTDARK && sc->data[SC_ELEMENTDARK].timer != -1 )		// ��
		status_change_end(bl,SC_ELEMENTDARK,-1);
	if( type != SC_ELEMENTELEKINESIS && sc->data[SC_ELEMENTELEKINESIS].timer != -1 )	// �O
		status_change_end(bl,SC_ELEMENTELEKINESIS,-1);
	if( type != SC_ELEMENTPOISON && sc->data[SC_ELEMENTPOISON].timer != -1 )	// ��
		status_change_end(bl,SC_ELEMENTPOISON,-1);
	if( type != SC_ELEMENTUNDEAD && sc->data[SC_ELEMENTUNDEAD].timer != -1 )	// �s��
		status_change_end(bl,SC_ELEMENTUNDEAD,-1);

	return 0;
}

/*==========================================
 * �X�e�[�^�X�ُ�(�푰�ύX)�I��
 *------------------------------------------
 */
int status_change_race_end(struct block_list *bl,int type)
{
	struct status_change *sc;

	nullpo_retr(0, bl);
	nullpo_retr(0, sc = status_get_sc(bl));

	if( type != SC_RACEUNDEAD && sc->data[SC_RACEUNDEAD].timer != -1 )
		status_change_end(bl,SC_RACEUNDEAD,-1);
	if( type != SC_RACEBEAST && sc->data[SC_RACEBEAST].timer != -1 )
		status_change_end(bl,SC_RACEBEAST,-1);
	if( type != SC_RACEPLANT && sc->data[SC_RACEPLANT].timer != -1 )
		status_change_end(bl,SC_RACEPLANT,-1);
	if( type != SC_RACEINSECT && sc->data[SC_RACEINSECT].timer != -1 )
		status_change_end(bl,SC_RACEINSECT,-1);
	if( type != SC_RACEFISH && sc->data[SC_RACEFISH].timer != -1 )
		status_change_end(bl,SC_RACEFISH,-1);
	if( type != SC_RACEDEVIL && sc->data[SC_RACEDEVIL].timer != -1 )
		status_change_end(bl,SC_RACEDEVIL,-1);
	if( type != SC_RACEHUMAN && sc->data[SC_RACEHUMAN].timer != -1 )
		status_change_end(bl,SC_RACEHUMAN,-1);
	if( type != SC_RACEANGEL && sc->data[SC_RACEANGEL].timer != -1 )
		status_change_end(bl,SC_RACEANGEL,-1);
	if( type != SC_RACEDRAGON && sc->data[SC_RACEDRAGON].timer != -1 )
		status_change_end(bl,SC_RACEDRAGON,-1);

	return 0;
}

/*==========================================
 * �X�e�[�^�X�ُ�(�푰�ύX)�I��
 *------------------------------------------
 */
int status_change_resistclear(struct block_list *bl)
{
	struct status_change *sc;

	nullpo_retr(0, bl);
	nullpo_retr(0, sc = status_get_sc(bl));

	status_calc_pc_stop_begin(bl);

	if(sc->data[SC_RESISTWATER].timer != -1)
		status_change_end(bl,SC_RESISTWATER,-1);
	if(sc->data[SC_RESISTGROUND].timer != -1)
		status_change_end(bl,SC_RESISTGROUND,-1);
	if(sc->data[SC_RESISTFIRE].timer != -1)
		status_change_end(bl,SC_RESISTFIRE,-1);
	if(sc->data[SC_RESISTWIND].timer != -1)
		status_change_end(bl,SC_RESISTWIND,-1);
	if(sc->data[SC_RESISTPOISON].timer != -1)
		status_change_end(bl,SC_RESISTPOISON,-1);
	if(sc->data[SC_RESISTHOLY].timer != -1)
		status_change_end(bl,SC_RESISTHOLY,-1);
	if(sc->data[SC_RESISTDARK].timer != -1)
		status_change_end(bl,SC_RESISTDARK,-1);
	if(sc->data[SC_RESISTTELEKINESIS].timer != -1)
		status_change_end(bl,SC_RESISTTELEKINESIS,-1);
	if(sc->data[SC_RESISTUNDEAD].timer != -1)
		status_change_end(bl,SC_RESISTUNDEAD,-1);

	status_calc_pc_stop_end(bl);

	return 0;
}

/*==========================================
 * �X�e�[�^�X�ُ�(��)�J�n
 *------------------------------------------
 */
int status_change_soulstart(struct block_list *bl,int val1,int val2,int val3,int val4,int tick,int flag)
{
	int type = -1;
	struct map_session_data *sd;

	nullpo_retr(0, bl);

	if(bl->type != BL_PC || (sd = (struct map_session_data *)bl) == NULL)
		return 0;

	switch(sd->s_class.job) {
		case PC_JOB_MO:
		case PC_JOB_SR:
			type = SC_MONK;
			break;
		case PC_JOB_SG:
			type = SC_STAR;
			break;
		case PC_JOB_SA:
		case PC_JOB_SO:
			type = SC_SAGE;
		 	break;
		case PC_JOB_CR:
		case PC_JOB_LG:
			type = SC_CRUSADER;
			break;
		case PC_JOB_WZ:
		case PC_JOB_WL:
			type = SC_WIZARD;
			break;
		case PC_JOB_PR:
		case PC_JOB_AB:
			type = SC_PRIEST;
			break;
		case PC_JOB_RG:
		case PC_JOB_SC:
			type = SC_ROGUE;
			break;
		case PC_JOB_AS:
		case PC_JOB_GC:
			type = SC_ASSASIN;
			break;
		case PC_JOB_SL:
			type = SC_SOULLINKER;
			break;
		case PC_JOB_KN:
		case PC_JOB_RK:
			type = SC_KNIGHT;
			break;
		case PC_JOB_AM:
		case PC_JOB_GN:
			type = SC_ALCHEMIST;
			break;
		case PC_JOB_BA:
		case PC_JOB_DC:
		case PC_JOB_MI:
		case PC_JOB_WA:
			type = SC_BARDDANCER;
			break;
		case PC_JOB_BS:
		case PC_JOB_NC:
			type = SC_BLACKSMITH;
			break;
		case PC_JOB_HT:
		case PC_JOB_RA:
			type = SC_HUNTER;
			break;
		case PC_JOB_SNV:
		case PC_JOB_ESNV:
			type = SC_SUPERNOVICE;
			break;
		case PC_JOB_GS:
			type = SC_GUNNER;
			break;
		case PC_JOB_NJ:
			type = SC_NINJA;
			break;
		case PC_JOB_DK:
			type = SC_DEATHKINGHT;
			break;
		case PC_JOB_DA:
			type = SC_COLLECTOR;
			break;
		default:
			if(sd->s_class.upper == PC_UPPER_HIGH && sd->s_class.job >= PC_JOB_SM && sd->s_class.job <= PC_JOB_TF)
				type = SC_HIGH;
			break;
	}
	if(type >= 0)
		status_change_start(bl,type,val1,val2,val3,val4,tick,flag);
	return 0;
}

/*==========================================
 * �X�e�[�^�X�ُ�(��)�I��
 *------------------------------------------
 */
int status_change_soulclear(struct block_list *bl)
{
	struct status_change *sc;

	nullpo_retr(0, bl);
	nullpo_retr(0, sc = status_get_sc(bl));

	if(sc->data[SC_MONK].timer != -1)
		status_change_end(bl,SC_MONK,-1);
	if(sc->data[SC_STAR].timer != -1)
		status_change_end(bl,SC_STAR,-1);
	if(sc->data[SC_SAGE].timer != -1)
		status_change_end(bl,SC_SAGE,-1);
	if(sc->data[SC_CRUSADER].timer != -1)
		status_change_end(bl,SC_CRUSADER,-1);
	if(sc->data[SC_WIZARD].timer != -1)
		status_change_end(bl,SC_WIZARD,-1);
	if(sc->data[SC_PRIEST].timer != -1)
		status_change_end(bl,SC_PRIEST,-1);
	if(sc->data[SC_ROGUE].timer != -1)
		status_change_end(bl,SC_ROGUE,-1);
	if(sc->data[SC_ASSASIN].timer != -1)
		status_change_end(bl,SC_ASSASIN,-1);
	if(sc->data[SC_SOULLINKER].timer != -1)
		status_change_end(bl,SC_SOULLINKER,-1);
	if(sc->data[SC_KNIGHT].timer != -1)
		status_change_end(bl,SC_KNIGHT,-1);
	if(sc->data[SC_ALCHEMIST].timer != -1)
		status_change_end(bl,SC_ALCHEMIST,-1);
	if(sc->data[SC_BARDDANCER].timer != -1)
		status_change_end(bl,SC_BARDDANCER,-1);
	if(sc->data[SC_BLACKSMITH].timer != -1)
		status_change_end(bl,SC_BLACKSMITH,-1);
	if(sc->data[SC_HUNTER].timer != -1)
		status_change_end(bl,SC_HUNTER,-1);
	if(sc->data[SC_HIGH].timer != -1)
		status_change_end(bl,SC_HIGH,-1);
	if(sc->data[SC_SUPERNOVICE].timer != -1)
		status_change_end(bl,SC_SUPERNOVICE,-1);
	if(sc->data[SC_GUNNER].timer != -1)
		status_change_end(bl,SC_GUNNER,-1);
	if(sc->data[SC_NINJA].timer != -1)
		status_change_end(bl,SC_NINJA,-1);
	if(sc->data[SC_DEATHKINGHT].timer != -1)
		status_change_end(bl,SC_DEATHKINGHT,-1);
	if(sc->data[SC_COLLECTOR].timer != -1)
		status_change_end(bl,SC_COLLECTOR,-1);

	return 0;
}

/*==========================================
 * �X�e�[�^�X�ُ�(����)�I��
 *------------------------------------------
 */
int status_change_elemclear(struct block_list *bl)
{
	struct status_change *sc;

	nullpo_retr(0, bl);
	nullpo_retr(0, sc = status_get_sc(bl));

	if(sc->data[SC_CIRCLE_OF_FIRE].timer != -1)
		status_change_end(bl,SC_CIRCLE_OF_FIRE,-1);
	if(sc->data[SC_CIRCLE_OF_FIRE_OPTION].timer != -1)
		status_change_end(bl,SC_CIRCLE_OF_FIRE_OPTION,-1);
	if(sc->data[SC_FIRE_CLOAK].timer != -1)
		status_change_end(bl,SC_FIRE_CLOAK,-1);
	if(sc->data[SC_FIRE_CLOAK_OPTION].timer != -1)
		status_change_end(bl,SC_FIRE_CLOAK_OPTION,-1);
	if(sc->data[SC_WATER_SCREEN].timer != -1)
		status_change_end(bl,SC_WATER_SCREEN,-1);
	if(sc->data[SC_WATER_SCREEN_OPTION].timer != -1)
		status_change_end(bl,SC_WATER_SCREEN_OPTION,-1);
	if(sc->data[SC_WATER_DROP].timer != -1)
		status_change_end(bl,SC_WATER_DROP,-1);
	if(sc->data[SC_WATER_DROP_OPTION].timer != -1)
		status_change_end(bl,SC_WATER_DROP_OPTION,-1);
	//if(sc->data[SC_WATER_BARRIER].timer != -1)
	//	status_change_end(bl,SC_WATER_BARRIER,-1);
	if(sc->data[SC_WIND_STEP].timer != -1)
		status_change_end(bl,SC_WIND_STEP,-1);
	if(sc->data[SC_WIND_STEP_OPTION].timer != -1)
		status_change_end(bl,SC_WIND_STEP_OPTION,-1);
	if(sc->data[SC_WIND_CURTAIN].timer != -1)
		status_change_end(bl,SC_WIND_CURTAIN,-1);
	if(sc->data[SC_WIND_CURTAIN_OPTION].timer != -1)
		status_change_end(bl,SC_WIND_CURTAIN_OPTION,-1);
	//if(sc->data[SC_ZEPHYR].timer != -1)
	//	status_change_end(bl,SC_ZEPHYR,-1);
	if(sc->data[SC_SOLID_SKIN].timer != -1)
		status_change_end(bl,SC_SOLID_SKIN,-1);
	if(sc->data[SC_SOLID_SKIN_OPTION].timer != -1)
		status_change_end(bl,SC_SOLID_SKIN_OPTION,-1);
	if(sc->data[SC_STONE_SHIELD].timer != -1)
		status_change_end(bl,SC_STONE_SHIELD,-1);
	if(sc->data[SC_STONE_SHIELD_OPTION].timer != -1)
		status_change_end(bl,SC_STONE_SHIELD_OPTION,-1);
	//if(sc->data[SC_POWER_OF_GAIA].timer != -1)
	//	status_change_end(bl,SC_POWER_OF_GAIA,-1);
	if(sc->data[SC_PYROTECHNIC].timer != -1)
		status_change_end(bl,SC_PYROTECHNIC,-1);
	if(sc->data[SC_PYROTECHNIC_OPTION].timer != -1)
		status_change_end(bl,SC_PYROTECHNIC_OPTION,-1);
	if(sc->data[SC_HEATER].timer != -1)
		status_change_end(bl,SC_HEATER,-1);
	if(sc->data[SC_HEATER_OPTION].timer != -1)
		status_change_end(bl,SC_HEATER_OPTION,-1);
	if(sc->data[SC_TROPIC].timer != -1)
		status_change_end(bl,SC_TROPIC,-1);
	if(sc->data[SC_TROPIC_OPTION].timer != -1)
		status_change_end(bl,SC_TROPIC_OPTION,-1);
	if(sc->data[SC_AQUAPLAY].timer != -1)
		status_change_end(bl,SC_AQUAPLAY,-1);
	if(sc->data[SC_AQUAPLAY_OPTION].timer != -1)
		status_change_end(bl,SC_AQUAPLAY_OPTION,-1);
	if(sc->data[SC_COOLER].timer != -1)
		status_change_end(bl,SC_COOLER,-1);
	if(sc->data[SC_COOLER_OPTION].timer != -1)
		status_change_end(bl,SC_COOLER_OPTION,-1);
	if(sc->data[SC_CHILLY_AIR].timer != -1)
		status_change_end(bl,SC_CHILLY_AIR,-1);
	if(sc->data[SC_CHILLY_AIR_OPTION].timer != -1)
		status_change_end(bl,SC_CHILLY_AIR_OPTION,-1);
	if(sc->data[SC_GUST].timer != -1)
		status_change_end(bl,SC_GUST,-1);
	if(sc->data[SC_GUST_OPTION].timer != -1)
		status_change_end(bl,SC_GUST_OPTION,-1);
	if(sc->data[SC_BLAST].timer != -1)
		status_change_end(bl,SC_BLAST,-1);
	if(sc->data[SC_BLAST_OPTION].timer != -1)
		status_change_end(bl,SC_BLAST_OPTION,-1);
	if(sc->data[SC_WILD_STORM].timer != -1)
		status_change_end(bl,SC_WILD_STORM,-1);
	if(sc->data[SC_WILD_STORM_OPTION].timer != -1)
		status_change_end(bl,SC_WILD_STORM_OPTION,-1);
	if(sc->data[SC_PETROLOGY].timer != -1)
		status_change_end(bl,SC_PETROLOGY,-1);
	if(sc->data[SC_PETROLOGY_OPTION].timer != -1)
		status_change_end(bl,SC_PETROLOGY_OPTION,-1);
	if(sc->data[SC_CURSED_SOIL].timer != -1)
		status_change_end(bl,SC_CURSED_SOIL,-1);
	if(sc->data[SC_CURSED_SOIL_OPTION].timer != -1)
		status_change_end(bl,SC_CURSED_SOIL_OPTION,-1);
	if(sc->data[SC_UPHEAVAL].timer != -1)
		status_change_end(bl,SC_UPHEAVAL,-1);
	if(sc->data[SC_UPHEAVAL_OPTION].timer != -1)
		status_change_end(bl,SC_UPHEAVAL_OPTION,-1);
	if(sc->data[SC_TIDAL_WEAPON].timer != -1)
		status_change_end(bl,SC_TIDAL_WEAPON,-1);
	if(sc->data[SC_TIDAL_WEAPON_OPTION].timer != -1)
		status_change_end(bl,SC_TIDAL_WEAPON_OPTION,-1);
	//if(sc->data[SC_ROCK_CRUSHER].timer != -1)
	//	status_change_end(bl,SC_ROCK_CRUSHER,-1);
	//if(sc->data[SC_ROCK_CRUSHER_ATK].timer != -1)
	//	status_change_end(bl,SC_ROCK_CRUSHER_ATK,-1);

	return 0;
}

/*==========================================
 * �X�e�[�^�X�ُ�(�����E�Ή��E����)�I��
 *------------------------------------------
 */
int status_change_attacked_end(struct block_list *bl)
{
	struct status_change *sc;

	nullpo_retr(0, bl);
	nullpo_retr(0, sc = status_get_sc(bl));

	if(sc->data[SC_FREEZE].timer != -1)
		status_change_end(bl,SC_FREEZE,-1);
	if(sc->data[SC_STONE].timer != -1 && sc->data[SC_STONE].val2 == 0)
		status_change_end(bl,SC_STONE,-1);
	if(sc->data[SC_SLEEP].timer != -1)
		status_change_end(bl,SC_SLEEP,-1);
	if(sc->data[SC_WINKCHARM].timer != -1)
		status_change_end(bl,SC_WINKCHARM,-1);
	if(sc->data[SC_DEEP_SLEEP].timer != -1)
		status_change_end(bl,SC_DEEP_SLEEP,-1);
	if(sc->data[SC_SIREN].timer != -1)
		status_change_end(bl,SC_SIREN,-1);

	return 0;
}

/*==========================================
 * �X�e�[�^�X�ُ�(�n�C�h)�I��
 *------------------------------------------
 */
int status_change_hidden_end(struct block_list *bl)
{
	struct status_change *sc;

	nullpo_retr(0, bl);

	sc = status_get_sc(bl);

	if(sc) {
		if(sc->option > 0) {
			if(sc->option & OPTION_HIDE)
				status_change_end(bl,SC_HIDING,-1);
			if((sc->option & (OPTION_CLOAKING | OPTION_FOOTPRINT)) == OPTION_CLOAKING)
				status_change_end(bl,SC_CLOAKING,-1);
			if((sc->option & (OPTION_CLOAKING | OPTION_FOOTPRINT)) == (OPTION_CLOAKING | OPTION_FOOTPRINT))
				status_change_end(bl,SC_CHASEWALK,-1);
		}
	 	if(sc->data[SC_CAMOUFLAGE].timer != -1)
			status_change_end(bl,SC_CAMOUFLAGE,-1);
	 	if(sc->data[SC_STEALTHFIELD].timer != -1)
			status_change_end(bl,SC_STEALTHFIELD,-1);
	}
	return 0;
}

/*==========================================
 * �X�e�[�^�X�ُ�(�}�b�v���E��)�I��
 *------------------------------------------
 */
int status_change_removemap_end(struct block_list *bl)
{
	struct status_change *sc;

	nullpo_retr(0, bl);

	sc = status_get_sc(bl);
	if(sc == NULL || sc->count <= 0)
		return 0;

	status_calc_pc_stop_begin(bl);

	if(sc->data[SC_BLADESTOP].timer != -1)
		status_change_end(bl, SC_BLADESTOP, -1);
	if(sc->data[SC_GRAFFITI].timer != -1)
		status_change_end(bl, SC_GRAFFITI, -1);
	if(sc->data[SC_ANKLE].timer != -1)
		status_change_end(bl, SC_ANKLE, -1);
	if(sc->data[SC_GOSPEL].timer != -1)
		status_change_end(bl, SC_GOSPEL, -1);
	if(sc->data[SC_GRAVITATION_USER].timer != -1)
		status_change_end(bl, SC_GRAVITATION_USER, -1);
	if(sc->data[SC_TRICKDEAD].timer != -1)
		status_change_end(bl, SC_TRICKDEAD, -1);
	if(sc->data[SC_RUN].timer != -1)
		status_change_end(bl, SC_RUN, -1);
	if(sc->data[SC_MARIONETTE].timer != -1)
		status_change_end(bl, SC_MARIONETTE, -1);
	if(sc->data[SC_MARIONETTE2].timer != -1)
		status_change_end(bl, SC_MARIONETTE2, -1);
	if(sc->data[SC_BASILICA].timer != -1) {
		skill_basilica_cancel(bl);
		status_change_end(bl, SC_BASILICA, -1);
	}
	if(sc->data[SC_WUGDASH].timer != -1)
		status_change_end(bl, SC_WUGDASH, -1);

	if(battle_config.homun_statuschange_reset) {
		if(sc->data[SC_AVOID].timer != -1)
			status_change_end(bl, SC_AVOID, -1);
		if(sc->data[SC_CHANGE].timer != -1)
			status_change_end(bl, SC_CHANGE, -1);
		if(sc->data[SC_DEFENCE].timer != -1)
			status_change_end(bl, SC_DEFENCE, -1);
		if(sc->data[SC_BLOODLUST].timer != -1)
			status_change_end(bl, SC_BLOODLUST, -1);
		if(sc->data[SC_FLEET].timer != -1)
			status_change_end(bl, SC_FLEET, -1);
		if(sc->data[SC_SPEED].timer != -1)
			status_change_end(bl, SC_SPEED, -1);
	}
	status_calc_pc_stop_end(bl);

	return 0;
}

/*==========================================
 * �A�C�e���ɂ��X�e�[�^�X�ُ�J�n
 *------------------------------------------
 */
int status_change_addeff_start(struct block_list *src, struct block_list *bl, int id, int rate, int type, unsigned int tick)
{
	struct map_session_data *sd = NULL;

	static const int sc2[] = {
		MG_STONECURSE,MG_FROSTDIVER,NPC_STUNATTACK,
		NPC_SLEEPATTACK,TF_POISON,NPC_CURSEATTACK,
		NPC_SILENCEATTACK,0,NPC_BLINDATTACK,LK_HEADCRUSH,
		NPC_WIDEHEALTHFEAR,NPC_WIDEBODYBURNNING,NPC_WIDE_DEEP_SLEEP,
		NPC_WIDEFROSTMISTY,NPC_WIDECOLD,NPC_WIDESIREN
	};
	static const int sc_id[] = {
		SC_STONE,SC_FREEZE,SC_STUN,
		SC_SLEEP,SC_POISON,SC_CURSE,
		SC_SILENCE,SC_CONFUSION,SC_BLIND,SC_BLEED,
		SC_FEAR,SC_HELLINFERNO,SC_DEEP_SLEEP,
		SC_FROSTMISTY,SC_DIAMONDDUST,SC_SIREN
	};

	nullpo_retr(0, src);
	nullpo_retr(0, bl);

	if(src->type != BL_PC || !(bl->type & BL_CHAR))
		return 0;

	sd = (struct map_session_data *)src;

	if(atn_rand() % 10000 < status_change_rate(bl,sc_id[id],rate,sd->status.base_level)) {

		if(battle_config.battle_log) {
			if(type==1)
				printf("PC %d skill_skilladdeff: card�ɂ���Ԉُ픭�� %d %d %d\n",sd->bl.id,sd->skill_addeff.id[id],id,rate);
			else if(type==2)
				printf("PC %d skill_addeff: card�ɂ���Ԉُ픭�� %d %d\n",sd->bl.id,id,rate);
			else if(type==3)
				printf("PC %d skill_addeff2: card�ɂ���Ԉُ픭�� %d %d\n",sd->bl.id,id,rate);
			else if(type==4)
				printf("PC %d magic_addeff: card�ɂ���Ԉُ픭�� %d %d\n",sd->bl.id,id,rate);
			else
				printf("PC %d skill_addreveff: card�ɂ��ُ픭�� %d %d\n",sd->bl.id,id,sd->addreveff[id]);
		}

		switch(sc_id[id]) {
			case SC_STONE:
			case SC_FREEZE:
			case SC_STUN:
			case SC_SLEEP:
			case SC_POISON:
			case SC_CURSE:
			case SC_SILENCE:
			case SC_CONFUSION:
			case SC_BLIND:
			case SC_BLEED:
				if(type)
					status_change_pretimer(bl,sc_id[id],7,0,0,0,((sc_id[id] == SC_CONFUSION)? 10000+7000: skill_get_time2(sc2[id],7)),0,tick+status_get_amotion(src)+500);
				else
					status_change_start(bl,sc_id[id],7,0,0,0,((sc_id[id] == SC_CONFUSION)? 10000+7000: skill_get_time2(sc2[id],7)),0);
				break;
			case SC_FEAR:
				if(type)
					status_change_pretimer(bl,sc_id[id],1,0,0,0,skill_get_time2(sc2[id],1),0,tick+status_get_amotion(src)+500);
				else
					status_change_start(bl,sc_id[id],1,0,0,0,skill_get_time2(sc2[id],1),0);
				break;
			case SC_FROSTMISTY:
			case SC_HELLINFERNO:
				if(type)
					status_change_pretimer(bl,sc_id[id],1,0,0,0,10000+skill_get_time2(sc2[id],1),0,tick+status_get_amotion(src)+500);
				else
					status_change_start(bl,sc_id[id],1,0,0,0,10000+skill_get_time2(sc2[id],1),0);
				break;
			case SC_DEEP_SLEEP:
			case SC_DIAMONDDUST:
				if(type)
					status_change_pretimer(bl,sc_id[id],1,0,0,0,3000+skill_get_time2(sc2[id],1),0,tick+status_get_amotion(src)+500);
				else
					status_change_start(bl,sc_id[id],1,0,0,0,3000+skill_get_time2(sc2[id],1),0);
				break;
			case SC_SIREN:
				if(type)
					status_change_pretimer(bl,sc_id[id],1,src->id,0,0,skill_get_time2(sc2[id],1),0,tick+status_get_amotion(src)+500);
				else
					status_change_start(bl,sc_id[id],1,src->id,0,0,skill_get_time2(sc2[id],1),0);
				break;
		}
	}
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
static void status_split_atoi(char *str, int *num1, int *num2)
{
	int i, val[2];

	for (i=0; i<2; i++) {
		if(str) {
			val[i] = atoi(str);
			str = strchr(str,':');
			if (str)
				*str++=0;
		} else {
			val[i] = 0;
		}
	}
	if(val[0])
		*num1 = val[0];
	if(val[1])
		*num2 = val[1];
	return;
}

/*==========================================
 * �f�[�^�x�[�X�ǂݍ���
 *------------------------------------------
 */
int status_readdb(void)
{
	int i,j,k;
	FILE *fp;
	char line[1024],*p;
	const char *filename;

	memset(&job_db, 0, sizeof(job_db));

	// JOB�␳���l�P
#ifdef PRE_RENEWAL
	filename = "db/pre/job_db1_pre.txt";
#else
	filename = "db/job_db1.txt";
#endif
	fp = fopen(filename, "r");
	if(fp == NULL) {
		printf("status_readdb: open [%s] failed !\n", filename);
		return 1;
	}
	i=0;
	while(fgets(line,1020,fp)){
		char *split[WT_MAX+5];
		int hp_coefficient, sp_coefficient;

		if(line[0] == '\0' || line[0] == '\r' || line[0] == '\n')
			continue;
		if(line[0]=='/' && line[1]=='/')
			continue;
		memset(split,0,sizeof(split));
		for(j=0,p=line;j<WT_MAX+5 && p;j++){
			split[j]=p;
			p=strchr(p,',');
			if(p) *p++=0;
		}
		if(j < 4)
			continue;
		job_db[i].max_weight_base = atoi(split[0]);

		hp_coefficient = atoi(split[1]);
		if(hp_coefficient >= 0) {
			int hp_coefficient2 = atoi(split[2]);
			int sigma = 0;
			for(j = 1; j <= MAX_LEVEL; j++) {
				// ��{HP = 35 + BaseLevel * Job�{�� + Job�{�[�i�X
				job_db[i].hp_base[j-1] = (3500 + j * hp_coefficient2 + sigma) / 100;
				sigma += hp_coefficient * (j + 1) + 50;
				sigma -= sigma % 100;
			}
		}

		sp_coefficient = atoi(split[3]);
		if(sp_coefficient >= 0) {
			for(j = 1; j <= MAX_LEVEL; j++) {
				// ��{SP = 10 + BaseLevel * Job�W��
				job_db[i].sp_base[j-1] = (1000 + j * sp_coefficient) / 100;
			}
		}

		for(j=0; j<=WT_MAX && split[j+4]; j++) {
			job_db[i].aspd_base[j] = atoi(split[j+4]);
		}
		if(++i >= PC_JOB_MAX)
			break;
	}
	fclose(fp);
	printf("read %s done\n", filename);

	// ��{HP�ʐݒ�
	filename = "db/job_hp_db.txt";
	fp = fopen(filename, "r");
	if(fp == NULL) {
		printf("status_readdb: open [%s] failed !\n", filename);
		return 1;
	}
	i=0;
	while(fgets(line,1020,fp)){
		if(line[0] == '\0' || line[0] == '\r' || line[0] == '\n')
			continue;
		if(line[0]=='/' && line[1]=='/')
			continue;
		for(j=0,p=line;j<PC_JOB_MAX && p;j++){
			if(sscanf(p,"%d",&k) == 0)
				break;
			if(job_db[j].hp_base[i] == 0) {
				// job_db1.txt��HP���ݒ�̏ꍇ�̂ݕ⊮
				job_db[j].hp_base[i] = (k > 0)? k: 1;
			}
			p=strchr(p,',');
			if(p) *p++=0;
		}
		if(++i >= MAX_LEVEL)
			break;
	}
	fclose(fp);
	printf("read %s done\n", filename);

	// ��{SP�ʐݒ�
	filename = "db/job_sp_db.txt";
	fp = fopen(filename, "r");
	if(fp == NULL) {
		printf("status_readdb: open [%s] failed !\n", filename);
		return 1;
	}
	i=0;
	while(fgets(line,1020,fp)){
		if(line[0] == '\0' || line[0] == '\r' || line[0] == '\n')
			continue;
		if(line[0]=='/' && line[1]=='/')
			continue;
		for(j=0,p=line;j<PC_JOB_MAX && p;j++){
			if(sscanf(p,"%d",&k) == 0)
				break;
			if(job_db[j].sp_base[i] == 0) {
				// job_db1.txt��SP���ݒ�̏ꍇ�̂ݕ⊮
				job_db[j].sp_base[i] = (k > 0)? k: 1;
			}
			p=strchr(p,',');
			if(p) *p++=0;
		}
		if(++i >= MAX_LEVEL)
			break;
	}
	fclose(fp);
	printf("read %s done\n", filename);

	// JOB�{�[�i�X
	filename = "db/job_db2.txt";
	fp = fopen(filename, "r");
	if(fp == NULL) {
		printf("status_readdb: open [%s] failed !\n", filename);
		return 1;
	}
	i=0;
	while(fgets(line,1020,fp)){
		if(line[0] == '\0' || line[0] == '\r' || line[0] == '\n')
			continue;
		if(line[0]=='/' && line[1]=='/')
			continue;
		for(j=0,p=line;j<MAX_LEVEL && p;j++){
			if(sscanf(p,"%d",&k)==0)
				break;
			job_db[i].bonus[PC_UPPER_NORMAL][j] = k;
			job_db[i].bonus[PC_UPPER_BABY][j] = k;
			p=strchr(p,',');
			if(p) p++;
		}
		if(++i >= PC_JOB_MAX)
			break;
	}
	fclose(fp);
	printf("read %s done\n", filename);

	// JOB�{�[�i�X2 �]���E�p
	filename = "db/job_db2-2.txt";
	fp = fopen(filename, "r");
	if(fp == NULL) {
		printf("status_readdb: open [%s] failed !\n", filename);
		return 1;
	}
	i=0;
	while(fgets(line,1020,fp)){
		if(line[0] == '\0' || line[0] == '\r' || line[0] == '\n')
			continue;
		if(line[0]=='/' && line[1]=='/')
			continue;
		for(j=0,p=line;j<MAX_LEVEL && p;j++){
			if(sscanf(p,"%d",&k)==0)
				break;
			job_db[i].bonus[PC_UPPER_HIGH][j] = k;
			p=strchr(p,',');
			if(p) p++;
		}
		if(++i >= PC_JOB_MAX)
			break;
	}
	fclose(fp);
	printf("read %s done\n", filename);

	// ���B�f�[�^�e�[�u��
	for(i=0; i<MAX_WEAPON_LEVEL+1; i++) {
		for(j=0; j<MAX_REFINE; j++)
			refine_db[i].safety_bonus[j] = 0;
		refine_db[i].over_bonus   = 0;
		refine_db[i].limit        = MAX_REFINE;
		for(j=0; j<MAX_REFINE; j++)
			refine_db[i].per[j] = 0;
	}

#ifdef PRE_RENEWAL
	filename = "db/pre/refine_db_pre.txt";
#else
	filename = "db/refine_db.txt";
#endif
	fp = fopen(filename, "r");
	if(fp == NULL) {
		printf("status_readdb: open [%s] failed !\n", filename);
		return 1;
	}
	i=0;
	while(fgets(line,1020,fp)){
		char *split[MAX_REFINE+3];
		if(line[0] == '\0' || line[0] == '\r' || line[0] == '\n')
			continue;
		if(line[0]=='/' && line[1]=='/')
			continue;
		if(atoi(line) <= 0)
			continue;
		memset(split,0,sizeof(split));
		for(j=0,p=line;j<MAX_REFINE+3 && p;j++){
			split[j]=p;
			p=strchr(p,',');
			if(p) *p++=0;
		}
		for(j=0; j<MAX_REFINE; j++) {
			refine_db[i].safety_bonus[j] = atoi(split[0]) * (j+1);	// ���B�{�[�i�X
		}
		refine_db[i].over_bonus   = atoi(split[1]);	// �ߏ萸�B�{�[�i�X
		refine_db[i].limit        = atoi(split[2]);	// ���S���B���E
		for(j=0; j<MAX_REFINE && split[j+3]; j++) {
			status_split_atoi(split[j+3], &refine_db[i].per[j], &refine_db[i].safety_bonus[j]);
		}
		if(++i > MAX_WEAPON_LEVEL)
			break;
	}
	fclose(fp);
	printf("read %s done\n", filename);

	// �T�C�Y�␳�e�[�u��
	for(i=0; i<MAX_SIZE_FIX; i++) {
		for(j=0; j<WT_MAX; j++)
			atkmods[i][j] = 100;
	}

	filename = "db/size_fix.txt";
	fp = fopen(filename, "r");
	if(fp == NULL) {
		printf("status_readdb: open [%s] failed !\n", filename);
		return 1;
	}
	i=0;
	while(fgets(line,1020,fp)){
		char *split[WT_MAX];
		if(line[0] == '\0' || line[0] == '\r' || line[0] == '\n')
			continue;
		if(line[0]=='/' && line[1]=='/')
			continue;
		if(atoi(line) <= 0)
			continue;
		memset(split,0,sizeof(split));
		for(j=0,p=line;j<WT_MAX && p;j++){
			split[j]=p;
			p=strchr(p,',');
			if(p) *p++=0;
		}
		for(j=0; j<WT_MAX && split[j]; j++) {
			atkmods[i][j] = atoi(split[j]);
		}
		if(++i > MAX_SIZE_FIX)
			break;
	}
	fclose(fp);
	printf("read %s done\n", filename);

	// �X�e�[�^�X�ُ�e�[�u��
	memset(&scdata_db, 0, sizeof(scdata_db));
	filename = "db/scdata_db.txt";
	fp = fopen(filename, "r");
	if(fp == NULL) {
		printf("status_readdb: open [%s] failed !\n", filename);
		return 1;
	}
	i=0;
	while(fgets(line,1020,fp)){
		char *split[6];
		if(line[0] == '\0' || line[0] == '\r' || line[0] == '\n')
			continue;
		if(line[0] == '/' && line[1] == '/')
			continue;
		memset(split,0,sizeof(split));
		for(j=0,p=line;j<6 && p;j++){
			split[j]=p;
			p=strchr(p,',');
			if(p) *p++=0;
		}
		j = atoi(split[0]);
		if(j < 0 || j >= MAX_STATUSCHANGE)
			continue;
		scdata_db[j].save       = (short)atoi(split[2]);
		scdata_db[j].releasable = atoi(split[3]);
		scdata_db[j].disable    = atoi(split[4]);
		scdata_db[j].copyable   = atoi(split[5]);
		i++;
	}
	fclose(fp);
	printf("read %s done (count=%d)\n", filename, i);

#ifdef DYNAMIC_SC_DATA
	for(i=0; i<MAX_STATUSCHANGE; i++) {
		dummy_sc_data[i].timer = -1;
		dummy_sc_data[i].val1  = 0;
		dummy_sc_data[i].val2  = 0;
		dummy_sc_data[i].val3  = 0;
		dummy_sc_data[i].val4  = 0;
	}
	printf("status_readdb: enable dynamic sc_data.\n");
#endif

	return 0;
}

/*==========================================
 * �X�L���֌W����������
 *------------------------------------------
 */
int do_init_status(void)
{
	status_readdb();

	add_timer_func_list(status_change_timer);
	add_timer_func_list(status_pretimer_timer);

	return 0;
}
