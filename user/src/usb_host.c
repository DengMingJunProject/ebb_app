#include "system.h"
#include "usb_host.h"
#include "rl_usb.h"
#include "rl_usbd_lib.h"
#include "usb_hid.h"
#include "election.h"

static void usb_hsot_task(void *pvParameters)
{
	wait_for_startup();
	
	uint8_t but;
	usbStatus usb_status;                 // USB status
	usbStatus hid_status;                 // HID status
	usbHID_MouseState MouseState;
	int       status;
	static int				instance;
	int       ch;                         // Character
	uint8_t   con = 0U;                   // Connection status of keyboard

	
	usb_status = USBH_Initialize(0U);     // Initialize USB Host 0
	if (usb_status != usbOK) {
		for (;;) {}                         // Handle USB Host 0 init failure
	}

	while(1){
		hid_status = USBH_HID_GetDeviceStatus(0U);  // Get HID device status
		if (hid_status == usbOK) {
			if (con == 0U) {                  // If keyboard was not connected previously
				con = 1U;                       // Keyboard got connected
				instance = USBH_HID_GetDevice(0);
				printf("Mouse connected!\n");
			}
		} else {
			if (con == 1U) {                  // If keyboard was connected previously
				con = 0U;                       // Keyboard got disconnected
				printf("\nMouse disconnected!\n");
			}
		}
		if (con != 0U) {                    // If keyboard is active
			if (usbOK ==USBH_HID_GetMouseState(0, &MouseState)) { // If mouse move
//				printf("button %d\r\n",MouseState.button);
//				printf("button x %d\r\n",MouseState.x);
//				printf("button y %d\r\n",MouseState.y);
			}
		}
		
		if (con != 0U) {                    // If keyboard is active
			uint8_t data[16];
			
			if (USBH_HID_Read (0, data, 1)) {  // If data received from HID device
				printf("button %d\r\n",MouseState.button);
			}
		}
		
		vTaskDelay(10);
		//if (USBD_Configured (0)) { USBD_HID_GetReportTrigger(0U, 0U, &but, 1U); }
	}
}

static void usb_host_gpio_init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOH_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_5, GPIO_PIN_RESET);

	/*Configure GPIO pin : PC5 */
	GPIO_InitStruct.Pin = GPIO_PIN_5;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	/*Configure GPIO pins : PA8 PA10 PA11 PA12 */
	GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_12;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF10_OTG_FS;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	/*Configure GPIO pin : PA9 */
	GPIO_InitStruct.Pin = GPIO_PIN_9;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

FUN_STATUS_T usb_host_init(void){
	
	usb_host_gpio_init();
	if( xTaskCreate( usb_hsot_task, "USB HOST", USB_HOST_STACK_SIZE, NULL, USB_HOST_TASK_PRIORITY, NULL ) == pdPASS)
		return FUN_OK;
	else
		return FUN_ERROR;
}