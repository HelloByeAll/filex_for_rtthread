/*
 * @Author: Hello
 * @Date: 2021-01-21 13:33:55
 * @LastEditors: Hello
 * @LastEditTime: 2021-07-06 11:07:22
 * @FilePath: \Threadx_to_rt-thread\port\threadx_to_rtthread.h
 */
#include <rtthread.h>
#include <rthw.h>

#define VOID void
typedef char CHAR;
typedef unsigned char UCHAR;
typedef int INT;
typedef unsigned int UINT;
typedef long LONG;
typedef unsigned long ULONG;
typedef short SHORT;
typedef unsigned short USHORT;

#define _tx_thread_current_ptr rt_current_thread
#define TX_INTERRUPT_SAVE_AREA rt_base_t interrupt_save;

#define TX_DISABLE interrupt_save = rt_hw_interrupt_disable();
#define TX_RESTORE rt_hw_interrupt_enable(interrupt_save);

#define TX_EMPTY ((ULONG)0)
#define TX_TIMER struct rt_timer
#define TX_THREAD struct rt_thread
#define TX_AUTO_START ((UINT)1)
#define TX_DONT_START ((UINT)0)
#define TX_AUTO_ACTIVATE ((UINT)1)
#define TX_NO_ACTIVATE ((UINT)0)
#define TX_THREAD_GET_SYSTEM_STATE() (rt_interrupt_get_nest())

#define TX_NO_WAIT ((ULONG)0)
#define TX_WAIT_FOREVER ((ULONG)0xFFFFFFFFUL)

#define TX_SEMAPHORE struct rt_semaphore
#define TX_MUTEX struct rt_mutex
#define TX_EVENT_FLAGS_GROUP struct rt_event
#define TX_AND RT_EVENT_FLAG_AND
#define TX_AND_CLEAR (RT_EVENT_FLAG_AND | RT_EVENT_FLAG_CLEAR)
#define TX_OR RT_EVENT_FLAG_OR
#define TX_OR_CLEAR (RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR)

#define TX_NULL ((UINT)0)
#define TX_NO_INHERIT ((UINT)0)
#define TX_SUCCESS ((UINT)0x00)

#ifndef ALIGN_TYPE_DEFINED
#define ALIGN_TYPE ULONG
#endif

TX_THREAD *tx_thread_identify(void);
UINT _tx_thread_info_get(TX_THREAD *thread_ptr, CHAR **name, UINT *state, ULONG *run_count,
                         UINT *priority, UINT *preemption_threshold, ULONG *time_slice,
                         TX_THREAD **next_thread, TX_THREAD **next_suspended_thread);
UINT tx_thread_priority_change(TX_THREAD *thread_ptr, UINT new_priority, UINT *old_priority);
UINT tx_thread_sleep(ULONG ms);

UINT tx_event_flags_create(TX_EVENT_FLAGS_GROUP *group_ptr, CHAR *name_ptr);
UINT tx_event_flags_delete(TX_EVENT_FLAGS_GROUP *group_ptr);
UINT tx_event_flags_get(TX_EVENT_FLAGS_GROUP *group_ptr, ULONG requested_flags,
                        UINT get_option, ULONG *actual_flags_ptr, ULONG wait_option);
UINT tx_event_flags_set(TX_EVENT_FLAGS_GROUP *group_ptr, ULONG flags_to_set,
                        UINT set_option);

UINT tx_mutex_create(TX_MUTEX *mutex_ptr, CHAR *name_ptr, UINT inherit);
UINT tx_mutex_delete(TX_MUTEX *mutex_ptr);
UINT tx_mutex_get(TX_MUTEX *mutex_ptr, ULONG wait_option);
UINT tx_mutex_put(TX_MUTEX *mutex_ptr);

UINT tx_semaphore_create(TX_SEMAPHORE *semaphore_ptr, CHAR *name_ptr, ULONG initial_count);
UINT tx_semaphore_delete(TX_SEMAPHORE *semaphore_ptr);
UINT tx_semaphore_get(TX_SEMAPHORE *semaphore_ptr, ULONG wait_option);
UINT tx_semaphore_put(TX_SEMAPHORE *semaphore_ptr);

UINT tx_timer_create(TX_TIMER *timer_ptr, CHAR *name_ptr,
                     VOID (*expiration_function)(ULONG input), ULONG expiration_input,
                     ULONG initial_ticks, ULONG reschedule_ticks, UINT auto_activate);
UINT tx_timer_delete(TX_TIMER *timer_ptr);

UINT tx_thread_create(TX_THREAD *thread_ptr, CHAR *name_ptr,
                      VOID (*entry_function)(ULONG entry_input), ULONG entry_input,
                      VOID *stack_start, ULONG stack_size,
                      UINT priority, UINT preempt_threshold,
                      ULONG time_slice, UINT auto_start);
UINT tx_thread_delete(TX_THREAD *thread_ptr);
UINT tx_thread_resume(TX_THREAD *thread_ptr);
UINT tx_thread_suspend(TX_THREAD *thread_ptr);
UINT tx_thread_terminate(TX_THREAD *thread_ptr);
VOID tx_thread_relinquish(VOID);
