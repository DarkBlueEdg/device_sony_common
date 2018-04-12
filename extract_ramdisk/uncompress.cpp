/*
 * This binary extracts the ramdisk from a Sony ELF or ANDROID boot or
 * recovery image, gunzips it, checks to make sure that it is not a
 * stock recovery ramdisk, and then places it in the destination.
 * The goal is to use the FOTA partition to store a recovery image
 * and extract the ramdisk to be used during boot on Sony devices that
 * typically have a recovery-in-boot setup due to Sony disabling the
 * ability to boot the FOTA partition on unlocked devices. This binary
 * allows the recovery ramdisk to be separate from the boot partition
 * so that users can decide what recovery they want to have instead of
 * being stuck with whatever recovery their kernel or ROM maker decided
 * to include in the boot image.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 *
 * The code was written from scratch by Dees_Troy dees_troy at
 * yahoo
 *
 * Copyright (c) 2013
 * Copyright (C) 2016 The CyanogenMod Project
 * Copyright (C) 2018 The OmniRom Project
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "gelf.h"
#include "bootimg.h"
#include "zlib.h"

#include "uncompress.h"

size_t uncompress_gzip_memory(const byte_p compressed_data,
	const size_t compressed_data_size, const byte_p uncompressed_data,
	const size_t uncompressed_max_size) {
	z_stream zInfo = {0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	zInfo.avail_in = compressed_data_size;
	zInfo.total_in = compressed_data_size;
	zInfo.avail_out = uncompressed_max_size;
	zInfo.total_out = uncompressed_max_size;
	zInfo.next_in = (unsigned char *)compressed_data;
	zInfo.next_out = (unsigned char *)uncompressed_data;
	size_t return_value = 0;
	unsigned long err = inflateInit2(&zInfo, 16 + MAX_WBITS); // zlib function

	if (err == Z_OK) {
		err = inflate( &zInfo, Z_FINISH); // zlib function
		if (err == Z_STREAM_END) {
			return_value = zInfo.total_out;
		} else {
			printf("gunzip error -- Err:inflate %lu\n", err);
		}
	} else {
		printf("gunzip error -- Err:inflateInit2 %lu\n", err);
	}
	inflateEnd(&zInfo);
	return(return_value);
}

size_t uncompress_memory(byte_p uncompressed_buffer, byte_p buffer, unsigned long file_size) {
	if (uncompressed_buffer == NULL) {
		free(buffer);
		printf("Unable to malloc memory for gunzip.\nFailed\n");
		return -1;
	}
	size_t uncompressed_size;
	uncompressed_size = uncompress_gzip_memory(buffer, file_size,
		uncompressed_buffer, MEMORY_BUFFER_SIZE);
	free(buffer);
	if (uncompressed_size <= 0) {
		free(uncompressed_buffer);
		printf("Failed to gunzip\n");
		return -1;
	}
	printf("Original size: %lu, gunzipped: %zu\n", file_size,
		uncompressed_size);
	buffer = uncompressed_buffer;
	return(uncompressed_size);
}

