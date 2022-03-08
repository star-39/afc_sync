﻿#define _CRT_SECURE_NO_WARNINGS
#include <libimobiledevice/afc.h>
#include <libimobiledevice/libimobiledevice.h>
#include <libimobiledevice/lockdown.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <direct.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/utime.h>
#include <time.h>

#define TOOL_NAME "idevice_afc"
#define AFC_SERVICE_NAME "com.apple.afc"

#define mkdir _mkdir
#define utime _utime
#define utimbuf _utimbuf
#define stat _stat


int skip_count;

struct options {
    char* src;
    char* dst;
    int skip_exist;
    int max_skips;
    int network;
} options;

static int comp(const void* a, const void* b)
{
    return strcmp(*(const char**)b, *(const char**)a);
}

void reverse_sort(char* arr[])
{
    int len = 0;
    for (; arr[len]; len++)
        ;
    qsort(arr, len, sizeof(char*), comp);
}

void pull_afc(afc_client_t afc, char* src, char* dst, long long st_size, time_t st_mtime)
{
    printf("pulling [ %s --> %s ] (st_size: %lld, st_mtime: %lld)\r\n", src, dst, st_size, st_mtime);
    struct stat stat_buf;
    struct _utimbuf utime_buf;
    if (stat(dst, &stat_buf) == 0) {
        if (options.skip_exist && stat_buf.st_size == st_size) {
            printf("dst file exists! skipping...\r\n");
            if (options.max_skips != 0)
                skip_count += 1;
            return;
        }
    }

    char* buf = (char*)malloc(st_size);
    uint32_t bytes = 0;
    uint32_t bytes_read = 0;
    FILE* fp;
    uint64_t handle;
    afc_file_mode_t mode = AFC_FOPEN_RDONLY;
    fp = fopen(dst, "wb");
    afc_error_t err = afc_file_open(afc, src, mode, &handle);
    if (err != AFC_E_SUCCESS) {
        printf("error opening file!\n\r");
        goto cleanup;
    }

    while (bytes_read < st_size) {
        afc_error_t errr = afc_file_read(afc, handle, buf, st_size, &bytes);
        if (errr != AFC_E_SUCCESS) {
            printf("error reading file!\n\r");
            goto cleanup;
        }
        fwrite(buf, bytes, 1, fp);
        bytes_read += bytes;
    }

    fclose(fp);

    utime_buf.modtime = st_mtime;
    if (_utime(dst, &utime_buf) == -1) {
        printf("pull OK. Failed setting mtime\n\r");
    } else {
        printf("pull OK.\n\r");
    }

cleanup:
    fclose(fp);
    afc_file_close(afc, handle);
    free(buf);

}

int check_file_isdir_afc(afc_client_t afc, char* src, long long* st_size, time_t* st_mtime)
{
    char** info = NULL;
    afc_error_t err = afc_get_file_info(afc, src, &info);
    if (err != AFC_E_SUCCESS) {
        printf("failed getting file info!\r\n");
        return 1;
    }
    for (int i = 0; info[i]; i += 2) {
        if (!strcmp(info[i], "st_size")) {
            *st_size = atoll(info[i + 1]);
        } else if (!strcmp(info[i], "st_mtime")) {
            *st_mtime = (time_t)(atoll(info[i + 1]) / 1000000000);
        }
        if (!strcmp(info[i], "st_ifmt")) {
            if (!strcmp(info[i + 1], "S_IFDIR")) {
                afc_dictionary_free(info);
                return 1;
            }
        }
    }
    afc_dictionary_free(info);
    return 0;
}

void init_pull_afc(afc_client_t afc, char* src, char* dst)
{
    long long st_size = 0;
    time_t st_mtime = 0;
    char** dirs = NULL;
    if (check_file_isdir_afc(afc, src, &st_size, &st_mtime)) {

        mkdir(dst);
        afc_read_directory(afc, src, &dirs);
        reverse_sort(dirs);
        for (int i = 0; dirs[i]; i++) {
            if (skip_count > options.max_skips) {
                printf("max skips reached! exiting...\r\n");
                break;
            }
            if (!strcmp(dirs[i], ".") || !strcmp(dirs[i], "..")) {
                continue;
            }
            char src_new[2048];
            strcpy(src_new, src);
            strcat(src_new, "/");
            strcat(src_new, dirs[i]);
            char dst_new[2048];
            strcpy(dst_new, dst);
            strcat(dst_new, "/");
            strcat(dst_new, dirs[i]);
            if (check_file_isdir_afc(afc, src_new, &st_size, &st_mtime)) {
                mkdir(dst_new);
                init_pull_afc(afc, src_new, dst_new);
            } else {
                pull_afc(afc, src_new, dst_new, st_size, st_mtime);
            }
        }
        afc_dictionary_free(dirs);
    } else {
        pull_afc(afc, src, dst, st_size, st_mtime);
    }
}

int main(int argc, // Number of strings in array argv
    char* argv[], // Array of command-line argument strings
    char** envp) // Array of environment variable strings)
{
    time_t time_start = time(NULL);

    options.skip_exist = 0;
    options.max_skips = 0;
    skip_count = 0;
    options.network = 0;

    for (int c = 0; c < argc; c++) {
        if (!strcmp(argv[c], "--src")) {
            options.src = argv[c + 1];
        } else if (!strcmp(argv[c], "--dst")) {
            options.dst = argv[c + 1];
        } else if (!strcmp(argv[c], "--skip-exist")) {
            options.skip_exist = 1;
        } else if (!strcmp(argv[c], "--max-skips")) {
            options.max_skips = atoi(argv[c + 1]);
        } else if (!strcmp(argv[c], "--network")) {
            options.network = 1;
        }
    }

    const char* udid = NULL;
    idevice_t device = NULL;
    lockdownd_service_descriptor_t service = NULL;

    if (idevice_new_with_options(&device, udid, (options.network) ? IDEVICE_LOOKUP_NETWORK : IDEVICE_LOOKUP_USBMUX) != IDEVICE_E_SUCCESS) {
        printf("ERROR: No device found.\n");
        return -1;
    }
    lockdownd_client_t lockdown = NULL;
    lockdownd_error_t lerr = lockdownd_client_new_with_handshake(device, &lockdown, TOOL_NAME);
    if (lerr != LOCKDOWN_E_SUCCESS) {
        idevice_free(device);
        printf("ERROR: Could not connect to lockdownd, error code %d\n", lerr);
        return -1;
    }
    if ((lockdownd_start_service(lockdown, AFC_SERVICE_NAME, &service) != LOCKDOWN_E_SUCCESS) || !service) {
        lockdownd_client_free(lockdown);
        idevice_free(device);
        printf("failed to start service!");
    }

    afc_client_t afc = NULL;
    afc_client_new(device, service, &afc);
    lockdownd_client_free(lockdown);
    lockdown = NULL;

    if (afc) {
        printf("afc OK!\n\r");
        init_pull_afc(afc, options.src, options.dst);
    }

    printf("cleaning up...\r\n");
    afc_client_free(afc);
    lockdownd_client_free(lockdown);
    idevice_free(device);

    time_t time_end = time(NULL);
    printf("time elapsed: %lld s\r\n", time_end - time_start);

    return 0;
}
