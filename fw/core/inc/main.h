/**
 * @file     main.h
 * @author   Fabio Scatozza <s315216@studenti.polito.it>
 * @date     05.01.2025
 */

#ifndef MAIN_H
#define MAIN_H

#include "FileManager.hpp"
#include "HwAlarm.hpp"
#include "PushButton.hpp"

/*
 * Lazy construction of the file manager
 * (the function members are invoked as implementation of system calls)
 */

using FileManagerType = FileManager<1>;
FileManagerType &File_Manager();

/*
 * Lazy construction of resources requiring exception handling
 * (the function members are invoked as implementation of interrupt handlers)
 */
using HwAlarmType = HwAlarm<TIM9_BASE>;
HwAlarmType &Hw_Alarm();

using PushButtonType = PushButton<HwAlarmType>;
PushButtonType &Push_Button();

#endif //MAIN_H
