#ifndef USER_CONFIG_H
#define USER_CONFIG_H

#if defined(SUNPLUS) || defined(unSP) || defined(__CHAR_16__)
/* GPCE500A native int/long/long long are 16/32/64 bits; char occupies 16 bits. */
typedef unsigned int u16;
typedef unsigned long u32;
typedef unsigned long long u64;
typedef signed int s16;
typedef signed long s32;
typedef signed long long s64;

/* Keep the existing GPIO pin-index alias; it is not an 8-bit storage type. */
typedef unsigned char uint8_t;
typedef unsigned char u8;
typedef unsigned int uint16_t;
typedef signed int int16_t;
typedef unsigned long uint32_t;
typedef signed long int32_t;
#ifndef __cplusplus
typedef enum
{
    false = 0,
    true = 1
} bool;
#endif
#else
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;
#endif

#define null NULL

// clang-format off

/** Audio output mode configuration */
#define AUDIO_OUTMODE_DAC       0
#define AUDIO_OUTMODE_PWM       1

#define AUDIO_VOLUME_MUTE       0
#define AUDIO_VOLUME_DEFAULT    9        
#define AUDIO_VOLUME_MAX        15

/** UART/SPI2 pin selection; choose SERIAL_PORT_A or SERIAL_PORT_B. */
#define SERIAL_PORT_A           0
#define SERIAL_PORT_B           1

#define UART_PORT               SERIAL_PORT_A
#define SPI_PORT                SERIAL_PORT_B

/** Blocking waits are bounded by polling iterations, not milliseconds. */
#define SERIAL_WAIT_LIMIT       0x0001FFFFUL
#define UART_BAUD_ERROR_PERCENT 3UL

#if UART_PORT != SERIAL_PORT_A && UART_PORT != SERIAL_PORT_B
#error Unsupported UART_PORT
#endif
#if SPI_PORT != SERIAL_PORT_A && SPI_PORT != SERIAL_PORT_B
#error Unsupported SPI_PORT
#endif






// clang-format on

#endif
