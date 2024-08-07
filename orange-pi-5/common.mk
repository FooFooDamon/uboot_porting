ARCH ?= arm
CROSS_COMPILE ?= aarch64-linux-gnu-
INSTALL_DIR ?= ${HOME}/tftpd/orange-pi-5
DEFCONFIG ?= configs/orangepi_5_defconfig
EXT_TARGETS += dtbs
CUSTOM_FILES += arch/${ARCH}/dts/rk3588s-orangepi-5.dts
USER_HELP_PRINTS ?= echo "  * ${MAKE} bootscript"; \
    echo "  * ${MAKE} bootscript_install"; \
    echo "  * ${MAKE} fix_clangd_db";

.PHONY: bootscript bootscript_install

#install: bootscript_install

bootscript_install: bootscript
	install ../boot.cmd ../boot.scr /boot/

bootscript: ../boot.scr

../boot.scr: ../boot.cmd
	mkimage -C none -A arm -T script -d $< $@

fix_clangd_db:
	sed -i -e '/"-f/d' -e '/"-mabi=lp64"/d' ${SRC_ROOT_DIR}/compile_commands.json

