#ifndef COMMON_MEMORY_MANAGEMENT_DEFS_H_
#define COMMON_MEMORY_MANAGEMENT_DEFS_H_

#include <stdlib.h>
#include <string.h>

/* osal - operating system abstract layer (a name adopted from TI z-stack)
 */

#ifndef osal_mem_alloc
#define osal_mem_alloc(size) malloc(size)
#endif

#ifndef osal_mem_calloc // @n - how many blocks, @size - block size.
#define osal_mem_calloc(n, size) calloc((n), (size))
#endif

#ifndef osal_mem_free
#define osal_mem_free(block) free(block)
#endif

#ifndef osal_memcpy
#define osal_memcpy(dst, src, size) memcpy((dst), (src), (size))
#endif

#endif // COMMON_MEMORY_MANAGEMENT_DEFS_H_

