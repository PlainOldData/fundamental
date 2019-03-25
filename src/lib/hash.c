#include <lib/hash.h>
#include <lib/assert.h>
#include <string.h>


/* https://stackoverflow.com/questions/2535284/how-can-i-hash-a-string-to-an-int-using-c#13487193 */
/* test this, attribute or remove */
uint64_t
lib_hash(const char *name)
{
        LIB_ASSERT(name);
        LIB_ASSERT(strlen(name));
  
        uint64_t hash = 5381;
        int c;

        while (c = *name++, c) {
                hash = ((hash << 5) + hash) + c;
        }

        return hash;
}
