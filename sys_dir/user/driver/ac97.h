/*
 * Emuxki BeOS Driver for Creative Labs SBLive!/Audigy series
 *
 * Copyright (c) 2002, Jerome Duval (jerome.duval@free.fr)
 *
 * Original code : BeOS Driver for Intel ICH AC'97 Link interface
 * Copyright (c) 2002, Marcus Overhagen <marcus@overhagen.de>
 *
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice, 
 *   this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation 
 *   and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR 
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS 
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, 
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
#ifndef _AC97_H_
#define _AC97_H_
#include <ktype.h>
#include <stdlib.h>
#include <stdbool.h>
//#include "config.h"

enum AC97_REGISTER {
	AC97_RESET				= 0x00,
	AC97_MASTER_VOLUME		= 0x02,
	AC97_AUX_OUT_VOLUME		= 0x04,
	AC97_MONO_VOLUME		= 0x06,
	AC97_MASTER_TONE		= 0x08,
	AC97_PC_BEEP_VOLUME		= 0x0A,
	AC97_PHONE_VOLUME		= 0x0C,
	AC97_MIC_VOLUME			= 0x0E,
	AC97_LINE_IN_VOLUME		= 0x10,
	AC97_CD_VOLUME			= 0x12,
	AC97_VIDEO_VOLUME		= 0x14,
	AC97_AUX_IN_VOLUME		= 0x16,
	AC97_PCM_OUT_VOLUME		= 0x18,
	AC97_RECORD_SELECT		= 0x1A,
	AC97_RECORD_GAIN		= 0x1C,
	AC97_RECORD_GAIN_MIC	= 0x1E,
	AC97_GENERAL_PURPOSE	= 0x20,
	AC97_3D_CONTROL			= 0x22,
	AC97_PAGING				= 0x24,
	AC97_POWERDOWN			= 0x26,
	AC97_EXTENDED_AUDIO_ID 	= 0x28,
	AC97_EXTENDED_AUDIO_STATUS	= 0x2A,
	AC97_PCM_FRONT_DAC_RATE	= 0x2C,
	AC97_PCM_SURR_DAC_RATE	= 0x2E,
	AC97_PCM_LFE_DAC_RATE	= 0x30,
	AC97_PCM_LR_ADC_RATE	= 0x32,
	AC97_MIC_ADC_RATE		= 0x34,
	AC97_CENTER_LFE_VOLUME	= 0x36,
	AC97_SURROUND_VOLUME	= 0x38,
	AC97_SPDIF_CONTROL		= 0x3A,
	AC97_VENDOR_ID1			= 0x7C,
	AC97_VENDOR_ID2			= 0x7E
};
typedef int32_t area_id;
typedef unsigned short 		ushort;



const char *	ac97_get_3d_stereo_enhancement(device_config *config);
const char *	ac97_get_vendor_id_description(device_config *config);
uint32_t			ac97_get_vendor_id(device_config *config);
void			ac97_init(device_config *config);

void ac97_amp_enable(device_config *config, bool yesno);
typedef struct
{
	uint32_t	nabmbar;
	uint32_t	nambar;
	uint32_t	irq;
	uint32_t	type;
	uint32_t	mmbar; // ich4
	uint32_t	mbbar; // ich4
	void *	log_mmbar; // ich4
	void *	log_mbbar; // ich4
	area_id area_mmbar; // ich4
	area_id area_mbbar; // ich4

	ushort	subvendor_id;
	ushort	subsystem_id;

	ac97_dev *ac97;
} device_config;
typedef enum {
	B_MIX_GAIN = 1 << 0,
	B_MIX_MUTE = 1 << 1,
	B_MIX_MONO = 1 << 2,
	B_MIX_STEREO = 1 << 3,
	B_MIX_MUX = 1 << 4,
	B_MIX_MICBOOST = 1 << 5,
	B_MIX_RECORDMUX = 1 << 6
} ac97_mixer_type;

typedef struct _ac97_source_info {
	const char *name;
	ac97_mixer_type  type;
	
	int32_t	id;
	uint8_t	reg;
	uint16_t	default_value;
	uint8_t 	bits:3;
	uint8_t	ofs:4;
	uint8_t	mute:1;
	uint8_t	polarity:1; // max_gain -> 0
	float	min_gain;
	float	max_gain;
	float	granularity;
} ac97_source_info;


typedef struct ac97_dev ac97_dev;
typedef void	(* codec_init)(ac97_dev * dev);
typedef	uint16_t	(* codec_reg_read)(void * cookie, uint8_t reg);
typedef	void	(* codec_reg_write)(void * cookie, uint8_t reg, uint16_t value);
typedef bool	(* codec_set_rate)(ac97_dev *dev, uint8_t reg, uint32_t rate);
typedef bool	(* codec_get_rate)(ac97_dev *dev, uint8_t reg, uint32_t *rate);

struct ac97_dev {
	uint16_t			reg_cache[0x7f];

	void *				cookie;

	uint32_t				codec_id;
	const char *		codec_info;
	const char *		codec_3d_stereo_enhancement;

	codec_init			init;
	codec_reg_read		reg_read;
	codec_reg_write		reg_write;
	codec_set_rate		set_rate;
	codec_get_rate		get_rate;

	uint32_t				max_vsr;
	uint32_t				min_vsr;
	uint32_t 				clock;
	uint64_t				capabilities;
	bool				reversed_eamp_polarity;
	uint32_t				subsystem;
};
typedef struct
{
	uint32_t	nabmbar;
	uint32_t	nambar;
	uint32_t	irq;
	uint32_t	type;
	uint32_t	mmbar; // ich4
	uint32_t	mbbar; // ich4
	void *	log_mmbar; // ich4
	void *	log_mbbar; // ich4
	area_id area_mmbar; // ich4
	area_id area_mbbar; // ich4

	ushort	subvendor_id;
	ushort	subsystem_id;

	ac97_dev *ac97;
} device_config;
extern const ac97_source_info source_info[];
extern const int32_t source_info_size;

#endif