ARCH ?= arm
CROSS_COMPILE ?= aarch64-linux-gnu-
INSTALL_DIR ?= ${HOME}/tftpd/orange-pi-5
DEFCONFIG ?= configs/orangepi_5_defconfig
EXT_TARGETS += dtbs
CUSTOM_FILES += arch/${ARCH}/dts/rk3588s-orangepi-5.dts

