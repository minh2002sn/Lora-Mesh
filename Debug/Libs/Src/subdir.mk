################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Libs/Src/Command_Line.c \
../Libs/Src/Frame_Sync.c \
../Libs/Src/button.c \
../Libs/Src/ring_buffer.c \
../Libs/Src/uart.c 

OBJS += \
./Libs/Src/Command_Line.o \
./Libs/Src/Frame_Sync.o \
./Libs/Src/button.o \
./Libs/Src/ring_buffer.o \
./Libs/Src/uart.o 

C_DEPS += \
./Libs/Src/Command_Line.d \
./Libs/Src/Frame_Sync.d \
./Libs/Src/button.d \
./Libs/Src/ring_buffer.d \
./Libs/Src/uart.d 


# Each subdirectory must supply rules for building sources it contributes
Libs/Src/%.o Libs/Src/%.su: ../Libs/Src/%.c Libs/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F411xE -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I"D:/A_Minh/B_Big Projects/Lora Mesh/Firmware/Lora Mesh_1.0/Libs/Inc" -I"D:/A_Minh/B_Big Projects/Lora Mesh/Firmware/Lora Mesh_1.0/Libs/CRC/Inc" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Libs-2f-Src

clean-Libs-2f-Src:
	-$(RM) ./Libs/Src/Command_Line.d ./Libs/Src/Command_Line.o ./Libs/Src/Command_Line.su ./Libs/Src/Frame_Sync.d ./Libs/Src/Frame_Sync.o ./Libs/Src/Frame_Sync.su ./Libs/Src/button.d ./Libs/Src/button.o ./Libs/Src/button.su ./Libs/Src/ring_buffer.d ./Libs/Src/ring_buffer.o ./Libs/Src/ring_buffer.su ./Libs/Src/uart.d ./Libs/Src/uart.o ./Libs/Src/uart.su

.PHONY: clean-Libs-2f-Src

