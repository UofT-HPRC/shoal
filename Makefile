################################################################################
# Include
################################################################################

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

app_dir=$(SHOAL_PATH)/tests
app_build_dir=$(app_dir)/build
app_bin_dir=$(app_dir)/build/bin

lib_files := active_messages main_wrapper platforms thegasnet_core \
	thegasnet_globals

app_files := commtest_gascorev2
commtest_gascorev2_args := --tgn_myip=192.168.1.103 \
	--tgn_config=$(SHOAL_PATH)/src/gasnet_config --tgn_nopin

test_files := queue_test

obj = $(shell find $(test_build_dir) -name '*.o' -printf '%f\n' | \
sort -k 1nr | cut -f2-)
dep = $(obj:%.o=$(test_build_dir)/%.d)

SW_build_dirs := $(shell find $(SHOAL_PATH) -type d -name 'build' -not -path "$(SHOAL_PATH)/GAScore/*")

CC = g++
CFLAGS = -g -Wall -O0 -I$(include_dir) -I$(GAScore_dir) -I$(SHOAL_HLS_PATH)\
	-Wno-unused-value -Wno-unused-variable -Wno-comment \
	-Wno-unused-but-set-variable -Wno-unused-function -MMD -MP -pthread -std=c++11
LIBS = -Wl,--wrap=main -lpthread -lrt
APP_LIBS = -L$(SHOAL_PATH)/build -lTHeGASnet

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
	# rm $(SHOAL_PATH)/build/libTHeGASnet.a
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

clean-2: clean-1
	@$(foreach path,$(SW_build_dirs),find $(path) -type f -delete)

purge:
	@rm -rf ~/.shoal
	@sed -i '/added by shoal/d' ~/.bashrc
