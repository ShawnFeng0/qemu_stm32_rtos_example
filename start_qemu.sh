#!/usr/bin/env bash

if [ $# -lt 1 ] ; then
echo "USAGE:"
echo "$0 image_file_path"
exit 1;
fi

image_file_path=$1
shift

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
--image $image_file_path \
--semihosting-config enable=on,target=native \
"$@"
