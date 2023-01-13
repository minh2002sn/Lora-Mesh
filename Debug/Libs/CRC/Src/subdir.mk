################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Libs/CRC/Src/crc16.c \
../Libs/CRC/Src/crc32.c \
../Libs/CRC/Src/crc8.c \
../Libs/CRC/Src/crcccitt.c \
../Libs/CRC/Src/crcdnp.c \
../Libs/CRC/Src/crckrmit.c \
../Libs/CRC/Src/crcsick.c \
../Libs/CRC/Src/nmea-chk.c 

OBJS += \
./Libs/CRC/Src/crc16.o \
./Libs/CRC/Src/crc32.o \
./Libs/CRC/Src/crc8.o \
./Libs/CRC/Src/crcccitt.o \
./Libs/CRC/Src/crcdnp.o \
./Libs/CRC/Src/crckrmit.o \
./Libs/CRC/Src/crcsick.o \
./Libs/CRC/Src/nmea-chk.o 

C_DEPS += \
./Libs/CRC/Src/crc16.d \
./Libs/CRC/Src/crc32.d \
./Libs/CRC/Src/crc8.d \
./Libs/CRC/Src/crcccitt.d \
./Libs/CRC/Src/crcdnp.d \
./Libs/CRC/Src/crckrmit.d \
./Libs/CRC/Src/crcsick.d \
./Libs/CRC/Src/nmea-chk.d 


# Each subdirectory must supply rules for building sources it contributes
Libs/CRC/Src/%.o Libs/CRC/Src/%.su: ../Libs/CRC/Src/%.c Libs/CRC/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F103xB -c -I../Core/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -I"D:/A_Minh/B_Big Projects/Lora Mesh/Firmware/Lora Mesh_F103/Libs/Inc" -I"D:/A_Minh/B_Big Projects/Lora Mesh/Firmware/Lora Mesh_F103/Libs/CRC/Inc" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Libs-2f-CRC-2f-Src

clean-Libs-2f-CRC-2f-Src:
	-$(RM) ./Libs/CRC/Src/crc16.d ./Libs/CRC/Src/crc16.o ./Libs/CRC/Src/crc16.su ./Libs/CRC/Src/crc32.d ./Libs/CRC/Src/crc32.o ./Libs/CRC/Src/crc32.su ./Libs/CRC/Src/crc8.d ./Libs/CRC/Src/crc8.o ./Libs/CRC/Src/crc8.su ./Libs/CRC/Src/crcccitt.d ./Libs/CRC/Src/crcccitt.o ./Libs/CRC/Src/crcccitt.su ./Libs/CRC/Src/crcdnp.d ./Libs/CRC/Src/crcdnp.o ./Libs/CRC/Src/crcdnp.su ./Libs/CRC/Src/crckrmit.d ./Libs/CRC/Src/crckrmit.o ./Libs/CRC/Src/crckrmit.su ./Libs/CRC/Src/crcsick.d ./Libs/CRC/Src/crcsick.o ./Libs/CRC/Src/crcsick.su ./Libs/CRC/Src/nmea-chk.d ./Libs/CRC/Src/nmea-chk.o ./Libs/CRC/Src/nmea-chk.su

.PHONY: clean-Libs-2f-CRC-2f-Src

