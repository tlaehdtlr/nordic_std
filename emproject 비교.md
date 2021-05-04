## emProject 설정

- 잘 모르겠으니까 일단 전부 비교를 해버린다.



#### blinky

##### 10100e

```
linker_section_placement_macros="FLASH_PH_START=0x0;FLASH_PH_SIZE=0x40000;RAM_PH_START=0x20000000;RAM_PH_SIZE=0x8000;FLASH_START=0x0;FLASH_SIZE=0x40000;RAM_START=0x20000000;RAM_SIZE=0x8000"
      
linker_section_placements_segments="FLASH RX 0x0 0x40000;RAM RWX 0x20000000 0x8000"
```

##### 10056

```
linker_section_placement_macros="FLASH_PH_START=0x0;FLASH_PH_SIZE=0x100000;RAM_PH_START=0x20000000;RAM_PH_SIZE=0x40000;FLASH_START=0x0;FLASH_SIZE=0x100000;RAM_START=0x20000000;RAM_SIZE=0x40000"
      
linker_section_placements_segments="FLASH RX 0x0 0x100000;RAM RWX 0x20000000 0x40000"
```



#### ble_blinky

##### 10100

```
linker_section_placement_macros="FLASH_PH_START=0x0;FLASH_PH_SIZE=0x80000;RAM_PH_START=0x20000000;RAM_PH_SIZE=0x20000;FLASH_START=0x27000;FLASH_SIZE=0x79000;RAM_START=0x20002ae8;RAM_SIZE=0x1d518"
linker_section_placements_segments="FLASH RX 0x0 0x100000;RAM RWX 0x20000000 0x40000"
```

##### 10056

```
linker_section_placement_macros="FLASH_PH_START=0x0;FLASH_PH_SIZE=0x100000;RAM_PH_START=0x20000000;RAM_PH_SIZE=0x40000;FLASH_START=0x27000;FLASH_SIZE=0xd9000;RAM_START=0x20002300;RAM_SIZE=0x3dd00"
      
linker_section_placements_segments="FLASH RX 0x0 0x100000;RAM RWX 0x20000000 0x40000"
```



#### spi

##### 10056

```
linker_section_placement_macros="FLASH_PH_START=0x0;FLASH_PH_SIZE=0x100000;RAM_PH_START=0x20000000;RAM_PH_SIZE=0x40000;FLASH_START=0x0;FLASH_SIZE=0x100000;RAM_START=0x20000000;RAM_SIZE=0x40000"
      
linker_section_placements_segments="FLASH RX 0x0 0x100000;RAM RWX 0x20000000 0x40000"
```



#### uart

##### 10100

```
linker_section_placement_macros="FLASH_PH_START=0x0;FLASH_PH_SIZE=0x80000;RAM_PH_START=0x20000000;RAM_PH_SIZE=0x20000;FLASH_START=0x0;FLASH_SIZE=0x80000;RAM_START=0x20000000;RAM_SIZE=0x20000"
      
linker_section_placements_segments="FLASH RX 0x0 0x80000;RAM RWX 0x20000000 0x20000"
```

##### 10100e

```
linker_section_placement_macros="FLASH_PH_START=0x0;FLASH_PH_SIZE=0x40000;RAM_PH_START=0x20000000;RAM_PH_SIZE=0x8000;FLASH_START=0x0;FLASH_SIZE=0x40000;RAM_START=0x20000000;RAM_SIZE=0x8000"
      
linker_section_placements_segments="FLASH RX 0x0 0x40000;RAM RWX 0x20000000 0x8000"
```

##### 10056

```
linker_section_placement_macros="FLASH_PH_START=0x0;FLASH_PH_SIZE=0x100000;RAM_PH_START=0x20000000;RAM_PH_SIZE=0x40000;FLASH_START=0x0;FLASH_SIZE=0x100000;RAM_START=0x20000000;RAM_SIZE=0x40000"
      
linker_section_placements_segments="FLASH RX 0x0 0x100000;RAM RWX 0x20000000 0x40000"
```





### 같은 peripheral , 다른 pca

| uart           | 10100              | 10100e            | 10056              |
| -------------- | ------------------ | ----------------- | ------------------ |
| FLASH_PH_START | 0x0                | 0x0               | 0x0                |
| FLASH_PH_SIZE  | 0x40000            | 0x100000          | 0x100000           |
| RAM_PH_START   | 0x20000000         | 0x20000000        | 0x20000000         |
| RAM_PH_SIZE    | 0x8000             | 0x40000           | 0x40000            |
| FLASH_START    | 0x0                | 0x0               | 0x0                |
| FLASH_SIZE     | 0x40000            | 0x100000          | 0x100000           |
| RAM_START      | 0x20000000         | 0x20000000        | 0x20000000         |
| RAM_SIZE       | 0x8000             | 0x40000           | 0x40000            |
| FLASH RX       | 0x0 0x80000        | 0x0 0x100000      | 0x0 0x100000       |
| RAM RWX        | 0x20000000 0x20000 | 0x20000000 0x8000 | 0x20000000 0x40000 |

- 10100e 랑 10056 이랑 RAM RWX  만 다름
- 10100은 좀 많이 다른데...



| uart                           | 10100                          | 10100e                                           | 10056                        |
| ------------------------------ | ------------------------------ | ------------------------------------------------ | ---------------------------- |
| arm_target_device_name         | nRF52833_xxAA                  | nRF52820_xxAA                                    | nRF52840_xxAA                |
| c_preprocessor_definitions     | BOARD_PCA10100   NRF52833_XXAA | BOARD_PCA10100 DEVELOP_IN_NRF52833 NRF52820_XXAA | BOARD_PCA10056 NRF52840_XXAA |
| debug_register_definition_file | nrf52833                       | nrf52820                                         | nrf52840                     |
| folder Name                    | ses_startup_nrf52833           | ses_startup_nrf52820                             | ses_startup_nrf52840         |



### 같은 pca 다른 peripheral

| 10100          | ble_app_blinky     | uart               | cli                |
| -------------- | ------------------ | ------------------ | ------------------ |
| FLASH_PH_START | 0x0                | 0x0                | 0x0                |
| FLASH_PH_SIZE  | 0x80000            | 0x80000            | 0x80000            |
| RAM_PH_START   | 0x20000000         | 0x20000000         | 0x20000000         |
| RAM_PH_SIZE    | 0x20000            | 0x20000            | 0x20000            |
| FLASH_START    | 0x27000            | 0x0                | 0x1000             |
| FLASH_SIZE     | 0x79000            | 0x80000            | 0x7f000            |
| RAM_START      | 0x20002ae8         | 0x20000000         | 0x20000008         |
| RAM_SIZE       | 0x1d518            | 0x20000            | 0x1fff8            |
| FLASH RX       | 0x0 0x100000       | 0x0 0x80000        | 0x0 0x80000        |
| RAM RWX        | 0x20000000 0x40000 | 0x20000000 0x20000 | 0x20000000 0x20000 |

| 10100                          | ble_app_blinky               | uart                         | cli                          |
| ------------------------------ | ---------------------------- | ---------------------------- | ---------------------------- |
| arm_target_device_name         | nRF52840_xxAA                | nRF52833_xxAA                | nRF52840_xxAA                |
| c_preprocessor_definitions     | BOARD_PCA10056 NRF52840_XXAA | BOARD_PCA10100 NRF52833_XXAA | BOARD_PCA10100 NRF52833_XXAA |
| debug_register_definition_file | nrf52840                     | nrf52833                     | nrf52833                     |
| folder Name                    | ses_startup_nrf52840         | ses_startup_nrf52833         | ses_startup_nrf52840         |



### 10100 모음

- ble_app_blinky_pca10100_s140.emProject

```
c_preprocessor_definitions="APP_TIMER_V2;APP_TIMER_V2_RTC1_ENABLED;BOARD_PCA10056;CONFIG_GPIO_AS_PINRESET;FLOAT_ABI_HARD;INITIALIZE_USER_SECTIONS;NO_VTOR_CONFIG;NRF52840_XXAA;NRF_SD_BLE_API_VERSION=7;S140;SOFTDEVICE_PRESENT;"
      linker_section_placement_macros="FLASH_PH_START=0x0;FLASH_PH_SIZE=0x80000;RAM_PH_START=0x20000000;RAM_PH_SIZE=0x20000;FLASH_START=0x27000;FLASH_SIZE=0x79000;RAM_START=0x20002ae8;RAM_SIZE=0x1d518"
      linker_section_placements_segments="FLASH RX 0x0 0x100000;RAM RWX 0x20000000 0x40000"
      
ses_startup_nrf52840
```

- blinky_FreeRTOS_pca10100

```
c_preprocessor_definitions="BOARD_PCA10100;CONFIG_GPIO_AS_PINRESET;FLOAT_ABI_HARD;FREERTOS;INITIALIZE_USER_SECTIONS;NO_VTOR_CONFIG;NRF52833_XXAA;"

      linker_section_placement_macros="FLASH_PH_START=0x0;FLASH_PH_SIZE=0x80000;RAM_PH_START=0x20000000;RAM_PH_SIZE=0x20000;FLASH_START=0x0;FLASH_SIZE=0x80000;RAM_START=0x20000000;RAM_SIZE=0x20000"
      
      linker_section_placements_segments="FLASH RX 0x0 0x80000;RAM RWX 0x20000000 0x20000"
      
ses_startup_nrf52833
```

- cli_pca10100_mbr

```
c_preprocessor_definitions="APP_TIMER_V2;APP_TIMER_V2_RTC1_ENABLED;BOARD_PCA10100;CONFIG_GPIO_AS_PINRESET;DEBUG;DEBUG_NRF;FLOAT_ABI_HARD;INITIALIZE_USER_SECTIONS;MBR_PRESENT;NO_VTOR_CONFIG;NRF52833_XXAA;"

      linker_section_placement_macros="FLASH_PH_START=0x0;FLASH_PH_SIZE=0x80000;RAM_PH_START=0x20000000;RAM_PH_SIZE=0x20000;FLASH_START=0x1000;FLASH_SIZE=0x7f000;RAM_START=0x20000008;RAM_SIZE=0x1fff8"
      
      linker_section_placements_segments="FLASH RX 0x0 0x80000;RAM RWX 0x20000000 0x20000"
      
      ses_startup_nrf52840
```

- uart_pca10100

```
c_preprocessor_definitions="BOARD_PCA10100;BSP_DEFINES_ONLY;CONFIG_GPIO_AS_PINRESET;FLOAT_ABI_HARD;INITIALIZE_USER_SECTIONS;NO_VTOR_CONFIG;NRF52833_XXAA;"

linker_section_placement_macros="FLASH_PH_START=0x0;FLASH_PH_SIZE=0x80000;RAM_PH_START=0x20000000;RAM_PH_SIZE=0x20000;FLASH_START=0x0;FLASH_SIZE=0x80000;RAM_START=0x20000000;RAM_SIZE=0x20000"
      
      linker_section_placements_segments="FLASH RX 0x0 0x80000;RAM RWX 0x20000000 0x20000"
      
      ses_startup_nrf52833
```





## 참고

- ram , flash  설정
  - https://devzone.nordicsemi.com/nordic/short-range-guides/b/getting-started/posts/adjustment-of-ram-and-flash-memory
    - 8번을 보면
      - 디버그 모드로 실행해서 ram 의 시작 주소를 찾는 방법이 나온다
- porting
  - https://infocenter.nordicsemi.com/index.jsp?topic=%2Fcom.nordic.infocenter.sdk5.v15.2.0%2Fnrf52810_user_guide.html&anchor=ug_52810_own_project
  - https://infocenter.nordicsemi.com/index.jsp?topic=%2Fcom.nordic.infocenter.sdk5.v15.2.0%2Fnrf52810_user_guide.html&cp=4_0_0_5_0
    - user guide 에서 
- interrupt
  - https://infocenter.nordicsemi.com/index.jsp?topic=%2Fstruct_nrf52%2Fstruct%2Fnrf52833.html&cp=4_1