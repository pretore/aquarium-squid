#ifndef _SQUID_SQUID_H_
#define _SQUID_SQUID_H_

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#include <squid/error.h>
#include <squid/executor.h>
#include <squid/future.h>

#define SQUID_ERROR_LIBRARY_UNINITIALIZED   \
    (1 + SQUID_EXECUTOR_ERROR_OUT_IS_NULL)

/**
 * @brief Initialize library before use.
 */
void squid_on_load(void);

/**
 * @brief Clean up library variables before unloading.
 */
void squid_on_unload(void);

/**
 * @brief Retrieve global executor instance.
 * @param [out] out receive global executor instance.
 * @throws SQUID_ERROR_LIBRARY_UNINITIALIZED if library is uninitialized.
 * @throws SQUID_EXECUTOR_ERROR_OUT_IS_NULL if out is <i>NULL</i>.
 */
bool squid_executor_instance(struct squid_executor **out);

#endif /* _SQUID_SQUID_H_ */
