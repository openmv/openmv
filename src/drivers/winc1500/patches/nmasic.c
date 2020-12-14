diff --git a/src/winc1500/src/nmasic.c b/src/winc1500/src/nmasic.c
index dca1df01..d2fd4340 100644
--- a/src/winc1500/src/nmasic.c
+++ b/src/winc1500/src/nmasic.c
@@ -39,6 +39,7 @@
  *
  */
 
+#include STM32_HAL_H
 #include "common/include/nm_common.h"
 #include "driver/include/nmbus.h"
 #include "bsp/include/nm_bsp.h"
@@ -444,17 +445,23 @@ sint8 chip_reset(void)
 
 sint8 wait_for_bootrom(uint8 arg)
 {
-	sint8 ret = M2M_SUCCESS;
-	uint32 reg = 0, cnt = 0;
 	uint32 u32GpReg1 = 0;
+	uint32 reg = 0, cnt = 0;
+	sint8 ret = M2M_SUCCESS;
 
-	reg = 0;
-	while(1) {
+    uint32_t timeout = 1000;
+    uint32_t start = HAL_GetTick();
+    while (1) {
 		reg = nm_read_reg(0x1014);	/* wait for efuse loading done */
 		if (reg & 0x80000000) {
 			break;
 		}
 		nm_bsp_sleep(1); /* TODO: Why bus error if this delay is not here. */
+        // Add a timeout here, otherwise it gets stuck if WINC is not connected.
+        // Will have to assume the code won't get stuck somewhere else after init.
+        if (((HAL_GetTick() - start) < timeout)) {
+            return M2M_ERR_INIT;
+        }
 	}
 	reg = nm_read_reg(M2M_WAIT_FOR_HOST_REG);
 	reg &= 0x1;
