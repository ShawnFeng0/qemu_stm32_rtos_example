#!/usr/bin/env bash

# The command line used by the QEMU Eclipse plug-in to start a debug session looks like this:
#
#$ qemu-system-gnuarmeclipse --verbose --verbose --board STM32F4-Discovery \
#--mcu STM32F407VG --gdb tcp::1234 -d unimp,guest_errors \
#--semihosting-config enable=on,target=native \
#--semihosting-cmdline blinky

# A typical emulation session started outside Eclipse looks like this:
#
#$ qemu-system-gnuarmeclipse --verbose --verbose --board STM32F4-Discovery \
#--mcu STM32F407VG -d unimp,guest_errors \
#--nographic --image test.elf \
#--semihosting-config enable=on,target=native \
#--semihosting-cmdline test 1 2 3

qemu-system-gnuarmeclipse --verbose --verbose --board STM32F4-Discovery \
--mcu STM32F407VG -d unimp,guest_errors \
--image cmake-build-debug/qemu_stm32.elf \
--semihosting-config enable=on,target=native