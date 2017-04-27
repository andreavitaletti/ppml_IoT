#ifndef TEST_LSM303DLHC_I2C
#error "TEST_LSM303DLHC_I2C not defined"
#endif
#ifndef TEST_LSM303DLHC_MAG_ADDR
#error "TEST_LSM303DLHC_MAG_ADDR not defined"
#endif
#ifndef TEST_LSM303DLHC_ACC_ADDR
#error "TEST_LSM303DLHC_ACC_ADDR not defined"
#endif
#ifndef TEST_LSM303DLHC_ACC_PIN
#error "TEST_LSM303DLHC_ACC_PIN not defined"
#endif
#ifndef TEST_LSM303DLHC_MAG_PIN
#error "TEST_LSM303DLHC_MAG_PIN not defined"
#endif

#ifndef TEST_LPS331AP_I2C
#error "TEST_LPS331AP_I2C not defined"
#endif
#ifndef TEST_LPS331AP_ADDR
#error "TEST_LPS331AP_ADDR not defined"
#endif

#ifndef TEST_L3G4200D_I2C
#error "TEST_L3G4200D_I2C not defined"
#endif
#ifndef TEST_L3G4200D_ADDR
#error "TEST_L3G4200D_ADDR not defined"
#endif
#ifndef TEST_L3G4200D_INT
#error "TEST_L3G4200D_INT not defined"
#endif
#ifndef TEST_L3G4200D_DRDY
#error "TEST_L3G4200D_DRDY not defined"
#endif

#ifndef TEST_ISL29020_I2C
#error "TEST_ISL29020_I2C not defined"
#endif
#ifndef TEST_ISL29020_ADDR
#error "TEST_ISL29020_ADDR not defined"
#endif


#include <stdio.h>

#include "xtimer.h"
#include "lsm303dlhc.h"
#include "lps331ap.h"
#include "l3g4200d.h"
#include "isl29020.h"


#define SLEEP       (1000U * 1000U)


#define LSM303DLHC_ACC_S_RATE  LSM303DLHC_ACC_SAMPLE_RATE_10HZ
#define LSM303DLHC_ACC_SCALE   LSM303DLHC_ACC_SCALE_2G
#define LSM303DLHC_MAG_S_RATE  LSM303DLHC_MAG_SAMPLE_RATE_75HZ
#define LSM303DLHC_MAG_GAIN    LSM303DLHC_MAG_GAIN_400_355_GAUSS

#define LPS331AP_RATE        LPS331AP_RATE_7HZ

#define L3G4200D_MODE        L3G4200D_MODE_100_25
#define L3G4200D_SCALE       L3G4200D_SCALE_500DPS

#define ISL29020_MODE        ISL29020_MODE_AMBIENT
#define ISL29020_RANGE       ISL29020_RANGE_16K


int main(void)
{
    lsm303dlhc_t lsm303dlhc_dev;
    lps331ap_t lps331ap_dev;
    l3g4200d_t l3g4200d_dev;
    isl29020_t isl29020_dev;

    int value;
    int temp, pres;
    int temp_abs, pres_abs;
    int16_t temp_value;

    lsm303dlhc_3d_data_t mag_value;
    lsm303dlhc_3d_data_t acc_value;
    l3g4200d_data_t acc_data;

    puts("IoT-Lab M3 node sensors test application\n");

    printf("Initializing LSM303DLHC sensor at I2C_%i... ", TEST_LSM303DLHC_I2C);

    if (lsm303dlhc_init(&lsm303dlhc_dev, TEST_LSM303DLHC_I2C, TEST_LSM303DLHC_ACC_PIN, TEST_LSM303DLHC_MAG_PIN,
                        TEST_LSM303DLHC_ACC_ADDR, LSM303DLHC_ACC_S_RATE, LSM303DLHC_ACC_SCALE,
                        TEST_LSM303DLHC_MAG_ADDR, LSM303DLHC_MAG_S_RATE, LSM303DLHC_MAG_GAIN) == 0) {
        puts("[OK]\n");
    }
    else {
        puts("[Failed]");
        return 1;
    }

    printf("Initializing LPS331AP sensor at I2C_%i... ", TEST_LPS331AP_I2C);
    if (lps331ap_init(&lps331ap_dev, TEST_LPS331AP_I2C, TEST_LPS331AP_ADDR, LPS331AP_RATE) == 0) {
        puts("[OK]\n");
    }
    else {
        puts("[Failed]");
        return 1;
    }

    printf("Initializing L3G4200 sensor at I2C_%i... ", TEST_L3G4200D_I2C);
    if (l3g4200d_init(&l3g4200d_dev, TEST_L3G4200D_I2C, TEST_L3G4200D_ADDR,
                      TEST_L3G4200D_INT, TEST_L3G4200D_DRDY, L3G4200D_MODE, L3G4200D_SCALE) == 0) {
        puts("[OK]\n");
    }
    else {
        puts("[Failed]");
        return 1;
    }

    printf("Initializing ISL29020 sensor at I2C_%i... ", TEST_ISL29020_I2C);
    if (isl29020_init(&isl29020_dev, TEST_ISL29020_I2C, TEST_ISL29020_ADDR, ISL29020_RANGE, ISL29020_MODE) == 0) {
        puts("[OK]\n");
    }
    else {
        puts("[Failed]");
        return 1;
    }

    puts("\n");
    xtimer_usleep(5 * SLEEP);


    while (1)
    {
        if (lsm303dlhc_read_acc(&lsm303dlhc_dev, &acc_value) == 0) {
            printf("Accelerometer x: %i y: %i z: %i\n", acc_value.x_axis,
                                                        acc_value.y_axis,
                                                        acc_value.z_axis);
        }
        else {
            puts("\nFailed reading accelerometer values\n");
        }
        if (lsm303dlhc_read_temp(&lsm303dlhc_dev, &temp_value) == 0) {
            printf("Temperature value: %i degrees\n", temp_value);
        }
        else {
            puts("\nFailed reading value\n");
        }

        if (lsm303dlhc_read_mag(&lsm303dlhc_dev, &mag_value) == 0) {
            printf("Magnetometer x: %i y: %i z: %i\n", mag_value.x_axis,
                                                       mag_value.y_axis,
                                                       mag_value.z_axis);
        }
        else {
            puts("\nFailed reading magnetometer values\n");
        }


        pres = lps331ap_read_pres(&lps331ap_dev);
        temp = lps331ap_read_temp(&lps331ap_dev);

        pres_abs = pres / 1000;
        pres -= pres_abs * 1000;
        temp_abs = temp / 1000;
        temp -= temp_abs * 1000;

        printf("Pressure value: %2i.%03i bar - Temperature: %2i.%03i °C\n",
               pres_abs, pres, temp_abs, temp);



        l3g4200d_read(&l3g4200d_dev, &acc_data);

        printf("Gyro data [dps] - X: %6i   Y: %6i   Z: %6i\n",
               acc_data.acc_x, acc_data.acc_y, acc_data.acc_z);


        value = isl29020_read(&isl29020_dev);
        printf("Light value: %5i LUX\n", value);


        xtimer_usleep(SLEEP);
    }

    return 0;
}
