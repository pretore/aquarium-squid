#ifndef _SQUID_FUTURE_H_
#define _SQUID_FUTURE_H_

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#define SQUID_FUTURE_ERROR_OBJECT_IS_NULL                   1
#define SQUID_FUTURE_ERROR_OUT_IS_NULL                      2
#define SQUID_FUTURE_ERROR_FUTURE_IS_CANCELLED              3
#define SQUID_FUTURE_ERROR_FUTURE_IS_DONE                   4

struct triggerfish_strong;
struct squid_future;

enum squid_future_status {
    SQUID_FUTURE_STATUS_PENDING = 0,
    SQUID_FUTURE_STATUS_RUNNING = 1,
    SQUID_FUTURE_STATUS_DONE = 2,
    SQUID_FUTURE_STATUS_CANCELLED = 3
};

/**
 * @brief Retrieve status.
 * @param [in] object future instance.
 * @param [out] out receive status of future.
 * @return On success true, otherwise false if an error has occurred.
 * @throws SQUID_FUTURE_ERROR_OBJECT_IS_NULL if object is <i>NULL</i>.
 * @throws SQUID_FUTURE_ERROR_OUT_IS_NULL if out is <i>NULL</i>.
 */
bool squid_future_status(const struct squid_future *object,
                         enum squid_future_status *out);

/**
 * @brief Cancel future's task.
 * @param [in] object future instance.
 * @param [out] out receive status of future before it was cancelled.
 * @return On success true, otherwise false if an error has occurred.
 * @throws SQUID_FUTURE_ERROR_OBJECT_IS_NULL if object is <i>NULL</i>.
 * @throws SQUID_FUTURE_ERROR_FUTURE_IS_DONE if future's task is already done.
 */
bool squid_future_cancel(struct squid_future *object,
                         enum squid_future_status *out);

/**
 * @brief Retrieve result.
 * @param [in] object future instance.
 * @param [out] out receive result.
 * @param [out] error optionally receive error code.
 * @return On success true, otherwise false if an error has occurred.
 * @throws SQUID_FUTURE_ERROR_OBJECT_IS_NULL if object is <i>NULL</i>.
 * @throws SQUID_FUTURE_ERROR_OUT_IS_NULL if out is <i>NULL</i>.
 * @throws SQUID_FUTURE_ERROR_FUTURE_IS_CANCELLED if future was cancelled.
 * @note <b>out</b> must be released once done with it.
 */
bool squid_future_get(struct squid_future *object,
                      struct triggerfish_strong **out,
                      uintmax_t *error);


#endif /* _SQUID_FUTURE_H_ */
