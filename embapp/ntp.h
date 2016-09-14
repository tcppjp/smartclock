#ifndef _NTP_H_
#define _NTP_H_

#ifdef __cplusplus
extern "C" {
#endif

extern void ntp_setup();

// DEFINITIONS BELOW ARE PRIVATE
extern void ntp_update_cyc(intptr_t exinf);
extern void ntp_task(intptr_t exinf);

#ifdef __cplusplus
}
#endif

#endif /* _NTP_H_ */
