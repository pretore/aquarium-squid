#ifndef _SQUID_PRIVATE_EXECUTOR_H_
#define _SQUID_PRIVATE_EXECUTOR_H_

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdatomic.h>
#include <pthread.h>
#include <triggerfish.h>
#include <lionfish.h>

#define SQUID_EXECUTOR_ERROR_IS_RUNNING                     (-1)

struct squid_executor {
    struct triggerfish_weak *self;
    struct lionfish_concurrent_linked_queue_sr tasks;
    struct {
        pthread_mutex_t mutex;
        pthread_cond_t condition;
        atomic_uintmax_t ready;
        atomic_uintmax_t count;
    } threads;
    atomic_bool is_running;
};

/**
 * @brief Initialize executor.
 * @param [in] object instance to be initialized.
 * @return On success true, otherwise false if an error has occurred.
 * @throws SQUID_EXECUTOR_ERROR_OBJECT_IS_NULL if object is <i>NULL</i>.
 * @throws SQUID_EXECUTOR_ERROR_MEMORY_ALLOCATION_FAILED if there is
 * insufficient memory to initialize instance.
 */
bool squid_executor_init(struct squid_executor *object);

/**
 * @brief Invalidate executor.
 * <p>The actual <u>executor instance is not deallocated</u> since it may
 * have been embedded in a larger structure.</p>
 * @param [in] object instance to be invalidated.
 * @return On success true, otherwise false if an error has occurred.
 * @throws SQUID_EXECUTOR_ERROR_OBJECT_IS_NULL if object is <i>NULL</i>.
 * @throws SQUID_EXECUTOR_ERROR_IS_RUNNING if executor has not been requested
 * to be shutdown.
 */
bool squid_executor_invalidate(struct squid_executor *object);

#endif /* _SQUID_PRIVATE_EXECUTOR_H_ */
