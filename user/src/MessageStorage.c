#include "system.h"
#include "MessageStorage.h"
#include "rl_usb.h"                     /* RL-USB function prototypes         */
#ifdef RTE_Compiler_EventRecorder
#include "EventRecorder.h"
#endif

#include "USBH_MSC.h"

void message_storage_task(void *pvParameters)
{
	usbStatus usb_status;                 // USB status
	int32_t   msc_status;                 // MSC status
	FILE     *f;                          // Pointer to stream object
	uint8_t   con = 0U;                   // Connection status of MSC(s)


#ifdef RTE_Compiler_EventRecorder
  EventRecorderInitialize(0, 1);
  EventRecorderEnable (EventRecordError, 0xB0U, 0xB5U);  /* USBH Error Events */
  EventRecorderEnable (EventRecordAll  , 0xB0U, 0xB0U);  /* USBH Core Events */
  EventRecorderEnable (EventRecordAll  , 0xB5U, 0xB5U);  /* USBH MSC Events */
#endif

	usb_status = USBH_Initialize(0U);     // Initialize USB Host 0
	if (usb_status != usbOK) {
		for (;;) {}                         // Handle USB Host 0 init failure
	}

	for (;;) {
		msc_status = USBH_MSC_DriveGetMediaStatus("U0:");   // Get MSC device status
		if (msc_status == USBH_MSC_OK) {
			if (con == 0U) {                  // If stick was not connected previously
				con = 1U;                       // Stick got connected
				msc_status = USBH_MSC_DriveMount("U0:");
				if (msc_status != USBH_MSC_OK) {
					continue;                     // Handle U0: mount failure
				}
				f = fopen("Test.txt", "w");     // Open/create file for writing
				if (f == NULL) {
					continue;                     // Handle file opening/creation failure
				}
				fprintf(f, "USB Host Mass Storage!\n");
				fclose (f);                     // Close file
				msc_status = USBH_MSC_DriveUnmount("U0:");
				if (msc_status != USBH_MSC_OK) {
					continue;                     // Handle U0: dismount failure
				}
			}
		} else {
			if (con == 1U) {                  // If stick was connected previously
				con = 0U;                       // Stick got disconnected
			}
		}
		vTaskDelay(100U);
	}
}

void message_storage_init(void)
{
	xTaskCreate( message_storage_task, "MESSAGE STORAGE", MESSAGE_STORAGE_STACK_SIZE, NULL, MESSAGE_STORAGE_TASK_PRIORITY, NULL );
}