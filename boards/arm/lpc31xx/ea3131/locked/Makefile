############################################################################
# boards/arm/lpc31xx/ea3131/locked/Makefile
#
#   Copyright (C) 2010 Gregory Nutt. All rights reserved.
#   Author: Gregory Nutt <gnutt@nuttx.org>
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in
#    the documentation and/or other materials provided with the
#    distribution.
# 3. Neither the name NuttX nor the names of its contributors may be
#    used to endorse or promote products derived from this software
#    without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
# FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
# COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
# BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
# OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
# AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
# ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
#
############################################################################

include $(TOPDIR)/Make.defs

# Board-specific directory, board library, and application library

PASS1_SRCDIR   = arch$(DELIM)$(CONFIG_ARCH)$(DELIM)src
PASS1_LIBBOARD = libboard$(LIBEXT)
PASS1_LIBAPPS  = libapps$(LIBEXT)

# Remove the application library (libapps) from the list of libraries.  Add
# the boad library (liboard)

PASS1_LINKLIBS  = $(filter-out $(PASS1_LIBAPPS),$(LINKLIBS))
PASS1_LINKLIBS += $(PASS1_LIBBOARD)

# Get the paths to the libraries and the links script path in format that
# is appropriate for the host OS

ifeq ($(CONFIG_CYGWIN_WINTOOL),y)
  # Windows-native toolchains
  PASS1_LIBPATHS += -L"${shell cygpath -w "$(TOPDIR)$(DELIM)staging"}"
  PASS1_LIBPATHS += -L"${shell cygpath -w "$(TOPDIR)$(DELIM)boards$(DELIM)arm$(DELIM)lpc31xx$(DELIM)ea3131$(DELIM)src"}"
  PASS1_LDSCRIPT = -T "${shell cygpath -w $(TOPDIR)$(DELIM)boards$(DELIM)arm$(DELIM)lpc31xx$(DELIM)ea3131$(DELIM)locked$(DELIM)ld-locked.inc}"
else
  # Linux/Cygwin-native toolchain
  PASS1_LIBPATHS += -L"$(TOPDIR)$(DELIM)staging"
  PASS1_LIBPATHS += -L"$(TOPDIR)$(DELIM)boards$(DELIM)arm$(DELIM)lpc31xx$(DELIM)ea3131$(DELIM)src"
  PASS1_LDSCRIPT = -T$(TOPDIR)$(DELIM)boards$(DELIM)arm$(DELIM)lpc31xx$(DELIM)ea3131$(DELIM)locked$(DELIM)ld-locked.inc
endif

PASS1_LDFLAGS = -r $(PASS1_LDSCRIPT)
PASS1_LDLIBS = $(patsubst %.a,%,$(patsubst lib%,-l%,$(PASS1_LINKLIBS)))
PASS1_LIBGCC = "${shell $(CC) -print-libgcc-file-name}"

# Targets:

all: $(PASS1_SRCDIR)$(DELIM)locked.r

.PHONY:	depend clean distclean

# Create include-able linker script that specifies all of the symbols to be
# resolved in the locked.r file.

ld-locked.inc: mklocked.sh $(TOPDIR)$(DELIM).config
	@echo "MK:  ld-locked.inc"
	$(Q) .$(DELIM)mklocked.sh "$(TOPDIR)"

# Make the board library. This is normally done in arch/arm/src/Makefile.
# However, we need it earlier here when doing a two-pass build so that libboard.a
# is available to link against.

$(PASS1_LIBBOARD):
	$(Q) $(MAKE) -C $(TOPDIR)$(DELIM)boards$(DELIM)arm$(DELIM)lpc31xx$(DELIM)ea3131$(DELIM)src libboard$(LIBEXT) EXTRAFLAGS="$(EXTRAFLAGS)"

# Create the locked.r file containing all of the code (except the start-up code)
# that needs to lie in the locked text region.

locked.r: ld-locked.inc $(PASS1_LIBBOARD)
	@echo "LD:  locked.r"
	$(Q) $(LD) -o $@ $(PASS1_LDFLAGS) $(PASS1_LIBPATHS) --start-group $(PASS1_LDLIBS) --end-group $(PASS1_LIBGCC)
	$(Q) $(NM) $@ > locked.map
	$(Q) fgrep " U " locked.map | grep -v nx_start
	$(Q) $(CROSSDEV)size $@

$(PASS1_SRCDIR)$(DELIM)locked.r: locked.r
	$(Q) cp -a locked.r $(TOPDIR)$(DELIM)$(PASS1_SRCDIR)$(DELIM)locked.r

.depend:

depend: .depend

clean:
	$(call DELFILE, locked.r)
	$(call DELFILE, $(TOPDIR)$(DELIM)$(PASS1_SRCDIR)$(DELIM)locked.r)
	$(call DELFILE, locked.map)
	$(call CLEAN)

distclean: clean
	$(call DELFILE, ld-locked.inc)
