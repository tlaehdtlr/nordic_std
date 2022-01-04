[Software Development Kit](https://infocenter.nordicsemi.com/topic/struct_sdk/struct/sdk.html) > [nRF5 SDK v17.0.2](https://infocenter.nordicsemi.com/topic/struct_sdk/struct/sdk_nrf5_latest.html) > [Libraries](https://infocenter.nordicsemi.com/topic/sdk_nrf5_v17.0.2/general_libraries.html) > [Bootloader and DFU modules](https://infocenter.nordicsemi.com/topic/sdk_nrf5_v17.0.2/lib_bootloader_modules.html) > [DFU protocol](https://infocenter.nordicsemi.com/topic/sdk_nrf5_v17.0.2/lib_dfu_transport.html)

*nRF5 SDK v17.0.2*

[Copy URL](https://infocenter.nordicsemi.com/topic/sdk_nrf5_v17.0.2/lib_dfu_transport_serial.html)[Download offline documentation](https://developer.nordicsemi.com/nRF5_SDK/nRF5_SDK_v17.x.x/nRF5_SDK_17.0.2_offline_doc.zip)

Serial

The serial transport makes it possible to perform secure Device Firmware Updates (DFU) over a serial link using USB (nRF52840 only) or UART. The transport layer uses the [SLIP library](https://infocenter.nordicsemi.com/topic/sdk_nrf5_v17.0.2/lib_slip.html) for encoding and decoding packets.

# USB CDC ACM DFU Transport

The USB Secure Device Firmware Update (DFU) uses CDC ACM USB class, commonly known as Virtual COM port. After connecting the USB cable, the development kit will enumerate as a COMx port on Windows hosts or as a `/dev/ttyACMx` device on Linux/Unix hosts. The port can be opened and closed just like a traditional serial port. Refer to [Testing](https://infocenter.nordicsemi.com/topic/sdk_nrf5_v17.0.2/sdk_app_serial_dfu_bootloader.html#serial_sdk_app_dfu_bootloader_test) for driver installation instructions.

# Message sequence charts

The following message sequence charts show the initialization and the transfer of an init packet and a firmware image, respectively.

Note that the message sequence charts do not show how the sent data is encapsulated into SLIP packets. See the documentation of the [SLIP library](https://infocenter.nordicsemi.com/topic/sdk_nrf5_v17.0.2/lib_slip.html) for more information about how the encoding works.

## Initialization

Before the actual DFU process can start, the DFU controller must set the Packet Receipt Notification (PRN) value and obtain the maximum transmission unit (MTU). In most cases, the PRN can be set to 0 to disable checksums being sent back to the controller during transfers. However, if the transport layer is unreliable, set the PRN to a value other than 0.

The MTU of the DFU target sets a limit on the packet size that can be sent. The MTU includes the SLIP encapsulation, which means that the maximum data size that can be sent is (MTU/2) - 2. (See [SLIP library](https://infocenter.nordicsemi.com/topic/sdk_nrf5_v17.0.2/lib_slip.html): The MTU must be divided by 2 to address the case where all bytes are escape characters. The 2 bytes that are subtracted are [NRF_DFU_OP_OBJECT_WRITE](https://infocenter.nordicsemi.com/topic/sdk_nrf5_v17.0.2/group__sdk__nrf__dfu__req__handler.html#gga3ba70c9f7cf5a1e3b34df7fad58d5eeaaf37a7098f30bc2329aa6dca9ad0bd1f0) and the start byte of SLIP.)

The following message sequence chart shows how the PRN is set and the MTU is obtained:

![msc_inline_mscgraph_21](https://infocenter.nordicsemi.com/topic/sdk_nrf5_v17.0.2/msc_inline_mscgraph_21.png)

## Transfer of an init packet

The DFU controller first checks if the init packet has already been transferred successfully. If not, the DFU controller checks if it has been transferred partially. If some data has been transferred already, the transfer is continued. Otherwise, the DFU controller sends a Create command to create a new data object and then transfers the init packet. When the init packet is available, the DFU controller issues an Execute command to initiate the validation of the init packet.

![msc_inline_mscgraph_22](https://infocenter.nordicsemi.com/topic/sdk_nrf5_v17.0.2/msc_inline_mscgraph_22.png)

## Transfer of a firmware image

A firmware image is split up into several data objects that are transferred consecutively. If the transfer of a data object fails (for example, because of a power loss), the transfer can be continued instead of restarted. Therefore, the DFU controller starts by selecting the last data object that was sent and checks if it is complete and valid. If it is, the controller issues an Execute command and then continues the transfer with the next data object. Otherwise, the DFU controller sends a Create command to create a new data object if required (thus if the transfer for this data object has not started yet or the received data is corrupted) and then transfers the next data object.

When all packets are transferred, the DFU controller issues an Execute command to trigger the actual firmware update.

The DFU controller is responsible for keeping track of the progress. The response to each Select command contains information about the maximum object size, the current offset, and the CRC. For example, if the image size is 10 kB and the maximum object size is 4 kB, 3 data objects must be transferred. If the returned offset is 6 kB, the DFU controller knows that the current object is the second object to transfer and that is has not been transferred completely.

![msc_inline_mscgraph_23](https://infocenter.nordicsemi.com/topic/sdk_nrf5_v17.0.2/msc_inline_mscgraph_23.png)

------

[Documentation feedback](mailto:docfeedback@nordicsemi.no?subject=Documentation feedback&body=Link%3A https%3A%2F%2Finfocenter.nordicsemi.com%2Ftopic%2Fsdk_nrf5_v17.0.2%2Flib_dfu_transport_serial.html) | [Developer Zone](https://devzone.nordicsemi.com/questions/) | [Subscribe](http://response.nordicsemi.com/subscribe-to-our-newsletters) | Updated 2020-09-14