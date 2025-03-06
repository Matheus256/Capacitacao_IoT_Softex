#include "ssd_card.h"
#include "board.h"

#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"

#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE
#include "esp_log.h"

// https://github.com/espressif/esp-idf/blob/master/components/driver/include/driver/sdspi_host.h
#if CONFIG_IDF_TARGET_ESP32 || CONFIG_IDF_TARGET_ESP32S2
#define SDSPI_DEFAULT_HOST HSPI_HOST
#define SDSPI_DEFAULT_DMA  SDSPI_DEFAULT_HOST
#else
#define SDSPI_DEFAULT_HOST SPI2_HOST
#define SDSPI_DEFAULT_DMA  SPI_DMA_CH_AUTO
#endif

static const char *TAG = "sd_card_example";

#define MOUNT_POINT "/sdcard"
const char *file_hello = MOUNT_POINT"/hello.txt";

void ssd_card_init(void){
  //https://www.esp32.com/viewtopic.php?t=1183
  gpio_set_pull_mode(PIN_NUM_MISO, GPIO_PULLUP_ONLY);
  gpio_set_pull_mode(PIN_NUM_MOSI, GPIO_PULLUP_ONLY);
  gpio_set_pull_mode(PIN_NUM_CLK, GPIO_PULLUP_ONLY);
  gpio_set_pull_mode(PIN_NUM_CS, GPIO_PULLUP_ONLY);

  gpio_set_direction(PIN_NUM_MISO, GPIO_MODE_INPUT);
  gpio_set_direction(PIN_NUM_MOSI, GPIO_MODE_OUTPUT);
  gpio_set_direction(PIN_NUM_CLK, GPIO_MODE_OUTPUT);
  gpio_set_direction(PIN_NUM_CS, GPIO_MODE_OUTPUT);

  esp_err_t ret;

    // Options for mounting the filesystem.
    // If format_if_mount_failed is set to true, SD card will be partitioned and
    // formatted in case when mounting fails.
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
#ifdef CONFIG_EXAMPLE_FORMAT_IF_MOUNT_FAILED
        .format_if_mount_failed = true,
#else
        .format_if_mount_failed = false,
#endif // EXAMPLE_FORMAT_IF_MOUNT_FAILED
        .max_files = 5,
        .allocation_unit_size = 16 * 1024
    };
    sdmmc_card_t *card;
    const char mount_point[] = MOUNT_POINT;
    printf("Initializing SD card\n");

    // Use settings defined above to initialize SD card and mount FAT filesystem.
    // Note: esp_vfs_fat_sdmmc/sdspi_mount is all-in-one convenience functions.
    // Please check its source code and implement error recovery when developing
    // production applications.
    printf("Using SPI peripheral\n");

    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    // https://github.com/espressif/esp-idf/issues/2478
    //host.max_freq_khz = 400;
    host.max_freq_khz = 5000;
    
    spi_bus_config_t bus_cfg = {
        .mosi_io_num = PIN_NUM_MOSI,
        .miso_io_num = PIN_NUM_MISO,
        .sclk_io_num = PIN_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4000,
    };
    ret = spi_bus_initialize(host.slot, &bus_cfg, SDSPI_DEFAULT_DMA);

    if (ret != ESP_OK) {
        printf("Failed to initialize bus.\n");
        return;
    }

    // This initializes the slot without card detect (CD) and write protect (WP) signals.
    // Modify slot_config.gpio_cd and slot_config.gpio_wp if your board has these signals.
    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = PIN_NUM_CS;
    slot_config.host_id = host.slot;
    
    // https://www.esp32.com/viewtopic.php?t=27338
    //slot_config.flags |= SDMMC_SLOT_FLAG_INTERNAL_PULLUP;

    printf("Mounting filesystem\n");
    ret = esp_vfs_fat_sdspi_mount(mount_point, &host, &slot_config, &mount_config, &card);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            printf("Failed to mount filesystem. "
                     "If you want the card to be formatted, set the CONFIG_EXAMPLE_FORMAT_IF_MOUNT_FAILED menuconfig option.\n");
        } else {
            printf("Failed to initialize the card (%s). "
                     "Make sure SD card lines have pull-up resistors in place.\n", esp_err_to_name(ret));
        }
        return;
    }

    printf("Filesystem mounted\n");
}

void ssd_card_write(float *mean_pointer, double *deviation_pointer){
    printf( "Opening file %s\n", file_hello);
    FILE *f = fopen(file_hello, "w");
    if (f == NULL) {
        printf("Failed to open file for writing\n");
        return;
    }
    fprintf(f, "Media = %.2f, Desvio padrão = %.2lf\n", *(mean_pointer), *(deviation_pointer));
    fclose(f);
    printf("File written\n");
}

void ssd_card_read(){
  //printf("Reading file %s\n", file_foo);
  printf("Reading file...\n");
    FILE *f = fopen(file_hello, "r");
    if (f == NULL) {
        printf("Failed to open file for reading\n");
        return;
    }

    // Read a line from file
    char line[64];
    fgets(line, sizeof(line), f);
    fclose(f);

    char *pos = strchr(line, '\n');
    if (pos) {
      *pos = '\0';
    }
    printf("Read from file: '%s'\n", line);
}

