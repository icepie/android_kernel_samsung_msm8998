/*
 * sec_multi_port_charger.h
 * Samsung Mobile Charger Header
 *
 * Copyright (C) 2015 Samsung Electronics, Inc.
 *
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#ifndef __SEC_MULTI_PORT_CHARGER_H
#define __SEC_MULTI_PORT_CHARGER_H __FILE__

#include "include/sec_battery.h"
#include "sec_charging_common.h"

#define SEC_MAIN_CHARGER_CONDITION_NONE			0x00
#define SEC_MAIN_CHARGER_CONDITION_TYPE			0x80

#define SEC_SUB_CHARGER_CONDITION_NONE			0x00
#define SEC_SUB_CHARGER_CONDITION_ONLINE		0x01
#define SEC_SUB_CHARGER_CONDITION_CV			0x02
#define SEC_SUB_CHARGER_CONDITION_CHARGE_DONE	0x04
#define SEC_SUB_CHARGER_CONDITION_CURRENT_MAX	0x10
#define SEC_SUB_CHARGER_CONDITION_CURRENT_NOW	0x20
#define SEC_SUB_CHARGER_CONDITION_CHARGE_POWER	0x40
#define SEC_SUB_CHARGER_CONDITION_CHARGE_TYPE 	0x80

/* main charger off in case of otg */
#define SEC_MAIN_CHARGER_CONDITION_OTG_OFF 	0x01

#define SEC_SUB_CHARGER_CURRENT_MARGIN			100

#define SEC_MULTI_CHARGER_TEST_MASTER_MODE_CURRENT	1450
#define SEC_MULTI_CHARGER_TEST_SLAVE_MODE_CURRENT	700

enum sec_multi_charger_mode {
	SEC_MULTI_CHARGER_NORMAL = 0, /* Main Charger Default ON;  Sub charger: depend on sub_charger_condition */
	SEC_MULTI_CHARGER_MAIN_ONLY,
	SEC_MULTI_CHARGER_SUB_ONLY,
	SEC_MULTI_CHARGER_ALL_ENABLE,
};

struct sec_multi_charger_platform_data {
	char *battery_name;
	char *main_charger_name;
	char *sub_charger_name;

	bool aicl_disable;

	unsigned int sub_charger_condition;
	unsigned int main_charger_condition;
	unsigned int sub_charger_condition_charge_power;
	int sub_charger_condition_current_max;
	unsigned int *sub_charger_condition_online;
	unsigned int sub_charger_condition_online_size;
	unsigned int sub_charger_condition_current_margin;

	int max_sub_port_input_current;		/* input current limit for main wcin + sub chgin dual charging */
	int max_sub_port_charging_current;	/* charging current limit for main wcin + sub chgin dual charging */
	int max_wcin_input_current;			/* input current limit for main wcin */
	int max_sub_input_current;			/* input current limit for sub chgin */
	int max_sub_charging_current;		/* charging current limit for sub chgin */
	int max_total_charging_current;		/* charging current limit for the battery */
};

struct sec_multi_charger_info {
	struct device *dev;
	struct sec_multi_charger_platform_data *pdata;
	struct power_supply		*psy_chg;
	struct mutex charger_mutex;
#if defined(CONFIG_SEC_FACTORY)
	struct mutex charger_test_mutex;
#endif

	int cable_type;
	int status;

	int main_input_current;
	int main_charging_current;

	int sub_input_current;
	int sub_charging_current;

	/* sub_charger should be disabled before 1st EOC */
	bool sub_is_charging;
	int multi_mode;

	sec_charging_current_t main_current;
	sec_charging_current_t sub_current;

	unsigned int full_check_current_1st;

	int chg_mode;
};

#endif /* __SEC_MULTI_PORT_CHARGER_H */
