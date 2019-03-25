#ifndef TIMER_INCLUDED_EBBE459C_2FC4_4263_8FC8_7E2899F3E9A0
#define TIMER_INCLUDED_EBBE459C_2FC4_4263_8FC8_7E2899F3E9A0


#ifdef __cplusplus
extern "C" {
#endif


unsigned long
lib_time_get_current_ms();


void
lib_sleep_ms(unsigned long ms);


#ifdef __cplusplus
}
#endif


#endif /* inc guard */
