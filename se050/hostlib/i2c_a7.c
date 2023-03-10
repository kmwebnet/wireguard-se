// SPDX-License-Identifier: GPL v2
/*
 * Copyright (C) 2023 kmwebnet <kmwebnet@gmail.com>. All Rights Reserved.
 */

/**
 * kernel module HAL for NXP SE050
 *
 **/
#include "i2c_a7.h"

#include <linux/string.h>

#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <linux/version.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/mutex.h>

#include "nxLog_smCom.h"

static struct i2c_adapter *adapter;
static struct i2c_client *g_client;

struct mutex busmp;

i2c_error_t axI2CInit(void **conn_ctx, const char *pDevName)
{
    mutex_init(&busmp);
    int axSmDevice = 1;
    struct i2c_board_info se050_i2c_devices = 
	{
		I2C_BOARD_INFO("se050", 0x48),
	};

    AX_UNUSED_ARG (pDevName);

    LOG_D("I2CInit: opening");
    
	adapter = i2c_get_adapter(1);
	if (adapter == NULL)
    {
       LOG_E("opening failed...");
       return I2C_FAILED;
    }

	g_client = i2c_new_client_device(adapter, &se050_i2c_devices);
	if(g_client == NULL) 
    {
        LOG_E("Cannot get i2c adapter functionality\n");
        i2c_put_adapter(adapter);
        return I2C_FAILED;
    }

    *conn_ctx = kmalloc(sizeof(int), GFP_KERNEL);
    if(*conn_ctx == NULL)
    {
        LOG_E("I2C driver: Memory allocation failed!\n");
        i2c_unregister_device(g_client); 
        i2c_put_adapter(adapter);
        return I2C_FAILED;
    }
    else{
        *(int*)(*conn_ctx) = axSmDevice;
        return I2C_OK;
    }
}

void axI2CTerm(void* conn_ctx, int mode)
{
    AX_UNUSED_ARG(mode);
    if (conn_ctx != NULL) {
        i2c_unregister_device(g_client);
        kfree(conn_ctx);
        conn_ctx = NULL;
        i2c_put_adapter(adapter);
    }
    g_client = NULL;
    adapter = NULL;
    return;
}

#if defined T1oI2C
i2c_error_t axI2CWrite(void* conn_ctx, unsigned char bus, unsigned char addr, unsigned char * pTx, unsigned short txLen)
{
    int nrWritten = -1;
    i2c_error_t rv;
    AX_UNUSED_ARG(conn_ctx);

    if(g_client == NULL)
    {
        return I2C_FAILED;
    }

    if(pTx == NULL || txLen > MAX_DATA_LEN)
    {
        return I2C_FAILED;
    }

    if (bus != I2C_BUS_0)
    {
        LOG_E("axI2CWrite on wrong bus %x (addr %x)\n", bus, addr);
    }
    LOG_MAU8_D("TX (axI2CWrite) > ",pTx,txLen);
    nrWritten = i2c_master_send_dmasafe(g_client, pTx, txLen);
    if (nrWritten < 0)
    {
       LOG_E("Failed writing data (nrWritten=%d).\n", nrWritten);
       rv = I2C_FAILED;
    }
    else
    {
        if (nrWritten == txLen) // okay
        {
            rv = I2C_OK;
        }
        else
        {
            rv = I2C_FAILED;
        }
    }
    LOG_D("Done with rv = %02x ", rv);

    return rv;
}
#endif // defined T1oI2C 

#ifdef T1oI2C
i2c_error_t axI2CRead(void* conn_ctx, unsigned char bus, unsigned char addr, unsigned char * pRx, unsigned short rxLen)
{
    int nrRead = -1;
    i2c_error_t rv;
    AX_UNUSED_ARG(conn_ctx);

    if(g_client == NULL)
    {
        return I2C_FAILED;
    }

    if(pRx == NULL || rxLen > MAX_DATA_LEN)
    {
        return I2C_FAILED;
    }

    if (bus != I2C_BUS_0)
    {
        LOG_E("axI2CRead on wrong bus %x (addr %x)\n", bus, addr);
    }
   mutex_lock(&busmp);
   nrRead = i2c_master_recv_dmasafe(g_client, pRx, rxLen);
   if (nrRead < 0)
   {
      //LOG_E("Failed Read data (nrRead=%d).\n", nrRead);
      rv = I2C_FAILED;
   }
   else
   {
        if (nrRead == rxLen) // okay
        {
            rv = I2C_OK;
        }
        else
        {
            rv = I2C_FAILED;
        }
   }
    LOG_D("Done with rv = %02x ", rv);
    LOG_MAU8_D("TX (axI2CRead): ",pRx,rxLen);
    mutex_unlock(&busmp);
    return rv;
}
#endif // T1oI2C
