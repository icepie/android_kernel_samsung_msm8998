/*
*  Copyright (C) 2012, Samsung Electronics Co. Ltd. All Rights Reserved.
*
*  This program is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation; either version 2 of the License, or
*  (at your option) any later version.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*/
#include <linux/init.h>
#include <linux/module.h>
#include "adsp.h"
#ifdef CONFIG_SLPI_MAG_CALIB_RESET
#include <linux/adsp/slpi_mag_calib_reset.h>
#endif
#define VENDOR "AKM"
#ifdef CONFIG_AK09918C_FACTORY
#define CHIP_ID "AK09918C"
#else
#define CHIP_ID "AK09916C"
#endif
#define RAWDATA_TIMER_MS 200
#define RAWDATA_TIMER_MARGIN_MS 20
#define MAG_SELFTEST_TRY_CNT 3
#define AK09911C_MODE_POWERDOWN 0x00
#define SNS_EFAIL 1
#define SNS_SUCCEES 0

#ifdef CONFIG_SLPI_MAG_CALIB_RESET
struct mag_calib_reset_data {
	struct workqueue_struct *slpi_mag_calib_reset_wq;
	struct work_struct work_slpi_mag_calib_reset;
};

struct mag_calib_reset_data *magdata;

void slpi_mag_calib_reset_work_func(struct work_struct *work);
int (*sensorCallback)(void);
#endif

static int mag_read_register(struct device *dev,
	struct device_attribute *attr)
{
	struct adsp_data *data = dev_get_drvdata(dev);
	struct msg_data message;
	uint8_t cnt = 0;

	message.sensor_type = ADSP_FACTORY_MAG;
	data->magtest_ready_flag &= ~(1 << ADSP_FACTORY_MAG);
	adsp_unicast(&message, sizeof(message),
		NETLINK_MESSAGE_MAG_READ_REGISTERS, 0, 0);

	while (!(data->magtest_ready_flag & 1 << ADSP_FACTORY_MAG) &&
		cnt++ < TIMEOUT_CNT)
		msleep(20);

	data->magtest_ready_flag &= ~(1 << ADSP_FACTORY_MAG);

	if (cnt >= TIMEOUT_CNT) {
		pr_err("[FACTORY] %s: Timeout!!!\n", __func__);
		return -SNS_EFAIL;
	}

	pr_info("[FACTORY] %s: result(%d), mag_x(%d), mag_y(%d), mag_z(%d)\n",
		__func__,
		data->sensor_mag_factory_result.result,
		data->sensor_mag_factory_result.registers[0],
		data->sensor_mag_factory_result.registers[1],
		data->sensor_mag_factory_result.registers[2]);

	if (data->sensor_mag_factory_result.result == -1)
		return -SNS_EFAIL;
	else
		return SNS_SUCCEES;
}

static ssize_t mag_vendor_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%s\n", VENDOR);
}

static ssize_t mag_name_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%s\n", CHIP_ID);
}

static ssize_t mag_read_adc(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct adsp_data *data = dev_get_drvdata(dev);

	if (adsp_start_raw_data(ADSP_FACTORY_MAG) == false) {
		pr_err("[FACTORY] %s: NG\n", __func__);

		return snprintf(buf, PAGE_SIZE,
			"%s,%d,%d,%d\n", "NG", 0, 0, 0);
	}

	pr_info("[FACTORY] %s: %s,%d,%d,%d\n", __func__, "OK",
		data->sensor_data[ADSP_FACTORY_MAG].x,
		data->sensor_data[ADSP_FACTORY_MAG].y,
		data->sensor_data[ADSP_FACTORY_MAG].z);

	return snprintf(buf, PAGE_SIZE, "%s,%d,%d,%d\n", "OK",
		data->sensor_data[ADSP_FACTORY_MAG].x,
		data->sensor_data[ADSP_FACTORY_MAG].y,
		data->sensor_data[ADSP_FACTORY_MAG].z);
}

static ssize_t mag_check_cntl(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct adsp_data *data = dev_get_drvdata(dev);
	int8_t reg;

	reg = mag_read_register(dev, attr);

	if (reg < 0) {
		data->sensor_mag_factory_result.registers[13] = 0;
		pr_err("[FACTORY] %s: failed!! = %d\n", __func__, reg);
	}

	return snprintf(buf, PAGE_SIZE, "%s\n",
			(((data->sensor_mag_factory_result.registers[13] ==
			AK09911C_MODE_POWERDOWN) &&
			(reg == 0)) ? "OK" : "NG"));
}

static ssize_t mag_check_registers(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct adsp_data *data = dev_get_drvdata(dev);
	int reg;

	reg = mag_read_register(dev, attr);

	if (reg < 0) {
		pr_info("[FACTORY] %s: Fail!! = %d\n", __func__, reg);
		return snprintf(buf, PAGE_SIZE,
			"%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	} else {
		pr_info("[FACTORY] %s: Pass!!\n", __func__);
		return snprintf(buf, PAGE_SIZE,
			"%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
			data->sensor_mag_factory_result.registers[0],
			data->sensor_mag_factory_result.registers[1],
			data->sensor_mag_factory_result.registers[2],
			data->sensor_mag_factory_result.registers[3],
			data->sensor_mag_factory_result.registers[4],
			data->sensor_mag_factory_result.registers[5],
			data->sensor_mag_factory_result.registers[6],
			data->sensor_mag_factory_result.registers[7],
			data->sensor_mag_factory_result.registers[8],
			data->sensor_mag_factory_result.registers[9],
			data->sensor_mag_factory_result.registers[10],
			data->sensor_mag_factory_result.registers[11],
			data->sensor_mag_factory_result.registers[12]);
	}
}

static ssize_t mag_get_asa(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	/* Do not have Fuserom */
	return snprintf(buf, PAGE_SIZE, "%u,%u,%u\n", 128, 128, 128);
}

static ssize_t mag_get_status(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	/* Do not have Fuserom */
	return snprintf(buf, PAGE_SIZE, "%s\n", "OK");
}

static ssize_t mag_raw_data_read(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	static uint8_t sample_cnt;
	struct adsp_data *data = dev_get_drvdata(dev);

	if (adsp_start_raw_data(ADSP_FACTORY_MAG) == false)
		return snprintf(buf, PAGE_SIZE, "%d\n", 0);

	if (++sample_cnt > 20) {/* sample log 1.6s */
		sample_cnt = 0;
		pr_info("[FACTORY] %s: %d,%d,%d\n", __func__,
			data->sensor_data[ADSP_FACTORY_MAG].x,
			data->sensor_data[ADSP_FACTORY_MAG].y,
			data->sensor_data[ADSP_FACTORY_MAG].z);
	}

	return snprintf(buf, PAGE_SIZE, "%d,%d,%d\n",
		data->sensor_data[ADSP_FACTORY_MAG].x,
		data->sensor_data[ADSP_FACTORY_MAG].y,
		data->sensor_data[ADSP_FACTORY_MAG].z);
}

static ssize_t mag_selftest_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct adsp_data *data = dev_get_drvdata(dev);
	struct msg_data message;
	int result1 = 0;
	int temp[10];
	uint8_t ret;
	uint8_t retry = 0;
	uint8_t cnt = 0;

retry_mag_selftest:
	message.sensor_type = ADSP_FACTORY_MAG;
	data->selftest_ready_flag &= ~(1 << ADSP_FACTORY_MAG);
	adsp_unicast(&message, sizeof(message),
		NETLINK_MESSAGE_SELFTEST_SHOW_DATA, 0, 0);

	while (!(data->selftest_ready_flag & 1 << ADSP_FACTORY_MAG) &&
		cnt++ < TIMEOUT_CNT)
		msleep(20);

	data->selftest_ready_flag &= ~(1 << ADSP_FACTORY_MAG);

	if (cnt >= TIMEOUT_CNT) {
		pr_err("[FACTORY] %s: Timeout!!!\n", __func__);
		goto timeout;
	}

	temp[0] = data->sensor_selftest_result[ADSP_FACTORY_MAG].result1;
	temp[1] = data->sensor_selftest_result[ADSP_FACTORY_MAG].result2;
	temp[2] = data->sensor_selftest_result[ADSP_FACTORY_MAG].offset_x;
	temp[3] = data->sensor_selftest_result[ADSP_FACTORY_MAG].offset_y;
	temp[4] = data->sensor_selftest_result[ADSP_FACTORY_MAG].offset_z;
	temp[5] = data->sensor_selftest_result[ADSP_FACTORY_MAG].dac_ret;
	temp[6] = data->sensor_selftest_result[ADSP_FACTORY_MAG].adc_ret;
	temp[7] = data->sensor_selftest_result[ADSP_FACTORY_MAG].ohx;
	temp[8] = data->sensor_selftest_result[ADSP_FACTORY_MAG].ohy;
	temp[9] = data->sensor_selftest_result[ADSP_FACTORY_MAG].ohz;

	/* Data Process */
	if (temp[0] == 0)
		result1 = 1;
	else
		result1 = 0;

	pr_info("[FACTORY] status=%d, sf_status=%d, sf_x=%d, sf_y=%d, sf_z=%d\n"
		"[FACTORY] dac=%d, adc=%d, adc_x=%d, adc_y=%d, adc_z=%d\n",
		temp[0], temp[1], temp[2], temp[3], temp[4],
		temp[5], temp[6], temp[7], temp[8], temp[9]);

timeout:
	if (!result1) {
		if (retry < MAG_SELFTEST_TRY_CNT) {
			retry++;
			for (ret = 0; ret < 10; ret++)
				temp[ret] = 0;
			msleep(RAWDATA_TIMER_MS + RAWDATA_TIMER_MARGIN_MS);
			goto retry_mag_selftest;
		}
	}

	return snprintf(buf, PAGE_SIZE,
		"%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
		temp[0], temp[1], temp[2], temp[3], temp[4],
		temp[5], temp[6], temp[7], temp[8], temp[9]);
}

static ssize_t mag_dhr_sensor_info_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct adsp_data *data = dev_get_drvdata(dev);
	uint8_t cnt = 0;

	struct msg_data message;
	message.sensor_type = ADSP_FACTORY_MAG;
	data->calib_ready_flag &= ~(1 << ADSP_FACTORY_MAG);
	adsp_unicast(&message, sizeof(message),
		NETLINK_MESSAGE_GET_CALIB_DATA, 0, 0);

	while (!(data->calib_ready_flag & 1 << ADSP_FACTORY_MAG) &&
		cnt++ < TIMEOUT_CNT)
		msleep(20);

	data->calib_ready_flag &= ~(1 << ADSP_FACTORY_MAG);

	if (cnt >= TIMEOUT_CNT)
		pr_err("[FACTORY] %s: Timeout!!!\n", __func__);

	pr_info("[FACTORY] %s\n", __func__);
	pr_info("[FACTORY] 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x"\
		" 0x%08x 0x%08x 0x%08x 0x%08x\n",
		data->sensor_calib_data[ADSP_FACTORY_MAG].si_mat[0],
		data->sensor_calib_data[ADSP_FACTORY_MAG].si_mat[1],
		data->sensor_calib_data[ADSP_FACTORY_MAG].si_mat[2],
		data->sensor_calib_data[ADSP_FACTORY_MAG].si_mat[3],
		data->sensor_calib_data[ADSP_FACTORY_MAG].si_mat[4],
		data->sensor_calib_data[ADSP_FACTORY_MAG].si_mat[5],
		data->sensor_calib_data[ADSP_FACTORY_MAG].si_mat[6],
		data->sensor_calib_data[ADSP_FACTORY_MAG].si_mat[7],
		data->sensor_calib_data[ADSP_FACTORY_MAG].si_mat[8]);

	return snprintf(buf, PAGE_SIZE,
		"\"SI_PARAMETER\":\"0x%08x 0x%08x 0x%08x 0x%08x"\
		" 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x\"\n",
		data->sensor_calib_data[ADSP_FACTORY_MAG].si_mat[0],
		data->sensor_calib_data[ADSP_FACTORY_MAG].si_mat[1],
		data->sensor_calib_data[ADSP_FACTORY_MAG].si_mat[2],
		data->sensor_calib_data[ADSP_FACTORY_MAG].si_mat[3],
		data->sensor_calib_data[ADSP_FACTORY_MAG].si_mat[4],
		data->sensor_calib_data[ADSP_FACTORY_MAG].si_mat[5],
		data->sensor_calib_data[ADSP_FACTORY_MAG].si_mat[6],
		data->sensor_calib_data[ADSP_FACTORY_MAG].si_mat[7],
		data->sensor_calib_data[ADSP_FACTORY_MAG].si_mat[8]);
}

#ifdef CONFIG_SLPI_MAG_CALIB_RESET
int slpi_mag_calib_reset_callback(void)
{
	pr_info("[FACTORY] %s: start!", __func__);
	queue_work(magdata->slpi_mag_calib_reset_wq, &magdata->work_slpi_mag_calib_reset);
	return 0;
}

int (*getMagCalibResetCallback(void))(void)
{
	return slpi_mag_calib_reset_callback;
}

void slpi_mag_calib_reset_work_func(struct work_struct *work)
{
	struct msg_data message;
	int calib_reset = 0;

	calib_reset = NETLINK_MESSAGE_MAG_CALIB_RESET;
	message.sensor_type = ADSP_FACTORY_MAG_CALIB_RESET;

	pr_info("[FACTORY] %s: start!\n", __func__);

//	msleep(RAWDATA_TIMER_MS + RAWDATA_TIMER_MARGIN_MS);
	adsp_unicast(&message, sizeof(message), calib_reset, 0, 0);
}
#endif

static DEVICE_ATTR(name, S_IRUGO, mag_name_show, NULL);
static DEVICE_ATTR(vendor, S_IRUGO, mag_vendor_show, NULL);
static DEVICE_ATTR(raw_data, S_IRUGO, mag_raw_data_read, NULL);
static DEVICE_ATTR(adc, S_IRUGO, mag_read_adc, NULL);
static DEVICE_ATTR(dac, S_IRUGO, mag_check_cntl, NULL);
static DEVICE_ATTR(chk_registers, S_IRUGO, mag_check_registers, NULL);
static DEVICE_ATTR(selftest, S_IRUSR | S_IRGRP,
	mag_selftest_show, NULL);
static DEVICE_ATTR(asa, S_IRUGO, mag_get_asa, NULL);
static DEVICE_ATTR(status, S_IRUGO, mag_get_status, NULL);
static DEVICE_ATTR(dhr_sensor_info, S_IRUSR | S_IRGRP,
	mag_dhr_sensor_info_show, NULL);

static struct device_attribute *mag_attrs[] = {
	&dev_attr_name,
	&dev_attr_vendor,
	&dev_attr_raw_data,
	&dev_attr_adc,
	&dev_attr_dac,
	&dev_attr_chk_registers,
	&dev_attr_selftest,
	&dev_attr_asa,
	&dev_attr_status,
	&dev_attr_dhr_sensor_info,
	NULL,
};

static int __init ak09916c_factory_init(void)
{
	adsp_factory_register(ADSP_FACTORY_MAG, mag_attrs);

#ifdef CONFIG_SLPI_MAG_CALIB_RESET
        magdata = kzalloc(sizeof(*magdata), GFP_KERNEL);
    
        if (magdata == NULL) {
            pr_err("[FACTORY]: %s - could not allocate memory\n", __func__);
            return 0;
        }
    
        magdata->slpi_mag_calib_reset_wq = 
            create_singlethread_workqueue("slpi_mag_calib_reset_wq");
    
        if (magdata->slpi_mag_calib_reset_wq == NULL) {
            pr_err("[FACTORY]: %s - could not create mag calib reset wq\n", __func__);
            return 0;
        }
    
        INIT_WORK(&magdata->work_slpi_mag_calib_reset, slpi_mag_calib_reset_work_func);
#endif

	pr_info("[FACTORY] %s\n", __func__);

	return 0;
}

static void __exit ak09916c_factory_exit(void)
{
	adsp_factory_unregister(ADSP_FACTORY_MAG);
    
#ifdef CONFIG_SLPI_MAG_CALIB_RESET
        if (magdata != NULL && magdata->slpi_mag_calib_reset_wq != NULL) {
            cancel_work_sync(&magdata->work_slpi_mag_calib_reset);
            destroy_workqueue(magdata->slpi_mag_calib_reset_wq);
            magdata->slpi_mag_calib_reset_wq = NULL;
        }
#endif

	pr_info("[FACTORY] %s\n", __func__);
}

module_init(ak09916c_factory_init);
module_exit(ak09916c_factory_exit);
