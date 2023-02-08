### Environment constants

#ARCH			:= MT7620
#ARCH			:= MT7620MUL

ifeq ($(ARCH),MT7620MUL)
CROSSTOOLDIR                           := /home/au/all/gwork/tmp/tools/openwrt-sdk-ramips-mt76x8_gcc-7.3.0_musl.Linux-x86_64
CROSS                                                  := mipsel-openwrt-linux-
export  STAGING_DIR := $(CROSSTOOLDIR)/staging_dir
export  PATH                           :=$(PATH):$(STAGING_DIR)/toolchain-mipsel_24kc_gcc-7.3.0_musl/bin
CROSS_CFLAGS                           := -I$(CROSSTOOLDIR)/staging_dir/toolchain-mipsel_24kc_gcc-7.3.0_musl/usr/include
CROSS_CFLAGS                           += -I$(CROSSTOOLDIR)/staging_dir/target-mipsel_24kc_musl/usr/include
CROSS_LDFLAGS                  := -L$(CROSSTOOLDIR)/staging_dir/toolchain-mipsel_24kc_gcc-7.3.0_musl/usr/lib
CROSS_LDFLAGS                  += -L$(CROSSTOOLDIR)/staging_dir/target-mipsel_24kc_musl/usr/lib
endif

ifeq ($(ARCH),MT7620)
#CROSSTOOLDIR       	:= $(ROOTDIR)/../tools/OpenWrt-SDK-ramips-mt7688_gcc-4.8-linaro_uClibc-0.9.33.2.Linux-x86_64
CROSSTOOLDIR      	:= /home/au/all/gwork/openwrt/
CROSS   						:= mipsel-openwrt-linux-
export  STAGING_DIR	:= $(CROSSTOOLDIR)/staging_dir
export  PATH				:= $(PATH):$(STAGING_DIR)/toolchain-mipsel_24kec+dsp_gcc-4.8-linaro_uClibc-0.9.33.2/bin
CROSS_CFLAGS				:= -I$(CROSSTOOLDIR)/staging_dir/toolchain-mipsel_24kec+dsp_gcc-4.8-linaro_uClibc-0.9.33.2/usr/include
CROSS_CFLAGS				+= -I$(CROSSTOOLDIR)/staging_dir/target-mipsel_24kec+dsp_uClibc-0.9.33.2/usr/include
CROSS_LDFLAGS				:= -L$(CROSSTOOLDIR)/staging_dir/toolchain-mipsel_24kec+dsp_gcc-4.8-linaro_uClibc-0.9.33.2/usr/lib
CROSS_LDFLAGS				+= -L$(CROSSTOOLDIR)/staging_dir/target-mipsel_24kec+dsp_uClibc-0.9.33.2/usr/lib 
endif


#ARCH = arm64
#CROSS_COMPILE = aarch64-buildroot-linux-gnu-
##ARCH ?= 
##CROSS_COMPILE ?=
##export

#export ARCH := mipsel
#export CROSS_COMPILE := mipsel-openwrt-linux-



### general build targets

.PHONY: all clean install install_conf libtools libloragw packet_forwarder util_net_downlink util_chip_id util_boot util_spectral_scan

all: libtools libloragw packet_forwarder util_net_downlink util_chip_id util_boot util_spectral_scan

libtools:
	$(MAKE) all -e -C $@

libloragw: libtools
	$(MAKE) all -e -C $@

packet_forwarder: libloragw
	$(MAKE) all -e -C $@

util_net_downlink: libtools
	$(MAKE) all -e -C $@

util_chip_id: libloragw
	$(MAKE) all -e -C $@

util_boot: libloragw
	$(MAKE) all -e -C $@

util_spectral_scan: libloragw
	$(MAKE) all -e -C $@

clean:
	$(MAKE) clean -e -C libtools
	$(MAKE) clean -e -C libloragw
	$(MAKE) clean -e -C packet_forwarder
	$(MAKE) clean -e -C util_net_downlink
	$(MAKE) clean -e -C util_chip_id
	$(MAKE) clean -e -C util_boot
	$(MAKE) clean -e -C util_spectral_scan
	rm -rf ./bin/*

install:
	mkdir -p ./bin/
	$(MAKE) install -e -C libloragw
	$(MAKE) install -e -C packet_forwarder
	$(MAKE) install -e -C util_net_downlink
	$(MAKE) install -e -C util_chip_id
	$(MAKE) install -e -C util_boot
	$(MAKE) install -e -C util_spectral_scan

install_conf:
	$(MAKE) install_conf -e -C packet_forwarder

scp:
	scp -rp -P2200 ./bin/ root@192.168.0.230:/tmp/

### EOF
