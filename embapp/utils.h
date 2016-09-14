#ifndef _UTILS_H
#define _UTILS_H

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

#endif
