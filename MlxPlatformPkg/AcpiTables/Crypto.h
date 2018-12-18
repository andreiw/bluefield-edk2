#ifndef __CRYPTO_H__
#define __CRYPTO_H__

#include <bf1_irqs.h>

//
// Crypto general information
//

#define NUM_CRYPTO_PKA_DEVICE       4 // Number of Crypto PKA devices
#define NUM_CRYPTO_PKA_RING_DEVICE  4 // Number of Crypto PKA Ring devices

// Crypto PKA device identifier.
#define CRYPTO_0_PKA_0_ID       0
#define CRYPTO_0_PKA_1_ID       1
#define CRYPTO_1_PKA_0_ID       2
#define CRYPTO_1_PKA_1_ID       3

// Crypto PKA Ring device identifier.
#define CRYPTO_PKA_RING_0_ID    0
#define CRYPTO_PKA_RING_1_ID    1
#define CRYPTO_PKA_RING_2_ID    2
#define CRYPTO_PKA_RING_3_ID    3

// Crypto PKA block base addresses.
#define CRYPTO_0_PKA_0_BASE         0x45000000
#define CRYPTO_0_PKA_1_BASE         0x47000000
#define CRYPTO_1_PKA_0_BASE         0x4d000000
#define CRYPTO_1_PKA_1_BASE         0x4f000000

// Crypto PKA block size in bytes.
#define CRYPTO_PKA_SIZE             0x1000000

// Crypto PKA master block interrupt numbers. MUST be consistent with
// the interrupt mapping.
#define CRYPTO_0_PKA_MASTER_0_IRQ   BF1_CRYPTO0_PKA_MASTER_0_INT
#define CRYPTO_0_PKA_MASTER_1_IRQ   BF1_CRYPTO0_PKA_MASTER_1_INT
#define CRYPTO_1_PKA_MASTER_0_IRQ   BF1_CRYPTO1_PKA_MASTER_0_INT
#define CRYPTO_1_PKA_MASTER_1_IRQ   BF1_CRYPTO1_PKA_MASTER_1_INT

//
// Crypto ACPI information
//

// Crypto PKA device ACPI Hardware ID (_HID)
#define CRYPTO_PKA_DEVICE_ACPIHID          "MLNXBF10"
#define CRYPTO_PKA_RING_DEVICE_ACPIHID     "MLNXBF11"

// Crypto PKA Ring device Address (_ADR)
//
// Macro that computes the Ring address with regards to the PKA Identifier
// and the Ring identifier.
#define CRYPTO_PKA_RING_DEVICE_ADR(pka, ring) \
    ((pka * NUM_CRYPTO_PKA_DEVICE) + ring)

//
// Crypto device object name
//

// Concatenate preprocessor tokens without expanding macro definitions.
#define PPCAT_NX(X, Y)  X ## Y
// Concatenate preprocessor tokens after macro-expanding them.
#define PPCAT(X, Y)     PPCAT_NX(X, Y)

// Turn X into a string literal without expanding macro definitions.
#define STRINGIZE_NX(X) #X
// Turn X into a string literal after macro-expanding it.
#define STRINGIZE(X) STRINGIZE_NX(X)

#define CRYPTO_DEVICE_OBJECT_NAME_NX(pka, ring) \
    STRINGIZE(\\_SB_.pka.ring)

// Return the ASCII Null terminated string with the full path to the entry
// in the namespace for the PKA rings.
#define CRYPTO_DEVICE_OBJECT_NAME(pka, ring) \
    CRYPTO_DEVICE_OBJECT_NAME_NX( \
        PPCAT(CRYPTO_PKA_DEVICE_PREFIX, pka),      \
        PPCAT(CRYPTO_PKA_RING_DEVICE_PREFIX, ring) \
    )


#define CRYPTO_PKA_DEVICE_PREFIX        PKA
// Returns the Crypto PKA device name
#define CRYPTO_PKA_DEVICE_NAME(id) \
            PPCAT(CRYPTO_PKA_DEVICE_PREFIX, id)

#define CRYPTO_PKA_RING_DEVICE_PREFIX   RNG
// Returns the Crypto PKA Ring device name
#define CRYPTO_PKA_RING_DEVICE_NAME(id) \
        PPCAT(CRYPTO_PKA_RING_DEVICE_PREFIX, id)


#define CRYPTO_PKA_DEVICE_PROPERTY(base, irq) \
    Name (_HID, CRYPTO_PKA_DEVICE_ACPIHID) \
    Name (_UID, Zero) \
    Name (_CCA,    1) \
    \
    Name(_CRS, ResourceTemplate() { \
        Memory32Fixed(ReadWrite, base, CRYPTO_PKA_SIZE) \
        Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { irq } \
    })

#endif // __CRYPTO_H__
