################################################################################
# Include
################################################################################

include include.mk

################################################################################
# Prologue
################################################################################

MAKEFLAGS += --warn-undefined-variables
SHELL := bash
.SHELLFLAGS := -eu -o pipefail -c
.DEFAULT_GOAL :=
.DELETE_ON_ERROR:
.SUFFIXES:
.SECONDEXPANSION:

################################################################################
# Variables
################################################################################

ifndef SHOAL_PATH
$(error SHOAL_PATH not set in env -- must be set to the absolute path of \
of the repository root. Did you source init.sh?)
endif

ifndef SHOAL_HLS_PATH
$(error SHOAL_HLS_PATH not set in env -- must be set to the absolute path of \
of the Vivado HLS directory. Did you source init.sh?)
endif

include_dir=$(SHOAL_PATH)/include
GAScore_dir=$(SHOAL_PATH)/GAScore/include

src_dir=$(SHOAL_PATH)/src
build_dir=$(SHOAL_PATH)/build

test_dir=$(SHOAL_PATH)/tests
test_build_dir=$(test_dir)/build
test_bin_dir=$(test_dir)/build/bin

app_dir=$(SHOAL_PATH)/apps
app_build_dir=$(app_dir)/build
app_bin_dir=$(app_dir)/build/bin

lib_files := active_messages am_gasnet am_globals shoal_node shoal_kernel

app_files := commtest_gascorev2

test_files := queue_test

galapagos_files := node_1_kern_2 node_2_kern_2 hls_kernel benchmark

obj = $(shell find $(test_build_dir) -name '*.o' -printf '%f\n' | \
sort -k 1nr | cut -f2-)
dep = $(obj:%.o=$(test_build_dir)/%.d)

GAScore_build_dirs := $(shell find $(SHOAL_PATH) -type d -name 'build' -not -path "$(SHOAL_PATH)/GAScore/*")

DEBUG ?= 1
ifeq ($(DEBUG), 0)
OPT = -O2
else
OPT = -g -O0 -DDEBUG
endif

CC = /usr/bin/g++-7
CFLAGS = $(OPT) -Wall -I$(include_dir) -I$(GAScore_dir) -isystem $(SHOAL_HLS_PATH)/$(SHOAL_HLS_VERSION)/include\
	-I$(GALAPAGOS_PATH)/middleware/libGalapagos -I$(GALAPAGOS_PATH)/middleware/include\
	-I$(GALAPAGOS_PATH)/util/spdlog/include\
	-Wno-unused-value -Wno-unused-variable -Wno-comment -Wno-unknown-pragmas\
	-Wno-unused-but-set-variable -Wno-unused-function -MMD -MP -pthread -std=c++17
LIBS = -lpthread -lrt
APP_LIBS = -L$(SHOAL_PATH)/build -lTHeGASnet -lGalapagos

GALAPAGOS_LIBS = -L/usr/local/lib -lboost_thread -lboost_system -lpthread $(APP_LIBS)

MYFLAGS += $(foreach N,$(NUM_RANGE),-DMYVAR$N=$(MYVAR$N) )

K_START ?= 0
K_END ?= 0

################################################################################
# Body
################################################################################

.PHONY: init clean

#-------------------------------------------------------------------------------
# Main
#-------------------------------------------------------------------------------

init:
	./init.sh

lib_modules=$(patsubst %, lib-%, $(lib_files))
lib: $(lib_modules)
	rm -f $(SHOAL_PATH)/build/libTHeGASnet.a
	ar -cr $(SHOAL_PATH)/build/libTHeGASnet.a $(SHOAL_PATH)/build/*.o
	cp $(GALAPAGOS_PATH)/middleware/libGalapagos/libGalapagos.a $(SHOAL_PATH)/build/

define make-libs
lib-$1: $(src_dir)/$1.cpp
	$(CC) $(CFLAGS) -o $(build_dir)/$1.o -c $(src_dir)/$1.cpp $(LIBS)
endef
$(foreach file, $(lib_files),$(eval $(call make-libs,$(file))))

#-------------------------------------------------------------------------------
# Executables
#-------------------------------------------------------------------------------

app_modules=$(patsubst %, app-%, $(app_files))
app: $(app_modules)

define make-app-executable
app-$1:  $(app_dir)/$1.cpp
	$(CC) $(CFLAGS) -o $(app_bin_dir)/$1 $(app_dir)/$1.cpp $(LIBS) $(APP_LIBS)
	# $(app_bin_dir)/$1 $($1_args)
endef
$(foreach file, $(app_files),$(eval $(call make-app-executable,$(file))))

test_modules=$(patsubst %, test-%, $(test_files))
test: $(test_modules)

define make-test-executable
test-$1:  $(test_build_dir)/$1.o
	$(CC) $(CFLAGS) -o $(test_bin_dir)/$1 $(test_build_dir)/$1.o
	$(test_bin_dir)/$1 $1_args
endef
$(foreach file, $(test_files),$(eval $(call make-test-executable,$(file))))

galapagos_modules=$(patsubst %, galapagos-%, $(galapagos_files))
galapagos: $(galapagos_modules)

ifeq ($(BUILD_SUFFIX), _0)
KERN_BUILD = 0
else ifeq ($(BUILD_SUFFIX), _1)
KERN_BUILD = 1
else
KERN_BUILD = -1
endif

ifeq ($(MODE),x86)
# .SECONDEXPANSION is needed here to evaluate NUM_RANGE and WRAP in succession.
# The expansion works without this special target if the body isn't inside a
# define.
define make-galapagos-executable
galapagos-$1: NUM_RANGE = $(shell seq $(K_START) $(K_END))
galapagos-$1: WRAP += $$(foreach N,$$(NUM_RANGE),-Wl,--wrap=kern$$N )
galapagos-$1: $(test_build_dir)/$1$(BUILD_SUFFIX).o $(test_build_dir)/$1_main$(BUILD_SUFFIX).o
	@echo $$(WRAP)
	$(CC) $(CFLAGS) -o $(test_bin_dir)/$1$(BUILD_SUFFIX) $(test_build_dir)/$1$(BUILD_SUFFIX).o \
		$(test_build_dir)/$1_main$(BUILD_SUFFIX).o $$(WRAP) $(GALAPAGOS_LIBS)

endef
else ifeq ($(MODE),HLS)
# Kernels fail to synthesize in 2017.2 but work in 2018.1/2018.2/2019.1. Haven't tested others
define make-galapagos-executable
galapagos-$1: guard-KERNEL
	$(SHOAL_PATH)/tests/generate.sh $1$(BUILD_SUFFIX) $(KERNEL)
endef
endif
$(foreach file, $(galapagos_files),$(eval $(call make-galapagos-executable,$(file))))

tmp:
	$(CC) $(CFLAGS) tmp.cpp -o tmp.exe $(GALAPAGOS_LIBS)

#-------------------------------------------------------------------------------
# Object Files
#-------------------------------------------------------------------------------

# define make-test-object-test
# $(test_build_dir)/$1_test.o: $(test_dir)/$1_test.cpp
# 	$(CC) $(CFLAGS) -o $(test_build_dir)/$1_test.o -c $(test_dir)/$1_test.cpp
# endef
# $(foreach file, $(test_files),$(eval $(call make-test-object-test,$(file))))

define make-app-object
$(test_build_dir)/$1.o: $(test_dir)/$1.cpp
	$(CC) $(CFLAGS) -o $(test_build_dir)/$1.o -c $(test_dir)/$1.cpp
endef
$(foreach file, $(app_files),$(eval $(call make-app-object,$(file))))

define make-test-object
$(test_build_dir)/$1.o: $(test_dir)/$1.cpp
	$(CC) $(CFLAGS) -o $(test_build_dir)/$1.o -c $(test_dir)/$1.cpp $(LIBS)
endef
$(foreach file, $(test_files),$(eval $(call make-test-object,$(file))))

define make-galapagos-object
$(test_build_dir)/$1$(BUILD_SUFFIX).o: $(test_dir)/$1.cpp
	$(CC) $(CFLAGS) -o $(test_build_dir)/$1$(BUILD_SUFFIX).o -c $(test_dir)/$1.cpp $(LIBS) -DKERN_BUILD=$(KERN_BUILD)

$(test_build_dir)/$1_main$(BUILD_SUFFIX).o: $(test_dir)/$1_main.cpp
	$(CC) $(CFLAGS) -o $(test_build_dir)/$1_main$(BUILD_SUFFIX).o -c $(test_dir)/$1_main.cpp $(LIBS) -DKERN_BUILD=$(KERN_BUILD)
endef
$(foreach file, $(galapagos_files),$(eval $(call make-galapagos-object,$(file))))

-include $(dep)

#-------------------------------------------------------------------------------
# Cleanup
#-------------------------------------------------------------------------------

clean:
	@echo "Run 'make clean-x' where x is a level"
	@echo "Make Clean levels:"
	@echo "    1. Removes junk (e.g. Vivado log files etc.)"

clean-1:
	@find . -name "vivado*.log" -type f -delete
	@find . -name "vivado*.jou" -type f -delete
	@rm -rf `find -type d -name .Xil`
	@find . -name "vivado_pid*.str" -type f -delete
	@find . -name "hs_err_pid*.log" -type f -delete

clean-2a: clean-1
	@$(foreach path,$(GAScore_build_dirs),find $(path) -type f -delete)

clean-2b: clean-1
	@find $(SHOAL_PATH)/tests/build -type f -exec rm -f {} \;

clean-2c: clean-1
	@find $(SHOAL_PATH)/build -type f -exec rm -f {} \;

purge:
	@rm -rf ~/.shoal
	@sed -i '/added by shoal/d' ~/.bashrc
