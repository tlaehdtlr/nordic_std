:::::::::::::::::::::::::::
::
:: Written by Woojin Yoon
::
:: EM-Tech CTO SW Team
:::::::::::::::::::::::::::

@echo off

set APP_DIRECTORY=..\ses\Output\Debug\Exe
set DFU_DIRECTORY=output
set TARGET=n100_ble

:: 1. argument count calculation
set argc=0
for %%x in (%*) do Set /A argc+=1
::echo argc=%argc%

: check the argument count
IF %argc% == 0 (
	echo "usage) secure_dfu.bat (gen|flash|dfu)"
	echo "       secure_dfu.bat gen (app|total)"
	echo "       secure_dfu.bat flash (app|total)"
	echo "       secure_dfu.bat dfu gen (debug|prod)"
	echo "       secure_dfu.bat dfu disp"
	echo "       secure_dfu.bat dfu upg comport_number ble_address"
	echo "         ex) secure_dfu.bat dfu upg 12 C9A69D98845D"
	GOTO end
)

:: 2. make directory, if not exist
if not exist %DFU_DIRECTORY% (
	mkdir %DFU_DIRECTORY%
)

:: 3. create dfu output directory
IF "%1"=="gen" IF "%2"=="app" GOTO app
IF "%1"=="gen" IF "%2"=="total" GOTO total
IF "%1"=="flash" GOTO flash
IF "%1"=="dfu" IF "%2"=="gen" GOTO dfu
IF "%1"=="dfu" IF "%2"=="upg" GOTO upgrade
IF "%1"=="dfu" IF "%2"=="disp" GOTO display

echo Wrong argument
echo "usage) secure_dfu.bat (gen|flash|dfu)"
echo "       secure_dfu.bat gen (app|total)"
echo "       secure_dfu.bat flash (app|total)"
echo "       secure_dfu.bat dfu gen (debug|prod)"
echo "       secure_dfu.bat dfu disp"
echo "       secure_dfu.bat dfu upg comport_number ble_address"
echo "         ex) secure_dfu.bat dfu upg 12 C9A69D98845D"
GOTO end

:: 4. generate firmware for mass production
:app
:total

echo "Step 1. Generating bootloader settings image file..."
nrfutil settings generate --family NRF52 ^
					      --application %APP_DIRECTORY%\%TARGET%.hex ^
						  --application-version 1 ^
						  --bootloader-version 1 ^
						  --bl-settings-version 1 ^
						  %DFU_DIRECTORY%\bootloader_settings.hex

echo "Step 2. Generating application image file..."
mergehex -m %APP_DIRECTORY%\%TARGET%.hex ^
			%DFU_DIRECTORY%\bootloader_settings.hex ^
			-o %DFU_DIRECTORY%\%TARGET%_app.hex

IF "%2"=="app" (
	GOTO end
)

echo.
echo "Step 3. Generating integrated image file..."
mergehex -m %DFU_DIRECTORY%\%TARGET%_app.hex ^
		    n100_ble_secure_bootloader_s140.hex ^
			s140_nrf52_7.0.1_softdevice.hex ^
			-o %DFU_DIRECTORY%\%TARGET%_total.hex

GOTO end

:: 5. generate firmware package for dfu
:dfu
echo "Generating application dfu image file..."
IF "%3"=="debug" (
	nrfutil pkg generate --hw-version 52 ^
						 --debug-mode ^
						 --application %APP_DIRECTORY%\%TARGET%.hex ^
						 --sd-req 0xCA ^
						 --key-file priv.pem ^
						 %DFU_DIRECTORY%\%TARGET%_dfu_debug.zip
) ELSE IF "%3"=="prod" (
	nrfutil pkg generate --hw-version 52 ^
						 --application-version 1 ^
						 --application %APP_DIRECTORY%\%TARGET%.hex ^
						 --sd-req 0xCA ^
						 --key-file priv.pem ^
						 %DFU_DIRECTORY%\%TARGET%_dfu.zip
)

GOTO end

:: 6. flash application image
:flash
IF "%2"=="app" (
	IF "%3"=="" (
		nrfjprog -f nrf52 --sectorerase --program %DFU_DIRECTORY%\%TARGET%_app.hex
	) ELSE (
		nrfjprog -f nrf52 --sectorerase --program %3
	)
) ELSE IF "%2"=="total" (
	IF "%3"=="" (
		nrfjprog -f nrf52 --sectorerase --program %DFU_DIRECTORY%\%TARGET%_total.hex
	) ELSE (
		nrfjprog -f nrf52 --sectorerase --program %3
	)
)
nrfjprog -f nrf52 --reset

GOTO end

:: 7. do dfu upgrade
:upgrade
IF "%5"=="" (
	nrfutil dfu ble -ic NRF52 -p COM%3 -a %4 -pkg %DFU_DIRECTORY%\%TARGET%_dfu.zip
) ELSE (
	nrfutil dfu ble -ic NRF52 -p COM%3 -a %4 -pkg %5
)

GOTO end

:: 8. display dfu package info
:display
IF "%3"=="" (
	nrfutil pkg display %DFU_DIRECTORY%\%TARGET%_dfu.zip
) ELSE (
	nrfutil pkg display %3
)

:end
