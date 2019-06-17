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

################################################################################
# Variables
################################################################################

ifndef SHOAL_PATH
$(error SHOAL_PATH not set in env -- must be set to the absolute path of \
of the repository root. Did you source init.sh?)
endif

ifndef SHOAL_HLS_PATH
$(error SHOAL_HLS_PATH not set in env -- must be set to the absolute path of \
of the Vivado HLS include/ directory. Did you source init.sh?)
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

galapagos_files := one_node

obj = $(shell find $(test_build_dir) -name '*.o' -printf '%f\n' | \
sort -k 1nr | cut -f2-)
dep = $(obj:%.o=$(test_build_dir)/%.d)

GAScore_build_dirs := $(shell find $(SHOAL_PATH) -type d -name 'build' -not -path "$(SHOAL_PATH)/GAScore/*")

CC = /usr/bin/g++-7
CFLAGS = -g -Wall -O0 -I$(include_dir) -I$(GAScore_dir) -isystem $(SHOAL_HLS_PATH)\
	-I$(GALAPAGOS_PATH)/middleware/CPP_lib/Galapagos_lib -I$(GALAPAGOS_PATH)/middleware/include\
	-Wno-unused-value -Wno-unused-variable -Wno-comment \
	-Wno-unused-but-set-variable -Wno-unused-function -MMD -MP -pthread -std=c++17
LIBS = -lpthread -lrt
APP_LIBS = -L$(SHOAL_PATH)/build -lTHeGASnet

GALAPAGOS_LIBS = -lboost_thread -lboost_system -lpthread -L$(SHOAL_PATH)/build -lTHeGASnet

MYFLAGS += $(foreach N,$(NUM_RANGE),-DMYVAR$N=$(MYVAR$N) )

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
	rm $(SHOAL_PATH)/build/libTHeGASnet.a
	ar -cr $(SHOAL_PATH)/build/libTHeGASnet.a $(SHOAL_PATH)/build/*.o

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

define make-galapagos-executable
galapagos-$1:  $(test_build_dir)/$1.o $(test_build_dir)/$1_kern.o guard-KERNELS
	$(eval NUM_RANGE := $(shell seq 0 $(KERNELS)))
	$(eval WRAP += $(foreach N,$(NUM_RANGE),-Wl,--wrap=kern$N ))
	$(CC) $(CFLAGS) -o $(test_bin_dir)/$1 $(test_build_dir)/$1.o \
		$(test_build_dir)/$1_kern.o $(WRAP) $(GALAPAGOS_LIBS)
endef
$(foreach file, $(galapagos_files),$(eval $(call make-galapagos-executable,$(file))))

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
$(test_build_dir)/$1.o: $(test_dir)/$1.cpp
	$(CC) $(CFLAGS) -o $(test_build_dir)/$1.o -c $(test_dir)/$1.cpp $(LIBS)

$(test_build_dir)/$1_kern.o: $(test_dir)/$1_kern.cpp
	$(CC) $(CFLAGS) -o $(test_build_dir)/$1_kern.o -c $(test_dir)/$1_kern.cpp $(LIBS)
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
