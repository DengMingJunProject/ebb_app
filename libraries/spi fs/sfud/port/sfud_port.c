/*
 * This file is part of the Serial Flash Universal Driver Library.
 *
 * Copyright (c) 2016-2018, Armink, <armink.ztl@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * 'Software'), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Function: Portable interface for each platform.
 * Created on: 2016-04-23
 */

#include <sfud.h>
#include <stdarg.h>
#include "system.h"
#include "fs_spi.h"

static char log_buf[256];

void sfud_log_debug(const char *file, const long line, const char *format, ...);

#define SPI_CS_PIN(x )  spi_fs_cs(x )
#define FLASH_SPIX      hspi_fs

typedef  SPI_HandleTypeDef *spi_user_data_t;

SemaphoreHandle_t    sfud_mutex_id;

static void spi_lock( const sfud_spi *spi )
{
    xSemaphoreTake( sfud_mutex_id, portMAX_DELAY );
}

static void spi_unlock( const sfud_spi *spi )
{
    xSemaphoreGive( sfud_mutex_id );
}

static void retry_delay_100us( void )
{
    /* 100 microsecond delay */
    vTaskDelay( 100 / portTICK_RATE_MS );
}


/**
 * SPI write data then read data
 */
static sfud_err spi_write_read(const sfud_spi *spi, const uint8_t *write_buf, size_t write_size, uint8_t *read_buf,
        size_t read_size) {
    sfud_err result = SFUD_SUCCESS;
    //  uint8_t send_data, read_data;
    spi_user_data_t spi_dev = ( spi_user_data_t ) spi->user_data;
    SPI_CS_PIN( 0 );
    if( write_size )
    {
        if( HAL_SPI_Transmit( spi_dev,( uint8_t * )write_buf,write_size,1000 ) != HAL_OK )
        {
            result = SFUD_ERR_WRITE;
        }
    }
    if( ( result == SFUD_SUCCESS ) && ( read_size ) )
    {
        HAL_SPI_Receive( spi_dev,( uint8_t * )read_buf,read_size,100 );
    }
    SPI_CS_PIN( 1 );
    return result;
}

#ifdef SFUD_USING_QSPI
/**
 * read flash data by QSPI
 */
static sfud_err qspi_read(const struct __sfud_spi *spi, uint32_t addr, sfud_qspi_read_cmd_format *qspi_read_cmd_format,
        uint8_t *read_buf, size_t read_size) {
    sfud_err result = SFUD_SUCCESS;

    /**
     * add your qspi read flash data code
     */

    return result;
}
#endif /* SFUD_USING_QSPI */

sfud_err sfud_spi_port_init(sfud_flash *flash) {
    sfud_err result = SFUD_SUCCESS;

    /**
     * add your port spi bus and device object initialize code like this:
     * 1. rcc initialize
     * 2. gpio initialize
     * 3. spi device initialize
     * 4. flash->spi and flash->retry item initialize
     *    flash->spi.wr = spi_write_read; //Required
     *    flash->spi.qspi_read = qspi_read; //Required when QSPI mode enable
     *    flash->spi.lock = spi_lock;
     *    flash->spi.unlock = spi_unlock;
     *    flash->spi.user_data = &spix;
     *    flash->retry.delay = null;
     *    flash->retry.times = 10000; //Required
     */
    sfud_mutex_id = xSemaphoreCreateMutex();
    flash->spi.wr = spi_write_read;
    flash->spi.lock = spi_lock;
    flash->spi.unlock = spi_unlock;
    flash->spi.user_data = &FLASH_SPIX;
    /* about 100 microsecond delay */
    flash->retry.delay = retry_delay_100us;
    /* adout 60 seconds timeout */
    flash->retry.times = 60 * 10000;
    return result;
}

/**
 * This function is print debug info.
 *
 * @param file the file which has call this function
 * @param line the line number which has call this function
 * @param format output format
 * @param ... args
 */
void sfud_log_debug(const char *file, const long line, const char *format, ...) {
    va_list args;

    /* args point to the first variable parameter */
    va_start(args, format);
	#ifdef __DEBUG_VERSION__
    DBG_LOG("[SFUD](%s:%ld) ", file, line);
	#endif
    /* must use vprintf to print */
    vsnprintf(log_buf, sizeof(log_buf), format, args);
	#ifdef __DEBUG_VERSION__
    DBG_LOG("%s\n", log_buf);
	#endif
    va_end(args);
}

/**
 * This function is print routine info.
 *
 * @param format output format
 * @param ... args
 */
void sfud_log_info(const char *format, ...) {
    va_list args;

    /* args point to the first variable parameter */
    va_start(args, format);
	#ifdef __DEBUG_VERSION__
    DBG_LOG("[SFUD]");
	#endif
    /* must use vprintf to print */
    vsnprintf(log_buf, sizeof(log_buf), format, args);
	#ifdef __DEBUG_VERSION__
    DBG_LOG("%s\n", log_buf);
	#endif
    va_end(args);
}
