#include "i2c.h"

#include <stdint.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>

// Generic i2c interface to read and write to each sensor
// One descriptor per device

// i2c_open: opens a connection/bus to the device and returns its descriptor
// usage: int fd = i2c_open("/dev/i2c-1", 0x5A); -> fd, or -1 on failure
// preconditions: bus is a valid i2c device path, addr is the 7-bit slave
//                address
int i2c_open(const char *bus, uint8_t addr) {
    int fd = open(bus, O_RDWR);
    if (fd < 0) {
        return -1;
    }
    if (ioctl(fd, I2C_SLAVE, addr) < 0) { // i2c living in ancient egypt
        close(fd);
        return -1;
    }
    return fd;
}

// i2c_write: writes n bytes to the device with descriptor fd
// usage: if (i2c_write(fd, cmd, 2) != 0) fprintf(stderr, "write fail");
// preconditions: fd from i2c_open, buf holds at least n bytes
int i2c_write(int fd, const uint8_t *buf, size_t n) {
    return (write(fd, buf, n) == (ssize_t)n) ? 0 : -1;
}

// i2c_read: reads n bytes from device with descriptor fd
// usage: if (i2c_read(fd, buf, 6) != 0) fprintf(stderr, "read fail");
// preconditions: fd from i2c_open, buf holds at least n bytes
int i2c_read(int fd, uint8_t *buf, size_t n) {
    return (read(fd, buf, n) == (ssize_t)n) ? 0 : -1;
}

// i2c_read_word: SMBus read of a 16-bit word from a command register
// usage: if (i2c_read_word(fd, 0x07, &raw) != 0) fprintf(stderr, "read fail");
// preconditions: fd from i2c_open, value non-NULL
int i2c_read_word(int fd, uint8_t command, uint16_t *value) {
    union i2c_smbus_data data;
    struct i2c_smbus_ioctl_data args = {
        .read_write = I2C_SMBUS_READ,
        .command = command,
        .size = I2C_SMBUS_WORD_DATA,
        .data = &data,
    };
    if (ioctl(fd, I2C_SMBUS, &args) < 0) {
        return -1;
    }
    *value = data.word;
    return 0;
}

// i2c_close: close connection to device
// usage: i2c_close(fd);
// preconditions: fd from i2c_open
void i2c_close(int fd) {
    if (fd >= 0) {
        close(fd);
    }
}

