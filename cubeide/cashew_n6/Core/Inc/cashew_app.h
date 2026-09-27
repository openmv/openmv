/*
 * cashew_app.h -- detection loop and serial command interface.
 */
#ifndef CASHEW_APP_H
#define CASHEW_APP_H

#include <stdbool.h>

// Start the detection loop automatically after boot (0 = wait for "run").
#ifndef APP_AUTORUN
#define APP_AUTORUN     1
#endif

// Returns 0 on success. Prints its own errors.
int  app_init(bool camera_ok);
void app_poll(void);            // call forever from main()
void app_toggle_run(void);      // user button

#endif
