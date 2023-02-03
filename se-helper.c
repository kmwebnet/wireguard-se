// SPDX-License-Identifier: GPL v2
/*
 * Copyright (C) 2023 kmwebnet <kmwebnet@gmail.com>. All Rights Reserved.
 */

#include <linux/mutex.h>
#include <linux/string.h>
#include "se-helper.h"

#define I2C_DEFAULT_PORT "/dev/i2c-1"

#define ECDH_KEY_BYTE_LENGTH 32

/* 
reverse endianness
*/

static int reverse_key_endianness(uint8_t* result, uint8_t* source)
{

    if (sizeof(result) == ECDH_KEY_BYTE_LENGTH || sizeof (source) == ECDH_KEY_BYTE_LENGTH ) goto error;

    for (int i = 0; i < 32; i++) {
        result[i] = source[31 -i];
        }
return 0;

error:
return -1;
};        

int ecdh_calc(uint8_t * shared_secret, uint8_t * peer_pubkey,uint8_t * keyID)
{
    //prepare LOCK
    struct mutex mp;
    mutex_init(&mp);

    //do LOCK
    mutex_lock(&mp);

    uint32_t key = 0;
	memcpy (&key, keyID, 4);

	sss_status_t status = kStatus_SSS_Success;

    static ex_sss_boot_ctx_t pCtx;
    status = ex_sss_boot_open(&pCtx, I2C_DEFAULT_PORT);
    if (status != kStatus_SSS_Success) {
        LOG_E("ex_sss_boot_open failed");
        goto failure;
    };    

    status = ex_sss_key_store_and_object_init(&pCtx);
	if (status != kStatus_SSS_Success)
	{
        LOG_E("ex_sss_key_store_and_object_init failed");
        goto failurelv1;
    };    

    sss_se05x_session_t *pSession = (sss_se05x_session_t *)&pCtx.session;

    smStatus_t sw_status;
    SE05x_Result_t result = kSE05x_Result_NA;

    sw_status = Se05x_API_CheckObjectExists(
        &pSession->s_ctx, key, &result);
    if (SM_OK != sw_status) {
        LOG_E("Failed Se05x_API_CheckObjectExists");
        goto failurelv1;
    }
    if (result != kSE05x_Result_SUCCESS) {
        printk("private key does not exist. generate key first.\n");
        goto failurelv1;
    }


	/* ==============  DERIVE ECDH KEY  ===================================================== */

    uint8_t tempkey [ECDH_KEY_BYTE_LENGTH] = {0};
    uint8_t tempkey2[ECDH_KEY_BYTE_LENGTH] = {0};
    size_t tempkey2len = ECDH_KEY_BYTE_LENGTH;

    if (reverse_key_endianness(tempkey, peer_pubkey)) goto failurelv1;
    
	sw_status = Se05x_API_ECDHGenerateSharedSecret(
		&pSession->s_ctx,					//pSe05xSession_t session_ctx,	//session_ctx Session Context [0:kSE05x_pSession]
		key,					        //uint32_t objectID,			//objectID objectID [1:kSE05x_TAG_1]
		tempkey,			            //const uint8_t *pubKey,		//pubKey pubKey [2:kSE05x_TAG_2]
		sizeof(tempkey),	            //size_t pubKeyLen,				//pubKeyLen Length of pubKey
		tempkey2,						//uint8_t *sharedSecret,		//sharedSecret  [0:kSE05x_TAG_1]
		&tempkey2len						//size_t *psharedSecretLen		//psharedSecretLen Length for sharedSecret
	);
    if (sw_status != SM_OK)    
	{
        LOG_E("Se05x_API_ECDHGenerateSharedSecret failed");
        goto failurelv1;
    }; 

    if (reverse_key_endianness(shared_secret, tempkey2)) goto failurelv1;

// cleanup
    memzero_explicit(tempkey, ECDH_KEY_BYTE_LENGTH);
    memzero_explicit(tempkey2, ECDH_KEY_BYTE_LENGTH);    
    ex_sss_session_close(&pCtx);
    mutex_unlock(&mp);
	return 0;

failurelv1:
    ex_sss_session_close(&pCtx);

failure:
    mutex_unlock(&mp);
    return -1;
}

int gen_se_key(uint8_t * keyID)
{

    //prepare LOCK

    struct mutex mp;
    mutex_init(&mp);

    //do LOCK
    mutex_lock(&mp);

    uint32_t key = 0;
	memcpy (&key, keyID, 4);

	sss_status_t status = kStatus_SSS_Success;

    static ex_sss_boot_ctx_t pCtx;
    status = ex_sss_boot_open(&pCtx, I2C_DEFAULT_PORT);
    if (status != kStatus_SSS_Success) {
        LOG_E("ex_sss_boot_open failed");
        goto failure;
    };    

    status = ex_sss_key_store_and_object_init(&pCtx);
	if (status != kStatus_SSS_Success)
	{
 
        LOG_E("ex_sss_key_store_and_object_init failed");
        goto failure;
    }    

    sss_object_t keypair;

	status = sss_key_store_context_init(&pCtx.ks, &pCtx.session);
	if (status != kStatus_SSS_Success)
	{
        ex_sss_session_close(&pCtx);
        LOG_E("sss_key_store_context_init failed");
        goto failure;
    }

    status = sss_key_store_allocate(&pCtx.ks, key);
    if (status != kStatus_SSS_Success)
	{
        sss_key_store_context_free(&pCtx.ks);
        ex_sss_session_close(&pCtx);
        LOG_E("sss_key_store_allocate failed");
        goto failure;
    }

	status = sss_key_object_init(&keypair, &pCtx.ks);
	if (status != kStatus_SSS_Success)
	{
        sss_key_store_context_free(&pCtx.ks);
        ex_sss_session_close(&pCtx);
        LOG_E("sss_key_object_init failed");
        goto failure;
    }

	status = sss_key_object_allocate_handle(
		&keypair,
		key,
		kSSS_KeyPart_Pair,
		kSSS_CipherType_EC_MONTGOMERY,
		256 ,
		kKeyObject_Mode_Persistent
	);
	if (status != kStatus_SSS_Success)
	{
        sss_key_object_free(&keypair);
        sss_key_store_context_free(&pCtx.ks);
        ex_sss_session_close(&pCtx);
        LOG_E("sss_key_object_allocate_handle failed");
        goto failure;
    }

    status = sss_key_store_generate_key(&pCtx.ks, &keypair, 256, 0);
	if (status != kStatus_SSS_Success)
	{
        sss_key_object_free(&keypair);
        sss_key_store_context_free(&pCtx.ks);
        ex_sss_session_close(&pCtx);
        LOG_E("sss_key_store_generate_key failed");
        goto failure;
    }

    status = sss_key_store_save(&pCtx.ks);
	if (status != kStatus_SSS_Success)
	{
        sss_key_object_free(&keypair);
        sss_key_store_context_free(&pCtx.ks);
        ex_sss_session_close(&pCtx);
        LOG_E("sss_key_store_save failed");
        goto failure;
    }	
    sss_key_object_free(&keypair);
    sss_key_store_context_free(&pCtx.ks);
    ex_sss_session_close(&pCtx);

// cleanup
    mutex_unlock(&mp);
	return 0;

failure:
    mutex_unlock(&mp);
    return -1;
};

int get_se_key(uint8_t * outkey, uint8_t * keyID)
{

    //prepare LOCK
    struct mutex mp;
    mutex_init(&mp);

    //do LOCK
    mutex_lock(&mp);

    uint32_t key = 0;
	memcpy (&key, keyID, 4);

	sss_status_t status = kStatus_SSS_Success;

    static ex_sss_boot_ctx_t pCtx;
    status = ex_sss_boot_open(&pCtx, I2C_DEFAULT_PORT);
    if (status != kStatus_SSS_Success) {
        LOG_E("ex_sss_boot_open failed");
        goto failure;
    };    

    status = ex_sss_key_store_and_object_init(&pCtx);
	if (status != kStatus_SSS_Success)
	{
 
        LOG_E("ex_sss_key_store_and_object_init failed");
        goto failure;
    }    

    sss_se05x_session_t *pSession = (sss_se05x_session_t *)&pCtx.session;

    smStatus_t sw_status;
    SE05x_Result_t result = kSE05x_Result_NA;

    sw_status = Se05x_API_CheckObjectExists(
        &pSession->s_ctx, key, &result);
    if (SM_OK != sw_status) {
        LOG_E("Failed Se05x_API_CheckObjectExists");
        goto failure;
    }
    if (result != kSE05x_Result_SUCCESS) {
        printk("private key does not exist. generate key first.\n");
        goto failure;
    }

    sss_object_t keypair;

	status = sss_key_store_context_init(&pCtx.ks, &pCtx.session);
	if (status != kStatus_SSS_Success)
	{
        ex_sss_session_close(&pCtx);
        LOG_E("sss_key_store_context_init failed");
        goto failure;
    }

    status = sss_key_store_allocate(&pCtx.ks, key);
    if (status != kStatus_SSS_Success)
	{
        sss_key_store_context_free(&pCtx.ks);
        ex_sss_session_close(&pCtx);
        LOG_E("sss_key_store_allocate failed");
        goto failure;
    }

	status = sss_key_object_init(&keypair, &pCtx.ks);
	if (status != kStatus_SSS_Success)
	{
        sss_key_store_context_free(&pCtx.ks);
        ex_sss_session_close(&pCtx);
        LOG_E("sss_key_object_init failed");
        goto failure;
    }

	status = sss_key_object_get_handle(
		&keypair,
		key
	);
	if (status != kStatus_SSS_Success)
	{
        sss_key_object_free(&keypair);
        sss_key_store_context_free(&pCtx.ks);
        ex_sss_session_close(&pCtx);
        LOG_E("sss_key_object_get_handle failed");
        goto failure;
    }

    uint8_t           derbuf[64];
    size_t            dersz = sizeof(derbuf);
    size_t            derszbits = dersz * 8;

    status = sss_key_store_get_key(&pCtx.ks, &keypair, derbuf, &dersz, &derszbits);
	if (status != kStatus_SSS_Success)
	{
        sss_key_object_free(&keypair);
        sss_key_store_context_free(&pCtx.ks);
        ex_sss_session_close(&pCtx);
        LOG_E("sss_key_store_get_key failed");
        goto failure;
    }	

    sss_key_object_free(&keypair);
    sss_key_store_context_free(&pCtx.ks);
    ex_sss_session_close(&pCtx);
    memcpy(outkey, &derbuf[12], 32);
    memzero_explicit(derbuf, dersz);

// cleanup
    mutex_unlock(&mp);
	return 0;

failure:
    mutex_unlock(&mp);
    return -1;
};

int get_se_rand(uint8_t * random, size_t rand_len)
{

    //prepare LOCK
    struct mutex mp;
    mutex_init(&mp);

    //do LOCK
    mutex_lock(&mp);

	sss_status_t status = kStatus_SSS_Success;

    static ex_sss_boot_ctx_t pCtx;
    status = ex_sss_boot_open(&pCtx, I2C_DEFAULT_PORT);
    if (status != kStatus_SSS_Success) {
        LOG_E("ex_sss_boot_open failed");
        goto failure;
    };    

	sss_rng_context_t rng;

	status = sss_rng_context_init(&rng, &pCtx.session);
    if (status != kStatus_SSS_Success)
	{
        ex_sss_session_close(&pCtx);
        LOG_E("sss_rng_context_init failed");
        goto failure;
    }	

	status = sss_rng_get_random(&rng, random, rand_len);
	sss_rng_context_free(&rng);

	if (status != kStatus_SSS_Success)
	{
        ex_sss_session_close(&pCtx);
        goto failure;
    }	
    ex_sss_session_close(&pCtx);

// cleanup
    mutex_unlock(&mp);
	return 0;

failure:
    mutex_unlock(&mp);
    return -1;
};