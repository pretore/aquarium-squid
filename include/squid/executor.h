#ifndef _SQUID_EXECUTOR_H_
#define _SQUID_EXECUTOR_H_

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#define SQUID_EXECUTOR_ERROR_OUT_IS_NULL                    1
#define SQUID_EXECUTOR_ERROR_OBJECT_IS_NULL                 2
#define SQUID_EXECUTOR_ERROR_MEMORY_ALLOCATION_FAILED       3
#define SQUID_EXECUTOR_ERROR_IS_BUSY_SHUTTING_DOWN          4
#define SQUID_EXECUTOR_ERROR_FUNCTION_IS_NULL               5

struct triggerfish_strong;
struct squid_executor;
struct squid_future;

/**
 * @brief Create executor instance.
 * @param [out] out receive newly created executor.
 * @return On success true, otherwise false if an error has occurred.
 * @throws SQUID_EXECUTOR_ERROR_OUT_IS_NULL if out is <i>NULL</i>.
 * @throws SQUID_EXECUTOR_ERROR_MEMORY_ALLOCATION_FAILED if there is
 * insufficient memory to create instance.
 * @note <b>out</b> must be released once done with it.
 */
bool squid_executor_of(struct triggerfish_strong **out);

/**
 * @brief Shutdown executor.
 * @param [in] object instance to be shutdown.
 * @return On success true, otherwise false if an error has occurred.
 * @throws SQUID_EXECUTOR_ERROR_OBJECT_IS_NULL if object is <i>NULL</i>.
 * @throws SQUID_EXECUTOR_ERROR_IS_BUSY_SHUTTING_DOWN if executor is already
 * in the process of shutting down.
 */
bool squid_executor_shutdown(struct squid_executor *object);

/**
 * @brief Retrieve count of total threads.
 * @param [in] object executor instance.
 * @param [out] out receive count of total threads.
 * @return On success true, otherwise false if an error has occurred.
 * @throws SQUID_EXECUTOR_ERROR_OBJECT_IS_NULL if object is <i>NULL</i>.
 * @throws SQUID_EXECUTOR_ERROR_OUT_IS_NULL if out is <i>NULL</i>.
 */
bool squid_executor_count(const struct squid_executor *object,
                          uintmax_t *out);

/**
 * @brief Retrieve count of ready threads.
 * <p>Ready threads are threads waiting to receive tasks to do.</p>
 * @param [in] object executor instance.
 * @param [out] out receive count of ready threads.
 * @return On success true, otherwise false if an error has occurred.
 * @throws SQUID_EXECUTOR_ERROR_OBJECT_IS_NULL if object is <i>NULL</i>.
 * @throws SQUID_EXECUTOR_ERROR_OUT_IS_NULL if out is <i>NULL</i>.
 */
bool squid_executor_ready(const struct squid_executor *object,
                          uintmax_t *out);

/**
 * @brief Check if executor will accept tasks.
 * @param [in] object executor instance.
 * @param [out] out receive if executor will accept tasks.
 * @return On success true, otherwise false if an error has occurred.
 * @throws SQUID_EXECUTOR_ERROR_OBJECT_IS_NULL if object is <i>NULL</i>.
 * @throws SQUID_EXECUTOR_ERROR_OUT_IS_NULL if out is <i>NULL</i>.
 */
bool squid_executor_is_running(const struct squid_executor *object,
                               bool *out);

typedef void (*squid_function)(void *args,
                               bool (*is_cancelled)(void),
                               struct triggerfish_strong **out,
                               uintmax_t *error);

/**
 * @brief Submit task for execution.
 * @param [in] object executor instance.
 * @param [in] function of the task to run.
 * @param [in] args to pass on to the executing function.
 * @param [out] out receive future strong reference.
 * @return On success true, otherwise false if an error has occurred.
 * @throws SQUID_EXECUTOR_ERROR_OBJECT_IS_NULL if object is <i>NULL</i>.
 * @throws SQUID_EXECUTOR_ERROR_FUNCTION_IS_NULL if function is <i>NULL</i>.
 * @throws SQUID_EXECUTOR_ERROR_OUT_IS_NULL if out is <i>NULL</i>.
 * @throws SQUID_EXECUTOR_ERROR_IS_BUSY_SHUTTING_DOWN if executor is busy
 * shutting down and therefore not accepting anymore requests.
 * @note <b>out</b> must be released once done with it.
 */
bool squid_executor_submit(struct squid_executor *object,
                           squid_function function,
                           void *args,
                           struct triggerfish_strong **out);

#endif /* _SQUID_EXECUTOR_H_ */
