#include "mlx90614.h"
#include "i2c.h"

#include <math.h>
#include <stdint.h>

// RAM addresses holding the readings, taken from Adafruit library
#define MLX90614_TAMBIENT 0x06
#define MLX90614_TOBJECT1 0x07

// read_temp: read a temperature register, check error flag, return temp (C)
static int read_temp(int fd, uint8_t reg, double *celsius) {
    uint16_t raw;
    if (i2c_read_word(fd, reg, &raw) != 0) {
        return -1;
    }
    if (raw & 0x8000)
        return -1;
    // mlx90614 gives temperatures in units of 0.02K, bits[14:0] only
    *celsius = (raw & 0x7FFF) * 0.02 - 273.15;
    return 0;
}

// mlx90614_init: probe the sensor with a test ambient read
// usage: if (mlx90614_init(fd) != 0) fprintf(stderr, "no mlx");
// preconditions: fd from i2c_open at MLX90614_ADDR
int mlx90614_init(int fd) {
    double t;
    return mlx90614_read_ambient(fd, &t);
}

// mlx90614_read_object: object (canopy) temperature in Celsius
// usage: if (mlx90614_read_object(fd, &t) != 0) fprintf(stderr, "read fail");
// preconditions: fd from i2c_open at MLX90614_ADDR, celsius non-NULL
int mlx90614_read_object(int fd, double *celsius) {
    return read_temp(fd, MLX90614_TOBJECT1, celsius);
}

// mlx90614_read_ambient: ambient temperature in Celsius
// usage: if (mlx90614_read_ambient(fd, &t) != 0) fprintf(stderr, "read fail");
// preconditions: fd from i2c_open at MLX90614_ADDR, celsius non-NULL
int mlx90614_read_ambient(int fd, double *celsius) {
    return read_temp(fd, MLX90614_TAMBIENT, celsius);
}

// mlx90614_correct_emissivity: correct for pure leaf surface temp from
// raw object reading. The chip doesn't take into account the reflected
// background, correction equation:
//   T_obj_K^4 = Ta_K^4 + (T_meas_K^4 - Ta_K^4) / epsilon
// note: t_amb must be the MLX ambient (not the SHT air temp)
// From Melexis "Changing emissivity".
// preconditions: epsilon in (0..1]
double mlx90614_correct_emissivity(double t_obj, double t_amb, double epsilon) {
    double obj_k = t_obj + 273.15;
    double amb_k = t_amb + 273.15;
    double obj_k4 = pow(amb_k, 4.0)
                    + (pow(obj_k, 4.0) - pow(amb_k, 4.0)) / epsilon;
    return pow(obj_k4, 0.25) - 273.15;
}

// mlx90614_read_object_corrected: emissivity-corrected canopy temperature (C).
// Reads the object and the sensor's own ambient, then applies the correction.
// preconditions: fd from i2c_open at MLX90614_ADDR, epsilon in (0..1]
int mlx90614_read_object_corrected(int fd, double epsilon, double *celsius) {
    double t_obj, t_amb;
    if (mlx90614_read_object(fd, &t_obj) != 0 ||
        mlx90614_read_ambient(fd, &t_amb) != 0) {
        return -1;
    }
    *celsius = mlx90614_correct_emissivity(t_obj, t_amb, epsilon);
    return 0;
}

