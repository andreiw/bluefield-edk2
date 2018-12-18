# This Makefile builds Tianocore UEFI (edk2) for BlueField.
#
# Note that edk2 requires building in the source tree.  Also, the edk2
# build system fails with parallel build, so you must build with -j1.
#
# The image built is
# BLUEFIELD_EFI.fd and/or BLUEFIELD_EFI_SEC.fd in the
# Build/BlueField/RELEASE_GCC49/FV directory

# ------- Customizable build options -------

# The mode to build EDK2 in (DEBUG or RELEASE).
EDK2_MODE = RELEASE
SHELL = /bin/bash

# Any particular defines to use when building edk2.
EDK2_DEFINES = \
  -DSECURE_BOOT_ENABLE=TRUE \
  -DFIRMWARE_VER=BlueField:0.99-2f89df3 \

# Path to openssl tarball.  Set it to an already-downloaded location for
# the tarball, or else this makefile will download it into the source tree.
OPENSSL_TARBALL = CryptoPkg/Library/OpensslLib/openssl-1.0.2d.tar.gz

# The compiler toolchain prefix to use for the tools.  This can
# include the full path and prefix if the tools are not in $(PATH).
# Note that gcc 4.9 or later will work; we use Yocto Poky gcc 6.3 or later.
GCC49_AARCH64_PREFIX = aarch64-linux-gnu-

# If "iasl" is not in your path, specify its directory here.
# We use https://github.com/acpica/acpica.git at commit ed0389cb or later.
IASL_PREFIX =

# If "dtc" is not in your path, specify its directory here.
# You can typically find it in a Linux build tree in scripts/dtc.
# If you are using a Yocto SDK you can use the DTC contained within
# it.
#
# DTC_PREFIX=/opt/poky/2.4.1/sysroots/x86_64-pokysdk-linux/usr/bin/
#
DTC_PREFIX =

# Device tree source files
DTS_FILES = bf-full.dts
DTS_DIR = ../../dts

# IMPORTANT: When you actually build edk2 here make sure you are NOT in an
# environment/bash shell where environment-setup-aarch64-poky-linux was run.
# Simply point GCC49_AARCH64_PREFIX to
# /opt/poky/2.4.1/sysroots/x86_64-pokysdk-linux/usr/bin/aarch64-poky-linux/aarch64-poky-linux-
# Running the SDK environment-setup-aarch64-poky-linux script will confuse
# the UEFI make environment.  It just needs a pointer to the tools.

# ------- End customizable build options -------


# Export variables that need to be set in the environment.
export IASL_PREFIX GCC49_AARCH64_PREFIX

FD_FILE = Build/BlueField/$(EDK2_MODE)_GCC49/FV/BLUEFIELD_EFI.fd

all: $(FD_FILE)

DTB_FILES = $(addprefix dtb/, $(DTS_FILES:.dts=.dtb))

$(FD_FILE): FORCE CryptoPkg/Library/OpensslLib/openssl-1.0.2d $(DTB_FILES)
	$(MAKE) -C BaseTools/Source/C
	set --; . ./edksetup.sh; \
	  build -n 6 -t GCC49 -a AARCH64 -p MlxPlatformPkg/BlueField.dsc \
	    -b $(EDK2_MODE) -DDTB_DIR=$(PWD)/dtb $(EDK2_DEFINES)

FORCE:

# Build device tree blobs for specified device tree source(s)

$(DTB_FILES): dtb/%.dtb: $(DTS_DIR)/%.dts
	mkdir -p $(@D)
	cpp -P -x assembler-with-cpp -o- $< | \
	  $(DTC_PREFIX)dtc -b 0 -O dtb -o $@.tmp -I dts -
	mv -f $@.tmp $@


# These steps are documented in CryptoPkg/Library/OpensslLib/Patch-HOWTO.txt

CryptoPkg/Library/OpensslLib/openssl-1.0.2d: $(OPENSSL_TARBALL)
	tar -C $(@D) -xf $<
	cd $@ && patch -p0 < ../EDKII_openssl-1.0.2d.patch
	cd $(@D) && ./Install.sh

$(OPENSSL_TARBALL):
	curl https://www.openssl.org/source/openssl-1.0.2d.tar.gz > $@.tmp
	mv -f $@.tmp $@
