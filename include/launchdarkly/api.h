/*!
 * @file api.h
 * @brief Public API. Include this for every public operation.
 */

#pragma once

/** @brief The current SDK version string. This value adheres to semantic
 * versioning and is included in the HTTP user agent sent to LaunchDarkly.
 */
#define LD_SDK_VERSION "2.3.0"

#ifdef __cplusplus
extern "C" {
#endif

#include <launchdarkly/json.h>
#include <launchdarkly/config.h>
#include <launchdarkly/user.h>
#include <launchdarkly/client.h>
#include <launchdarkly/memory.h>
#include <launchdarkly/logging.h>
#include <launchdarkly/variations.h>
#include <launchdarkly/store.h>
#include <launchdarkly/boolean.h>

#ifdef __cplusplus
}
#endif
