ifndef SHOAL_PATH
$(error SHOAL_PATH not set in env -- must be set to the absolute path of \
of the repository root. Did you source init.sh?)
endif

ifndef SHOAL_HLS_PATH
$(error SHOAL_HLS_PATH not set in env -- must be set to the absolute path of \
of the Vivado HLS directory. Did you source init.sh?)
endif

ifndef SHOAL_PART
$(error SHOAL_PART not set in env -- must be set to the FPGA part number Did \
you source init.sh?)
endif

# Asserts that the specified variable exists. It can be used as a prerequisite 
# for running other targets. E.g. adding guard-FOO to a target enforces that 
# the variable FOO is defined before running the target.

guard-%:
	@ if [ "${${*}}" = "" ]; then \
		echo "Variable $* not set"; \
		exit 1; \
	fi
