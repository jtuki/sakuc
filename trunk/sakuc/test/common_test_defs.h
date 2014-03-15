#ifndef COMMON_TEST_DEFS_H_
#define COMMON_TEST_DEFS_H_

#define sakuc_assert(condition) do {      \
    if (! (condition))                    \
        goto sakuc_assert_failed;         \
} while (__LINE__ == -1)

#endif // COMMON_TEST_DEFS_H_
