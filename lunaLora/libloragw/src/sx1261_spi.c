/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
  (C)2019 Semtech

Description:
    Functions used to handle LoRa concentrator SX1250 radios.

License: Revised BSD License, see LICENSE.TXT file include in the project
*/


/* -------------------------------------------------------------------------- */
/* --- DEPENDANCIES --------------------------------------------------------- */

#include <stdint.h>     /* C99 types */
#include <stdio.h>      /* printf fprintf */
#include <unistd.h>     /* lseek, close */
#include <fcntl.h>      /* open */
#include <string.h>     /* memset */

#include <sys/ioctl.h>
#include <linux/spi/spidev.h>

#include "loragw_spi.h"
#include "loragw_aux.h"
#include "sx1261_spi.h"

/*
 * led stuff 
 */
static int  write_led_attribute(char * led, char * att, char * value) {
    char path[100];
    char str[20];
    sprintf(path, "/sys/class/leds/%s/%s", led, att);
    sprintf(str, "%s\n", value);
    FILE * fp = fopen(path, "w");
    if (!fp)
        return -1;

    fputs(str, fp);
    fclose(fp);
    return 0;
}

int system_led_on(char * led)
{

  write_led_attribute(led, "trigger", "none");
#if 1 // 010c
  write_led_attribute(led, "brightness", "1");
#else // 090a
  write_led_attribute(led, "brightness", "0");
#endif
	return 0;
}

int system_led_off(char * led)
{
	
  write_led_attribute(led, "trigger", "none");
#if 1 // 010c
  write_led_attribute(led, "brightness", "0");
#else // 090a
  write_led_attribute(led, "brightness", "1");
#endif
	return 0;
}

int system_led_blink(char * led, int delay_on, int delay_off)
{

  char delay_on_str[16];
  char delay_off_str[16];

  sprintf(delay_on_str, "%d", delay_on);
  sprintf(delay_off_str, "%d", delay_off);

  write_led_attribute(led, "trigger", "timer");
  write_led_attribute(led, "delay_on", delay_on_str);
  write_led_attribute(led, "delay_off", delay_off_str);
	return 0;
}

int system_led_shot(char * led)
{
  write_led_attribute(led, "trigger", "oneshot");
  write_led_attribute(led, "shot", "1");
	return 0;
}



/* -------------------------------------------------------------------------- */
/* --- PRIVATE MACROS ------------------------------------------------------- */

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#if DEBUG_LBT == 1
    #define DEBUG_MSG(str)                fprintf(stdout, str)
    #define DEBUG_PRINTF(fmt, args...)    fprintf(stdout,"%s:%d: "fmt, __FUNCTION__, __LINE__, args)
    #define CHECK_NULL(a)                if(a==NULL){fprintf(stderr,"%s:%d: ERROR: NULL POINTER AS ARGUMENT\n", __FUNCTION__, __LINE__);return LGW_SPI_ERROR;}
#else
    #define DEBUG_MSG(str)
    #define DEBUG_PRINTF(fmt, args...)
    #define CHECK_NULL(a)                if(a==NULL){return LGW_SPI_ERROR;}
#endif

/* -------------------------------------------------------------------------- */
/* --- PRIVATE CONSTANTS ---------------------------------------------------- */

#define WAIT_BUSY_SX1250_MS  1

#define LGW_BURST_CHUNK 1024

/* -------------------------------------------------------------------------- */
/* --- PUBLIC FUNCTIONS DEFINITION ------------------------------------------ */

int sx1261_spi_w(void *com_target, sx1261_op_code_t op_code, uint8_t *data, uint16_t size) {
#if 1
    int com_device;
    int cmd_size = 1; /* op_code */
    uint8_t out_buf[cmd_size + size];
    uint8_t command_size;
    struct spi_ioc_transfer k;
    int a, i;

    /* wait BUSY */
    wait_ms(WAIT_BUSY_SX1250_MS);

    /* check input variables */
    CHECK_NULL(com_target);
    CHECK_NULL(data);

    com_device = *(int *)com_target;

    /* prepare frame to be sent */
    out_buf[0] = (uint8_t)op_code;
    for(i = 0; i < (int)size; i++) {
        out_buf[cmd_size + i] = data[i];
    }
    command_size = cmd_size + size;

    /* I/O transaction */
    memset(&k, 0, sizeof(k)); /* clear k */
    k.tx_buf = (unsigned long) out_buf;
    k.len = command_size;
    k.speed_hz = SPI_SPEED;
    k.cs_change = 0;
    k.bits_per_word = 8;
	system_led_off("loranss");
    a = ioctl(com_device, SPI_IOC_MESSAGE(1), &k);
	system_led_on("loranss");

    /* determine return code */
    if (a != (int)k.len) {
        DEBUG_MSG("ERROR: SPI WRITE FAILURE\n");
        return LGW_SPI_ERROR;
    } else {
        DEBUG_MSG("Note: SPI write success\n");
        return LGW_SPI_SUCCESS;
    }
#else
    int spi_device;
    uint8_t command[3];
    uint8_t command_size;
    struct spi_ioc_transfer k[2];
    int size_to_do, chunk_size, offset;
    int byte_transfered = 0;
    int i;

    /* check input parameters */
    CHECK_NULL(com_target);
    CHECK_NULL(data);
    /*
    if (size == 0) {
        DEBUG_MSG("ERROR: BURST OF NULL LENGTH\n");
        return LGW_SPI_ERROR;
    }
    */

    spi_device = *(int *)com_target; /* must check that com_target is not null beforehand */

    /* prepare command byte */
    //command[0] = spi_mux_target;
		command[0] = (uint8_t)op_code;
    command_size = 1;

    size_to_do = size;

    /* I/O transaction */
    memset(&k, 0, sizeof(k)); /* clear k */
    k[0].tx_buf = (unsigned long) &command[0];
    k[0].len = command_size;
    k[0].speed_hz = SPI_SPEED;
    k[0].bits_per_word = 8;
    k[0].cs_change = 0;

    k[1].cs_change = 0;
    k[1].speed_hz = SPI_SPEED;
    k[1].bits_per_word = 8;

    int flag = 0;
    if (size_to_do == 0) {
	flag = 1;
    }
    for (i=0; flag || size_to_do > 0; ++i) {
	if (flag == 1) flag = 0;

        chunk_size = (size_to_do < LGW_BURST_CHUNK) ? size_to_do : LGW_BURST_CHUNK;
        offset = i * LGW_BURST_CHUNK;
        k[1].tx_buf = (unsigned long)(data + offset);
        k[1].len = chunk_size;
	system_led_off("loranss");
        byte_transfered += (ioctl(spi_device, SPI_IOC_MESSAGE(2), &k) - k[0].len );
	system_led_on("loranss");
        DEBUG_PRINTF("BURST WRITE: to trans %d # chunk %d # transferred %d \n", size_to_do, chunk_size, byte_transfered);
        size_to_do -= chunk_size; /* subtract the quantity of data already transferred */
    }

    /* determine return code */
    if (byte_transfered != size) {
        printf("ERROR: SPI BURST WRITE FAILURE, byte_transfered:%d, size:%d\n", byte_transfered, size);
        return LGW_SPI_ERROR;
    } else {
        DEBUG_MSG("Note: SPI burst write success\n");
        return LGW_SPI_SUCCESS;
    }
#endif
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

int sx1261_spi_r(void *com_target, sx1261_op_code_t op_code, uint8_t *data, uint16_t size) {
#if 1
    int com_device;
    int cmd_size = 1; /* op_code */
    uint8_t out_buf[cmd_size + size];
    uint8_t command_size;
    uint8_t in_buf[ARRAY_SIZE(out_buf)];
    struct spi_ioc_transfer k;
    int a, i;

    /* wait BUSY */
    wait_ms(WAIT_BUSY_SX1250_MS);

    /* check input variables */
    CHECK_NULL(com_target);
    CHECK_NULL(data);

    com_device = *(int *)com_target;

    /* prepare frame to be sent */
    out_buf[0] = (uint8_t)op_code;
    for(i = 0; i < (int)size; i++) {
        out_buf[cmd_size + i] = data[i];
    }
    command_size = cmd_size + size;

    /* I/O transaction */
    memset(&k, 0, sizeof(k)); /* clear k */
    k.tx_buf = (unsigned long) out_buf;
    k.rx_buf = (unsigned long) in_buf;
    k.len = command_size;
    k.cs_change = 0;
	system_led_off("loranss");
    a = ioctl(com_device, SPI_IOC_MESSAGE(1), &k);
	system_led_on("loranss");

    /* determine return code */
    if (a != (int)k.len) {
        DEBUG_MSG("ERROR: SPI READ FAILURE\n");
        return LGW_SPI_ERROR;
    } else {
        DEBUG_MSG("Note: SPI read success\n");
        //*data = in_buf[command_size - 1];
        memcpy(data, in_buf + cmd_size, size);
        return LGW_SPI_SUCCESS;
    }
#else
    int spi_device;
    uint8_t command[128];
    uint8_t command_size;
    struct spi_ioc_transfer k[2];
    int size_to_do, chunk_size, offset;
    int byte_transfered = 0;
    int i;

    uint8_t out_buf[command_size + size];

    /* check input parameters */
    CHECK_NULL(com_target);
    CHECK_NULL(data);
    /*
    if (size == 0) {
        DEBUG_MSG("ERROR: BURST OF NULL LENGTH\n");
        return LGW_SPI_ERROR;
    }
    */

    spi_device = *(int *)com_target; /* must check that com_target is not null beforehand */

    /* prepare command byte */
    //command[0] = spi_mux_target;
		command[0] = (uint8_t)op_code;
    command_size = 1;

    size_to_do = size;

    /* I/O transaction */
    memset(&k, 0, sizeof(k)); /* clear k */
    k[0].tx_buf = (unsigned long) &command[0];
#if 0 // 010c
    k[0].rx_buf = out_buf;
    k[0].len = command_size;
#else
    //k[0].rx_buf = out_buf;
    k[0].len = command_size + size;
#endif

    k[0].len = command_size;
    k[0].speed_hz = SPI_SPEED;
    k[0].bits_per_word = 8;
    k[0].cs_change = 0;

    k[1].speed_hz = SPI_SPEED;
    k[1].bits_per_word = 8;
    k[1].cs_change = 0;

    int flag = 0;
    if (size_to_do == 0) {
	flag = 1;
    }
    for (i=0; flag || size_to_do > 0; ++i) {
	if (flag == 1) flag = 0;

        chunk_size = (size_to_do < LGW_BURST_CHUNK) ? size_to_do : LGW_BURST_CHUNK;
        offset = i * LGW_BURST_CHUNK;
#if 0	// 010c
        k[1].tx_buf = (unsigned long)(data + offset);
        k[1].rx_buf = (unsigned long)(out_buf + offset + k[0].len);
#else
        //k[1].tx_buf = (unsigned long)(data + offset);
        k[1].rx_buf = (unsigned long)(out_buf + offset + k[0].len);
#endif
        k[1].len = chunk_size;
	system_led_off("loranss");
        byte_transfered += (ioctl(spi_device, SPI_IOC_MESSAGE(2), &k) - k[0].len );
	system_led_on("loranss");
        DEBUG_PRINTF("BURST READ: to trans %d # chunk %d # transferred %d \n", size_to_do, chunk_size, byte_transfered);
        size_to_do -= chunk_size;  /* subtract the quantity of data already transferred */
    }

		if (1) {
			int i = 0;
			for (i = 0; i < k[0].len + size; i++) {
				printf("[%02X] ", out_buf[i]&0xff);
			}
			printf("\n");
		}

  
		printf("byte_transfered:%d, size:%d\n", byte_transfered, size);
    /* determine return code */
    if (byte_transfered != size) {
        DEBUG_MSG("ERROR: SPI BURST READ FAILURE\n");
        return LGW_SPI_ERROR;
    } else {
	//memcpy(data, out_buf + command_size, size);
	memcpy(data, out_buf + k[0].len, size);
        DEBUG_MSG("Note: SPI burst read success\n");
        return LGW_SPI_SUCCESS;
    }
#endif
}

/* --- EOF ------------------------------------------------------------------ */
