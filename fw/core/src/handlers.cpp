/**
 * @file     IFile.cpp
 * @author   Fabio Scatozza <s315216@studenti.polito.it>
 * @date     27.01.2025
 */

#include "main.h"

void TIM1_BRK_TIM9_IRQHandler() {
  Hw_Alarm().handler();
}

void TIM1_UP_TIM10_IRQHandler() {
  Stepper().handler();
}

void EXTI15_10_IRQHandler() {
  Push_Button().handler();
}
