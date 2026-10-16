#include <stdint.h>
#include <stdlib.h>

int i2c_open(const char *bus, uint8_t addr);

int i2c_write(int fd, const uint8_t *buf, size_t n);
int i2c_read(int fd, uint8_t *buf, size_t n);
int i2c_read_word(int fd, uint8_t command, uint16_t *value);

void i2c_close(int fd);

