/**
 * \file
 *
 * \brief Unit tests for FatFS service.
 *
 * Copyright (c) 2011-2013 Atmel Corporation. All rights reserved.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an
 *    Atmel microcontroller product.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * \asf_license_stop
 *
 */
#include <asf.h>
#include <string.h>
#include "conf_test.h"
#include "memories_initialization.h"

/**
 * \mainpage
 *
 * \section intro Introduction
 * This is the unit test application for the FatFS service.
 * It consists of test cases for the following functionality:
 * - Mount the FatFS
 * - Make the file system
 * - Create a new file
 * - Write data to the file
 * - Read the data from the file and compare
 *
 * \section files Main Files
 * - \ref unit_tests.c
 * - \ref conf_access.h
 * - \ref conf_fatfs.h
 * - \ref conf_virtual_mem.h
 * - \ref conf_test.h
 * - \ref conf_board.h
 * - \ref conf_clock.h
 * - \ref conf_uart_serial.h
 *
 * \section device_info Device Info
 * All SAM devices can be used.
 * This example has been tested with the following setup:
 * - sam3x8h_sam3x_ek
 * - sam4s16c_sam4s_ek
 * - sam4s16c_sam4s_xplained
 * - XMEGA-A1 Xplained
 * - sam4e16e_sam4e_ek
 *
 * \section compinfo Compilation info
 * This software was written for the GNU GCC and IAR for ARM. Other compilers
 * may or may not work.
 *
 * \section contactinfo Contact Information
 * For further information, visit <a href="http://www.atmel.com/">Atmel</a>.\n
 * Support and FAQ: http://support.atmel.no/
 */

/* ! \name Unit test configuration */
/* @{ */

/**
 * \def CONF_TEST_FATFS
 * \brief Test the functions provided by the FatFS driver.
 */
/* @} */

/* FatFS mount root directory*/
#define STR_ROOT_DIRECTORY ""

/** Size of the file to write/read.*/
#define DATA_SIZE 2048

/** Test settings: Number of bytes to test */
#define TEST_SIZE   (4 * 1024)

/** Logical disk device number */
#define DISK_DEV_NUM    LUN_ID_0

/* Read/write buffer */
static uint8_t data_buffer[DATA_SIZE];

/* File name to be validated */
const char *file_name = STR_ROOT_DIRECTORY "Basic.bin";

/**
 * \brief Do FatFS tests.
 *
 * \return Test result, 1: success.
 */
static void run_fatfs_test(const struct test_case *test)
{
	uint32_t i;
	UINT byte_to_read;
	UINT byte_read;
	UINT byte_written;

	FRESULT res;
	DIR dirs;

	/* Declare these as static to avoid stack usage.
	 * They each contain an array of maximum sector size.
	 */
	static FATFS fs;
	static FIL file_object;

	/* Clear file system object */
	memset(&fs, 0, sizeof(FATFS));
	res = f_mount(DISK_DEV_NUM, &fs);
	if (res != FR_OK) {
		test_assert_true(test, 0, "FatFS mount error!");
	}

	/* Test if the disk is formated */
	res = f_opendir(&dirs, STR_ROOT_DIRECTORY);

	if ((res == FR_OK) || (res == FR_NO_FILESYSTEM)) {
		/* Format disk */
		res = f_mkfs(DISK_DEV_NUM, /* Drv */
				0, /* FDISK partition */
				512); /* AllocSize */

		if (res != FR_OK) {
			test_assert_true(test, 0,
					"FatFS make file system error!");
		}
	} else if (res != FR_OK) {
		test_assert_true(test, 0, "FatFS opendir error!");
	}

	/* Create a new file */
	res = f_open(&file_object, (char const *)file_name,
			FA_CREATE_ALWAYS | FA_WRITE);
	if (res != FR_OK) {
		test_assert_true(test, 0, "FatFS file open error!");
	}

	/* Write a checkerboard pattern in the buffer */
	for (i = 0; i < sizeof(data_buffer); i++) {
		if ((i & 1) == 0) {
			data_buffer[i] = (i & 0x55);
		} else {
			data_buffer[i] = (i & 0xAA);
		}
	}

	for (i = 0; i < TEST_SIZE; i += DATA_SIZE) {
		res = f_write(&file_object, data_buffer, DATA_SIZE,
				&byte_written);

		if (res != FR_OK) {
			test_assert_true(test, 0, "FatFS file write error!");
		}
	}

	/* Close the file */
	res = f_close(&file_object);
	if (res != FR_OK) {
		test_assert_true(test, 0, "FatFS file close error!");
	}

	/* Open the file */

	res = f_open(&file_object, (char const *)file_name,
			FA_OPEN_EXISTING | FA_READ);
	if (res != FR_OK) {
		test_assert_true(test, 0, "FatFS file open error!");
	}

	/* Read file */
	memset(data_buffer, 0, DATA_SIZE);
	byte_to_read = file_object.fsize;

	for (i = 0; i < byte_to_read; i += DATA_SIZE) {
		res = f_read(&file_object, data_buffer, DATA_SIZE, &byte_read);
		if (res != FR_OK) {
			test_assert_true(test, 0, "FatFS file read error!");
		}
	}

	/* Close the file*/
	res = f_close(&file_object);
	if (res != FR_OK) {
		test_assert_true(test, 0, "FatFS file close error!");
	}

	/* Compare read data with the expected data */
	for (i = 0; i < sizeof(data_buffer); i++) {
		if (!((((i & 1) == 0) && (data_buffer[i] == (i & 0x55))) ||
				(data_buffer[i] == (i & 0xAA)))) {
			test_assert_true(test, 0, "FatFS data compare error!");
		}
	}
}

/**
 * \brief Run FatFS driver unit tests.
 */
int main(void)
{
	const usart_serial_options_t usart_serial_options = {
		.baudrate   = CONF_TEST_BAUDRATE,
		.paritytype = CONF_TEST_PARITY,
#if !SAM
		.charlength = CONF_TEST_CHARLENGTH,
		.stopbits   = CONF_TEST_STOPBITS,
#endif
	};

	/* Initialize the system clock and board */
	sysclk_init();
	board_init();

	/* Enable the debug uart */
#if SAM
	sysclk_enable_peripheral_clock(CONSOLE_UART_ID);
#endif
	stdio_serial_init(CONF_TEST_USART, &usart_serial_options);

	/* Intialize the memory device */
	memories_initialization();

	/* Define all the test cases */
	DEFINE_TEST_CASE(fatfs_test, NULL, run_fatfs_test, NULL,
			"FatFS read/write test");

	/* Put test case addresses in an array */
	DEFINE_TEST_ARRAY(fatfs_tests) = {
		&fatfs_test
	};

	/* Define the test suite */
	DEFINE_TEST_SUITE(fatfs_suite, fatfs_tests,
			"SAM FatFS driver test suite");

	/* Run all tests in the test suite */
	test_suite_run(&fatfs_suite);

	while (1) {
		/* Busy-wait forever. */
	}
}