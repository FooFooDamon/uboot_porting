ARCH ?= arm
CROSS_COMPILE ?= aarch64-linux-gnu-
INSTALL_DIR ?= $(if $(shell uname -m | grep aarch64),$(shell ls -d /usr/lib/linux-u-boot-*),${HOME}/tftpd/orange-pi-5)
POST_INSTALL_CMD ?= ls ${INSTALL_DIR}/u-boot* | grep -v "u-boot\.itb" | xargs -I {} rm {}
DEFCONFIG ?= configs/orangepi_5_defconfig
EXT_TARGETS += u-boot.itb u-boot.dtb
CUSTOM_FILES += arch/${ARCH}/dts/rk3588s-orangepi-5.dts
USER_HELP_PRINTS ?= echo "  * ${MAKE} bootscript"; \
    echo "  * ${MAKE} bootscript_install"; \
    echo "  * ${MAKE} fix_clangd_db";

.PHONY: u-boot.itb bl31.elf u-boot.dtb install bootscript bootscript_install fix_clangd_db

u-boot.itb: bl31.elf u-boot.dtb

bl31.elf:
	bl31_dir=$$(${MAKE} showvars -s | grep SRC_ROOT_DIR | awk -F = '{ print $$2 }'); \
	[ ! -s $${bl31_dir}/$@ ] || exit 0; \
	sed -i '1s/\(python\)2/\1/' ${SRC_ROOT_DIR}/arch/arm/mach-rockchip/decode_bl31.py; \
	wget -O $${bl31_dir}/$@ 'https://github.com/armbian/rkbin/raw/master/rk35/rk3588_bl31_v1.45.elf'; \
	[ $$? -ne 0 ] || exit 0; \
	rm -f $${bl31_dir}/$@; \
	false

ifeq ($(shell uname -m),aarch64)

install: bootscript_install

endif

bootscript_install: bootscript
	install ../boot.cmd ../boot.scr /boot/

bootscript: ../boot.scr

../boot.scr: ../boot.cmd
	mkimage -C none -A arm -T script -d $< $@

fix_clangd_db:
	sed -i -e '/"-f/d' -e '/"-mabi=lp64"/d' ${SRC_ROOT_DIR}/compile_commands.json

