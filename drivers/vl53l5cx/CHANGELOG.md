********************************************************************************
*
* STMicroelectronics - VL53L5CX - Ultra Lite Driver
*
********************************************************************************

# Driver version history 

---------------------------------------------------------------------------------------------------------------

Version : 1.0.0
Date : 06/09/2021
Comments : Initial driver release.

---------------------------------------------------------------------------------------------------------------

Version : 1.0.1
Date : 06/17/2021
Comments : Fixed wrong array size computing. No impact for host.

---------------------------------------------------------------------------------------------------------------

Version : 1.0.2
Date : 06/24/2021
Comments : Corrected bug : 1 frame over 128 was ignored using function vl53l5cx_check_data_ready().

---------------------------------------------------------------------------------------------------------------


Version : 1.0.3
Date : 07/13/2021
Comments : Corrected bug : Macro missing for more than 1 target per zone. Compilation failed.

---------------------------------------------------------------------------------------------------------------


Version : 1.0.4
Date : 07/20/2021
Comments : Corrected bug : Invalid Xtalk calibration data after running Xtalk calibration with more than 1 target per zone.

---------------------------------------------------------------------------------------------------------------


Version : 1.1.0
Date : 09/03/2021
Comments : 
- Firmware update : Status 13 has been added, to indicate that the detected target has a mismatch between distance and signal.
- Added macro 'VL53L5CX_USE_RAW_FORMAT' allowing to use the default firmware format.


---------------------------------------------------------------------------------------------------------------


Version : 1.1.1
Date : 09/17/2021
Comments : 
- Updated function '_vl53l5cx_send_offset_data()' which may give warnings depending of compiler options.


---------------------------------------------------------------------------------------------------------------


Version : 1.1.2
Date : 10/12/2021
Comments : 
- Corrected bug : In some cases, detection thresholds were not applied when the AND operation was used with 2 thresholds.


---------------------------------------------------------------------------------------------------------------


Version : 1.2.0
Date : 01/14/2022
Comments : Several updates done in order to catch more errors from FW.
- Result structure : Added silicon temperature output
- Init function : Added several read registers used to improve initialization stability
- Start function : Added a check of memory size in orer to be sure that packet size has been correctly programmed
- Stop function : Return more relevant errors from FW
- Check data ready function : Return more relevant errors from FW

---------------------------------------------------------------------------------------------------------------


Version : 1.3.0
Date : 02/11/2021
Comments : 
- Corrected reflectance format (output divided by /2 vs previous driver version)
- Added macro VL53L5CX_STATUS_TIMEOUT_ERROR


---------------------------------------------------------------------------------------------------------------