ARCH ?= arm
CROSS_COMPILE ?= aarch64-linux-gnu-
__OPI5_HOSTNAME ?= $(shell uname -n | grep orangepi5)
INSTALL_DIR ?= $(if ${__OPI5_HOSTNAME},$(shell ls -d /usr/lib/linux-u-boot-*),${HOME}/tftpd/orange-pi-5)
POST_INSTALL_CMD ?= ls ${INSTALL_DIR}/u-boot* | grep -v "u-boot\.itb" | xargs -I {} rm {}; \
    cp idbloader.img ${INSTALL_DIR}/; \
    $(if ${__OPI5_HOSTNAME},printf '\nNow you can run "sudo nand-sata-install" to burn the installed images.\n\n')
UNINSTALL_CMD ?= rm -f ${INSTALL_DIR}/u-boot.itb ${INSTALL_DIR}/idbloader.img
DEFCONFIG ?= configs/orangepi_5_defconfig
EXT_TARGETS += u-boot.itb u-boot.dtb tpl/u-boot-tpl.bin spl/u-boot-spl.bin
CUSTOM_FILES += arch/${ARCH}/dts/rk3588s-orangepi-5.dts
USER_HELP_PRINTS ?= echo "  * ${MAKE} once4all"; \
    echo "  * ${MAKE} idbloader.img"; \
    echo "  * ${MAKE} idbloader.img-clean"; \
    echo "  * ${MAKE} /boot/boot.scr"; \
    echo "  * ${MAKE} fix_clangd_db";

.PHONY: once4all u-boot.itb bl31.elf u-boot.dtb idbloader.img-clean clean install fix_clangd_db

once4all: u-boot.itb idbloader.img

u-boot.itb: bl31.elf u-boot.dtb

bl31.elf:
	bl31_dir=$$(${MAKE} showvars -s | grep SRC_ROOT_DIR | awk -F = '{ print $$2 }'); \
	[ ! -s $${bl31_dir}/$@ ] || exit 0; \
	sed -i '1s/\(python\)2/\1/' ${SRC_ROOT_DIR}/arch/arm/mach-rockchip/decode_bl31.py; \
	wget -O $${bl31_dir}/$@ 'https://github.com/armbian/rkbin/raw/master/rk35/rk3588_bl31_v1.45.elf'; \
	[ $$? -ne 0 ] || exit 0; \
	rm -f $${bl31_dir}/$@; \
	false

idbloader.img: tpl/u-boot-tpl.bin spl/u-boot-spl.bin
	${SRC_ROOT_DIR}/tools/mkimage -n rk3588s -T rksd -d ${SRC_ROOT_DIR}/$< $@
	cat ${SRC_ROOT_DIR}/$(word 2, $^) >> $@

idbloader.img-clean:
	rm -f idbloader.img

clean: idbloader.img-clean

ifneq (${__OPI5_HOSTNAME},)

install: /boot/boot.scr

endif

/boot/boot.scr: ../boot.cmd
	mkimage -C none -A arm -T script -d $< $@
	install $< $(dir $@)

fix_clangd_db:
	sed -i -e '/"-f/d' -e '/"-mabi=lp64"/d' ${SRC_ROOT_DIR}/compile_commands.json

