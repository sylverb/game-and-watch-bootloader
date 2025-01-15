#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "ff.h"  // For FatFs
#include "tar.h"

#define BLOCKSIZE 512  // TAR block size

/* Create a directory, including parent directories if necessary */
static FRESULT create_dir(const char *path) {
    char temp_path[256];
    size_t len = strlen(path);

    // Ensure the path is not too long
    if (len >= sizeof(temp_path)) {
        return FR_INVALID_NAME;
    }

    strcpy(temp_path, path);

    // Remove trailing '/' if it exists
    if (temp_path[len - 1] == '/') {
        temp_path[len - 1] = '\0';
    }

    // Traverse the path and create each directory
    for (char *p = temp_path + 1; *p; ++p) {
        if (*p == '/') {
            *p = '\0';  // Temporarily terminate the string
            FRESULT res = f_mkdir(temp_path);
            if (res != FR_OK && res != FR_EXIST) {
                return res;  // Return error if directory creation fails
            }
            *p = '/';  // Restore the separator
        }
    }

    // Create the final directory
    FRESULT res = f_mkdir(temp_path);
    return (res == FR_EXIST) ? FR_OK : res;
}

/* Open a file for writing, creating parent directories as necessary */
static FRESULT create_file(const char *path, FIL *file) {
    FRESULT res;

    // Attempt to open the file
    res = f_open(file, path, FA_WRITE | FA_CREATE_ALWAYS);
    if (res == FR_OK) {
        return FR_OK;  // File opened successfully
    }

    // If the file could not be opened, create parent directories
    char temp_path[256];
    strcpy(temp_path, path);
    char *p = strrchr(temp_path, '/');
    if (p != NULL) {
        *p = '\0';  // Temporarily truncate to parent directory
        res = create_dir(temp_path);
        if (res != FR_OK) {
            return res;  // Return error if directory creation fails
        }
        *p = '/';  // Restore the original path
    }

    // Retry opening the file
    return f_open(file, path, FA_WRITE | FA_CREATE_ALWAYS);
}

/* Parse an octal number from a string */
static unsigned long parseoct(const char *p, size_t n) {
    unsigned long value = 0;

    while ((*p < '0' || *p > '7') && n > 0) {
        ++p;
        --n;
    }
    while (*p >= '0' && *p <= '7' && n > 0) {
        value = (value << 3) | (*p - '0');
        ++p;
        --n;
    }
    return value;
}

/* Verify the checksum of a TAR header block */
static int verify_checksum(const char *p) {
    unsigned int checksum = 0, stored_checksum = parseoct(p + 148, 8);
    for (int i = 0; i < BLOCKSIZE; i++) {
        checksum += (i >= 148 && i < 156) ? ' ' : (unsigned char)p[i];
    }
    return (checksum == stored_checksum);
}

/* Check if a block is the end of the archive (512 zero bytes) */
static int is_end_of_archive(const char *p) {
    for (int i = 0; i < BLOCKSIZE; i++) {
        if (p[i] != '\0') {
            return 0;
        }
    }
    return 1;
}

/* Extract a TAR file from FatFs */
bool extract_tar(const char *tar_path, const char *dest_path, progress_callback_t progress_callback) {
    char block[BLOCKSIZE];
    char tmp_path[256];
    char file_path[256];
    bool success = true;
    FIL out_file;
    FIL tar_file;
    FRESULT res;
    UINT br, bw;

    res = f_open(&tar_file, tar_path, FA_READ);
    if (res != FR_OK) {
        return false;
    }

    unsigned long total_tar_size = f_size(&tar_file);
    unsigned long current_position = 0;

    // Read blocks from the TAR file
    while ((res = f_read(&tar_file, block, BLOCKSIZE, &br)) == FR_OK && br == BLOCKSIZE) {
        current_position += BLOCKSIZE;

        // Check for end of archive
        if (is_end_of_archive(block)) {
            if (progress_callback) {
                progress_callback(100, file_path);
            }
            break;
        }

        // Verify the checksum
        if (!verify_checksum(block)) {
            success = false;
            break;
        }

        // Get file name and size
        strncpy(tmp_path, block, 100);
        tmp_path[100] = '\0';  // Ensure null-termination
        unsigned long file_size = parseoct(block + 124, 12);

        if (strlen(dest_path) + strlen(tmp_path) + 2 > sizeof(file_path)) {
            success = false;
            break;
        }

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-truncation"
        // Build full path for the extracted file
        snprintf(file_path, sizeof(file_path), "%s/%s", dest_path, tmp_path);
#pragma GCC diagnostic pop

        if (block[156] == '5') {  // Directory
            res = create_dir(file_path);
            if (res != FR_OK) {
                break;
            }
        } else {  // File
            res = create_file(file_path, &out_file);
            if (res != FR_OK) {
                success = false;
                break;
            }

            // Write the file content
            while (file_size > 0) {
                UINT to_read = (file_size > BLOCKSIZE) ? BLOCKSIZE : file_size;
                res = f_read(&tar_file, block, BLOCKSIZE, &br);
                current_position += BLOCKSIZE;
                if (res != FR_OK || br < BLOCKSIZE) {
                    success = false;
                    f_close(&out_file);
                    break;
                }

                res = f_write(&out_file, block, to_read, &bw);
                if (res != FR_OK || bw < to_read) {
                    f_close(&out_file);
                    break;
                }

                file_size -= to_read;
                if (progress_callback) {
                    unsigned int percentage = (unsigned int)((current_position * 100) / total_tar_size);
                    progress_callback(percentage, file_path);
                }
            }

            f_close(&out_file);
        }

        // Skip any padding
        unsigned long skip = ((file_size + BLOCKSIZE - 1) / BLOCKSIZE) * BLOCKSIZE - file_size;
        if (skip > 0) {
            f_lseek(&tar_file, f_tell(&tar_file) + skip);
        }
    }
    f_close(&tar_file);
    return success;
}
