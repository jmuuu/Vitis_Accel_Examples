#
# Copyright 2019-2021 Xilinx, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# makefile-generator v1.0.3
#

############################## Help Section ##############################
.PHONY: help

help::
	$(ECHO) "Makefile Usage:"
	$(ECHO) "  make all TARGET=<sw_emu/hw_emu/hw> DEVICE=<FPGA platform>"
	$(ECHO) "      Command to generate the design for specified Target and Shell."
	$(ECHO) ""
	$(ECHO) "  make clean "
	$(ECHO) "      Command to remove the generated non-hardware files."
	$(ECHO) ""
	$(ECHO) "  make cleanall"
	$(ECHO) "      Command to remove all the generated files."
	$(ECHO) ""
	$(ECHO) "  make test DEVICE=<FPGA platform>"
	$(ECHO) "      Command to run the application. This is same as 'run' target but does not have any makefile dependency."
	$(ECHO) ""
	$(ECHO) "  make run TARGET=<sw_emu/hw_emu/hw> DEVICE=<FPGA platform>"
	$(ECHO) "      Command to run application in emulation."
	$(ECHO) ""
	$(ECHO) "  make build TARGET=<sw_emu/hw_emu/hw> DEVICE=<FPGA platform>"
	$(ECHO) "      Command to build xclbin application."
	$(ECHO) ""
	$(ECHO) "  make host"
	$(ECHO) "      Command to build host application."
	$(ECHO) ""

############################## Setting up Project Variables ##############################
# Points to top directory of Git repository
MK_PATH := $(abspath $(lastword $(MAKEFILE_LIST)))
COMMON_REPO ?= $(shell bash -c 'export MK_PATH=$(MK_PATH); echo $${MK_PATH%library_examples/gemm/*}')
PWD = $(shell readlink -f .)
XF_PROJ_ROOT = $(shell readlink -f $(COMMON_REPO))

TARGET := hw
HOST_ARCH := x86
SYSROOT := 

include ./utils.mk

XSA := 
ifneq ($(DEVICE), )
XSA := $(call device2xsa, $(DEVICE))
endif
TEMP_DIR := ./_x.$(TARGET).$(XSA)
BUILD_DIR := ./build_dir.$(TARGET).$(XSA)

# SoC variables
RUN_APP_SCRIPT = ./run_app.sh
PACKAGE_OUT = ./package.$(TARGET)

LAUNCH_EMULATOR = $(PACKAGE_OUT)/launch_$(TARGET).sh
RESULT_STRING = TEST PASSED

VPP := v++
VPP_PFLAGS := 
CMD_ARGS = $(BUILD_DIR)/blas.xclbin 64 64 64 64 64 64 64 1 0

include $(XF_PROJ_ROOT)/common/includes/opencl/opencl.mk
CXXFLAGS += $(opencl_CXXFLAGS) -Wall -O0 -g -std=c++1y
LDFLAGS += $(opencl_LDFLAGS)

ifeq ($(findstring _u2_, $(DEVICE)), _u2_)
$(error [ERROR]: This example is not supported for $(DEVICE).)
endif
ifeq ($(findstring u25, $(DEVICE)), u25)
$(error [ERROR]: This example is not supported for $(DEVICE).)
endif
ifeq ($(findstring u30, $(DEVICE)), u30)
$(error [ERROR]: This example is not supported for $(DEVICE).)
endif
ifeq ($(findstring u50, $(DEVICE)), u50)
$(error [ERROR]: This example is not supported for $(DEVICE).)
endif
ifeq ($(findstring zc, $(DEVICE)), zc)
$(error [ERROR]: This example is not supported for $(DEVICE).)
endif
ifeq ($(findstring vck, $(DEVICE)), vck)
$(error [ERROR]: This example is not supported for $(DEVICE).)
endif
ifeq ($(findstring nodma, $(DEVICE)), nodma)
$(error [ERROR]: This example is not supported for $(DEVICE).)
endif
ifeq ($(findstring u250, $(DEVICE)), u250)
$(error [ERROR]: This example is not supported for $(DEVICE).)
endif

ifneq ($(findstring aws-vu9p-f1, $(DEVICE)), aws-vu9p-f1)
$(warning [WARNING]: This example has not been tested for $(DEVICE). It may or may not work.)
endif

############################## Setting up Host Variables ##############################
#Include Required Host Source Files
CXXFLAGS += -I./src
CXXFLAGS += -I./include/hw/xf_blas/helpers/utils
CXXFLAGS += -I./include/
CXXFLAGS += -I./include/sw
CXXFLAGS += -I./include/sw/xcl2
HOST_SRCS += ./src/sw/api_gemm.cpp ./src/sw/xcl2/xcl2.cpp 
# Host compiler global settings
CXXFLAGS += -fmessage-length=0 -O3 -Wall -Wno-unknown-pragmas -Wno-unused-label -Wno-uninitialized -Wno-unused-value
LDFLAGS += -lrt -lstdc++ 

############################## Setting up Kernel Variables ##############################
# Kernel compiler global settings
VPP_FLAGS += 
VPP_FLAGS += -t $(TARGET) --platform $(DEVICE) --save-temps 
ifneq ($(TARGET), hw)
	VPP_FLAGS += -g
endif
VPP_FLAGS_blasKernel +=  --kernel_frequency 200

CXXFLAGS += -DBLAS_ddrWidth=16 -DBLAS_numKernels=1 -DBLAS_dataType=float -DBLAS_gemmMBlocks=4 -DBLAS_gemmKBlocks=4 -DBLAS_gemmNBlocks=4 -DBLAS_argPipeline=2 -DBLAS_instructionSizeBytes=64 -DBLAS_XdataType=float -DBLAS_argInstrWidth=1 -DBLAS_runGemm=1

# Kernel linker flags
VPP_LDFLAGS_blas += --config ./opts.cfg --kernel_frequency 200

EXECUTABLE = ./api_gemm.exe
EMCONFIG_DIR = $(TEMP_DIR)

VPP_FLAGS += -I./include/hw
VPP_FLAGS += -I./include/hw/xf_blas
VPP_FLAGS += -I./include/hw/xf_blas/gemm
VPP_FLAGS += -I./include/hw/xf_blas/helpers/utils
VPP_FLAGS += -I./include

VPP_FLAGS += -DBLAS_ddrWidth=16 -DBLAS_XddrWidth=16 -DBLAS_numKernels=1 -DBLAS_dataType=float -DBLAS_gemmMBlocks=4 -DBLAS_gemmKBlocks=4 -DBLAS_gemmNBlocks=4 -DBLAS_argPipeline=2 -DBLAS_dataEqIntType=float -DBLAS_XdataType=float -DBLAS_argInstrWidth=1 -DBLAS_numInstr=64 -DTEST_MEMCPY=0 -DBLAS_CACHE=0 -DBLAS_XVEC=0 -DBLAS_runGemm=1 -DAP_INT_MAX_W=1026 

############################## Declaring Binary Containers ##############################
BINARY_CONTAINERS += $(BUILD_DIR)/blas.xclbin
BINARY_CONTAINER_blas_OBJS += $(TEMP_DIR)/blasKernel.xo

############################## Setting Targets ##############################
CP = cp -rf

.PHONY: all clean cleanall docs emconfig
all: check-devices $(EXECUTABLE) $(BINARY_CONTAINERS) emconfig

.PHONY: host
host: $(EXECUTABLE)

.PHONY: build
build: check-vitis $(BINARY_CONTAINERS)

.PHONY: xclbin
xclbin: build

############################## Setting Rules for Binary Containers (Building Kernels) ##############################
$(TEMP_DIR)/blasKernel.xo: ./src/hw/kernel.cpp
	mkdir -p $(TEMP_DIR)
	$(VPP) $(VPP_FLAGS) $(VPP_FLAGS_blasKernel) -c -k blasKernel --temp_dir $(TEMP_DIR)  -I'$(<D)' -o'$@' '$<'
$(BUILD_DIR)/blas.xclbin: $(BINARY_CONTAINER_blas_OBJS)
	mkdir -p $(BUILD_DIR)
	$(VPP) $(VPP_FLAGS) -l $(VPP_LDFLAGS) --temp_dir $(TEMP_DIR) $(VPP_LDFLAGS_blas) -o'$(BUILD_DIR)/blas.link.xclbin' $(+)
	$(VPP) -p $(BUILD_DIR)/blas.link.xclbin -t $(TARGET) --platform $(DEVICE) --package.out_dir $(PACKAGE_OUT) -o $(BUILD_DIR)/blas.xclbin

############################## Setting Rules for Host (Building Host Executable) ##############################
$(EXECUTABLE): $(HOST_SRCS) | check-xrt
		$(CXX) -o $@ $^ $(CXXFLAGS) $(LDFLAGS)

emconfig:$(EMCONFIG_DIR)/emconfig.json
$(EMCONFIG_DIR)/emconfig.json:
	emconfigutil --platform $(DEVICE) --od $(EMCONFIG_DIR)

############################## Setting Essential Checks and Running Rules ##############################
run: all
ifeq ($(TARGET),$(filter $(TARGET),sw_emu hw_emu))
	$(CP) $(EMCONFIG_DIR)/emconfig.json .
	XCL_EMULATION_MODE=$(TARGET) $(EXECUTABLE) $(CMD_ARGS)
else
	$(EXECUTABLE) $(CMD_ARGS)
endif


.PHONY: test
test: $(EXECUTABLE)
ifeq ($(TARGET),$(filter $(TARGET),sw_emu hw_emu))
	XCL_EMULATION_MODE=$(TARGET) $(EXECUTABLE) $(CMD_ARGS)
else
	$(EXECUTABLE) $(CMD_ARGS)
endif


############################## Cleaning Rules ##############################
# Cleaning stuff
clean:
	-$(RMDIR) $(EXECUTABLE) $(XCLBIN)/{*sw_emu*,*hw_emu*} 
	-$(RMDIR) profile_* TempConfig system_estimate.xtxt *.rpt *.csv 
	-$(RMDIR) src/*.ll *v++* .Xil emconfig.json dltmp* xmltmp* *.log *.jou *.wcfg *.wdb

cleanall: clean
	-$(RMDIR) build_dir*
	-$(RMDIR) package.*
	-$(RMDIR) _x* *xclbin.run_summary qemu-memory-_* emulation _vimage pl* start_simulation.sh *.xclbin

