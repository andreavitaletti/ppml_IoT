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

#include "hashes/sha256.h"
#include "uECC.h"
#include "string.h"


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

/* use pre-generated keys for no-HWRNG platforms */
uint8_t l_private1[] = {
    0x9b, 0x4c, 0x4b, 0xa0, 0xb7, 0xb1, 0x25, 0x23,
    0x9c, 0x09, 0x85, 0x4f, 0x9a, 0x21, 0xb4, 0x14,
    0x70, 0xe0, 0xce, 0x21, 0x25, 0x00, 0xa5, 0x62,
    0x34, 0xa4, 0x25, 0xf0, 0x0f, 0x00, 0xeb, 0xe7,
};

uint8_t l_public1[] = {
    0x54, 0x3e, 0x98, 0xf8, 0x14, 0x55, 0x08, 0x13,
    0xb5, 0x1a, 0x1d, 0x02, 0x02, 0xd7, 0x0e, 0xab,
    0xa0, 0x98, 0x74, 0x61, 0x91, 0x12, 0x3d, 0x96,
    0x50, 0xfa, 0xd5, 0x94, 0xa2, 0x86, 0xa8, 0xb0,
    0xd0, 0x7b, 0xda, 0x36, 0xba, 0x8e, 0xd3, 0x9a,
    0xa0, 0x16, 0x11, 0x0e, 0x1b, 0x6e, 0x81, 0x13,
    0xd7, 0xf4, 0x23, 0xa1, 0xb2, 0x9b, 0xaf, 0xf6,
    0x6b, 0xc4, 0x2a, 0xdf, 0xbd, 0xe4, 0x61, 0x5c,
};

typedef struct uECC_SHA256_HashContext {
    uECC_HashContext uECC;
    sha256_context_t ctx;
} uECC_SHA256_HashContext;

static void _init_sha256(const uECC_HashContext *base)
{
    uECC_SHA256_HashContext *context = (uECC_SHA256_HashContext*)base;
    sha256_init(&context->ctx);
}

static void _update_sha256(const uECC_HashContext *base,
                          const uint8_t *message,
                          unsigned message_size)
{
    uECC_SHA256_HashContext *context = (uECC_SHA256_HashContext*)base;
    sha256_update(&context->ctx, message, message_size);
}

static void _finish_sha256(const uECC_HashContext *base, uint8_t *hash_result)
{
    uECC_SHA256_HashContext *context = (uECC_SHA256_HashContext*)base;
    sha256_final(&context->ctx, hash_result);
}



int main(void)
{
    uint16_t genCounter;

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

    uECC_SHA256_HashContext ctx;

    /* ecc vars */
    const struct uECC_Curve_t *curve = uECC_secp256r1();

    //int curve_size = uECC_curve_private_key_size(curve);
    int public_key_size = uECC_curve_public_key_size(curve);
    

    /*
    uint8_t l_secret1[curve_size];
    uint8_t l_secret2[curve_size];
    */
    
    /* reserve space for a SHA-256 hash */
    uint8_t l_hash[32];
    uint8_t l_sig[public_key_size];

    int msgBuffer[14];

    uint8_t tmp[2 * SHA256_DIGEST_LENGTH + SHA256_INTERNAL_BLOCK_SIZE];


    puts("IoT-Lab M3 node sensors test application - with ecc sign\n");

    printf("Initializing LSM303DLHC sensor at I2C_%i... ", TEST_LSM303DLHC_I2C);

    if (lsm303dlhc_init(&lsm303dlhc_dev, TEST_LSM303DLHC_I2C, TEST_LSM303DLHC_ACC_PIN, TEST_LSM303DLHC_MAG_PIN,
                        TEST_LSM303DLHC_ACC_ADDR, LSM303DLHC_ACC_S_RATE, LSM303DLHC_ACC_SCALE,
                        TEST_LSM303DLHC_MAG_ADDR, LSM303DLHC_MAG_S_RATE, LSM303DLHC_MAG_GAIN) == 0) {
        puts("[OK]");
    }
    else {
        puts("[Failed]");
        return 1;
    }

    printf("Initializing LPS331AP sensor at I2C_%i... ", TEST_LPS331AP_I2C);
    if (lps331ap_init(&lps331ap_dev, TEST_LPS331AP_I2C, TEST_LPS331AP_ADDR, LPS331AP_RATE) == 0) {
        puts("[OK]");
    }
    else {
        puts("[Failed]");
        return 1;
    }

    printf("Initializing L3G4200 sensor at I2C_%i... ", TEST_L3G4200D_I2C);
    if (l3g4200d_init(&l3g4200d_dev, TEST_L3G4200D_I2C, TEST_L3G4200D_ADDR,
                      TEST_L3G4200D_INT, TEST_L3G4200D_DRDY, L3G4200D_MODE, L3G4200D_SCALE) == 0) {
        puts("[OK]");
    }
    else {
        puts("[Failed]");
        return 1;
    }

    printf("Initializing ISL29020 sensor at I2C_%i... ", TEST_ISL29020_I2C);
    if (isl29020_init(&isl29020_dev, TEST_ISL29020_I2C, TEST_ISL29020_ADDR, ISL29020_RANGE, ISL29020_MODE) == 0) {
        puts("[OK]");
    }
    else {
        puts("[Failed]");
        return 1;
    }

    printf("Initializing uECC hashcontext... ");
    ctx.uECC.init_hash = &_init_sha256;
    ctx.uECC.update_hash = &_update_sha256;
    ctx.uECC.finish_hash = &_finish_sha256;
    ctx.uECC.block_size = 64;
    ctx.uECC.result_size = 32;
    ctx.uECC.tmp = tmp;
    puts("[OK]");
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

        msgBuffer[0] = acc_value.x_axis;
        msgBuffer[1] = acc_value.y_axis;
        msgBuffer[2] = acc_value.z_axis;
        msgBuffer[3] = temp_value;
        msgBuffer[4] = mag_value.y_axis;
        msgBuffer[5] = mag_value.z_axis;
        msgBuffer[6] = pres_abs;
        msgBuffer[7] = pres;
        msgBuffer[8] = temp_abs;
        msgBuffer[9] = temp;
        msgBuffer[10] = acc_data.acc_x;
        msgBuffer[11] = acc_data.acc_y;
        msgBuffer[12] = acc_data.acc_z;
        msgBuffer[13] = value;

        /* calculate hash */
        printf("Calculating hash... ");
        _init_sha256(&ctx.uECC);
        _update_sha256(&ctx.uECC, (uint8_t *)&msgBuffer, sizeof(msgBuffer));
        _finish_sha256(&ctx.uECC, l_hash);
        printf("[OK]\n");

        /* sign it */
        printf("Signing deterministic... ");
        if (uECC_sign_deterministic(l_private1, l_hash, sizeof(l_hash), &ctx.uECC, l_sig, curve) != 1)
            printf("\nSignature generated\n");
        printf("[OK]\n");


        printf("\n\n");

        printf("Data: ");
        for(genCounter=0;genCounter<(sizeof(msgBuffer)/4);genCounter++)
            printf("%08X ", msgBuffer[genCounter]);
        printf("\n");

        printf("Hash: ");
        for(genCounter=0;genCounter<sizeof(l_hash);genCounter++)
            printf("%02X ", l_hash[genCounter]);
        printf("\n");

        printf("Signature: ");
        for(genCounter=0;genCounter<sizeof(l_sig);genCounter++)
            printf("%02X ", l_sig[genCounter]);
        printf("\n");

        printf("\n\n");


        xtimer_usleep(SLEEP);
    }

    return 0;
}
