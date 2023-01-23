#ifndef _SQUID_PRIVATE_FUTURE_H_
#define _SQUID_PRIVATE_FUTURE_H_

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdatomic.h>
#include <pthread.h>
#include <triggerfish.h>
#include <squid.h>

#define SQUID_FUTURE_ERROR_EXECUTOR_IS_NULL                 (-1)
#define SQUID_FUTURE_ERROR_FUNCTION_IS_NULL                 (-2)
#define SQUID_FUTURE_ERROR_MEMORY_ALLOCATION_FAILED         (-3)
#define SQUID_FUTURE_ERROR_EXECUTOR_IS_INVALID              (-4)


struct squid_future {
    pthread_mutex_t mutex;
    pthread_cond_t condition;
    struct triggerfish_strong *self;
    struct triggerfish_strong *executor;
    struct triggerfish_strong *out;
    atomic_int status; /* enum squid_future_status */
    void *args;
    uintmax_t error;
    squid_function function;
};

/**
 * @brief Initialize future.
 * @param [in] object instance to be initialized.
 * @return On success true, otherwise false if an error has occurred.
 * @throws SQUID_FUTURE_ERROR_OBJECT_IS_NULL if object is <i>NULL</i>.
 * @throws SQUID_FUTURE_ERROR_EXECUTOR_IS_NULL if executor is <i>NULL</i>.
 * @throws SQUID_FUTURE_ERROR_FUNCTION_IS_NULL if function is <i>NULL</i>.
 * @throws SQUID_FUTURE_ERROR_EXECUTOR_IS_INVALID if strong reference of
 * executor has been invalidated.
 */
bool squid_future_init(struct squid_future *object,
                       struct triggerfish_strong *executor,
                       squid_function function,
                       void *args);

/**
 * @brief Invalidate future.
 * <p>The actual <u>future instance is not deallocated</u> since it may
 * have been embedded in a larger structure.</p>
 * @param [in] object instance to be invalidated.
 * @return On success true, otherwise false if an error has occurred.
 * @throws SQUID_FUTURE_ERROR_OBJECT_IS_NULL if object is <i>NULL</i>.
 */
bool squid_future_invalidate(struct squid_future *object);

/**
 * @brief Create future instance.
 * @param [out] out receive newly created future.
 * @return On success true, otherwise false if an error has occurred.
 * @throws SQUID_FUTURE_ERROR_EXECUTOR_IS_NULL if executor is <i>NULL</i>.
 * @throws SQUID_FUTURE_ERROR_FUNCTION_IS_NULL if function is <i>NULL</i>.
 * @throws SQUID_FUTURE_ERROR_OUT_IS_NULL if out is <i>NULL</i>.
 * @throws SQUID_FUTURE_ERROR_EXECUTOR_IS_INVALID if strong reference of
 * executor has been invalidated.
 * @throws SQUID_FUTURE_ERROR_MEMORY_ALLOCATION_FAILED if there is
 * insufficient memory to create instance.
 * @note <b>out</b> must be released once done with it.
 */
bool squid_future_of(struct triggerfish_strong *executor,
                     squid_function function,
                     void *args,
                     struct triggerfish_strong **out);

#endif /* _SQUID_PRIVATE_FUTURE_H_ */
