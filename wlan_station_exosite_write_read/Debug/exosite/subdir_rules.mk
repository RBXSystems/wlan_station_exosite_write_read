################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
exosite/exosite.obj: C:/Users/richa/Documents/GitHub/wlan_station_exosite_write_read/wlan_station_exosite_write_read/exosite/exosite.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ccsv6/tools/compiler/ti-cgt-arm_5.2.7/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 --abi=eabi -me -O1 --opt_for_speed=2 --include_path="C:/Users/richa/Documents/GitHub/wlan_station_exosite_write_read/wlan_station_exosite_write_read/exosite" --include_path="C:/ti/ccsv6/tools/compiler/ti-cgt-arm_5.2.7/include" --include_path="C:/Users/richa/Documents/GitHub/wlan_station_exosite_write_read/wlan_station_exosite_write_read" --include_path="C:/ti/TivaWare_C_Series-2.1.2.111" --include_path="C:/ti/TivaWare_C_Series-2.1.2.111/examples/boards/ek-tm4c1294xl" --include_path="C:/ti/CC3100SDK_1.2.0/cc3100-sdk/examples/common" --include_path="C:/ti/CC3100SDK_1.2.0/cc3100-sdk/simplelink/include" --include_path="C:/ti/CC3100SDK_1.2.0/cc3100-sdk/simplelink/source" --include_path="C:/ti/CC3100SDK_1.2.0/cc3100-sdk/platform/tiva-c-connected-launchpad" --include_path="C:/ti/TivaWare_C_Series-2.1.2.111/third_party/lwip-1.4.1/src/include" -g --gcc --define=TARGET_IS_TM4C129_RA0 --define=PART_TM4C1294NCPDT --define=ccs="ccs" --define=UART_BUFFERED --diag_wrap=off --display_error_number --diag_warning=225 --preproc_with_compile --preproc_dependency="exosite/exosite.pp" --obj_directory="exosite" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

exosite/exosite_pal.obj: C:/Users/richa/Documents/GitHub/wlan_station_exosite_write_read/wlan_station_exosite_write_read/exosite/exosite_pal.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ccsv6/tools/compiler/ti-cgt-arm_5.2.7/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 --abi=eabi -me -O1 --opt_for_speed=2 --include_path="C:/Users/richa/Documents/GitHub/wlan_station_exosite_write_read/wlan_station_exosite_write_read/exosite" --include_path="C:/ti/ccsv6/tools/compiler/ti-cgt-arm_5.2.7/include" --include_path="C:/Users/richa/Documents/GitHub/wlan_station_exosite_write_read/wlan_station_exosite_write_read" --include_path="C:/ti/TivaWare_C_Series-2.1.2.111" --include_path="C:/ti/TivaWare_C_Series-2.1.2.111/examples/boards/ek-tm4c1294xl" --include_path="C:/ti/CC3100SDK_1.2.0/cc3100-sdk/examples/common" --include_path="C:/ti/CC3100SDK_1.2.0/cc3100-sdk/simplelink/include" --include_path="C:/ti/CC3100SDK_1.2.0/cc3100-sdk/simplelink/source" --include_path="C:/ti/CC3100SDK_1.2.0/cc3100-sdk/platform/tiva-c-connected-launchpad" --include_path="C:/ti/TivaWare_C_Series-2.1.2.111/third_party/lwip-1.4.1/src/include" -g --gcc --define=TARGET_IS_TM4C129_RA0 --define=PART_TM4C1294NCPDT --define=ccs="ccs" --define=UART_BUFFERED --diag_wrap=off --display_error_number --diag_warning=225 --preproc_with_compile --preproc_dependency="exosite/exosite_pal.pp" --obj_directory="exosite" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '


