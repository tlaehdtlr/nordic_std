# blinky
A = "../src;$(SDK_PATH)/components;$(SDK_PATH)/components/boards;$(SDK_PATH)/components/drivers_nrf/nrf_soc_nosd;$(SDK_PATH)/components/libraries/atomic;$(SDK_PATH)/components/libraries/balloc;$(SDK_PATH)/components/libraries/bsp;$(SDK_PATH)/components/libraries/delay;$(SDK_PATH)/components/libraries/experimental_section_vars;$(SDK_PATH)/components/libraries/log;$(SDK_PATH)/components/libraries/log/src;$(SDK_PATH)/components/libraries/memobj;$(SDK_PATH)/components/libraries/ringbuf;$(SDK_PATH)/components/libraries/strerror;$(SDK_PATH)/components/libraries/util;$(SDK_PATH)/components/toolchain/cmsis/include;../../..;$(SDK_PATH)/external/fprintf;$(SDK_PATH)/integration/nrfx;$(SDK_PATH)/modules/nrfx;$(SDK_PATH)/modules/nrfx/hal;$(SDK_PATH)/modules/nrfx/mdk;../config;"
# new
B = "../src;$(SDK_PATH)/components;$(SDK_PATH)/components/boards;$(SDK_PATH)/components/drivers_nrf/nrf_soc_nosd;$(SDK_PATH)/components/libraries/atomic;$(SDK_PATH)/components/libraries/balloc;$(SDK_PATH)/components/libraries/bsp;$(SDK_PATH)/components/libraries/delay;$(SDK_PATH)/components/libraries/experimental_section_vars;$(SDK_PATH)/components/libraries/log;$(SDK_PATH)/components/libraries/log/src;$(SDK_PATH)/components/libraries/memobj;$(SDK_PATH)/components/libraries/ringbuf;$(SDK_PATH)/components/libraries/strerror;$(SDK_PATH)/components/libraries/util;$(SDK_PATH)/components/toolchain/cmsis/include;../../..;$(SDK_PATH)/external/fprintf;$(SDK_PATH)/integration/nrfx;$(SDK_PATH)/integration/nrfx/legacy;$(SDK_PATH)/modules/nrfx/drivers/include;$(SDK_PATH)/modules/nrfx;$(SDK_PATH)/modules/nrfx/hal;$(SDK_PATH)/modules/nrfx/mdk;../config;"

A = set(map(str, A.split(';')))
B = set(map(str, B.split(';')))

print(B-A)
print((A-B)|(B-A))