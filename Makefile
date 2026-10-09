ifeq ($(OS),Windows_NT)
    VENV_PYTHON := tools/netmap-generator/.venv/Scripts/python
else
    VENV_PYTHON := tools/netmap-generator/.venv/bin/python
endif

NETMAP_INPUT ?= 
NETMAP_OUTPUT ?= ./components/common/include

gen-netmap:
	$(if $(strip $(NETMAP_INPUT)),,$(error NETMAP_INPUT is required. Usage: make gen-netmap NETMAP_INPUT=path/to/file.yaml))
	$(VENV_PYTHON) tools/netmap-generator/netmap-generator.py -i "$(NETMAP_INPUT)" -o "$(NETMAP_OUTPUT)/netmap.h"

.PHONY: gen-netmap