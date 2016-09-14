#include "rca.h"
#include "utils.h"
#include "ui.h"

void SCFatalError(const char *msg)
{
	ui_showFatalError(msg);
	while (1) {}
}
