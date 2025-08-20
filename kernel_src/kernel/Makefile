#-----------------------------------------------------------------------------
#       Directory Definition
#-----------------------------------------------------------------------------
TOP_DIR				:= $(shell pwd)
APPS_DIR			:= $(TOP_DIR)/../../../src
UTILS_DIR			:= $(APPS_DIR)/utils
USER_DIR			:= $(TOP_DIR)/../tftp
TFTP_DIR			:= /tftpboot/saturn7/$(USER)
KER_DIR				:= $(TOP_DIR)/linux-2.6.26-saturn7
ifeq ($(MAKECMDGOALS), flash)
OBJ_DIR				:= obj-flash
CONFIG_FILE			:= config-flash
TARGET_IMAG			:= uImage_flash
else
OBJ_DIR				:= obj-normal
CONFIG_FILE			:= config-normal
TARGET_IMAG			:= uImage
endif

#----------------------------------------------------------------------------
#	Tools Definition
#----------------------------------------------------------------------------
CROSS_TOOL			= mipsel-linux-
RM					= rm -f
LN					= ln -s
CP					= cp
CD					= cd
CAT					= cat
ECHO				= echo
RMDIR				= rm -rf
MKDIR				= mkdir -p
TOUCH				= touch
STRIP				= $(CROSS_TOOL)strip
MKEPAK				= $(TOP_DIR)/../tools/mkepk
PROD_GSYNC			= /prod/bin/prodgsync
IS_DIFFER			= $(UTILS_DIR)/is_differ.sh
TFTP_CHANGE_MARK	= ${TFTP_DIR}/.timestamp
TFTP_CHANGE_LIST	= ${TFTP_DIR}/changed_file_list.txt
CPIO_CMD			= LC_ALL=C cpio -pudm --quiet

#----------------------------------------------------------------------------
#	Variables
#---------------------------------------------------------------------------
BUILD_VERS			= $(shell grep -i "^[0-9]*\\.[0-9]*\\..*$i" $(TOP_DIR)/kernel_history.txt | head -1 | awk '{ print $$1 }' | tr -d ":")

normal flash: pre_build build_kernel post_build

pre_build:
	@if [ -x $(PROD_GSYNC) ]; then			\
		$(TOUCH) $(TFTP_CHANGE_MARK);		\
	fi

build_kernel:
	@$(CD) $(KER_DIR) && $(CP) $(CONFIG_FILE) $(OBJ_DIR)/.config
	@$(MAKE) -C $(KER_DIR) O=$(OBJ_DIR) $(TARGET_IMAG) BUILD_VERS="$(BUILD_VERS)"

post_build:
	@$(CP) -fv $(KER_DIR)/$(OBJ_DIR)/$(TARGET_IMAG) $(USER_DIR)
	@$(CD) $(KER_DIR)/$(OBJ_DIR) && $(IS_DIFFER) $(TARGET_IMAG) $(TFTP_DIR) $(TFTP_DIR) | $(CPIO_CMD) $(TFTP_DIR)
	@if [ -x $(PROD_GSYNC) -a -s $(TFTP_CHANGE_LIST) ]; then							\
		GZIP="--fast" $(PROD_GSYNC) $(TFTP_DIR) | tr ":" ".";							\
		$(RM) $(TFTP_CHANGE_MARK);														\
	fi

menuconfig_n:
	@$(MAKE) -C $(KER_DIR) O=obj-normal menuconfig
	@$(CP) -fv $(KER_DIR)/obj-normal/.config $(KER_DIR)/config-normal

menuconfig_f:
	@$(MAKE) -C $(KER_DIR) O=obj-flash menuconfig
	@$(CP) -fv $(KER_DIR)/obj-flash/.config $(KER_DIR)/config-flash

clean:
	@$(CD) $(KER_DIR) && mkclean.sh

help:
	@$(ECHO) ' '
	@$(ECHO) '*******************************************************'
	@$(ECHO) '  #  make normal'
	@$(ECHO) '  -> This command will build normal version kernel image'
	@$(ECHO) '-------------------------------------------------------'
	@$(ECHO) '  #  make flash'
	@$(ECHO) '  -> This command will build flash version kernel image'
	@$(ECHO) '-------------------------------------------------------'
	@$(ECHO) '  #  make memnuconfig_n'
	@$(ECHO) '  -> This command will config for normal version kernel'
	@$(ECHO) '-------------------------------------------------------'
	@$(ECHO) '  #  make memnuconfig_f'
	@$(ECHO) '  -> This command will config for flash version kernel'
	@$(ECHO) '-------------------------------------------------------'
	@$(ECHO) '  #  make clean'
	@$(ECHO) '  -> This command will clean all kernel including normal/flash'
	@$(ECHO) '-------------------------------------------------------'
	@$(ECHO) '  #  make help'
	@$(ECHO) '  -> This command will show this help messages.'
	@$(ECHO) '*******************************************************'
	@$(ECHO) ' '
