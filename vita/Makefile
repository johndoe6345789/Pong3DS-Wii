# PS Vita build. Needs VitaSDK (https://vitasdk.org) on PATH.
#
# COMPILES, not yet run on hardware. Written without a VitaSDK to hand and first
# built in CI, where every source file compiled and only the link failed -- for
# a missing stub library, since corrected. Whether it actually draws anything on
# a Vita is still unknown.

TARGET   := pong-vita
TITLE    := PONG3DSV1
ROOT     := ..

CC       := arm-vita-eabi-gcc
CFLAGS   := -std=c99 -Wall -Wextra -O2 -fno-lto \
            -I$(ROOT)/src/gfx -I$(ROOT)/src/ui \
            -I$(ROOT)/3ds/source/game -I$(ROOT)/3ds/source/net
# SceSharedFb is what vita2d uses to share the framebuffer with the system,
# and leaving it out is the one thing that got past a clean compile: every
# source file built, and the link failed on sceSharedFbBegin/End/Close. Found
# by CI, which is exactly why the job exists.
LIBS     := -lvita2d -lSceDisplay_stub -lSceGxm_stub -lSceSysmodule_stub \
            -lSceCtrl_stub -lSceTouch_stub -lSceCommonDialog_stub \
            -lSceSharedFb_stub -lSceAppMgr_stub \
            -lSceNet_stub -lSceNetCtl_stub -lm

SRC := main_vita.c \
       $(ROOT)/src/gfx/vita/gfx_vita.c \
       $(ROOT)/src/ui/render.c \
       $(ROOT)/3ds/source/net/pong_proto.c \
       $(ROOT)/3ds/source/game/client.c

all: $(TARGET).vpk

$(TARGET).elf: $(SRC)
	$(CC) $(CFLAGS) -o $@ $(SRC) $(LIBS)

$(TARGET).velf: $(TARGET).elf
	arm-vita-eabi-strip -g $<
	vita-elf-create $< $@

eboot.bin: $(TARGET).velf
	vita-make-fself -s $< $@

param.sfo:
	vita-mksfoex -s TITLE_ID=$(TITLE) "Pong Multiplayer" $@

$(TARGET).vpk: eboot.bin param.sfo
	vita-pack-vpk -s param.sfo -b eboot.bin $@

clean:
	rm -f $(TARGET).elf $(TARGET).velf eboot.bin param.sfo $(TARGET).vpk

.PHONY: all clean
