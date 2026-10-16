#include "sht31.h"
#include "i2c.h"

#include <stdint.h>
#include <stdbool.h>
#include <time.h>

#define SHT31_CMD_MSB 0x2C
#define SHT31_CMD_LSB 0x06

#define POLYNOMIAL 0x31

// crc8: SHT31 checksum over len bytes (CRC-8, polynomial 0x31, init 0xFF)
// Standard algorithm from SHT31 datasheet
static uint8_t crc8(const uint8_t *data, int len) {
    uint8_t crc = 0xFF;

    for (int i = 0; i < len; i++) {
        crc ^= *data;
        data++;
        for (int bit = 0; bit < 8; bit++) {
            crc = (crc & 0x80) ? (crc << 1) ^ POLYNOMIAL : (crc << 1);
        }
    }
    return crc;
}

// crc_ok: returns true if the 2-byte word at data matches expected crc
static bool crc_ok(const uint8_t *data, uint8_t crc) {
    return crc8(data, 2) == crc;
}

// sht31_read: take one reading (temperature (C), relative humidity (%))
// usage: if (sht31_read(fd, &t, &rh) != 0) fprintf(stderr, "sht read fail");
// preconditions: fd from i2c_open at SHT31_ADDR, temp_c and rh non-NULL
// steps: 1. write the measurement command to the device
//        2. wait for the conversion to finish
//        3. read back temperature and humidity
//        4. check the CRC (checksum) on each
int sht31_read(int fd, double *temp_c, double *rh) {
    uint8_t cmd[2] = { SHT31_CMD_MSB, SHT31_CMD_LSB };
    if (i2c_write(fd, cmd, 2) != 0) {
        return -1;
    }

    // datasheet says 15ms max measurement time for high repeatability
    struct timespec wait = { .tv_sec = 0, .tv_nsec = 20L * 1000 * 1000 };
    nanosleep(&wait, NULL);

    uint8_t buf[6];
    if (i2c_read(fd, buf, 6) != 0) {
        return -1;
    }
    if (!crc_ok(buf, buf[2]) || !crc_ok(buf + 3, buf[5])) {
        return -1;
    }

    uint16_t raw_t = (uint16_t)((buf[0] << 8) | buf[1]);
    uint16_t raw_h = (uint16_t)((buf[3] << 8) | buf[4]);
    // apply conversion formula from datasheet
    *temp_c = -45.0 + 175.0 * raw_t / 65535.0;
    *rh = 100.0 * raw_h / 65535.0;
    return 0;
}

