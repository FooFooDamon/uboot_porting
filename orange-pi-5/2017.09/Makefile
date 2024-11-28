PKG_FILE ?= ./u-boot-orangepi-0f7610abf7053e7b6cf9e7299aa1c0c1648efaa0.tar.gz
#PKG_URL ?= https://github.com/orangepi-xunlong/u-boot-orangepi/archive/refs/heads/v2017.09-rk3588.tar.gz
PKG_URL ?= https://github.com/orangepi-xunlong/u-boot-orangepi/archive/0f7610abf7053e7b6cf9e7299aa1c0c1648efaa0.tar.gz
EXT_TARGETS +=
CUSTOM_FILES += arch/arm/dts/rk3588s.dtsi \
    tools/rockchip/boot_merger.c \
    drivers/net/phy/Kconfig \
    drivers/net/phy/Makefile \
    include/phy.h \
    drivers/net/phy/phy.c \
    drivers/net/phy/motorcomm.c \
    drivers/power/regulator/rk806.c \
    include/configs/rk3588_common.h \
    scripts/Makefile.spl

include ../common.mk
include ../../wrapper.mk

