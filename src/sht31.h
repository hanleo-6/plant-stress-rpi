// Interface to SHT31 temperature & humidity sensor
#define SHT31_ADDR 0x44

int sht31_read(int fd, double *temp_c, double *humidity);

