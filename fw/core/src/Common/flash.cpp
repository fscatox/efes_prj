/**
 * @file     flash.cpp
 * @author   Fabio Scatozza <s315216@studenti.polito.it>
 * @date     06.02.2025
 */

#include "flash.h"

namespace flash {

bool isBusy() { return FLASH->SR & FLASH_SR_BSY; }

bool isActive(const Flag &f, bool clear) {
  const bool tmp = FLASH->SR & f;
  if (clear)
    FLASH->SR = f;
  return tmp;
}
bool isEnabledIt(const It &it) {
  return FLASH->CR & it;
}
bool isLocked() { return FLASH->CR & FLASH_CR_LOCK; }

bool unlock() {
  constexpr uint32_t key1 = 0x45670123;
  constexpr uint32_t key2 = 0xCDEF89AB;

  /* succeed if it's already unlocked */
  if (!isLocked())
    return true;

  FLASH->KEYR = key1;
  FLASH->KEYR = key2;

  return !isLocked();
}
void lock() { FLASH->CR |= FLASH_CR_LOCK; }

void enableIt(const It &it) {
  FLASH->CR |= it;
}
void disableIt(const It &it) { FLASH->CR &= ~it; }

void setSector(const Sector &s) {
  FLASH->CR &= ~FLASH_CR_SNB;
  FLASH->CR |= static_cast<uint32_t>(s) << FLASH_CR_SNB_Pos;
}

void setOperation(const Op &op) {
  clearOperation();
  FLASH->CR |= static_cast<uint32_t>(op);
}
void startErase() {
  FLASH->CR |= FLASH_CR_STRT;
}
void clearOperation() {
  FLASH->CR &= ~(FLASH_CR_MER | FLASH_CR_SER | FLASH_CR_PG);
}

} // namespace flash