################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../mpu6050/mpu6050.c \
../mpu6050/mpu6050dmp6.c 

C_DEPS += \
./mpu6050/mpu6050.d \
./mpu6050/mpu6050dmp6.d 

OBJS += \
./mpu6050/mpu6050.o \
./mpu6050/mpu6050dmp6.o 


# Each subdirectory must supply rules for building sources it contributes
mpu6050/%.o: ../mpu6050/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: AVR Compiler'
	avr-gcc -Wall -g2 -gstabs -O2 -fpack-struct -fshort-enums -ffunction-sections -fdata-sections -std=gnu99 -funsigned-char -funsigned-bitfields -mmcu=atmega8 -DF_CPU=8000000UL -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


