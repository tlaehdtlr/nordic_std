# blinky
A = "../../../../../../components;../../../../../../components/boards;../../../../../../components/drivers_nrf/nrf_soc_nosd;../../../../../../components/libraries/atomic;../../../../../../components/libraries/atomic_fifo;../../../../../../components/libraries/balloc;../../../../../../components/libraries/bsp;../../../../../../components/libraries/button;../../../../../../components/libraries/cli;../../../../../../components/libraries/cli/cdc_acm;../../../../../../components/libraries/cli/rtt;../../../../../../components/libraries/cli/uart;../../../../../../components/libraries/crc16;../../../../../../components/libraries/delay;../../../../../../components/libraries/experimental_section_vars;../../../../../../components/libraries/fds;../../../../../../components/libraries/fstorage;../../../../../../components/libraries/hardfault;../../../../../../components/libraries/hardfault/nrf52;../../../../../../components/libraries/log;../../../../../../components/libraries/log/src;../../../../../../components/libraries/memobj;../../../../../../components/libraries/mpu;../../../../../../components/libraries/mutex;../../../../../../components/libraries/pwr_mgmt;../../../../../../components/libraries/queue;../../../../../../components/libraries/ringbuf;../../../../../../components/libraries/scheduler;../../../../../../components/libraries/sortlist;../../../../../../components/libraries/stack_guard;../../../../../../components/libraries/strerror;../../../../../../components/libraries/timer;../../../../../../components/libraries/usbd;../../../../../../components/libraries/usbd/class/cdc;../../../../../../components/libraries/usbd/class/cdc/acm;../../../../../../components/libraries/util;../../../../../../components/softdevice/mbr/headers;../../../../../../components/toolchain/cmsis/include;../../..;../../../../../../external/fnmatch;../../../../../../external/fprintf;../../../../../../external/segger_rtt;../../../../../../external/utf_converter;../../../../../../integration/nrfx;../../../../../../integration/nrfx/legacy;../../../../../../modules/nrfx;../../../../../../modules/nrfx/drivers/include;../../../../../../modules/nrfx/hal;../../../../../../modules/nrfx/mdk;../config;"
# my
B = "../SRC;../../../../../../components;../../../../../../components/boards;../../../../../../components/drivers_nrf/nrf_soc_nosd;../../../../../../components/libraries/atomic;../../../../../../components/libraries/balloc;../../../../../../components/libraries/bsp;../../../../../../components/libraries/delay;../../../../../../components/libraries/experimental_section_vars;../../../../../../components/libraries/log;../../../../../../components/libraries/log/src;../../../../../../components/libraries/memobj;../../../../../../components/libraries/ringbuf;../../../../../../components/libraries/strerror;../../../../../../components/libraries/util;../../../../../../components/toolchain/cmsis/include;../../../../../../components/libraries/cli;../../../../../../components/libraries/cli/rtt;../../../../../../components/libraries/cli/uart;../../../../../../components/libraries/queue;../../../../../../components/libraries/timer;../../../../../../components/libraries/atomic_fifo;../../../../../../components/libraries/button;../../../../../../components/libraries/hardfault;../../../../../../components/libraries/hardfault/nrf52;../../../../../../components/libraries/mutex;../../../../../../components/libraries/sortlist;../../..;../../../../../../external/fprintf;../../../../../../integration/nrfx;../../../../../../modules/nrfx;../../../../../../modules/nrfx/hal;../../../../../../modules/nrfx/mdk;../../../../../../integration/nrfx/legacy;../../../../../../modules/nrfx/drivers/include;../config;"

A = set(map(str, A.split(';')))
B = set(map(str, B.split(';')))

print(A-B)
print()
print((A-B)|(B-A))