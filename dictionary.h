#include <stdlib.h>

typedef struct key_value{
    void* key;
    void* value;
    size_t key_length;
    struct key_value* next;
    struct key_value* next_dict_entry;
} key_value_t;

typedef struct dictionary{
    unsigned long size;
    size_t hash_size;
    key_value_t* first_entry;
    key_value_t** entries;
} dictionary_t;

key_value_t* new_key_value(void* key, void* value, size_t key_length);
int register_key_value(dictionary_t* dict, key_value_t* key_val);
void* get_value(dictionary_t* dict, void* key, size_t key_length);
key_value_t* get_key_value(dictionary_t* dict, void* key, size_t key_length);
void remove_key_value(dictionary_t* dict, void* key, size_t key_length);

dictionary_t* new_dictionary(const size_t size);