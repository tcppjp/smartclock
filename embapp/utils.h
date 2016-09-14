#ifndef _UTILS_H
#define _UTILS_H

#include "rca.h"
#include <utility>

#ifdef __cplusplus
template <class T>
inline T SCMin(const T &a, const T &b)
{
	return a < b ? a : b;
}

template <class T>
inline T SCMax(const T &a, const T &b)
{
	return a < b ? b : a;
}

void SCFatalError(const char *msg);

template <class T>
void SCSerialLock(T hdlr) {
	wai_sem(SERIAL_SEM);
	hdlr();
	sig_sem(SERIAL_SEM);
}

template <class T>
void SCSerialPrint(const T &arg) {
	SCSerialLock([&] {
		Serial.print(arg);
	});
}

// can't use variadic template; bug in compiler?

template <class T, class T2>
void SCSerialPrintLn(const T &arg, const T2 &arg2) {
	SCSerialLock([&] {
		Serial.println(arg, arg2);
	});
}

template <class T>
void SCSerialPrintLn(const T &arg) {
	SCSerialLock([&] {
		Serial.println(arg);
	});
}

inline void SCSerialPrintLn() {
	SCSerialLock([&] {
		Serial.println();
	});
}

#endif

#endif
