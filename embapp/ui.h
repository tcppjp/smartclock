#ifndef _LCD_H
#define _LCD_H

#ifdef __cplusplus
extern "C" {
#endif

void ui_setup(void);
void ui_task(intptr_t exinf);
void ui_checkTouch(intptr_t exinf);

#ifdef __cplusplus
};
#endif

#endif
