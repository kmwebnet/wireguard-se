# SPDX-License-Identifier: GPL-2.0
#
# Copyright (C) kmwebnet <kmwebnet@gmail.com>. All Rights Reserved.

KERNELRELEASE ?= $(shell uname -r)
KERNELDIR ?= /lib/modules/$(KERNELRELEASE)/build
PREFIX ?= /usr
DESTDIR ?=
SRCDIR ?= $(PREFIX)/src
DEPMOD ?= depmod
DEPMODBASEDIR ?= /

PWD := $(shell pwd)

WIREGUARD_VERSION :="1.0.0" 

wireguard-y := main.o
wireguard-y += noise.o
wireguard-y += device.o
wireguard-y += peer.o
wireguard-y += timers.o
wireguard-y += queueing.o
wireguard-y += send.o
wireguard-y += receive.o
wireguard-y += socket.o
wireguard-y += peerlookup.o
wireguard-y += allowedips.o
wireguard-y += ratelimiter.o
wireguard-y += cookie.o
wireguard-y += netlink.o
wireguard-y += se-helper.o
wireguard-y += se050/sss/ex_sss_boot.o
wireguard-y += se050/sss/ex_sss_se05x.o
wireguard-y += se050/sss/ex_sss_se05x_auth.o
wireguard-y += se050/sss/ex_sss_scp03_auth.o
wireguard-y += se050/sss/fsl_sss_apis.o
wireguard-y += se050/sss/fsl_sss_se05x_mw.o
wireguard-y += se050/sss/fsl_sss_se05x_scp03.o
wireguard-y += se050/sss/fsl_sss_se05x_policy.o
wireguard-y += se050/sss/fsl_sss_se05x_apis.o
wireguard-y += se050/sss/fsl_sss_util_asn1_der.o
wireguard-y += se050/sss/fsl_sss_user_impl.o

wireguard-y += se050/crypto/aes_cmac_multistep.o
wireguard-y += se050/crypto/aes_cmac.o
wireguard-y += se050/crypto/aes.o

wireguard-y += se050/hostlib/i2c_a7.o
wireguard-y += se050/hostlib/phNxpEse_Api.o
wireguard-y += se050/hostlib/phNxpEsePal_i2c.o
wireguard-y += se050/hostlib/phNxpEseProto7816_3.o
wireguard-y += se050/hostlib/nxScp03_Com.o
wireguard-y += se050/hostlib/smCom.o
wireguard-y += se050/hostlib/smComT1oI2C.o

wireguard-y += se050/hostlib/global_platf.o
wireguard-y += se050/hostlib/sm_apdu.o
wireguard-y += se050/hostlib/sm_timer.o
wireguard-y += se050/hostlib/sm_connect.o

wireguard-y += se050/hostlib/se05x_ECC_curves.o
wireguard-y += se050/hostlib/se05x_mw.o
wireguard-y += se050/hostlib/se05x_tlv.o
wireguard-y += se050/hostlib/se05x_APDU.o

wireguard-y += se-helper.o

obj-m := wireguard.o

ccflags-y += -I$(src)/se050/inc
ccflags-y += -I$(src)/se050/sss

ccflags-y += -DSSS_USE_FTR_FILE
ccflags-y += -DSMCOM_T1oI2C
ccflags-y += -DT1oI2C
ccflags-y += -DT1oI2C_UM11225
ccflags-y += -DUSE_SE
ccflags-y += -DFLOW_SILENT 
ccflags-y += -Wno-declaration-after-statement
ccflags-y += -Wframe-larger-than=3072
ccflags-y += -std=gnu11

all: module
debug: module-debug

module:
	@$(MAKE) -C $(KERNELDIR) M=$(PWD) WIREGUARD_VERSION="$(WIREGUARD_VERSION)" modules

module-debug:
	@$(MAKE) -C $(KERNELDIR) M=$(PWD) V=1 CONFIG_WIREGUARD_DEBUG=y WIREGUARD_VERSION="$(WIREGUARD_VERSION)" modules

clean:
	@$(MAKE) -C $(KERNELDIR) M=$(PWD) clean

module-install:
	@$(MAKE) INSTALL_MOD_DIR=kernel/drivers/net/wireguard -C $(KERNELDIR) M=$(PWD) WIREGUARD_VERSION="$(WIREGUARD_VERSION)" modules_install
	$(DEPMOD) -b "$(DEPMODBASEDIR)" -a $(KERNELRELEASE)

install: module-install

.PHONY: all module module-debug module-install install clean
