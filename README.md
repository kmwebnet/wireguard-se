# [wireguard-se](https://www.wireguard.com/)  

Private key hardened wireguard implementation by secure element.
This replaces wireguard kernel module that comes with original kernel.

**More information may be found at [WireGuard.com](https://www.wireguard.com/).**

this derivation enables secret key transport protection by secure element.

wireguard-se communicates by using ECDH calculation with secret key inside NXP SE050 secure element.

This utilizes Platform SCP03 encryption between Raspberry Pi and NXP SE050,  
note that the AES keys that protect I2C transfer are NXP factory default value.
Please consider rotate SCP03 keys.

## Environment

Raspberry Pi 3B+ or derivatives

NXP SE050E, the variants that can handle Curve25519. [See NXP datasheet.](https://www.nxp.jp/docs/en/application-note/AN12436.pdf)
This product comes with SE050E2 variant that can handle Curve25519.

The variant should keep noted that corresponds for the use of key selection on config.

Raspberry Pi OS 6.1.21-v7+

## Prepare

git clone --recursive 
Enable I2C on sudo raspi-config

make sure I2C communication as follows:

```
$ i2cdetect -y 1
     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f
00:                         -- -- -- -- -- -- -- --
10: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
20: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
30: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
40: -- -- -- -- -- -- -- -- 48 -- -- -- -- -- -- --
50: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
60: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
70: -- -- -- -- -- -- -- --
```


select the SE050E" PlatformSCP03 key variants on the code as follows in se050/inc/fsl_sss_ftr.h

```
/* Enable one of these
 * If none is selected, default config would be used
 */
#define SSS_PFSCP_ENABLE_SE050A1 0
#define SSS_PFSCP_ENABLE_SE050A2 0
#define SSS_PFSCP_ENABLE_SE050B1 0
#define SSS_PFSCP_ENABLE_SE050B2 0
#define SSS_PFSCP_ENABLE_SE050C1 0
#define SSS_PFSCP_ENABLE_SE050C2 0
#define SSS_PFSCP_ENABLE_SE050_DEVKIT 0
#define SSS_PFSCP_ENABLE_SE051A2 0
#define SSS_PFSCP_ENABLE_SE051C2 0
#define SSS_PFSCP_ENABLE_SE050F2 0
#define SSS_PFSCP_ENABLE_SE051C_0005A8FA 0
#define SSS_PFSCP_ENABLE_SE051A_0001A920 0
#define SSS_PFSCP_ENABLE_SE050E_0001A921 1
#define SSS_PFSCP_ENABLE_A5000_0004A736 0
#define SSS_PFSCP_ENABLE_SE050F2_0001A92A 0
#define SSS_PFSCP_ENABLE_OTHER 0
```

## Building

    $ sudo apt install raspberrypi-kernel-headers
    $ make

There are no dependencies other than a good C compiler and a sane libc.

## Installing

    # sudo make install
    # sudo depmod -a
    # reboot

## Using

This needs prior to prepare private key inside a secure element by using [wireguard-tools-se](https://github.com/kmwebnet/wireguard-tools-se).

## License

If a license is stated in the source code, that license is applied, otherwise the GPL v2 license is applied, see LICENSE.
