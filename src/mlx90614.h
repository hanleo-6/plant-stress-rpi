// Interface to MLX90614(BCC) IR thermometer
#define MLX90614_ADDR 0x5A
#define MLX90614_FOV 35 // degrees

// NOTE: CRC check skipped here for mlx bc not that simple

int mlx90614_init(int fd);

int mlx90614_read_object(int fd, double *temp_c);

int mlx90614_read_ambient(int fd, double *temp_c);

double mlx90614_correct_emissivity(double t_obj, double t_amb, double epsilon);

int mlx90614_read_object_corrected(int fd, double epsilon, double *temp_c);

