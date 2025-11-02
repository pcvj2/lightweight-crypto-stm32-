################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Hash_Auth/Src/hashcheck.c 

OBJS += \
./Hash_Auth/Src/hashcheck.o 

C_DEPS += \
./Hash_Auth/Src/hashcheck.d 


# Each subdirectory must supply rules for building sources it contributes
Hash_Auth/Src/%.o Hash_Auth/Src/%.su Hash_Auth/Src/%.cyclo: ../Hash_Auth/Src/%.c Hash_Auth/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F767xx -c -I../Core/Inc -I../Drivers/STM32F7xx_HAL_Driver/Inc -I../Drivers/STM32F7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F7xx/Include -I../Drivers/CMSIS/Include -I../App/Inc -I../STM32_Cryptographic/Inc -I../Hash_Auth/Inc -O3 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Hash_Auth-2f-Src

clean-Hash_Auth-2f-Src:
	-$(RM) ./Hash_Auth/Src/hashcheck.cyclo ./Hash_Auth/Src/hashcheck.d ./Hash_Auth/Src/hashcheck.o ./Hash_Auth/Src/hashcheck.su

.PHONY: clean-Hash_Auth-2f-Src

