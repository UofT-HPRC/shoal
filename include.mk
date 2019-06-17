# Asserts that the specified variable exists. It can be used as a prerequisite 
# for running other targets. E.g. adding guard-FOO to a target enforces that 
# the variable FOO is defined before running the target.

guard-%:
	@ if [ "${${*}}" = "" ]; then \
		echo "Variable $* not set"; \
		exit 1; \
	fi
