
#ifndef J1708_TASK_H_
#define J1708_TASK_H_

#include <event.h>

#define EVENT_J1708_RX	1

#ifdef __cplusplus
extern "C"
{
#endif

extern void * g_J1708_event_h;
extern _pool_id   g_out_message_pool;

extern bool validation_j1708_read_msg(uint8_t* data,uint32_t timeout);
extern void J1708_enable  (uint8_t priority);
extern void J1708_disable (void);
APPLICATION_QUEUE_T get_queue_target();
extern void set_queue_target(APPLICATION_QUEUE_T queue_target);
#ifdef __cplusplus
}
#endif

#endif /* J1708_TASK_H_ */

