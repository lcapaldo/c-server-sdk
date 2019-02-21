#include "ldinternal.h"
#include "ldoperators.h"

#include <time.h>
#include <regex.h>

typedef bool (*OpFn)(const struct LDJSON *const uvalue,
    const struct LDJSON *const cvalue);

typedef bool (*StringOpFn)(const char *const uvalue, const char *const cvalue);

typedef bool (*NumberOpFn)(const float uvalue, const float cvalue);

#define CHECKSTRING(uvalue, cvalue) \
    if (LDJSONGetType(uvalue) != LDText || \
        LDJSONGetType(cvalue) != LDText) \
    { \
        return false; \
    }

#define CHECKNUMBER(uvalue, cvalue) \
    if (LDJSONGetType(uvalue) != LDNumber || \
        LDJSONGetType(cvalue) != LDNumber) \
    { \
        return false; \
    }

bool
operatorInFn(const struct LDJSON *const uvalue,
    const struct LDJSON *const cvalue)
{
    if (LDJSONCompare(uvalue, cvalue)) {
        return true;
    }

    return false;
}

static bool
operatorStartsWithFn(const struct LDJSON *const uvalue,
    const struct LDJSON *const cvalue)
{
    size_t ulen;
    size_t clen;

    CHECKSTRING(uvalue, cvalue);

    ulen = strlen(LDGetText(uvalue));
    clen = strlen(LDGetText(cvalue));

    if (clen > ulen) {
        return false;
    }

    return strncmp(LDGetText(uvalue), LDGetText(cvalue), clen) == 0;
}

static bool
operatorEndsWithFn(const struct LDJSON *const uvalue,
    const struct LDJSON *const cvalue)
{
    size_t ulen;
    size_t clen;

    CHECKSTRING(uvalue, cvalue);

    ulen = strlen(LDGetText(uvalue));
    clen = strlen(LDGetText(cvalue));

    if (clen > ulen) {
        return false;
    }

    return strcmp(LDGetText(uvalue) + ulen - clen, LDGetText(cvalue)) == 0;
}

static bool
operatorMatchesFn(const struct LDJSON *const uvalue,
    const struct LDJSON *const cvalue)
{
    bool matches;
    regex_t context;

    CHECKSTRING(uvalue, cvalue);

    if (regcomp(&context, LDGetText(cvalue), REG_EXTENDED) != 0) {
        LD_LOG(LD_LOG_ERROR, "failed to compile regex");

        return false;
    }

    matches = regexec(&context, LDGetText(uvalue), 0, NULL, 0) == 0;

    regfree(&context);

    return matches;
}

static bool
operatorContainsFn(const struct LDJSON *const uvalue,
    const struct LDJSON *const cvalue)
{
    CHECKSTRING(uvalue, cvalue);

    return strstr(LDGetText(uvalue), LDGetText(cvalue)) != NULL;
}

static bool
operatorLessThanFn(const struct LDJSON *const uvalue,
    const struct LDJSON *const cvalue)
{
    CHECKNUMBER(uvalue, cvalue);

    return LDGetNumber(uvalue) < LDGetNumber(cvalue);
}

static bool
operatorLessThanOrEqualFn(const struct LDJSON *const uvalue,
    const struct LDJSON *const cvalue)
{
    CHECKNUMBER(uvalue, cvalue);

    return LDGetNumber(uvalue) <= LDGetNumber(cvalue);
}

static bool
operatorGreaterThanFn(const struct LDJSON *const uvalue,
    const struct LDJSON *const cvalue)
{
    CHECKNUMBER(uvalue, cvalue);

    return LDGetNumber(uvalue) > LDGetNumber(cvalue);
}

static bool
operatorGreaterThanOrEqualFn(const struct LDJSON *const uvalue,
    const struct LDJSON *const cvalue)
{
    CHECKNUMBER(uvalue, cvalue);

    return LDGetNumber(uvalue) >= LDGetNumber(cvalue);
}


static bool
compareTime(const struct LDJSON *const uvalue,
    const struct LDJSON *const cvalue, bool (*op) (double, double))
{
    if (LDJSONGetType(uvalue) == LDNumber &&
        LDJSONGetType(cvalue) == LDNumber)
    {
        return op(LDGetNumber(uvalue), LDGetNumber(cvalue));
    } else if (LDJSONGetType(uvalue) == LDText &&
        LDJSONGetType(cvalue) == LDText)
    {
        time_t utime;
        time_t ctime;

        struct tm utm;
        struct tm ctm;

        if (strcmp(LDGetText(uvalue), "") == 0) {
            return false;
        }

        if (!strptime(LDGetText(uvalue), "%FT%T%Z", &utm)) {
            LD_LOG(LD_LOG_ERROR, "failed to parse date uvalue");

            return false;
        }

        if (!strptime(LDGetText(cvalue), "%FT%T%Z", &ctm)) {
            LD_LOG(LD_LOG_ERROR, "failed to parse date cvalue");

            return false;
        }

        utime = mktime(&utm);
        ctime = mktime(&ctm);

        return op(difftime(utime, ctime), 0);
    } else {
        return false;
    }
}

static bool
fnLT(const double l, const double r)
{
    return l < r;
}

static bool
fnGT(const double l, const double r)
{
    return l > r;
}

static bool
operatorBefore(const struct LDJSON *const uvalue,
    const struct LDJSON *const cvalue)
{
    return compareTime(uvalue, cvalue, fnLT);
}

static bool
operatorAfter(const struct LDJSON *const uvalue,
    const struct LDJSON *const cvalue)
{
    return compareTime(uvalue, cvalue, fnGT);
}

OpFn
lookupOperation(const char *const operation)
{
    LD_ASSERT(operation);

    if (strcmp(operation, "in") == 0) {
        return operatorInFn;
    } else if (strcmp(operation, "endsWith") == 0) {
        return operatorEndsWithFn;
    } else if (strcmp(operation, "startsWith") == 0) {
        return operatorStartsWithFn;
    } else if (strcmp(operation, "matches") == 0) {
        return operatorMatchesFn;
    } else if (strcmp(operation, "contains") == 0) {
        return operatorContainsFn;
    } else if (strcmp(operation, "lessThan") == 0) {
        return operatorLessThanFn;
    } else if (strcmp(operation, "lessThanOrEqual") == 0) {
        return operatorLessThanOrEqualFn;
    } else if (strcmp(operation, "greaterThan") == 0) {
        return operatorGreaterThanFn;
    } else if (strcmp(operation, "greaterThanOrEqual") == 0) {
        return operatorGreaterThanOrEqualFn;
    } else if (strcmp(operation, "before") == 0) {
        return operatorBefore;
    } else if (strcmp(operation, "after") == 0) {
        return operatorAfter;
    }

    return NULL;
}
