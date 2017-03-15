/**
 * @file crypto_config.h
 * @brief CycloneCrypto configuration file
 *
 * @section License
 *
 * Copyright (C) 2010-2017 Oryx Embedded SARL. All rights reserved.
 *
 * This file is part of CycloneCrypto Open.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * @author Oryx Embedded SARL (www.oryx-embedded.com)
 * @version 1.7.6
 **/

#ifndef _CRYPTO_CONFIG_H
#define _CRYPTO_CONFIG_H

#define USE_NO_RTOS

//Desired trace level (for debugging purposes)
#define CRYPTO_TRACE_LEVEL TRACE_LEVEL_DEBUG

//Assembly optimizations for time-critical routines
#define MPI_ASM_SUPPORT DISABLED

//Base64 encoding support
#define BASE64_SUPPORT ENABLED

//MD2 hash support
#define MD2_SUPPORT DISABLED
//MD4 hash support
#define MD4_SUPPORT DISABLED
//MD5 hash support
#define MD5_SUPPORT ENABLED
//RIPEMD-128 hash support
#define RIPEMD128_SUPPORT DISABLED
//RIPEMD-160 hash support
#define RIPEMD160_SUPPORT DISABLED
//SHA-1 hash support
#define SHA1_SUPPORT DISABLED
//SHA-224 hash support
#define SHA224_SUPPORT DISABLED
//SHA-256 hash support
#define SHA256_SUPPORT ENABLED
//SHA-384 hash support
#define SHA384_SUPPORT DISABLED
//SHA-512 hash support
#define SHA512_SUPPORT DISABLED
//SHA-512/224 hash support
#define SHA512_224_SUPPORT DISABLED
//SHA-512/256 hash support
#define SHA512_256_SUPPORT DISABLED
//SHA3-224 hash support
#define SHA3_224_SUPPORT DISABLED
//SHA3-256 hash support
#define SHA3_256_SUPPORT DISABLED
//SHA3-384 hash support
#define SHA3_384_SUPPORT DISABLED
//SHA3-512 hash support
#define SHA3_512_SUPPORT DISABLED
//Keccak support
#define KECCAK_SUPPORT DISABLED
//Tiger hash support
#define TIGER_SUPPORT DISABLED
//Whirlpool hash support
#define WHIRLPOOL_SUPPORT DISABLED

//HMAC support
#define HMAC_SUPPORT ENABLED

//RC4 support
#define RC4_SUPPORT DISABLED
//RC6 support
#define RC6_SUPPORT DISABLED
//IDEA support
#define IDEA_SUPPORT DISABLED
//DES support
#define DES_SUPPORT DISABLED
//Triple DES support
#define DES3_SUPPORT DISABLED
//AES support
#define AES_SUPPORT ENABLED
//Camellia support
#define CAMELLIA_SUPPORT DISABLED
//SEED support
#define SEED_SUPPORT ENABLED
//ARIA support
#define ARIA_SUPPORT DISABLED

//ECB mode support
#define ECB_SUPPORT DISABLED
//CBC mode support
#define CBC_SUPPORT DISABLED
//CFB mode support
#define CFB_SUPPORT DISABLED
//OFB mode support
#define OFB_SUPPORT DISABLED
//CTR mode support
#define CTR_SUPPORT DISABLED
//CCM mode support
#define CCM_SUPPORT DISABLED
//GCM mode support
#define GCM_SUPPORT DISABLED

//Chacha support
#define CHACHA_SUPPORT DISABLED
//Poly1305 support
#define POLY1305_SUPPORT DISABLED
//Chacha20Poly1305 support
#define CHACHA20_POLY1305_SUPPORT DISABLED

//Diffie-Hellman support
#define DH_SUPPORT DISABLED
//RSA support
#define RSA_SUPPORT DISABLED
//DSA support
#define DSA_SUPPORT DISABLED

//Elliptic curve cryptography support
#define EC_SUPPORT ENABLED
//ECDH support
#define ECDH_SUPPORT ENABLED
//ECDSA support
#define ECDSA_SUPPORT ENABLED

#endif
