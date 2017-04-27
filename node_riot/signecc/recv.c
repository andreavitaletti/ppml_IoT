/*
 * Copyright (C) Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @{
 *
 * @file
 * @author Martine Lenders <mlenders@inf.fu-berlin.de>
 */

#include <stdio.h>

#include "at86rf2xx.h"
#include "od.h"
#include "net/ieee802154.h"
#include "net/netdev.h"
#include "hashes/sha256.h"
#include "uECC.h"
#include "string.h"
#include "common.h"

#define MAX_LINE    (128)

static uint8_t buffer[AT86RF2XX_MAX_PKT_LENGTH];
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

static int send2(int iface, le_uint16_t dst_pan, uint8_t *dst, size_t dst_len, char *data, uint8_t dataLength)
{
    int res;
    netdev_ieee802154_t *dev;
    const size_t count = 2;         /* mhr + payload */
    struct iovec vector[count];
    uint8_t *src;
    size_t src_len;
    uint8_t mhr[IEEE802154_MAX_HDR_LEN];
    uint8_t flags;
    le_uint16_t src_pan;

    if (((unsigned)iface) > (AT86RF2XX_NUM - 1)) {
        printf("txtsnd: %d is not an interface\n", iface);
        return 1;
    }

    dev = (netdev_ieee802154_t *)&devs[iface];
    flags = (uint8_t)(dev->flags & NETDEV_IEEE802154_SEND_MASK);
    flags |= IEEE802154_FCF_TYPE_DATA;
    vector[1].iov_base = data;
    vector[1].iov_len = dataLength;
    src_pan = byteorder_btols(byteorder_htons(dev->pan));
    if (dst_pan.u16 == 0) {
        dst_pan = src_pan;
    }
    if (dev->flags & NETDEV_IEEE802154_SRC_MODE_LONG) {
        src_len = 8;
        src = dev->long_addr;
    }
    else {
        src_len = 2;
        src = dev->short_addr;
    }
    /* fill MAC header, seq should be set by device */
    if ((res = ieee802154_set_frame_hdr(mhr, src, src_len,
                                        dst, dst_len,
                                        src_pan, dst_pan,
                                        flags, dev->seq++)) < 0) {
        puts("txtsnd: Error preperaring frame");
        return 1;
    }
    vector[0].iov_base = mhr;
    vector[0].iov_len = (size_t)res;
    res = dev->netdev.driver->send((netdev_t *)dev, vector, count);
    if (res < 0) {
        puts("txtsnd: Error on sending");
        return 1;
    }
    else {
        printf("txtsnd: send %u bytes to ", (unsigned)vector[1].iov_len);
        print_addr(dst, dst_len);
        printf(" (PAN: ");
        print_addr((uint8_t *)&dst_pan, sizeof(dst_pan));
        puts(")");
    }
    return 0;
}

void recv(netdev_t *dev)
{
    /* radio vars */
    uint8_t src[IEEE802154_LONG_ADDRESS_LEN], dst[IEEE802154_LONG_ADDRESS_LEN];
    size_t mhr_len, data_len, src_len, dst_len;
    netdev_ieee802154_rx_info_t rx_info;
    le_uint16_t src_pan, dst_pan;
    uint16_t i;

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

    uint8_t msgBuffer[512];
    uint8_t msgSize;
    char signReplyData[1 + sizeof(l_sig) + sizeof(l_hash) + 1];

    uint8_t tmp[2 * SHA256_DIGEST_LENGTH + SHA256_INTERNAL_BLOCK_SIZE];



    putchar('\n');
    data_len = dev->driver->recv(dev, buffer, sizeof(buffer), &rx_info);
    mhr_len = ieee802154_get_frame_hdr_len(buffer);
    if (mhr_len == 0) {
        puts("Unexpected MHR for incoming packet");
        return;
    }
    dst_len = ieee802154_get_dst(buffer, dst, &dst_pan);
    src_len = ieee802154_get_src(buffer, src, &src_pan);
    switch (buffer[0] & IEEE802154_FCF_TYPE_MASK) {
        case IEEE802154_FCF_TYPE_BEACON:
            puts("BEACON");
            break;
        case IEEE802154_FCF_TYPE_DATA:
            puts("DATA");
            break;
        case IEEE802154_FCF_TYPE_ACK:
            puts("ACK");
            break;
        case IEEE802154_FCF_TYPE_MACCMD:
            puts("MACCMD");
            break;
        default:
            puts("UNKNOWN");
            break;
    }
    printf("Dest. PAN: 0x%04x, Dest. addr.: ",
           byteorder_ntohs(byteorder_ltobs(dst_pan)));
    print_addr(dst, dst_len);
    printf("\nSrc. PAN: 0x%04x, Src. addr.: ",
           byteorder_ntohs(byteorder_ltobs(src_pan)));
    print_addr(src, src_len);
    printf("\nSecurity: ");
    if (buffer[0] & IEEE802154_FCF_SECURITY_EN) {
        printf("1, ");
    }
    else {
        printf("0, ");
    }
    printf("Frame pend.: ");
    if (buffer[0] & IEEE802154_FCF_FRAME_PEND) {
        printf("1, ");
    }
    else {
        printf("0, ");
    }
    printf("ACK req.: ");
    if (buffer[0] & IEEE802154_FCF_ACK_REQ) {
        printf("1, ");
    }
    else {
        printf("0, ");
    }
    printf("PAN comp.: ");
    if (buffer[0] & IEEE802154_FCF_PAN_COMP) {
        puts("1");
    }
    else {
        puts("0");
    }
    printf("Version: ");
    printf("%u, ", (unsigned)((buffer[1] & IEEE802154_FCF_VERS_MASK) >> 4));
    printf("Seq.: %u\n", (unsigned)ieee802154_get_seq(buffer));
    od_hex_dump(buffer + mhr_len, data_len - mhr_len, 0);

    uECC_SHA256_HashContext ctx;
    ctx.uECC.init_hash = &_init_sha256;
    ctx.uECC.update_hash = &_update_sha256;
    ctx.uECC.finish_hash = &_finish_sha256;
    ctx.uECC.block_size = 64;
    ctx.uECC.result_size = 32;
    ctx.uECC.tmp = tmp;

    switch(buffer[mhr_len])
    {
        /* request to sign */
        case 0xFF:
            msgSize = data_len - mhr_len - 1;
            memcpy(msgBuffer, &buffer[mhr_len + 1], msgSize);

            printf("MSG 0xFF: ");
            for(i=0;i<msgSize;i++)
                printf("%02X ", msgBuffer[i]);
            printf("\n");

            /* copy some bogus data into the hash */
            //memcpy(l_hash, l_public1, 32);

            /* calculate hash */
            printf("HASH calc\n");
            _init_sha256(&ctx.uECC);
            _update_sha256(&ctx.uECC, msgBuffer, msgSize);
            _finish_sha256(&ctx.uECC, l_hash);
            printf("DONE\n");

            memcpy(l_hash, l_public1, 32);

            /* sign it */
            printf("Sign deterministic\n");
            if (uECC_sign_deterministic(l_private1, l_hash, sizeof(l_hash), &ctx.uECC, l_sig, curve) != 1)
                printf("\nSignature generated\n");
            printf("DONE\n");

            printf("Make packet\n");
            signReplyData[0] = 0xFE;
            memcpy(&signReplyData[1], l_sig, sizeof(l_sig));
            printf("Generated signature: ");
            for(i=0;i<sizeof(l_sig);i++)
                printf("%02X ", l_sig[i]);
            printf("\n");
            memcpy(&signReplyData[sizeof(l_sig) + 1], l_hash, sizeof(l_hash));
            printf("Calculated hash: ");
            for(i=0;i<sizeof(l_hash);i++)
                printf("%02X ", l_hash[i]);
            printf("\n");
            signReplyData[sizeof(l_sig) + sizeof(l_hash) + 1] = 0;
            printf("DONE\n");

            /* send the sign and hash*/
            printf("Sending sign and hash\n");
            send2(0, src_pan, src, src_len, signReplyData, 2 + sizeof(l_sig) + sizeof(l_hash));
            printf("DONE\n");
            break;

        /* sign received */
        case 0xFE:
            printf("saving signature and hash to array\n");
            /* save signature in array */
            memcpy(l_sig, &buffer[mhr_len + 1], sizeof(l_sig));
            /* save hash in array */
            memcpy(l_hash, &buffer[mhr_len + sizeof(l_sig) + 1], sizeof(l_hash));

            printf("Received signature: ");
            for(i=0;i<sizeof(l_sig);i++)
                printf("%02X ", l_sig[i]);
            printf("\n");
            printf("Received hash: ");
            for(i=0;i<sizeof(l_hash);i++)
                printf("%02X ", l_hash[i]);
            printf("\n");
            printf("DONE\n");

            printf("MSG 0xFE: ");
            for(i=0;i<sizeof(l_sig)+sizeof(l_hash)+2;i++)
                printf("%02X ", buffer[i]);
            printf("\n");

            printf("Verify\n");

            /* verify */
            if (uECC_verify(l_public1, l_hash, sizeof(l_hash), l_sig, curve) != 1) {
                printf("\nSignature verification FAILED\n");
            }
            else
            {
                printf("\nSignature verification OK\n");
            }
            printf("DONE\n");
            break;

        default:
            printf("\nERROR, msg not recognized\n");
            break;
    }

    printf("Text: ");
    for (int i = mhr_len; i < data_len; i++) {
        if ((buffer[i] > 0x1F) && (buffer[i] < 0x80)) {
            putchar((char)buffer[i]);
        }
        else {
            putchar('?');
        }
        if (((((i - mhr_len) + 1) % (MAX_LINE - sizeof("txt: "))) == 1) &&
            (i - mhr_len) != 0) {
            printf("\n     ");
        }
    }
    printf("\n");
    printf("RSSI: %u, LQI: %u\n\n", rx_info.rssi, rx_info.lqi);
}