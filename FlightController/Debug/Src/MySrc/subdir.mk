################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Src/MySrc/terminal.c \
../Src/MySrc/terminalHAL.c 

OBJS += \
./Src/MySrc/terminal.o \
./Src/MySrc/terminalHAL.o 

C_DEPS += \
./Src/MySrc/terminal.d \
./Src/MySrc/terminalHAL.d 


# Each subdirectory must supply rules for building sources it contributes
Src/MySrc/%.o: ../Src/MySrc/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU GCC Compiler'
	@echo $(PWD)
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 '-D__weak=__attribute__((weak))' '-D__packed=__attribute__((__packed__))' -DUSE_HAL_DRIVER -DSTM32F429xx -I"C:/Users/owner/Downloads/ProjectLab4-master/ProjectLab4-master/FlightController/Inc" -I"C:/Users/owner/Downloads/ProjectLab4-master/ProjectLab4-master/FlightController/Drivers/STM32F4xx_HAL_Driver/Inc" -I"C:/Users/owner/Downloads/ProjectLab4-master/ProjectLab4-master/FlightController/Drivers/STM32F4xx_HAL_Driver/Inc/Legacy" -I"C:/Users/owner/Downloads/ProjectLab4-master/ProjectLab4-master/FlightController/Middlewares/ST/STM32_USB_Device_Library/Core/Inc" -I"C:/Users/owner/Downloads/ProjectLab4-master/ProjectLab4-master/FlightController/Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc" -I"C:/Users/owner/Downloads/ProjectLab4-master/ProjectLab4-master/FlightController/Drivers/CMSIS/Device/ST/STM32F4xx/Include" -I"C:/Users/owner/Downloads/ProjectLab4-master/ProjectLab4-master/FlightController/Drivers/CMSIS/Include"  -Og -g3 -Wall -fmessage-length=0 -ffunction-sections -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


