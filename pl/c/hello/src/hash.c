#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <gdsl/gdsl_types.h>
#include <gdsl/gdsl_hash.h>



char* strdup(const char* s) {
        size_t slen = strlen(s);
        char* result = malloc(slen + 1);
        if(result == NULL)
        {
                return NULL;
        }

        memcpy(result, s, slen+1);
        return result;
}

struct _my_struct
{
        int  integer;
        char*string;
};
typedef struct _my_struct* my_struct;

gdsl_element_t
my_struct_alloc (void* d)
{
        static int n = 10;

        my_struct e = (my_struct) malloc (sizeof (struct _my_struct));
        if (e == NULL)
        {
                return NULL;
        }

        e->integer = n++;
        e->string = strdup ((char*) d);

        return (gdsl_element_t) e;
}

void
my_struct_free (gdsl_element_t e)
{
        my_struct s = (my_struct) e;
        free (s->string);
        free (s);
}

void
my_struct_printf (gdsl_element_t e, FILE* file, gdsl_location_t location, void* d)
{
        my_struct s = (my_struct) e;
        fprintf (file, "%d:%s ", s->integer, s->string);
}

const char*
my_struct_key (gdsl_element_t e)
{
        my_struct s = (my_struct) e;
        return s->string;
}

int main(int argc, char *argv[]) {
        gdsl_hash_t ht;

        ht = gdsl_hash_alloc ("MY HASH TABLE", my_struct_alloc, my_struct_free, my_struct_key, NULL, 11);
        if (ht == NULL)
        {
                fprintf (stderr, "%s:%d: %s - gdsl_hash_alloc(): NULL",
                         __FILE__, __LINE__, __FUNCTION__);
                exit (EXIT_FAILURE);
        }

        gdsl_element_t e;
        char buf[32];
        for (int i=0; i < 10; i++) {
                sprintf(buf, "value%d", i);
                gdsl_hash_insert(ht, (void *)buf);
                e = gdsl_hash_search (ht, buf);
                printf("%p %s %d\n", e, buf, ((my_struct)e)->integer);
                assert(e != NULL);
        }


        return 0;
}
