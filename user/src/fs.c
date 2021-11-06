#include "system.h"
#include "fs.h"
#include "rl_fs.h"
#include "stm32f4xx_hal.h"
#include "main.h"

#ifdef FS_DEBUG
    #define	fs_log(...) do{if(DEBUG(DEBUG_ENABLE)){DBG_LOG("[FS](%ld) ",__LINE__);DBG_LOG(__VA_ARGS__);}}while(0)
    #define fs_usr(...) do{if(DEBUG(DEBUG_ENABLE)){DBG_LOG("[FS] ");DBG_USR(__VA_ARGS__);}}while(0)
    #define fs_err(...) do{if(DEBUG(DEBUG_ENABLE)){DBG_LOG("[FS] ");DBG_ERR(__VA_ARGS__);}}while(0)
    #define fs_dump(...) if(DEBUG(DEBUG_ENABLE)){DBG_DUMP(__VA_ARGS__);}
#else
    #define fs_log(...)
    #define fs_usr(...)
    #define fs_err(...)
    #define fs_dump(...)
#endif

NAND_HandleTypeDef hnand1;
SD_HandleTypeDef hsd;
DMA_HandleTypeDef hdma_sdio_rx;
DMA_HandleTypeDef hdma_sdio_tx;

static void fs_fsmc_init(void)
{
  FSMC_NAND_PCC_TimingTypeDef ComSpaceTiming;
  FSMC_NAND_PCC_TimingTypeDef AttSpaceTiming;

  /** Perform the NAND1 memory initialization sequence
  */
  hnand1.Instance = FSMC_NAND_DEVICE;
  /* hnand1.Init */
  hnand1.Init.NandBank = FSMC_NAND_BANK2;
  hnand1.Init.Waitfeature = FSMC_NAND_PCC_WAIT_FEATURE_ENABLE;
  hnand1.Init.MemoryDataWidth = FSMC_NAND_PCC_MEM_BUS_WIDTH_8;
  hnand1.Init.EccComputation = FSMC_NAND_ECC_ENABLE;
  hnand1.Init.ECCPageSize = FSMC_NAND_ECC_PAGE_SIZE_512BYTE;
  hnand1.Init.TCLRSetupTime = 1;
  hnand1.Init.TARSetupTime = 1;
  /* hnand1.Config */
  hnand1.Config.PageSize = 2048;
  hnand1.Config.SpareAreaSize = 64;
  hnand1.Config.BlockSize = 64;
  hnand1.Config.BlockNbr = 2048;
  hnand1.Config.PlaneNbr = 0;
  hnand1.Config.PlaneSize = 0;
  hnand1.Config.ExtraCommandEnable = DISABLE;
  /* ComSpaceTiming */
  ComSpaceTiming.SetupTime = 1;
  ComSpaceTiming.WaitSetupTime = 5;
  ComSpaceTiming.HoldSetupTime = 5;
  ComSpaceTiming.HiZSetupTime = 2;
  /* AttSpaceTiming */
  AttSpaceTiming.SetupTime = 1;
  AttSpaceTiming.WaitSetupTime = 5;
  AttSpaceTiming.HoldSetupTime = 5;
  AttSpaceTiming.HiZSetupTime = 2;

  if (HAL_NAND_Init(&hnand1, &ComSpaceTiming, &AttSpaceTiming) != HAL_OK)
  {
//    Error_Handler( );
  }

  GPIO_InitTypeDef GPIO_InitStruct;

  /* GPIO Ports Clock Enable */
  __GPIOE_CLK_ENABLE();
  __GPIOD_CLK_ENABLE();
  __GPIOC_CLK_ENABLE();
  __GPIOG_CLK_ENABLE();
  __GPIOH_CLK_ENABLE();
  __GPIOF_CLK_ENABLE();

  /* Peripheral clock enable */
  __FSMC_CLK_ENABLE();
  
  /* FSMC GPIO Configuration */
  GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull      = GPIO_NOPULL;
  GPIO_InitStruct.Speed     = GPIO_SPEED_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF12_FSMC;

  GPIO_InitStruct.Pin = GPIO_PIN_7  | GPIO_PIN_0  | GPIO_PIN_5 |
                        GPIO_PIN_1  | GPIO_PIN_4  | GPIO_PIN_15 | GPIO_PIN_14 |
                        GPIO_PIN_12 | GPIO_PIN_11;

  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
  
  
  GPIO_InitStruct.Pin = GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF12_FSMC;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
  
  GPIO_InitStruct.Pin = GPIO_PIN_8  | GPIO_PIN_9  |
                        GPIO_PIN_7  | GPIO_PIN_10 ;

  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

	FSMC_NAND_InitTypeDef       NandInit;
//	FSMC_NAND_PCC_TimingTypeDef ComSpaceTiming;
//  FSMC_NAND_PCC_TimingTypeDef AttSpaceTiming;
  /* NAND Bank Init Structure */
  NandInit.NandBank        = FSMC_NAND_BANK2;
  NandInit.Waitfeature     = FSMC_NAND_PCC_WAIT_FEATURE_DISABLE;
  NandInit.MemoryDataWidth = FSMC_NAND_PCC_MEM_BUS_WIDTH_8;
  NandInit.EccComputation  = FSMC_NAND_ECC_DISABLE;
  NandInit.ECCPageSize     = FSMC_NAND_ECC_PAGE_SIZE_512BYTE;
  NandInit.TCLRSetupTime   = 1;
  NandInit.TARSetupTime    = 1;

  /* ComSpaceTiming */
  ComSpaceTiming.SetupTime     = 1;
  ComSpaceTiming.WaitSetupTime = 3;
  ComSpaceTiming.HoldSetupTime = 2;
  ComSpaceTiming.HiZSetupTime  = 2;
  /* AttSpaceTiming */
  AttSpaceTiming.SetupTime     = 1;
  AttSpaceTiming.WaitSetupTime = 3;
  AttSpaceTiming.HoldSetupTime = 17;
  AttSpaceTiming.HiZSetupTime  = 2;

  /* Initialize NAND control Interface */
  FMC_NAND_Init(FSMC_NAND_DEVICE, &(NandInit));
  
  /* Initialize NAND common space timing Interface */  
  FMC_NAND_CommonSpace_Timing_Init(FSMC_NAND_DEVICE, &ComSpaceTiming, NandInit.NandBank);
  
  /* Initialize NAND attribute space timing Interface */  
  FMC_NAND_AttributeSpace_Timing_Init(FSMC_NAND_DEVICE, &AttSpaceTiming, NandInit.NandBank);
  
  /* Enable the NAND device */
  __FMC_NAND_ENABLE(FSMC_NAND_DEVICE, NandInit.NandBank);

}

static void fs_dma_init(void) 
{
  /* DMA controller clock enable */
  __HAL_RCC_DMA2_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA2_Stream3_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream3_IRQn);
  /* DMA2_Stream6_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream6_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream6_IRQn);

}

void HAL_SD_MspInit(SD_HandleTypeDef* hsd)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(hsd->Instance==SDIO)
  {
  /* USER CODE BEGIN SDIO_MspInit 0 */

  /* USER CODE END SDIO_MspInit 0 */
    /* Peripheral clock enable */
    __HAL_RCC_SDIO_CLK_ENABLE();
  
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    /**SDIO GPIO Configuration    
    PC8     ------> SDIO_D0
    PC9     ------> SDIO_D1
    PC10     ------> SDIO_D2
    PC11     ------> SDIO_D3
    PC12     ------> SDIO_CK
    PD2     ------> SDIO_CMD 
    */
    GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_11 
                          |GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF12_SDIO;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_2;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF12_SDIO;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    /* SDIO DMA Init */
    /* SDIO_RX Init */
    hdma_sdio_rx.Instance = DMA2_Stream3;
    hdma_sdio_rx.Init.Channel = DMA_CHANNEL_4;
    hdma_sdio_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_sdio_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_sdio_rx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_sdio_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    hdma_sdio_rx.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
    hdma_sdio_rx.Init.Mode = DMA_PFCTRL;
    hdma_sdio_rx.Init.Priority = DMA_PRIORITY_LOW;
    hdma_sdio_rx.Init.FIFOMode = DMA_FIFOMODE_ENABLE;
    hdma_sdio_rx.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
    hdma_sdio_rx.Init.MemBurst = DMA_MBURST_INC4;
    hdma_sdio_rx.Init.PeriphBurst = DMA_PBURST_INC4;
    if (HAL_DMA_Init(&hdma_sdio_rx) != HAL_OK)
    {
//      Error_Handler();
    }

    __HAL_LINKDMA(hsd,hdmarx,hdma_sdio_rx);

    /* SDIO_TX Init */
    hdma_sdio_tx.Instance = DMA2_Stream6;
    hdma_sdio_tx.Init.Channel = DMA_CHANNEL_4;
    hdma_sdio_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_sdio_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_sdio_tx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_sdio_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    hdma_sdio_tx.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
    hdma_sdio_tx.Init.Mode = DMA_PFCTRL;
    hdma_sdio_tx.Init.Priority = DMA_PRIORITY_LOW;
    hdma_sdio_tx.Init.FIFOMode = DMA_FIFOMODE_ENABLE;
    hdma_sdio_tx.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
    hdma_sdio_tx.Init.MemBurst = DMA_MBURST_INC4;
    hdma_sdio_tx.Init.PeriphBurst = DMA_PBURST_INC4;
    if (HAL_DMA_Init(&hdma_sdio_tx) != HAL_OK)
    {
//      Error_Handler();
    }

    __HAL_LINKDMA(hsd,hdmatx,hdma_sdio_tx);

  /* USER CODE BEGIN SDIO_MspInit 1 */

  /* USER CODE END SDIO_MspInit 1 */
  }

}

/**
* @brief SD MSP De-Initialization
* This function freeze the hardware resources used in this example
* @param hsd: SD handle pointer
* @retval None
*/

void HAL_SD_MspDeInit(SD_HandleTypeDef* hsd)
{

  if(hsd->Instance==SDIO)
  {
  /* USER CODE BEGIN SDIO_MspDeInit 0 */

  /* USER CODE END SDIO_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_SDIO_CLK_DISABLE();
  
    /**SDIO GPIO Configuration    
    PC8     ------> SDIO_D0
    PC9     ------> SDIO_D1
    PC10     ------> SDIO_D2
    PC11     ------> SDIO_D3
    PC12     ------> SDIO_CK
    PD2     ------> SDIO_CMD 
    */
    HAL_GPIO_DeInit(GPIOC, GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_11 
                          |GPIO_PIN_12);

    HAL_GPIO_DeInit(GPIOD, GPIO_PIN_2);

    /* SDIO DMA DeInit */
    HAL_DMA_DeInit(hsd->hdmarx);
    HAL_DMA_DeInit(hsd->hdmatx);
  /* USER CODE BEGIN SDIO_MspDeInit 1 */

  /* USER CODE END SDIO_MspDeInit 1 */
  }

}


/**
  * @brief SDIO Initialization Function
  * @param None
  * @retval None
  */
static void fs_sdio_sd_init(void)
{

  /* USER CODE BEGIN SDIO_Init 0 */

  /* USER CODE END SDIO_Init 0 */

  /* USER CODE BEGIN SDIO_Init 1 */

  /* USER CODE END SDIO_Init 1 */
  hsd.Instance = SDIO;
  hsd.Init.ClockEdge = SDIO_CLOCK_EDGE_RISING;
  hsd.Init.ClockBypass = SDIO_CLOCK_BYPASS_DISABLE;
  hsd.Init.ClockPowerSave = SDIO_CLOCK_POWER_SAVE_DISABLE;
  hsd.Init.BusWide = SDIO_BUS_WIDE_4B;
  hsd.Init.HardwareFlowControl = SDIO_HARDWARE_FLOW_CONTROL_DISABLE;
  hsd.Init.ClockDiv = 0;
  if (HAL_SD_Init(&hsd) != HAL_OK)
  {
//    Error_Handler();
  }
  if (HAL_SD_ConfigWideBusOperation(&hsd, SDIO_BUS_WIDE_4B) != HAL_OK)
  {
//    Error_Handler();
  }
  /* USER CODE BEGIN SDIO_Init 2 */

  /* USER CODE END SDIO_Init 2 */

}

static char *get_drive (char *src, char *dst, uint32_t dst_sz) {
  uint32_t i, n;

  i = 0;
  n = 0;
  while (!n && src && src[i] && (i < dst_sz)) {
    dst[i] = src[i];

    if (dst[i] == ':') {
      n = i + 1;
    }
    i++;
  }
  if (n == dst_sz) {
    n = 0;
  }
  dst[n] = '\0';

  return (src + n);
}

uint8_t fs_format(char *par){
	
	char  label[12];
	char  drive[4];
	
	par = get_drive (par, drive, 4);
	
	if (fformat (drive, "/LLEB") == fsOK) {
		fs_log ("Format completed.\n");
		if (fvol (drive, label, NULL) == 0) {
			if (label[0] != '\0') {
				fs_log ("Volume label is \"%s\"\n", label);
			}
		}
		return fsOK ;
	}
	else {
		fs_log ("Formatting failed.\n");
		return fsError;
	}
}

uint32_t fs_fsize(char *par)
{
	FILE *file;
	uint32_t fpos;
	char buf[20];
	
	file = fopen (par,"r");
	if (file != NULL)  {
		
		fseek	(	file,0,	SEEK_END );
		fpos = ftell (file);                             // Get position after read operation
		fclose (file);
		
		return fpos;
	}
	return 0;
}

FUN_STATUS_T fs_init(void){
	
	fs_fsmc_init();
	fs_dma_init();
//	fs_sdio_sd_init();
	
	fsStatus stat;
		
	stat = finit("N0:");
	if (stat == fsOK) {
		stat = fmount ("N0:");
		if (stat == fsOK) {
			fs_log ("Drive N0 ready!\n");
			sys_status.peripheral_init.bit.nand_flash = 1;
		}
		else {
			fs_log ("Drive N0 mount failed with error code %d\n", stat);
			if( fs_format ("N0:")== fsOK)
				sys_status.peripheral_init.bit.nand_flash = 1;
			else
				sys_status.peripheral_init.bit.nand_flash = 0;
		}
	}
	else {
		fs_log ("Drive N0 initialization failed!\n");
		sys_status.peripheral_init.bit.nand_flash = 0;
	}
	
	stat = finit ("M0:");
	if (stat == fsOK) {
		stat = fmount ("M0:");
		if (stat == fsOK) {
			fs_log ("Drive M0 ready!\n");
			sys_status.peripheral_init.bit.sd_card = 1;
		}
		else {
			fs_log ("Drive M0 mount failed with error code %d\n", stat);
			if( fs_format ("M0:")== fsOK)
				sys_status.peripheral_init.bit.sd_card = 1;
			else
				sys_status.peripheral_init.bit.sd_card = 0;
		}
	}
	else {
		fs_log ("Drive M0 initialization failed!\n");
		sys_status.peripheral_init.bit.sd_card = 0;
	}
	
//	static uint32_t size;
//	size	= fs_fsize("n0:/cc");
	
	if( sys_status.peripheral_init.bit.nand_flash && sys_status.peripheral_init.bit.sd_card ){
		return FUN_OK;
	}
	else{
		return FUN_ERROR;
	}
}
