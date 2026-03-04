################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
src/Extra/MSP430_GUI/%.obj: ../src/Extra/MSP430_GUI/%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: MSP430 Compiler'
	"D:/CCS Texas/ccs/tools/compiler/ti-cgt-msp430_21.6.1.LTS/bin/cl430" -vmspx --data_model=restricted -O3 --use_hw_mpy=F5 --include_path="D:/CCS Texas/ccs/ccs_base/msp430/include" --include_path="C:/Users/User/workspace_ccstheia/CCS Project" --include_path="C:/Users/User/workspace_ccstheia/CCS Project/include" --include_path="D:/CCS Texas/ccs/tools/compiler/ti-cgt-msp430_21.6.1.LTS/include" --advice:power="none" --advice:hw_config=all --define=__MSP430FR6989__ --define=_MPU_ENABLE --define=__ENABLE_GUI_COMM=1 --define=GLOBAL_Q=8 -g --printf_support=minimal --diag_warning=225 --diag_wrap=off --display_error_number --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU40 --preproc_with_compile --preproc_dependency="src/Extra/MSP430_GUI/$(basename $(<F)).d_raw" --obj_directory="src/Extra/MSP430_GUI" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


