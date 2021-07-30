#!/usr/bin/env bash

if [ $# -lt 2 ] ; then
echo "Usage: $0 COMMUNICATION PROGRAM_PATH"
echo "Example: $0 tcp:localhost:1234 qemu_test.elf"
exit 1;
fi

COMMUNICATION=$1
PROGRAM_PATH=$2

qemu-system-gnuarmeclipse --verbose --verbose --board STM32F4-Discovery \
--mcu STM32F407VG -d unimp,guest_errors \
--image $PROGRAM_PATH \
--semihosting-config enable=on,target=gdb \
--gdb $COMMUNICATION -S
