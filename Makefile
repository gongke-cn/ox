# Homeage
HOMEPAGE := https://gitee.com/gongke1978/ox

# Maintainer
MAINTAINER := OX maintainer

# OX version number
OX_VERSION := $(shell cat pkg/ox/build.ox | grep '\bversion:' | sed 's/\s*version:\s*"\(.\+\)"/\1/')

# Dynamic library version number
SO_VERSION := 0

# Quiet mode
Q ?= @

# Output directory
O ?= out

# OX executable program
OX_EXE ?= ox

# Cross compile toolchain prefix
CROSS_PREFIX ?=

# pkg-config prefix
PKGCONFIG_PREFIX ?=

# C lang as compiler
CLANG ?= 0

# Machine type
M ?=

# Only generate internal packages
INTERNAL_PKGS ?= 0

# Install prefix
INSTALL_PREFIX ?= /usr

# Package build compiler flags.
PB_CFLAGS ?=

# Package build linker flags.
PB_LIBS ?=

# External packages
ENABLE_SDL ?= 1
ENABLE_NCURSES ?= 1
ENABLE_GIR ?= 1
ENABLE_SSL ?= 1
ENABLE_HTTP ?= 1

ifneq ($(shell which $(OX_EXE)),)
	HAVE_OX_EXE := 1
endif

# C compiler
ifeq ($(CLANG),1)
	CC := $(CROSS_PREFIX)clang
	ifeq ($(ASAN),1)
		CFLAGS += -fsanitize=address -g -fno-omit-frame-pointer -O1
		ASAN_LIBS := -lasan
	else
		CFLAGS += -O2
	endif
else
	CC := $(CROSS_PREFIX)gcc
	CFLAGS += -O2
endif

# Get target architcture from compiler.
CC_TARGET := $(shell $(CC) -dumpmachine)

ifneq ($(shell echo $(CC_TARGET)|grep x86_64-.*linux),)
	ifeq ($(M),32)
		TARGET := i686-pc-linux-gnu
	else
		TARGET := x86_64-pc-linux-gnu
	endif
endif

ifneq ($(shell echo $(CC_TARGET)|grep i686-.*linux),)
	ifeq ($(M),64)
		TARGET := x86_64-pc-linux-gnu
	else
		TARGET := i686-pc-linux-gnu
	endif
endif

ifneq ($(shell echo $(CC_TARGET)|grep x86_64-w64),)
	ifeq ($(M),32)
		TARGET := i686-w64-windows-gnu
	else
		TARGET := x86_64-w64-windows-gnu
	endif
endif

ifneq ($(shell echo $(CC_TARGET)|grep i686-w64),)
	ifeq ($(M),64)
		TARGET := x86_64-w64-windows-gnu
	else
		TARGET := i686-w64-windows-gnu
	endif
endif

ifeq ($(TARGET)),)
	$(error unknown architcture $(CC_TARGET))
endif

OX_PB_CFLAGS := $(PB_CFLAGS)
OX_PB_LIBS := $(PB_LIBS)

ifneq ($(M),)
	OX_PB_CFLAGS += -m$(M)
	OX_PB_LIBS += -m$(M)
endif

# m32/m64
ifeq ($(M),32)
	CFLAGS += -m32
	LIBS += -m32
endif
ifeq ($(M),64)
	CFLAGS += -m64
	LIBS += -m64
endif

PTR_SIZE := $(shell $(CROSS_PREFIX)cpp $(CFLAGS) -dM </dev/null| grep '#define\s\+__SIZEOF_POINTER__' | sed 's/#define\s\+__SIZEOF_POINTER__\s\+//')

# Architecture.
ARCH := $(shell echo $(TARGET) | sed 's/\([^-]\+\)-\([^-]\+\)-\([^-]\+\)\(-\(.\+\)\)\?/\1/')
# Vendor
VENDOR := $(shell echo $(TARGET) | sed 's/\([^-]\+\)-\([^-]\+\)-\([^-]\+\)\(-\(.\+\)\)\?/\2/')
# OS
OS := $(shell echo $(TARGET) | sed 's/\([^-]\+\)-\([^-]\+\)-\([^-]\+\)\(-\(.\+\)\)\?/\3/')
# ABI
ABI := $(shell echo $(TARGET) | sed 's/\([^-]\+\)-\([^-]\+\)-\([^-]\+\)\(-\(.\+\)\)\?/\5/')

# Get library directory
ifeq ($(CROSS_PREFIX),)
ifeq ($(OS),linux)

ifeq ($(LIB_ARCH),)
ifeq ($(ARCH),x86_64)
ifneq ($(wildcard $(INSTALL_PREFIX)/lib/x86_64-linux-gnu/libc.so),)
LIB_ARCH := lib/x86_64-linux-gnu
endif
endif
endif

ifeq ($(LIB_ARCH),)
ifeq ($(PTR_SIZE),8)
ifneq ($(wildcard $(INSTALL_PREFIX)/lib64/libc.so),)
LIB_ARCH := lib64
endif
endif
endif

ifeq ($(LIB_ARCH),)
ifeq ($(ARCH),i686)
ifneq ($(wildcard $(INSTALL_PREFIX)/lib/i386-linux-gnu/libc.so),)
LIB_ARCH := lib/i386-linux-gnu
endif
endif
endif

ifeq ($(LIB_ARCH),)
ifeq ($(PTR_SIZE),4)
ifneq ($(wildcard $(INSTALL_PREFIX)/lib32/libc.so),)
LIB_ARCH := lib32
endif
endif
endif

endif
endif

ifeq ($(LIB_ARCH),)
	LIB_ARCH := lib
endif

ifneq ($(CROSS_PREFIX),)
CLANG_TARGET := -t $(shell echo $(CROSS_PREFIX)|sed s/-$$//)
endif

# Archives program
AR := $(CROSS_PREFIX)ar

# Archive index generter program
RANLIB := $(CROSS_PREFIX)ranlib

# Shell
SHELL := /bin/bash
# Remove command
RM    := rm -f
# Remove the directory
RMDIR := rm -rf
# Make directory command
MKDIR := mkdir -p
# Copy
COPY  := cp

# Compiling flags
CFLAGS += -Wall -Iinclude -I$(O)/include -Isrc/lib
# Linked libraries and flags
LIBS += -lm -lpthread

# Debug
ifeq ($(DEBUG),1)
CFLAGS += -g
OX_PB_CFLAGS += -g
endif

#PKGCONFIG
PKGCONFIG := $(PKGCONFIG_PREFIX)pkg-config

ifeq ($(OS),windows)
	# Top directory
	ifneq ($(shell which cygpath),)
		TOP := $(shell cygpath -w `pwd` | sed 's/\\/\//g')
	else
		TOP := $(shell pwd)
		LIBS += -lucrtbase
	endif
	# Suffix of static library
	SLIB_SUFFIX := .a
	# Suffix of dynamic library
	DLIB_SUFFIX := .dll
	# Suffix of executable program
	EXE_SUFFIX  := .exe
	# Link libintl and libiconv
	LIBS += -lintl -liconv
	# ARCH macro
	CFLAGS += -DARCH_WIN
	# OX build C flags
	OX_PB_CFLAGS += -DARCH_WIN
	# Dynamic library: libox.dll
	LIBOX_DLIB := $(O)/bin/libox-$(SO_VERSION)$(DLIB_SUFFIX)
	# Dynamic library: libox.dll.a
	LIBOX_DLIB_A := $(O)/$(LIB_ARCH)/libox$(DLIB_SUFFIX)$(SLIB_SUFFIX)
	# Program ox-win's ld flags
	OX_WIN_LIBS := -mwindows
	# Socket package LD flags
	std_LIBS := -lws2_32
	# Resource object
	OX_RES_OBJS := $(O)/winres.o
else
	# Top directory
	TOP := $(shell pwd)
	# Suffix of static library
	SLIB_SUFFIX := .a
	# Suffix of dynamic library
	DLIB_SUFFIX := .so
	# Suffix of executable program
	EXE_SUFFIX  :=
	# Link libdl
	LIBS += -ldl
	# ARCH macro
	CFLAGS += -DARCH_LINUX
	# OX build C flags
	OX_PB_CFLAGS += -DARCH_LINUX
	# Dynamic library: libox
	LIBOX_DLIB := $(O)/$(LIB_ARCH)/libox$(DLIB_SUFFIX).$(SO_VERSION)
	# deb arch name
	ifeq ($(ARCH),i686)
		DEB_ARCH := i386
	else
		DEB_ARCH := amd64
	endif
endif

O_REAL := $(shell $(MKDIR) $(O); realpath $(O))
OX_PB_LIBS += -L$(O_REAL)/$(LIB_ARCH)
ox_PB_PARAMS := -s outdir=$(O_REAL) -s so_version=$(SO_VERSION) -s libdir=$(LIB_ARCH)
ox_devel_PB_PARAMS := -s outdir=$(O_REAL) -s so_version=$(SO_VERSION) -s libdir=$(LIB_ARCH)

# FFI
FFI_CFLAGS := $(shell $(PKGCONFIG) libffi --cflags)
FFI_LIBS := $(shell $(PKGCONFIG) libffi --libs)
CFLAGS := $(CFLAGS) $(FFI_CFLAGS)
LIBS := $(LIBS) $(FFI_LIBS)

# Static library: libox
LIBOX_SLIB := $(O)/$(LIB_ARCH)/libox$(SLIB_SUFFIX)
# Source files of libox
LIBOX_SRCS := $(wildcard src/lib/*.c)
# Object files of libox
LIBOX_OBJS := $(patsubst %.c,$(O)/%.o,$(LIBOX_SRCS))
# Library: libox
LIBOX := $(LIBOX_SLIB)
ifneq ($(SLIB_ONLY),1)
	LIBOX += $(LIBOX_DLIB)
	CFLAGS += -fPIC
endif
TARGETS += $(LIBOX)
OBJS += $(LIBOX_OBJS)

# Executable program: ox
OX := $(O)/bin/ox-cli$(EXE_SUFFIX)
# Source files of ox
OX_SRCS := src/exe/ox.c src/exe/main.c
# Object files of ox
OX_OBJS := $(patsubst %.c,$(O)/%.o,$(OX_SRCS)) $(OX_RES_OBJS)
# C flags of executable program
TARGETS += $(OX)
OBJS += $(OX_OBJS)

ifeq ($(OS),windows)
	# Executable program: ox-win
	OX_WIN := $(O)/bin/ox-win$(EXE_SUFFIX)
	# Source files of ox
	OX_WIN_SRCS := src/exe/ox-win.c src/exe/main.c
	# Object files of ox
	OX_WIN_OBJS := $(patsubst %.c,$(O)/%.o,$(OX_WIN_SRCS)) $(OX_RES_OBJS)
	# Source files of ox
	OX_WRAPPER_SRCS := src/exe/ox-wrapper-win.c
	# LD flags of ox
	OX_WRAPPER_LIBS := -lshlwapi

	TARGETS += $(OX_WIN)
	OBJS += $(OX_WIN_OBJS)
else
	# Source files of ox
	OX_WRAPPER_SRCS := src/exe/ox-wrapper-linux.c
endif

# Executable program: ox wrapper executable program
OX_WRAPPER := $(O)/bin/ox$(EXE_SUFFIX)
# Object files of ox
OX_WRAPPER_OBJS := $(patsubst %.c,$(O)/%.o,$(OX_WRAPPER_SRCS)) $(OX_RES_OBJS)
# C flags of executable program
TARGETS += $(OX_WRAPPER)
OBJS += $(OX_WRAPPER_OBJS)

OX_CFLAGS := -DVERSION="\"$(OX_VERSION)\"" -DTARGET="\"$(TARGET)\"" -DLIBS="\"$(LIBS)\""

# Unit test program
UNIT_TEST := $(O)/bin/unit_test$(EXE_SUFFIX)
# Source files of unit test program
UNIT_TEST_SRCS := $(wildcard test/unit_test/*.c)
# Object files of unit test program
UNIT_TEST_OBJS := $(patsubst %.c,$(O)/%.o,$(UNIT_TEST_SRCS))
TEST += $(UNIT_TEST)
OBJS += $(UNIT_TEST_OBJS)

# OX packages
BASIC_PACKAGES := std oxngen doc json
ENV_PACKAGES := gettext curl archive oxp pm pb xml
EXT_PACKAGES := ox ox_devel

ifeq ($(ENABLE_SDL),1)
EXT_PACKAGES += sdl
endif

ifeq ($(ENABLE_GIR),1)
EXT_PACKAGES += gir
endif

ifeq ($(ENABLE_NCURSES),1)
EXT_PACKAGES += ncurses
endif

ifeq ($(ENABLE_SSL),1)
EXT_PACKAGES += ssl
endif

ifeq ($(ENABLE_HTTP),1)
EXT_PACKAGES += http
endif

SYS_PACKAGES := libffi libarchive libncurses libglib2_0 libssl libcrypto libcurl libsdl
SYS_PACKAGES += libffi_devel libarchive_devel libncurses_devel libglib2_0_devel openssl_devel libcurl_devel libsdl_devel
SYS_PACKAGES += gcc clang gnu_gettext pkgconf ca_certificates

ifeq ($(OS),windows)
SYS_PACKAGES += libiconv libintl
endif

PACKAGES := $(BASIC_PACKAGES) $(ENV_PACKAGES) $(EXT_PACKAGES)
DIST_PACKAGES := ox std json curl archive oxp pm

ifneq ($(CROSS_PREFIX),)
OX_PB_LIBS += $(LIBS)
endif

# std
std_CFLAGS := -DTARGET="\"$(TARGET)\""

# curl.oxn
curl_CFLAGS := $(shell $(PKGCONFIG) libcurl --cflags)
curl_LIBS := $(shell $(PKGCONFIG) libcurl --libs)

# archive.oxn
archive_CFLAGS := $(shell $(PKGCONFIG) libarchive --cflags)
archive_LIBS := $(shell $(PKGCONFIG) libarchive --libs)

# test.oxn
test_CFLAGS := -Ipkg/test

# Build the OX file.
define build_ox =
$(O)/share/ox/pkg/$$($(1)_ARCH)/$(1)/$(2): pkg/$(1)/$(2)
	$$(info GEN  $$@)
	$(Q)$(MKDIR) $$(dir $$@)
	$(Q)cp $$< $$@
endef

# Build the OXN file.
define build_oxn =
$(O)/share/ox/pkg/$$($(1)_ARCH)/$(1)/$(2).o: pkg/$(1)/$(2).c
	$$(info CC   $$@ <- $$<)
	$(Q)$(CC) -o $$@ -c $$< $$($(1)_CFLAGS) $(CFLAGS)
$(O)/share/ox/pkg/$$($(1)_ARCH)/$(1)/$(2): $(O)/share/ox/pkg/$$($(1)_ARCH)/$(1)/$(2).o $(LIBOX)
	$$(info CC   $$@ <- $(O)/share/ox/pkg/$$($(1)_ARCH)/$(1)/$(2).o)
	$(Q)$(CC) -o $$@ $(O)/share/ox/pkg/$$($(1)_ARCH)/$(1)/$(2).o -shared $(ASAN_LIBS) $$($(1)_LIBS) -L$(O)/$(LIB_ARCH) -lox $(LIBS)
endef

# Build the oxngen target.
define build_oxngen =
$(O)/share/ox/pkg/$$($(1)_ARCH)/$(1)/$(2).oxn.o: $(O)/share/ox/pkg/$$($(1)_ARCH)/$(1)/$(2).oxn.c
	$$(info CC   $$@ <- $$<)
	$(Q)$(CC) -o $$@ -c $$< $$($(1)_CFLAGS) $(CFLAGS)
$(O)/share/ox/pkg/$$($(1)_ARCH)/$(1)/$(2).oxn: $(O)/share/ox/pkg/$$($(1)_ARCH)/$(1)/$(2).oxn.o $(LIBOX)
	$$(info CC   $$@ <- $(O)/share/ox/pkg/$$($(1)_ARCH)/$(1)/$(2).oxn.o)
	$(Q)$(CC) -o $$@ $(O)/share/ox/pkg/$$($(1)_ARCH)/$(1)/$(2).oxn.o -shared $(ASAN_LIBS) $$($(1)_LIBS) -L$(O)/$(LIB_ARCH) -lox $(LIBS)
endef

# Build the package.
define build_package =

# Get the package's architecture.
ifneq ($$(shell cat pkg/$(1)/build.ox|grep "oxn_modules\|oxngen_target"),)
$(1)_ARCH := $(TARGET)
else
$(1)_ARCH := all
endif

# Generate package.ox.in
ifeq ($(HAVE_OX_EXE),1)
pkg/$(1)/package.ox.in: pkg/$(1)/build.ox build/package.ox
	$$(info GEN  $$@ <- $$<)
	$(Q)CC=$(CC) PC=$(PKGCONFIG) TARGET=$(TARGET) HOMEPAGE=$(HOMEPAGE) MAINTAINER="$(MAINTAINER)" $(OX_EXE) build/package.ox $$< > $$@
endif #HAVE_OX_EXE

# Generate package.ox
$(O)/share/ox/pkg/$$($(1)_ARCH)/$(1)/package.ox: pkg/$(1)/package.ox.in
	$$(info GEN  $$@ <- $$<)
	$(Q)$(MKDIR) $$(dir $$@)
	$(Q)cat $$< | sed "s+%ARCH%+$(TARGET)+" > $$@

$(1)_OXS_SRCS := $$(patsubst pkg/$(1)/%,%,$$(wildcard pkg/$(1)/*.ox))

$(1)_OXS := $$(patsubst %,$(O)/share/ox/pkg/$$($(1)_ARCH)/$(1)/%,$$($(1)_OXS_SRCS))
$(1)_OXS += $(O)/share/ox/pkg/$$($(1)_ARCH)/$(1)/package.ox

$$(foreach f,$$($(1)_OXS_SRCS),$$(eval $$(call build_ox,$1,$$f)))

$(1)_OXNS_SRCS := $$(patsubst pkg/$(1)/%.c,%,$$(wildcard pkg/$(1)/*.oxn.c))
$(1)_OXNS := $$(patsubst %,$(O)/share/ox/pkg/$$($(1)_ARCH)/$(1)/%,$$($(1)_OXNS_SRCS))

$$(foreach f,$$($(1)_OXNS_SRCS),$$(eval $$(call build_oxn,$1,$$f)))

# oxngen
ifeq ($(HAVE_OX_EXE),1)
$(1)_OXNGENS := $$(shell CC=$(CC) PC=$(PKGCONFIG) TARGET=$(TARGET) $(OX_EXE) build/package.ox pkg/$(1)/build.ox -o)
$(1)_OXNS += $$(patsubst %,$(O)/share/ox/pkg/$$($(1)_ARCH)/$(1)/%.oxn,$$($(1)_OXNGENS))
$(1)_OXNGENS_SRCS := $$(patsubst %,$(O)/share/ox/pkg/$$($(1)_ARCH)/$(1)/%.oxn.c,$$($(1)_OXNGENS))
ifneq ($$($(1)_OXNGENS_SRCS),)
$$($(1)_OXNGENS_SRCS): $(O)/share/ox/pkg/$$($(1)_ARCH)/$(1)/oxngen
	$$(info GEN  $$@)

$(O)/share/ox/pkg/$$($(1)_ARCH)/$(1)/oxngen: pkg/$(1)/build.ox
	$$(info GEN  $$@)
	$(Q)$(MKDIR) $$(dir $$@)
	$(Q)$(OX_EXE) -r oxngen $(CLANG_TARGET) --pc $(PKGCONFIG) -i pkg/$(1)/build.ox -o $(O)/share/ox/pkg/$$($(1)_ARCH)/$(1) $$($(1)_CFLAGS)
	$(Q)touch $$@

$$(foreach f,$$($(1)_OXNGENS),$$(eval $$(call build_oxngen,$1,$$f)))
endif #OXNGENS_SRCS
endif #HAVE_OX_EXE

endef # build_package

# Build basic and env packages.
$(foreach p,$(PACKAGES),$(eval $(call build_package,$(p))))

# Languages
LANGS := $(patsubst locale/%.po,%,$(wildcard locale/*.po))

# mo files
MO_FILES := $(patsubst %,$(O)/share/locale/%/LC_MESSAGES/ox.mo,$(LANGS))
TARGETS += $(MO_FILES)

all: packages

BASIC_TARGETS += $(foreach p,$(BASIC_PACKAGES),$($(p)_OXS) $($(p)_OXNS) )
ENV_TARGETS += $(foreach p,$(ENV_PACKAGES),$($(p)_OXS) $($(p)_OXNS) )

# Install packages
define install_packages =
$(foreach p,$(1),\
	$(MKDIR) $(INSTALL_PREFIX)/share/ox/pkg/$($(p)_ARCH)/$p;)\
$(foreach p,$(1),\
	$(foreach f,$($(p)_OXS),\
		install -m 644 -T $f $(INSTALL_PREFIX)/$(patsubst $(O)/%,%,$f);))\
$(foreach p,$(1),\
	$(foreach f,$($(p)_OXNS),\
		install -m 755 -T $f $(INSTALL_PREFIX)/$(patsubst $(O)/%,%,$f);))
endef

# Uninstall packages
define uninstall_packages =
$(foreach p,$(1),\
	$(RMDIR) $(INSTALL_PREFIX)/share/ox/pkg/$($(p)_ARCH)/$(p);)
endef

lib: $(LIBOX)

test: $(TEST)

ifneq ($(filter-out clean dist-clean uninstall-basic uninstall-env uninstall-all,$(MAKECMDGOALS)),)
INCLUDE_DEPS := 1
endif

ifeq ($(MAKECMDGOALS),)
INCLUDE_DEPS := 1
endif

ifeq ($(INCLUDE_DEPS),1)
# Dependence files
DEPS += $(patsubst %.o,%.d,$(OBJS))
include $(DEPS)
endif

$(OX_RES_OBJS): build/ox.rc build/ox.ico build/oxn.ico build/oxp.ico
	$(info GEN  $@)
	$(Q)windres -o $@ -i build/ox.rc

$(OBJS) $(DEPS): src/lib/ox_string_id.h src/lib/ox_object_id.h src/lib/ox_keyword.h src/lib/ox_punct.h src/lib/ox_ast.h src/lib/ox_bytecode.h

$(O)/src/lib/ox_context.o: src/lib/ox_string_table.h

$(O)/src/lib/ox_lex.o: src/lib/ox_keyword_table.h src/lib/ox_punct_table.h

$(O)/src/lib/ox_ast.o: src/lib/ox_ast_table.h

$(O)/src/lib/ox_compile.o: src/lib/ox_command.h src/lib/ox_bytecode_run.h

ifeq ($(HAVE_OX_EXE),1)
# Generate string id declaration
src/lib/ox_string_id.h: build/string.ox
	$(info GEN  $@ <- $<)
	$(Q)$(OX_EXE) build/string.ox -d > $@

# Generate string table
src/lib/ox_string_table.h: build/string.ox
	$(info GEN  $@ <- $<)
	$(Q)$(OX_EXE) build/string.ox > $@

# Generate object id declaration
src/lib/ox_object_id.h: build/object.ox
	$(info GEN  $@ <- $<)
	$(Q)$(OX_EXE) build/object.ox -d > $@

# Generate keywords declaration
src/lib/ox_keyword.h: build/keyword.ox
	$(info GEN  $@ <- $<)
	$(Q)$(OX_EXE) build/keyword.ox -d > $@

# Generate keyword table
src/lib/ox_keyword_table.h: build/keyword.ox
	$(info GEN  $@ <- $<)
	$(Q)$(OX_EXE) build/keyword.ox > $@

# Generate punctuation declaration
src/lib/ox_punct.h: build/punct.ox
	$(info GEN  $@ <- $<)
	$(Q)$(OX_EXE) build/punct.ox -d > $@

# Generate punctuation table
src/lib/ox_punct_table.h: build/punct.ox
	$(info GEN  $@ <- $<)
	$(Q)$(OX_EXE) build/punct.ox > $@

# Generate AST declaration
src/lib/ox_ast.h: build/ast.ox
	$(info GEN  $@ <- $<)
	$(Q)$(OX_EXE) build/ast.ox -d > $@

# Generate AST table
src/lib/ox_ast_table.h: build/ast.ox
	$(info GEN  $@ <- $<)
	$(Q)$(OX_EXE) build/ast.ox > $@

# Generate bytecode declaration
src/lib/ox_bytecode.h: build/bytecode.ox
	$(info GEN  $@ <- $<)
	$(Q)$(OX_EXE) build/bytecode.ox -d > $@

# Generate command file
src/lib/ox_command.h: build/bytecode.ox
	$(info GEN  $@ <- $<)
	$(Q)$(OX_EXE) build/bytecode.ox -c > $@

# Generate bytecode running table file
src/lib/ox_bytecode_run.h: build/bytecode.ox
	$(info GEN  $@ <- $<)
	$(Q)$(OX_EXE) build/bytecode.ox > $@

endif

# Build executable program.
define build_exe =
$(1): EXE_SHELL = $$(patsubst %$(EXE_SUFFIX),%.sh,$(1))
$(1): $(2) $(LIBOX)
	$$(info CC   $$@ <- $(2))
	$(Q)$(MKDIR) $$(dir $$@)
	$(Q)$(CC) -o $$@ $(2) $(3) -L$(O)/$(LIB_ARCH) $(ASAN_LIBS) -lox $(LIBS)
	$(Q)cat build/run.sh | sed "s+%LIB_ARCH%+$(LIB_ARCH)+" > $$(EXE_SHELL)
	$(Q)chmod a+x $$(EXE_SHELL)
endef

# Build program ox
$(eval $(call build_exe,$(OX),$(OX_OBJS),))

# Build wrapper program ox
$(eval $(call build_exe,$(OX_WRAPPER),$(OX_WRAPPER_OBJS),$(OX_WRAPPER_LIBS)))

ifeq ($(OS),windows)
# Build program ox-win
$(eval $(call build_exe,$(OX_WIN),$(OX_WIN_OBJS),$(OX_WIN_LIBS)))
endif

# Build unit test program
$(eval $(call build_exe,$(UNIT_TEST),$(UNIT_TEST_OBJS),))

$(O)/%.d: %.c
	$(info DEP  $@ <- $<)
	$(Q)$(MKDIR) $(dir $@)
	$(Q)$(CC) -E $(OX_CFLAGS) $(CFLAGS) $< -MMD -MF $@ -MT $(patsubst %.d,%.o,$@) > /dev/null

$(O)/%.o: %.c
	$(info CC   $@ <- $<)
	$(Q)$(MKDIR) $(dir $@)
	$(Q)$(CC) -c -o $@ $(OX_CFLAGS) $(CFLAGS) $<

# Build libox static library
$(LIBOX_SLIB): $(LIBOX_OBJS)
	$(info AR   $@ <- $^)
	$(Q)$(MKDIR) $(dir $@)
	$(Q)$(AR) rcs $@ $^
	$(Q)$(RANLIB) $@

# Build libox dynamic library
$(LIBOX_DLIB): $(LIBOX_OBJS)
	$(info DLIB $@ <- $^)
	$(Q)$(MKDIR) $(dir $@)
ifeq ($(OS),windows)
	$(Q)$(MKDIR) $(dir $(LIBOX_DLIB_A))
	$(Q)$(CC) -o $@ $^ -shared $(ASAN_LIBS) $(LIBS) -Wl,--out-implib,$(LIBOX_DLIB_A)
else
	$(Q)$(CC) -o $@ $^ -shared $(ASAN_LIBS) $(LIBS) -Wl,-soname,libox$(DLIB_SUFFIX).$(SO_VERSION)
	if [ ! -e $(O)/$(LIB_ARCH)/libox$(DLIB_SUFFIX) ]; then\
		ln -s libox$(DLIB_SUFFIX).$(SO_VERSION) $(O)/$(LIB_ARCH)/libox$(DLIB_SUFFIX);\
	fi
endif

POT_SRCS := $(shell find include -name "*.h")
POT_SRCS += $(shell find src -name "*.c")

ifeq ($(UPDATE_PO),1)

# ox.pot
locale/ox.pot: $(POT_SRCS)
	$(info GEN  $@)
	$(Q)xgettext -kOX_TEXT -o $@ $^

# Update po file
define update_po =
locale/$(1).po: locale/ox.pot
	$$(info GEN  $$@)
	$(Q)msgmerge -U $$@ $$<
endef

# po files
$(foreach l,$(LANGS),\
	$(eval $(call update_po,$(l))))

TARGETS += locale/ox.pot
TARGETS += $(foreach l,$(LANGS),locale/$(l).po )

endif # UPDATE_PO

# Server list
TARGETS += $(O)/share/ox/server/main.ox
$(O)/share/ox/server/main.ox: build/server.ox
	$(info GEN  $@)
	$(Q)$(MKDIR) $(dir $@)
	$(Q)cat $< | sed s/%TARGET%/$(TARGET)/ > $@

# libox.pc
TARGETS += $(O)/$(LIB_ARCH)/pkgconfig/libox.pc
$(O)/$(LIB_ARCH)/pkgconfig/libox.pc: build/libox.pc
	$(info GEN  $@)
	$(Q)$(MKDIR) $(dir $@)
	$(Q)cat $< | sed "s+%PREFIX%+$(INSTALL_PREFIX)+" | sed "s+%LIB_ARCH%+$(LIB_ARCH)+" | sed "s+%LIBS%+$(LIBS)+" | sed "s+%VERSION%+$(OX_VERSION)+" > $@

binary: $(TARGETS)

basic: binary $(BASIC_TARGETS)

# Generate mo file
define gen_mo =
$(O)/share/locale/$(1)/LC_MESSAGES/ox.mo: locale/$(1).po
	$$(info GEN  $$@)
	$(Q)$(MKDIR) $(O)/share/locale/$(1)/LC_MESSAGES
	$(Q)msgfmt $$< -o $$@
endef

# mo files
$(foreach l,$(LANGS),\
	$(eval $(call gen_mo,$(l))))

# Install OX binary files
install-binary: binary uninstall-binary
	$(info INSTALL BINARY FILES)
	$(Q)$(MKDIR) -m 755 $(INSTALL_PREFIX)/bin
	$(Q)install -m 755 -s $(OX) $(INSTALL_PREFIX)/bin
	$(Q)install -m 755 -s $(OX_WRAPPER) $(INSTALL_PREFIX)/bin
ifeq ($(OS),linux)
	$(Q)install -m 644 -T -s $(LIBOX_DLIB) $(INSTALL_PREFIX)/$(LIB_ARCH)/libox.so.$(SO_VERSION)
	$(Q)ln -s libox.so.$(SO_VERSION) $(INSTALL_PREFIX)/$(LIB_ARCH)/libox.so
	$(Q)ldconfig
else
	$(Q)install -m 755 -s $(OX_WIN) $(INSTALL_PREFIX)/bin
	$(Q)install -m 644 -T -s $(LIBOX_DLIB) $(INSTALL_PREFIX)/bin/libox-$(SO_VERSION)$(DLIB_SUFFIX)
	$(Q)install -m 644 -T $(LIBOX_DLIB_A) $(INSTALL_PREFIX)/$(LIB_ARCH)/libox$(DLIB_SUFFIX).a
endif
	$(Q)$(foreach l,$(LANGS),\
		$(MKDIR) $(INSTALL_PREFIX)/share/locale/$(l)/LC_MESSAGES;\
		install -T $(O)/share/locale/$(l)/LC_MESSAGES/ox.mo $(INSTALL_PREFIX)/share/locale/$(l)/LC_MESSAGES/ox.mo;)
	$(Q)install -m 644 $(LIBOX_SLIB) $(INSTALL_PREFIX)/$(LIB_ARCH)
	$(Q)install -m 644 include/ox.h $(INSTALL_PREFIX)/include
	$(Q)$(MKDIR) -m 755 $(INSTALL_PREFIX)/include/ox
	$(Q)install -m 644 include/ox/*.h $(INSTALL_PREFIX)/include/ox
	$(Q)install -m 644 $(O)/$(LIB_ARCH)/pkgconfig/libox.pc $(INSTALL_PREFIX)/$(LIB_ARCH)/pkgconfig

# Uninstall binary files
uninstall-binary:
	$(info UNINSTALL BINARY FILES)
	$(Q)$(RM) $(INSTALL_PREFIX)/bin/ox$(EXE_SUFFIX)
	$(Q)$(RM) $(INSTALL_PREFIX)/bin/ox-cli$(EXE_SUFFIX)
ifeq ($(OS),windows)
	$(Q)$(RM) $(INSTALL_PREFIX)/bin/ox-win$(EXE_SUFFIX)
	$(Q)$(RM) $(INSTALL_PREFIX)/bin/libox-$(SO_VERSION)$(DLIB_SUFFIX)
	$(Q)$(RM) $(INSTALL_PREFIX)/$(LIB_ARCH)/libox$(DLIB_SUFFIX).a
else
	$(Q)$(RM) $(INSTALL_PREFIX)/$(LIB_ARCH)/libox.so
	$(Q)$(RM) $(INSTALL_PREFIX)/$(LIB_ARCH)/libox.so.$(SO_VERSION)
	$(Q)ldconfig
endif
	$(Q)$(foreach l,$(LANGS),\
		$(RM) $(INSTALL_PREFIX)/share/locale/$(l)/LC_MESSAGES/ox.mo;)
	$(Q)$(RM) $(INSTALL_PREFIX)/$(LIB_ARCH)/libox$(SLIB_SUFFIX)
	$(Q)$(RM) $(INSTALL_PREFIX)/include/ox.h
	$(Q)$(RMDIR) $(INSTALL_PREFIX)/include/ox
	$(Q)$(RM) $(INSTALL_PREFIX)/$(LIB_ARCH)/pkgconfig/libox.pc

# Install basic files
install-basic: basic uninstall-basic install-binary
	$(info INSTALL BASIC FILES)
	$(Q)$(call install_packages,$(BASIC_PACKAGES))

# Uninstall basic files
uninstall-basic: uninstall-binary
	$(info UNINSTALL BASIC FILES)
	$(Q)$(call uninstall_packages,$(BASIC_PACKAGES))

ifeq ($(HAVE_OX_EXE),1)

# Build environemnt packages
env: basic $(ENV_TARGETS)
	$(GEN  DOCUMENT)
	$(Q)$(OX_EXE) -r doc -o $(O)/share/ox/doc/md/builtin src/lib/*.c
	$(Q)$(MKDIR) $(O)/share/ox/doc/md/ox/zh
	$(Q)install -m 644 doc/zh/* $(O)/share/ox/doc/md/ox/zh
	$(Q)$(MKDIR) $(O)/share/ox/doc/md/ox/en
	$(Q)install -m 644 doc/en/* $(O)/share/ox/doc/md/ox/en

# Install environemt packages
install-env: env uninstall-env
	$(info INSTALL ENVIRONMENT FILES)
	$(Q)$(call install_packages,$(ENV_PACKAGES))
	$(Q)$(MKDIR) $(INSTALL_PREFIX)/share/ox/doc/md/builtin
	$(Q)install -m 644 $(O)/share/ox/doc/md/builtin/* $(INSTALL_PREFIX)/share/ox/doc/md/builtin
	$(Q)$(MKDIR) $(INSTALL_PREFIX)/share/ox/server
	$(Q)install -m 644 -T $(O)/share/ox/server/main.ox $(INSTALL_PREFIX)/share/ox/server/main.ox

# Uninstall environment packages
uninstall-env:
	$(info UNINSTALL ENVIRONMENT FILES)
	$(Q)$(call uninstall_packages,$(ENV_PACKAGES))
	$(Q)$(RMDIR) $(INSTALL_PREFIX)/share/ox/doc/md/builtin
	$(Q)$(RM) $(INSTALL_PREFIX)/share/ox/server/main.ox

# Generate plist.ox
ifeq ($(HAVE_OX_EXE),1)
PLIST := $(O_REAL)/plist.ox
PLIST_PARAMS := -l $(LIB_ARCH)

ifeq ($(OS),windows)
PLIST_PARAMS += -r $(shell cygpath -u $(INSTALL_PREFIX))
else
PLIST_PARAMS += -r $(INSTALL_PREFIX)
endif

ifeq ($(INTERNAL_PKGS),1)
PLIST_PARAMS += -i
endif

$(PLIST): build/plist.ox
	$(info GEN  $@)
	$(Q)CC=$(CC) PC=$(PC) TARGET=$(TARGET) HOMEPAGE=$(HOMEPAGE) MAINTAINER="$(MAINTAINER)" $(OX_EXE) --log a build/plist.ox $(PLIST_PARAMS) -o $@

PB_PARAMS += -s plist=$(PLIST) -s "homepage=$(HOMEPAGE)" -s "maintainer=$(MAINTAINER)"
endif #HAVE_OX_EXE

# Use pb to build the package
define pb_build =
$(1)_PKG_NAME := $$(shell CC=$(CC) PC=$(PKGCONFIG) TARGET=$(TARGET) HOMEPAGE=$(HOMEPAGE) MAINTAINER="$(MAINTAINER)" $(OX_EXE) build/package.ox pkg/$(1)/build.ox -n)
ifeq ($(UPDATE_PO),1)
$(1)_POT := $$(wildcard pkg/$(1)/locale/$(1).pot)
$(1)_PO := $$(wildcard pkg/$(1)/locale/*.po)
ifneq ($$($(1)_PO),)
$$($(1)_PO) $$($(1)_POT): $$(wildcard pkg/$(1)/*.c pkg/$(1)/*.ox) $(PLIST)
	$$(info GEN  $$($(1)_POT) $$($(1)_PO))
	$(Q)PLIST=$(PLIST) $(OX_EXE) -r pb --ox "$(OX_EXE)" $(PB_PARAMS) -C pkg/$(1) --update-text
endif
endif
$(O)/oxp/$$($(1)_PKG_NAME): $$(wildcard pkg/$(1)/*.c pkg/$(1)/*.ox) $$($(1)_PO) $(PLIST)
	$$(info GEN  PACKAGE $(1))
	$(Q)$(MKDIR) $(O)/pb/$(1)
	$(Q)PLIST=$(PLIST) $(OX_EXE) -r pb --ox "$(OX_EXE)" $(PB_PARAMS) -C pkg/$(1) -o $(O)/pb/$(1) -p $(CLANG_TARGET) --cc $(CC) --pc $(PKGCONFIG) --cflags "$(OX_PB_CFLAGS) -I$(TOP)/src/lib -I$(TOP)/include -I." --libs "$(OX_PB_LIBS)" $$($(1)_PB_PARAMS)
	$(Q)$(MKDIR) $(O)/oxp
	$(Q)cp $(O)/pb/$(1)/$$($(1)_PKG_NAME) $$@
PKG_TARGETS += $(O)/oxp/$$($(1)_PKG_NAME)
endef

$(foreach p,$(PACKAGES),$(eval $(call pb_build,$(p))))

# Package list
$(O)/oxp/package_list.ox: $(PKG_TARGETS)
	$(info GEN  PACKAGE LIST)
	$(Q)$(OX_EXE) --log a build/syspkg.ox -r $(INSTALL_PREFIX) -o $(O)
	$(Q)$(OX_EXE) -r pm -t $(TARGET) --split 26214400 --gen-pl $(O)/oxp/*.oxp > $@

# Build all the packages.
packages: env $(O)/oxp/package_list.ox

# Install packages.
install-packages: packages
	$(info INSTALL PACKAGES)
	$(Q)$(MKDIR) $(INSTALL_PREFIX)/share/ox/pkg/$(TARGET)/ox
	$(Q)install -m 644 -T $(O)/pb/ox/package.ox $(INSTALL_PREFIX)/share/ox/pkg/$(TARGET)/ox/package.ox
	$(Q)$(MKDIR) $(INSTALL_PREFIX)/share/ox/pkg/$(TARGET)/ox_devel
	$(Q)install -m 644 -T $(O)/pb/ox_devel/package.ox $(INSTALL_PREFIX)/share/ox/pkg/$(TARGET)/ox_devel/package.ox
	$(Q)$(OX_EXE) -r pm -t $(TARGET) -S $(O)/oxp -sfu $(BASIC_PACKAGES) $(ENV_PACKAGES) ox ox_devel

endif #HAVE_OX_EXE

# Uninstall all
uninstall-all:
	$(info UNINSTALL ALL)
	$(Q)$(RM) $(INSTALL_PREFIX)/bin/ox$(EXE_SUFFIX)
	$(Q)$(RM) $(INSTALL_PREFIX)/bin/ox-cli$(EXE_SUFFIX)
ifeq ($(OS),windows)
	$(Q)$(RM) $(INSTALL_PREFIX)/bin/ox-win$(EXE_SUFFIX)
	$(Q)$(RM) $(INSTALL_PREFIX)/bin/libox-$(SO_VERSION)$(DLIB_SUFFIX)
	$(Q)$(RM) $(INSTALL_PREFIX)/$(LIB_ARCH)/libox$(DLIB_SUFFIX).a
else
	$(Q)$(RM) $(INSTALL_PREFIX)/$(LIB_ARCH)/libox.so
	$(Q)$(RM) $(INSTALL_PREFIX)/$(LIB_ARCH)/libox.so.$(SO_VERSION)
endif
	$(Q)$(RM) $(INSTALL_PREFIX)/$(LIB_ARCH)/libox$(SLIB_SUFFIX)
	$(Q)$(foreach l,$(LANGS),\
		$(RM) $(INSTALL_PREFIX)/share/locale/$(l)/LC_MESSAGES/ox.mo;)
	$(Q)$(RMDIR) $(INSTALL_PREFIX)/$(LIB_ARCH)/ox
	$(Q)$(RMDIR) $(INSTALL_PREFIX)/share/ox
	$(Q)$(RM) $(INSTALL_PREFIX)/include/ox.h
	$(Q)$(RMDIR) $(INSTALL_PREFIX)/include/ox
	$(Q)$(RM) $(INSTALL_PREFIX)/$(LIB_ARCH)/pkgconfig/libox.pc

ifeq ($(OS),linux)
# deb package
deb: packages
	$(info DEB  build)
	$(Q)$(MKDIR) $(O)/deb/DEBIAN
	$(Q)$(MKDIR) $(O)/deb/usr/bin
	$(Q)$(MKDIR) $(O)/deb/usr/$(LIB_ARCH)
	$(Q)$(MKDIR) $(O)/deb/usr/share/ox/pkg/all
	$(Q)$(MKDIR) $(O)/deb/usr/share/ox/pkg/$(TARGET)
	$(Q)$(O)/bin/ox.sh -r pm -t $(TARGET) -S $(O)/oxp -I $(O)/deb/usr --libdir $(LIB_ARCH) --no-dep -s $(DIST_PACKAGES)
	$(Q)install -m 755 -s $(OX_WRAPPER) $(O)/deb/usr/bin
	$(Q)sed s/%VERSION%/$(OX_VERSION)/ build/deb/control | sed s/%ARCH%/$(DEB_ARCH)/ > $(O)/deb/DEBIAN/control
	$(Q)dpkg-deb -b --root-owner-group $(O)/deb $(O)/ox_$(OX_VERSION)_$(DEB_ARCH).deb
	$(Q)$(RMDIR) $(O)/deb
endif

ifeq ($(OS),windows)
wininst:
	$(info GEN  WINDOWS INSTALLER)
	$(Q)$(OX_EXE) --log a build/wininst.ox -v $(OX_VERSION) -o $(O)
endif

# Create archive
archive:
	$(info ARCHIVE)
	$(Q)git archive --format=tar.gz --prefix=ox-$(OX_VERSION)/ HEAD > ox-$(OX_VERSION).tar.gz

# Get document source files
define doc_srcs =
$(filter-out pkg/$(1)/build.ox,$(wildcard pkg/$(1)/*.c pkg/$(1)/*.h pkg/$(1)/*.ox))
endef

# Clean target and object files
clean:
	$(info CLEAN $(TARGETS) $(ENV_TARGETS) $(OBJS))
	$(Q)$(RM) $(TARGETS) $(ENV_TARGETS) $(OBJS)
	$(Q)$(RMDIR) $(O)/pb $(O)/oxp

# Clean all intermediate files
dist-clean:
	$(info DIST CLEAN)
	$(Q)$(RMDIR) $(O)

.PHONY: clean dist-clean lib test binary basic env deb wininst archive packages
.PHONY: install-binary uninstall-binary install-basic uninstall-basic install-env uninstall-env install-packages uninstall-all
