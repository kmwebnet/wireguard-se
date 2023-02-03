// SPDX-License-Identifier: GPL v2
/*
 * Copyright (C) 2023 kmwebnet <kmwebnet@gmail.com>. All Rights Reserved.
 */

#ifndef SE_HELPER_H
#define SE_HELPER_H


#include <linux/string.h>

#include "fsl_sss_ftr.h"

#include <ex_sss.h>
#include <ex_sss_boot.h>
#include <nxLog_App.h>
#include <se05x_APDU.h>
#include "fsl_sss_api.h"
#include "ex_sss_objid.h"

#define OBJID_device_key (EX_SSS_OBJID_CUST_START + 0x10000001u)

int ecdh_calc(uint8_t * shared_secret, uint8_t * peer_pubkey,uint8_t * keyID);

int gen_se_key(uint8_t * keyID);

int get_se_key(uint8_t * key, uint8_t * keyID);

int get_se_rand(uint8_t * random, size_t rand_len);

#endif