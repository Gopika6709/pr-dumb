# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(hello_world)

target_sources(app PRIVATE 
		src/main.c
		src/gpio_stm32.c
		src/gpio.c
		src/gyro.c
		src/i2c.c)


