#include "rca.h"
#include "utils.h"
#include "ui.h"

void SCFatalError(const char *msg)
{
	SCSerialPrintLn(msg);
	SCSerialPrintLn("system halted.");
	while (1) {}
}
