# Nordic

- nRF52833 DK board, PCA10100



## development setting

#### IDE

- SEGGER Embedded Studio (SES)



#### SDK

- 다양한 examples
- base code? driver? 여튼 우리가 세팅하지 않아도 여기에서 한걸루 쓰면 된다



#### 빌드 디렉토리 변경

- SDK 적용하기 위함(<SDK_Install> 경로내에서만 사용 가능)
- 환경 변수 설정 및 global macro 사용

##### segger IDE

- SDK 관련 global macro 추가

  - Tools > Options > Building > Global Macros

  - Noric sdk path 입력

    - ```
      NORDIC_SDK_PATH=D:/Nordic/nRF5_SDK/nRF5_SDK_17.0.0_9d13099
      ```

- `.emProject` 파일 수정

  - ```
    ../../../../../.. → $(NORDIC_SDK_PATH)로 변경
    ```

#### Memory map 설정







## F/W Download

### FW 생성

#### Total image 생성

- Bootloader (chip 별로 메모리 설정하여 제공됨)

- Application

- SoftDevice

- secure bootloader 사용 위해서 bootloader setting file 생성

  - ```
    nrfutil settings generate --family NRF52 --application [application hex파일] --app-boot-validation VALIDATE_GENERATED_CRC --application-version 1 --bootloader-version 1 --softdevice [softdevice hex파일] --bl-settings-version 2 [output.hex]
    
    ```

  - ```
    nrfutil settings generate --family NRF52 --application imedisync_ble.hex --app-boot-validation VALIDATE_GENERATED_CRC --application-version 1 --bootloader-version 1 --softdevice s140_nrf52_7.0.1_softdevice.hex --bl-settings-version 2 n100_bl_settings.hex
    ```

- total image 생성

  - ```
    mergehex -m [bootloader hex 파일] [application hex 파일] [bootloader setting hex 파일] [softdevice hex 파일] -o [total 파일.hex]
    ```

  - ```
    mergehex -m imedisync_ble_secure_bootloader_s140.hex imedisync_ble.hex n100_bl_settings.hex s140_nrf52_7.0.1_softdevice.hex -o n100_total.hex
    ```

  - 이거할 때, nrf_command_line_tools 버전이 10.9는 [bootloader setting hex] 파일 받는 인자가 없어서 10.12.1 로 업그레이드 함

    - 10.12.1 은 mergehex 를 했을 때, 자꾸 overlapping address 라고 나왔음
    - nrf connect programmer 로 4개 파일 add 하면 괜찮길래 mergehex 프로그램 문제라고 생각함 
    - https://devzone.nordicsemi.com/f/nordic-q-a/69987/latest-mergehex-output-fails-to-be-programmed-mergehex-9-8-1-produces-different-hex-than-10-12-1/291803#291803
      - 여기 답변 보면 버그라고함
      - 결국 10.11.1 버전쓰며 해결

  

### J-Link 이용

#### nRF Connect 

- Desktop 용 APP을 열고 programmer app 이용

#### nRF Command Line Tools

- CLI

```
nrfjprog -f nrf52 --reset --program [hex 파일] --chiperase --verify
```

### DFU (Device Firmware Update)

- 폰 등에서 nRF Connect APP 실행
  - DFU 용 zip 파일 필요
  - BT 연결 후, DFU 다운
- 컴퓨터에서 할려면 nRF Connect PC tool
  - nRF dongle usb 연결
  - bluetooth low energy app 실행
  - dongle 찾고 scan -> connect -> dfu 눌러서 파일 찾아서 업뎃
    ![image-20210401150517897](README.assets/image-20210401150517897.png)





## 프로젝트 구성

- `.emProject` 파일에 경로 등이 저장 맞나?