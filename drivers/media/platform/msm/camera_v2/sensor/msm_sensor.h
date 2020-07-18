/* Copyright (c) 2011-2016, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef MSM_SENSOR_H
#define MSM_SENSOR_H

#include <linux/debugfs.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/clk.h>
#include <linux/types.h>
#include <linux/uaccess.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/gpio.h>
#include <soc/qcom/camera2.h>
#include <media/msm_cam_sensor.h>
#include <media/v4l2-subdev.h>
#include <media/v4l2-ioctl.h>
#include "msm_camera_i2c.h"
#include "msm_camera_dt_util.h"
#include "msm_sd.h"

#define DEFINE_MSM_MUTEX(mutexname) \
	static struct mutex mutexname = __MUTEX_INITIALIZER(mutexname)

enum msm_sensor_sensor_slave_info_type {
	MSM_SENSOR_SLAVEADDR_DATA,
	MSM_SENSOR_IDREGADDR_DATA,
	MSM_SENSOR_SENSOR_ID_DATA,
	MSM_SENSOR_SENIDMASK_DATA,
	MSM_SENSOR_NUM_ID_INFO_DATA,
};

struct msm_sensor_ctrl_t;

enum msm_sensor_state_t {
	MSM_SENSOR_POWER_DOWN,
	MSM_SENSOR_POWER_UP,
};

struct msm_sensor_fn_t {
	int (*sensor_config)(struct msm_sensor_ctrl_t *, void __user *);
#ifdef CONFIG_COMPAT
	int (*sensor_config32)(struct msm_sensor_ctrl_t *, void __user *);
#endif
	int (*sensor_power_down)(struct msm_sensor_ctrl_t *);
	int (*sensor_power_up)(struct msm_sensor_ctrl_t *);
	int (*sensor_match_id)(struct msm_sensor_ctrl_t *);
};

struct msm_sensor_ctrl_t {
	struct platform_device *pdev;
	struct mutex *msm_sensor_mutex;

	enum msm_camera_device_type_t sensor_device_type;
	struct msm_camera_sensor_board_info *sensordata;
	struct msm_sensor_power_setting_array power_setting_array;
	struct msm_sensor_packed_cfg_t *cfg_override;
	struct msm_sd_subdev msm_sd;
	enum cci_i2c_master_t cci_i2c_master;

	struct msm_camera_i2c_client *sensor_i2c_client;
	struct v4l2_subdev_info *sensor_v4l2_subdev_info;
	uint8_t sensor_v4l2_subdev_info_size;
	struct v4l2_subdev_ops *sensor_v4l2_subdev_ops;
	struct msm_sensor_fn_t *func_tbl;
	struct msm_camera_i2c_reg_setting stop_setting;
	void *misc_regulator;
	enum msm_sensor_state_t sensor_state;
	uint8_t is_probe_succeed;
	uint32_t id;
	struct device_node *of_node;
	enum msm_camera_stream_type_t camera_stream_type;
	uint32_t set_mclk_23880000;
	uint8_t is_csid_tg_mode;
	uint32_t is_secure;
};

#if defined(CONFIG_USE_CAMERA_HW_BIG_DATA)
#define TRUE	1
#define FALSE	0

#define HW_PARAMS_MI_INVALID	0
#define HW_PARAMS_MI_VALID	1
#define HW_PARAMS_MIR_ERR_0	2
#define HW_PARAMS_MIR_ERR_1	3

#define CAM_HW_PARM_CLK_CNT 2
#define CAM_HW_PARM_CC_CLK_CNT 4

#define CAM_HW_ERR_CNT_FILE_PATH "/data/camera/camera_hw_err_cnt.dat"

typedef enum
{
	HW_PARAMS_CREATED = 0,
	HW_PARAMS_NOT_CREATED,
} hw_params_check_type;

struct cam_hw_param {
	u32 i2c_sensor_err_cnt;
	u32 i2c_comp_err_cnt;
	u32 i2c_ois_err_cnt;
	u32 i2c_af_err_cnt;
	u32 mipi_sensor_err_cnt;
	u32 mipi_comp_err_cnt;
	u16 i2c_chk;
	u16 mipi_chk;
	u16 comp_chk;
	u16 need_update_to_file;
} __attribute__((__packed__));

struct cam_hw_param_collector {
	struct cam_hw_param rear_hwparam;
	struct cam_hw_param front_hwparam;
	struct cam_hw_param iris_hwparam;
	struct cam_hw_param rear2_hwparam;
} __attribute__((__packed__));

void msm_is_sec_init_all(void);
void msm_is_sec_dbg_check(void);
void msm_is_sec_init_err_cnt_file(struct cam_hw_param *hw_param);
void msm_is_sec_copy_err_cnt_from_file(void);
void msm_is_sec_copy_err_cnt_to_file(void);

int msm_is_sec_file_exist(char *filename, hw_params_check_type chktype);
int msm_is_sec_get_secure_mode(uint32_t **sec_is_secure);
int msm_is_sec_get_sensor_position(uint32_t **sensor_position);
int msm_is_sec_get_sensor_comp_mode(uint32_t **sensor_comp_mode);
int msm_is_sec_get_rear_hw_param(struct cam_hw_param **hw_param);
int msm_is_sec_get_front_hw_param(struct cam_hw_param **hw_param);
int msm_is_sec_get_iris_hw_param(struct cam_hw_param **hw_param);
int msm_is_sec_get_rear2_hw_param(struct cam_hw_param **hw_param);
#endif

int msm_sensor_config(struct msm_sensor_ctrl_t *s_ctrl, void __user *argp);

int msm_sensor_power_up(struct msm_sensor_ctrl_t *s_ctrl);

int msm_sensor_power_down(struct msm_sensor_ctrl_t *s_ctrl);

int msm_sensor_match_id(struct msm_sensor_ctrl_t *s_ctrl);

#if 1 //defined(CONFIG_SAMSUNG_SECURE_CAMERA)
int msm_sensor_check_resolution(struct msm_sensor_ctrl_t *s_ctrl);
#endif

int msm_sensor_update_cfg(struct msm_sensor_ctrl_t *s_ctrl);

int msm_sensor_free_sensor_data(struct msm_sensor_ctrl_t *s_ctrl);

int32_t msm_sensor_init_default_params(struct msm_sensor_ctrl_t *s_ctrl);

int32_t msm_sensor_get_dt_gpio_req_tbl(struct device_node *of_node,
	struct msm_camera_gpio_conf *gconf, uint16_t *gpio_array,
	uint16_t gpio_array_size);

int32_t msm_sensor_get_dt_gpio_set_tbl(struct device_node *of_node,
	struct msm_camera_gpio_conf *gconf, uint16_t *gpio_array,
	uint16_t gpio_array_size);

int32_t msm_sensor_init_gpio_pin_tbl(struct device_node *of_node,
	struct msm_camera_gpio_conf *gconf, uint16_t *gpio_array,
	uint16_t gpio_array_size);
#ifdef CONFIG_COMPAT
long msm_sensor_subdev_fops_ioctl(struct file *file,
	unsigned int cmd,
	unsigned long arg);
#endif
#if defined(CONFIG_CAMERA_IRIS)
extern void ir_led_on(int i);
extern void ir_led_off(void);
#endif
#endif
