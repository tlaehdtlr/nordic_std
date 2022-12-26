## 참고

### guide

- https://devzone.nordicsemi.com/guides/short-range-guides/b/bluetooth-low-energy/posts/bluetooth-smart-and-the-nordics-softdevices-part-1
- https://devzone.nordicsemi.com/guides/short-range-guides/b/bluetooth-low-energy/posts/bluetooth-smart-and-the-nordics-softdevices-part-2



## GAP (generic access profile)

- central, peripheral
- advertising, connection 제어

### advertising

#### data

- 31 bytes : advertising data payload (필수) , scan response payload (옵션)
  - peripheral 이 advertising 을 하고 있고
  - central 이 장치 이름 같은 추가적인 정보를 요구하기 위해 구현된 것이 scan response payload
    - 첨에 6bytes MAC address 만 떴다가 이름까지 뜰 수 있게함

#### process

- Broadcasting : advertising interval 때마다 advertising packet 을 전송
  - 이렇게 advertising 역할만 하는 장치가 beacon 임
- central / peripheral 연결되면 advertising 종료
  - GATT 로 양방향 통신 시작



## GATT (generic attribute profile)

- central/peripheral 간 service, characteristic 을 이용해서 데이터 주고 받는 방법 정의

### 구성

- Attribute protocol (ATT) : GATT 는 ATT 의 최상위 구현체 , 각각의 속성(attribute)은 UUID 를 가지고 128비트로 구성된다. ATT 에 의해 부여된 속성은 service, characteristic 을 결정
- service : characteristic 들의 집합
- characteristic : 하나의 값과 n개의 discriptor 를 포함
- discriptor : characteristic 의 값을 기술

#### data

- profile (실제 존재하는 것 아님 논리적 구분)
  - service 1 :UUID 가짐 (16bit : 공식 서비스, 128bit : 커스텀 서비스)
    - characteristic 1 : 하나의 데이터 포함
    - characteristic 2
  - service 2
    - characteristic ...

### process

- central 은 gatt client (master) 가 된다
  - 모든 동작 (transaction)은 얘가 시작
  - 데이터 요청 보냄
- peripheral 은 gatt server (slave) 가 된다
  - 연결될 때 connection inverval 정보를 전달함 (옵션)
- 항상 GATT client 가 요청해야 GATT server 가 응답 (read/write)
  - characteristic 에서 read/write 에 추가로 notify/indicate 설정 가능
    - GATT client 가 사용하겠다고 server에 알려줘야함
    - notify : characteristic 에 변화가 있음을 알려줌
    - indicate : 메시지를 받았는지 확인을 받는 과정 추가



## BLE 순서

#### GAP

1. scanning

2. scan request , scan response

3. ble connection
   - pairing 과정 , PIN 입력 과정으로 연결 성립

#### GATT

4. service discovery 작업
   - ble peripheral 이 가진 service/characteristic 구조 모두 가져옴
   - service 먼저 discover 요청/응답 받고, characteristic 도 discover

5. service, characteristic 확인하고, 어떤 characteristic 을 read/write 할지 확인

6. 필요시, notify/indicate 기능 있는 characteristic 의 CCCD 를 on 하도록 신호 보냄

7. 필요시, 특정 characteristic 값 read/write 가능, notify/indicate on 했으면 값이 바뀔 때 신호 보내줌