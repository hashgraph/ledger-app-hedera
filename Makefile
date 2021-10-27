#*******************************************************************************
#   Ledger App Hedera
#   (c) 2019 Hedera Hashgraph
#
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#*******************************************************************************

ifeq ($(BOLOS_SDK),)
$(error Environment variable BOLOS_SDK is not set)
endif
include $(BOLOS_SDK)/Makefile.defines

#########
#  App  #
#########
APP_LOAD_PARAMS= --curve ed25519 --path "44'/3030'" --appFlags 0x240 $(COMMON_LOAD_PARAMS)

APPVERSION_M = 1
APPVERSION_N = 0
APPVERSION_P = 5
APPVERSION = $(APPVERSION_M).$(APPVERSION_N).$(APPVERSION_P)
APPNAME = Hedera

COIN = HBAR

DEFINES += $(DEFINES_LIB)

ICONNAME = icons/nanos_app_hedera.gif
ifeq ($(TARGET_NAME),TARGET_NANOX)
	ICONNAME=icons/nanox_app_hedera.gif
else
	ICONNAME=icons/nanos_app_hedera.gif
endif


################
# Default rule #
################
# ifeq ($(TARGET_NAME), TARGET_NANOX)
# all: proto default
# else
all: default
#endif

############
# Platform #
############

DEFINES   += OS_IO_SEPROXYHAL
DEFINES   += HAVE_BAGL
DEFINES   += HAVE_IO_USB HAVE_L4_USBLIB IO_USB_MAX_ENDPOINTS=6 IO_HID_EP_LENGTH=64 HAVE_USB_APDU
DEFINES   += APPVERSION_M=$(APPVERSION_M) APPVERSION_N=$(APPVERSION_N) APPVERSION_P=$(APPVERSION_P)

# vendor/ledger-nanopb
DFEFINES  += PB_FIELD_32BIT=1

# vendor/printf
# ifneq ($(TARGET_NAME),TARGET_NANOX)
DEFINES   += PRINTF_DISABLE_SUPPORT_FLOAT PRINTF_DISABLE_SUPPORT_EXPONENTIAL PRINTF_DISABLE_SUPPORT_PTRDIFF_T
DEFINES   += PRINTF_NTOA_BUFFER_SIZE=9U PRINTF_FTOA_BUFFER_SIZE=0
# endif

# U2F
DEFINES   += HAVE_U2F HAVE_IO_U2F
DEFINES   += U2F_PROXY_MAGIC=\"BOIL\"
DEFINES   += USB_SEGMENT_SIZE=64
DEFINES   += BLE_SEGMENT_SIZE=32 #max MTU, min 20

WEBUSB_URL     = www.ledgerwallet.com
DEFINES       += HAVE_WEBUSB WEBUSB_URL_SIZE_B=$(shell echo -n $(WEBUSB_URL) | wc -c) WEBUSB_URL=$(shell echo -n $(WEBUSB_URL) | sed -e "s/./\\\'\0\\\',/g")

DEFINES   += UNUSED\(x\)=\(void\)x
DEFINES   += APPVERSION=\"$(APPVERSION)\"


ifeq ($(TARGET_NAME),TARGET_NANOX)
# Instead of vendor printf
DEFINES 	  += HAVE_SPRINTF

DEFINES       += IO_SEPROXYHAL_BUFFER_SIZE_B=300
DEFINES       += HAVE_BLE BLE_COMMAND_TIMEOUT_MS=2000
DEFINES       += HAVE_BLE_APDU # basic ledger apdu transport over BLE

DEFINES       += HAVE_GLO096
DEFINES       += HAVE_BAGL BAGL_WIDTH=128 BAGL_HEIGHT=64
DEFINES       += HAVE_BAGL_ELLIPSIS # long label truncation feature
DEFINES       += HAVE_BAGL_FONT_OPEN_SANS_REGULAR_11PX
DEFINES       += HAVE_BAGL_FONT_OPEN_SANS_EXTRABOLD_11PX
DEFINES       += HAVE_BAGL_FONT_OPEN_SANS_LIGHT_16PX
DEFINES		  += HAVE_UX_FLOW
else
DEFINES   	  += IO_SEPROXYHAL_BUFFER_SIZE_B=128
endif

# Enabling debug PRINTF
DEBUG = 1
ifneq ($(DEBUG),0)
        ifeq ($(TARGET_NAME),TARGET_NANOX)
                DEFINES   += HAVE_PRINTF PRINTF=mcu_usb_printf
        else
                DEFINES   += HAVE_PRINTF PRINTF=screen_printf
        endif
else
        DEFINES   += PRINTF\(...\)=
endif

##############
#  Compiler  #
##############
ifneq ($(BOLOS_ENV),)
CLANGPATH := $(BOLOS_ENV)/clang-arm-fropi/bin/
GCCPATH := $(BOLOS_ENV)/gcc-arm-none-eabi-5_3-2016q1/bin/
endif

CC       := $(CLANGPATH)clang

CFLAGS   += -Og -Iproto

# nanopb
CFLAGS   += -I. 

# printf
CFLAGS   += -Ivendor/printf/

# enable color from inside a script
CFLAGS   += -fcolor-diagnostics

AS     := $(GCCPATH)arm-none-eabi-gcc

LD       := $(GCCPATH)arm-none-eabi-gcc

LDFLAGS  += -Og -flto=thin
LDLIBS   += -lm -lgcc -lc

# import rules to compile glyphs(/pone)
include $(BOLOS_SDK)/Makefile.glyphs

### variables processed by the common makefile.rules of the SDK to grab source files and include dirs
APP_SOURCE_PATH  += src proto
SDK_SOURCE_PATH  += lib_stusb lib_stusb_impl lib_u2f

ifeq ($(TARGET_NAME),TARGET_NANOX)
SDK_SOURCE_PATH  += lib_blewbxx lib_blewbxx_impl
SDK_SOURCE_PATH  += lib_ux
endif

include vendor/nanopb/extra/nanopb.mk

DEFINES   += PB_NO_ERRMSG=1
SOURCE_FILES += $(NANOPB_CORE)
CFLAGS += "-I$(NANOPB_DIR)"

# Build rule for proto files
SOURCE_FILES += proto/BasicTypes.pb.c 
SOURCE_FILES += proto/CryptoCreateTransactionBody.pb.c 
SOURCE_FILES += proto/CryptoTransferTransactionBody.pb.c 
SOURCE_FILES += proto/TransactionBody.pb.c 

proto/BasicTypes.pb.c: proto/BasicTypes.proto
	$(PROTOC) $(PROTOC_OPTS) --nanopb_out=. proto/BasicTypes.proto

proto/CryptoCreateTransactionBody.pb.c: proto/BasicTypes.proto
	$(PROTOC) $(PROTOC_OPTS) --nanopb_out=. proto/CryptoCreateTransactionBody.proto

proto/CryptoTransferTransactionBody.pb.c: proto/BasicTypes.proto
	$(PROTOC) $(PROTOC_OPTS) --nanopb_out=. proto/CryptoTransferTransactionBody.proto

proto/TransactionBody.pb.c: proto/BasicTypes.proto
	$(PROTOC) $(PROTOC_OPTS) --nanopb_out=. proto/TransactionBody.proto

# target to also clean generated proto c files
.SILENT : cleanall
cleanall : clean
	-@rm -rf proto/*.pb.c proto/*.pb.h

load: all
	python -m ledgerblue.loadApp $(APP_LOAD_PARAMS)

load-offline: all
	python -m ledgerblue.loadApp $(APP_LOAD_PARAMS) --offline

delete:
	python -m ledgerblue.deleteApp $(COMMON_DELETE_PARAMS)

# import generic rules from the sdk
include $(BOLOS_SDK)/Makefile.rules

#add dependency on custom makefile filename
dep/%.d: %.c Makefile

listvariants:
	@echo VARIANTS COIN hedera

check:
	@ clang-tidy \
		$(foreach path, $(APP_SOURCE_PATH), $(shell find $(path) -name "*.c" -and -not -name "pb*" -and -not -name "glyphs*")) -- \
		$(CFLAGS) \
		$(addprefix -D, $(DEFINES)) \
		$(addprefix -I, $(INCLUDES_PATH))

