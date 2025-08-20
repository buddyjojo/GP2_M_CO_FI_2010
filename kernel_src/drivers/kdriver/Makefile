#
# Makefile for MStar device drivers.
#

# dhjung LGE
ifeq ($(KERNELVERSION),)

MAKE_TYPE			:= MODULE_STANDALONE
PLATFORM_CHIP		:= saturn7

TOP_DIR				:= $(shell pwd)
UTILS_DIR			:= $(TOP_DIR)/../../../../src/utils
KERNEL_DIR			:= $(TOP_DIR)/../../kernel/linux-2.6.26-$(PLATFORM_CHIP)

TFTP_DIR			:= /tftpboot/$(PLATFORM_CHIP)/$(USER)
OUT_DIR				:= /share/global_platform/$(PLATFORM_CHIP)/$(USER)

KDRV_DIR			:= $(OUT_DIR)/drivers
KDRV_BACK_DIR		:= $(OUT_DIR)/drivers_bak
CHANGE_FILE_LIST	:= $(OUT_DIR)/changed_file_list.txt
CHANGE_FILE_FLAG	:= $(OUT_DIR)/changed_file_flag.txt

IS_DIFFER			:= $(UTILS_DIR)/is_differ.sh
TRANS_IMAGES		:= $(UTILS_DIR)/trans_images.sh
PROD_SYNC			:= /prod/bin/prodsync
PROD_GSYNC			:= /prod/bin/prodgsync
CPIO_CMD		 	:= LC_ALL=C cpio -pudm --quiet

include $(TOP_DIR)/modules.mk

export TOP_DIR MAKE_TYPE

all : pre_build
	@if [ -x $(TRANS_IMAGES) ]; then $(TRANS_IMAGES) $(OUT_DIR); fi
	@if [ -x $(PROD_GSYNC) -a -s $(CHANGE_FILE_LIST) ]; then	\
		GZIP="--fast" $(PROD_GSYNC) $(OUT_DIR) | tr ":" ".";	\
	else														\
		touch $(CHANGE_FILE_FLAG);								\
	fi

pre_build :
	@$(RM) $(CHANGE_FILE_LIST) && touch $(CHANGE_FILE_LIST)
	@mkdir -p $(KERNEL_DIR)/obj-normal/drivers/mstar/h264/$(CONFIG_MSTAR_CHIP_NAME)
	@cp -a h264/$(CONFIG_MSTAR_CHIP_NAME)/libHVD_normal.lib $(KERNEL_DIR)/obj-normal/drivers/mstar/h264/$(CONFIG_MSTAR_CHIP_NAME)
	@mkdir -p $(KERNEL_DIR)/obj-normal/drivers/mstar/mvd/$(CONFIG_MSTAR_CHIP_NAME)
	@cp -a mvd/$(CONFIG_MSTAR_CHIP_NAME)/libMVD_normal.lib $(KERNEL_DIR)/obj-normal/drivers/mstar/mvd/$(CONFIG_MSTAR_CHIP_NAME)
	@$(MAKE) -C $(KERNEL_DIR) O=$(KERNEL_DIR)/obj-normal modules
	@echo '++ Copying kernel driver modules to $(KDRV_DIR)'
	@$(MAKE) -sC $(KERNEL_DIR) O=$(KERNEL_DIR)/obj-normal modules_install INSTALL_MOD_DIR=$(KDRV_BACK_DIR)
	@cd $(KDRV_BACK_DIR) && mipsel-linux-strip --strip-debug *
	@cd $(KDRV_BACK_DIR) && find * -exec $(IS_DIFFER) {} $(KDRV_DIR) $(OUT_DIR) \; | $(CPIO_CMD) $(KDRV_DIR)

clean :
	@$(MAKE) -C $(KERNEL_DIR) O=$(KERNEL_DIR)/obj-normal kdrv_clean

endif

obj-$(CONFIG_MSTAR_NAND)			+= nand/
obj-$(CONFIG_MSTAR_TSP)      	    += tsp/
obj-$(CONFIG_MSTAR_GE)       	    += ge/
obj-$(CONFIG_MSTAR_GOP)      	    += gop/
obj-$(CONFIG_MSTAR_MPOOL)    	    += mpool/
obj-$(CONFIG_MSTAR_IIC)      	    += iic/
obj-$(CONFIG_MSTAR_GPIO)     	    += gpio/
obj-$(CONFIG_MSTAR_EMAC)     	    += emac/
obj-$(CONFIG_MSTAR_PWM)      	    += pwm/
obj-$(CONFIG_MSTAR_SYSTEM)   	    += system/
obj-$(CONFIG_MSTAR_SCALER)   	    += scaler/
obj-$(CONFIG_MSTAR_MAD)      	    += mad/
obj-$(CONFIG_MSTAR_VD)       	    += vd/
obj-$(CONFIG_MSTAR_MVOP)     	    += mvop/
obj-$(CONFIG_MSTAR_MVD)      	    += mvd/
obj-$(CONFIG_MSTAR_H264)      	    += h264/
obj-$(CONFIG_MSTAR_IR)       	    += ir/
obj-$(CONFIG_MSTAR_MICOM)           += micom/
obj-$(CONFIG_MSTAR_CI)       	    += ci/
obj-$(CONFIG_MSTAR_TTX)             += ttx/
obj-$(CONFIG_MSTAR_VE)              += ve/
obj-$(CONFIG_MSTAR_CC)              += cc/
obj-$(CONFIG_MSTAR_JPEG)      	    += jpeg/
obj-$(CONFIG_MSTAR_MSMAILBOX)       += msmailbox/
obj-$(CONFIG_MSTAR_SPI)             += spi/
obj-$(CONFIG_MSTAR_STR)				+= str/
obj-$(CONFIG_MSTAR_M4VE)			+= m4ve/
obj-$(CONFIG_MSTAR_DIP)				+= dip/
obj-$(CONFIG_MSTAR_DEMOD)			+= demod/
obj-$(CONFIG_MSTAR_MPIF)			+= mpif/
obj-$(CONFIG_MSTAR_AESDMA)			+= aesdma/
obj-$(CONFIG_MSTAR_BDMA)			+= bdma/
obj-$(CONFIG_MSTAR_CEC)				+= cec/
obj-$(CONFIG_MSTAR_PWS)				+= pws/
obj-$(CONFIG_MSTAR_PROBE)			+= probe/
#obj-$(CONFIG_MSTAR_LOGO)			+= logo/
