#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------

ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM")
endif

TOPDIR ?= $(CURDIR)
include $(DEVKITARM)/3ds_rules

TARGET      := CitadelDefense3D
BUILD       := build
SOURCES     := source
DATA        := data
INCLUDES    := include
ROMFS       := romfs

export APP_TITLE       := Citadel Defense 3D
export APP_DESCRIPTION := 3D tower defense for Nintendo 3DS
export APP_AUTHOR      := grzegorz2047

ARCH        := -march=armv6k -mtune=mpcore -mfloat-abi=hard -mtp=soft
CFLAGS      := -g -Wall -Wextra -O2 -mword-relocations -ffunction-sections $(ARCH)
CFLAGS      += $(INCLUDE) -D__3DS__
CXXFLAGS    := $(CFLAGS) -fno-rtti -fno-exceptions -std=gnu++17
ASFLAGS     := -g $(ARCH)
LDFLAGS     := -specs=3dsx.specs -g $(ARCH) -Wl,-Map,$(notdir $*.map)
LIBS        := -lcitro2d -lcitro3d -lctru -lm
LIBDIRS     := $(CTRULIB)

ifneq ($(BUILD),$(notdir $(CURDIR)))

export OUTPUT  := $(CURDIR)/$(TARGET)
export TOPDIR  := $(CURDIR)
export VPATH   := $(foreach dir,$(SOURCES),$(CURDIR)/$(dir)) \
                  $(foreach dir,$(DATA),$(CURDIR)/$(dir))
export DEPSDIR := $(CURDIR)/$(BUILD)

CFILES      := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES    := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
SFILES      := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))
PICAFILES   := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.v.pica)))
BINFILES    := $(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.*)))

export LD := $(CXX)
export OFILES_SOURCES := $(CPPFILES:.cpp=.o) $(CFILES:.c=.o) $(SFILES:.s=.o)
export OFILES_BIN := $(addsuffix .o,$(BINFILES)) $(PICAFILES:.v.pica=.shbin.o)
export OFILES := $(OFILES_BIN) $(OFILES_SOURCES)
export HFILES := $(PICAFILES:.v.pica=_shbin.h) $(addsuffix .h,$(subst .,_,$(BINFILES)))
export INCLUDE := $(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
                  $(foreach dir,$(LIBDIRS),-I$(dir)/include) \
                  -I$(CURDIR)/$(BUILD)
export LIBPATHS := $(foreach dir,$(LIBDIRS),-L$(dir)/lib)
export _3DSXDEPS := $(if $(NO_SMDH),,$(OUTPUT).smdh)
export _3DSXFLAGS += --smdh=$(CURDIR)/$(TARGET).smdh --romfs=$(CURDIR)/$(ROMFS)

.PHONY: all clean cia checksums

all: $(BUILD)
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

$(BUILD):
	@mkdir -p $@

clean:
	@rm -fr $(BUILD) $(TARGET).3dsx $(TARGET).smdh $(TARGET).elf $(TARGET).map dist

cia: all
	@echo "CIA packaging requires makerom, bannertool and release metadata."
	@echo "See docs/BUILD_AND_RELEASE.md and issue #2."
	@exit 1

checksums: all
	@mkdir -p dist
	@cp $(TARGET).3dsx dist/
	@sha256sum dist/$(TARGET).3dsx > dist/SHA256SUMS.txt

else

$(OUTPUT).3dsx: $(OUTPUT).elf $(_3DSXDEPS)
$(OFILES_SOURCES): $(HFILES)
$(OUTPUT).elf: $(OFILES)

%.bin.o %_bin.h: %.bin
	@$(bin2o)

.PRECIOUS: %.shbin
%.shbin.o %_shbin.h: %.shbin
	@$(bin2o)

-include $(DEPSDIR)/*.d

endif
