/**
 * @file     PhaseCurrent.h
 * @author   Fabio Scatozza <s315216@studenti.polito.it>
 * @date     30.01.2025
 */

#ifndef PHASECURRENT_H
#define PHASECURRENT_H

#include <cstdint>

/**
 * @brief BSRR shift amounts for the phase pins masks
 *
 * BSRR[31:16] is set at position i+16 to reset pin i.
 * BSRR[15:0] is set at position i to set pin i.
 *
 * PhaseCurrent enumerators defines the shift amounts for the set masks of
 * the phase terminals, based on current direction. The shift for the positive
 * terminal is in the upper 8 bits; for the negative one, in the lower 8 bits.
 */
enum class PhaseCurrent : uint16_t {
  IN  = 0x00'10,  /*!< + set, - reset */
  OFF = 0x10'10, /*!< + reset, - reset */
  OUT = 0x10'00, /*!< + reset, - set */
};

#endif // PHASECURRENT_H
