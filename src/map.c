//
// Created by xaq on 10/9/17.
//

#include "map.h"
#include <stdlib.h>
#include <limits.h>

/* -------------------------- private prototypes ---------------------------- */
static int _map_init(map *m, map_type *type);

static void _map_reset(map_ht *ht);

static map_entry *_map_put_base(map *m, uint64_t key);

static map_entry *_map_find(map *m, uint64_t key);

static map_entry *_map_rm_base(map *m, uint64_t key);

static int _map_clear(map *m, map_ht *ht);

static void _map_rehash(map *m, map_ht *new_ht);

static int _map_key_index(map *m, uint64_t key);

static int _map_expand_if_needed(map *m);

static int _map_expand(map *m, unsigned long new_size);

/* ----------------------------- API implementation ------------------------- */
map *map_create(map_type *type){
    map *m = malloc(sizeof(map));
    _map_init(m, type);
    return m;
}

int map_put(map *m, uint64_t key, void *val){
    map_entry *entry = _map_put_base(m, key);

    if(!entry) return ERROR;
    if(m->type->value_dup)
        entry->v.val = m->type->value_dup(val);
    else
        entry->v.val = val;
    return SUCC;
}

int map_put_u64(map *m, uint64_t key, uint64_t val){
    map_entry *entry = _map_put_base(m, key);

    if(!entry) return ERROR;
    entry->v.u64 = val;
    return SUCC;
}

int map_put_s64(map *m, uint64_t key, int64_t val){
    map_entry *entry = _map_put_base(m, key);

    if(!entry) return ERROR;
    entry->v.s64 = val;
    return SUCC;
}

void *map_get(map *m, uint64_t key){
    map_entry *entry = _map_find(m, key);
    return entry ? entry->v.val : NULL;
}

int map_rm(map *m, uint64_t key){
    return _map_rm_base(m, key) ? SUCC : ERROR;
}

void map_free(map *m){
    _map_clear(m, &m->ht[0]);
    _map_clear(m, &m->ht[1]);
}

map_entry *map_find(map *m, uint64_t key){
    return _map_find(m, key);
}

uint64_t _default_int_hash_func(uint32_t key){
    key += ~(key << 15);
    key ^= (key >> 10);
    key += (key << 3);
    key ^= (key >> 6);
    key += ~(key << 11);
    key ^= (key >> 16);
    return key;
}

/* Murmur hash function 64bit version */
uint64_t _default_string_has_func(const void *key, int len, uint64_t seed){
    const uint64_t m = 0xc6a4a7935bd1e995ULL;
    const int r = 47;

    uint64_t h = seed ^(len * m);

    const uint64_t *data = (const uint64_t *) key;
    const uint64_t *end = data + (len / 8);

    while(data != end){
        uint64_t k = *data++;

        k *= m;
        k ^= k >> r;
        k *= m;

        h ^= k;
        h *= m;
    }

    const unsigned char *data2 = (const unsigned char *) data;

    switch(len & 7){
        case 7:
            h ^= (uint64_t) (data2[6]) << 48;
        case 6:
            h ^= (uint64_t) (data2[5]) << 40;
        case 5:
            h ^= (uint64_t) (data2[4]) << 32;
        case 4:
            h ^= (uint64_t) (data2[3]) << 24;
        case 3:
            h ^= (uint64_t) (data2[2]) << 16;
        case 2:
            h ^= (uint64_t) (data2[1]) << 8;
        case 1:
            h ^= (uint64_t) (data2[0]);
            h *= m;
    };

    h ^= h >> r;
    h *= m;
    h ^= h >> r;

    return h;
}

map_iterator *map_get_iter(map *m){
    map_iterator *iter = malloc(sizeof(map_iterator));

    iter->m = m;
    iter->table = 0;
    iter->index = -1;
    iter->entry = NULL;
    iter->next_entry = NULL;

    return iter;
}

map_entry *map_iter_next(map_iterator *iter){
    while(1){
        if(!iter->entry){
            map_ht *ht = &iter->m->ht[iter->table];
            iter->index++;
            if(iter->index >= (long) (ht->size)){
                if(iter->table == 0){
                    iter->table++;
                    iter->index = 0;
                    ht = &iter->m->ht[1];
                } else
                    break;
            }
            iter->entry = ht->buckets[iter->index];
        } else
            iter->entry = iter->next_entry;
        if(iter->entry){
            iter->next_entry = iter->entry->next;
            return iter->entry;
        }
    }
    return NULL;
}

void map_release_iter(map_iterator *iter){
    free(iter);
}

/* ------------------------ private API implementation ---------------------- */
int _map_init(map *m, map_type *type){
    m->ht[0].buckets = (map_entry **) calloc(MAP_HT0_INITIAL_SIZE, sizeof(map_entry *));
    m->ht[0].size = MAP_HT0_INITIAL_SIZE;
    m->ht[0].size_mask = m->ht[0].size - 1;
    m->ht[0].used = 0;
    m->ht[1].buckets = (map_entry **) calloc(MAP_HT1_INITIAL_SIZE, sizeof(map_entry *));
    m->ht[1].size = MAP_HT1_INITIAL_SIZE;
    m->ht[1].size_mask = m->ht[1].size - 1;
    m->ht[1].used = 0;
    m->type = type;
    return SUCC;
}

void _map_reset(map_ht *ht){
    ht->buckets = NULL;
    ht->size = 0;
    ht->size_mask = 0;
    ht->used = 0;
}

int _map_clear(map *m, map_ht *ht){
    unsigned long i;

    for(i = 0; i < ht->size && ht->used > 0; i++){
        map_entry *entry, *next_entry;
        if((entry = ht->buckets[i]) == NULL) continue;
        while(entry){
            next_entry = entry->next;
            if(m->type->value_free)
                m->type->value_free(entry->v.val);
            free(entry);
            ht->used--;
            entry = next_entry;
        }
    }
    free(ht->buckets);
    _map_reset(ht);
    return SUCC;
}

map_entry *_map_put_base(map *m, uint64_t key){
    int index;
    map_entry *entry;
    map_ht *ht;

    if(key < MAP_DIRECT_ACCESS){
        if(m->ht[0].buckets[key])
            return NULL;
        ht = &m->ht[0];
        index = key;
        goto END;
    }

    if((index = _map_key_index(m, key)) == -1)
        return NULL;
    ht = &m->ht[1];

END:
    entry = malloc(sizeof(map_entry));
    entry->next = ht->buckets[index];
    ht->buckets[index] = entry;
    ht->used++;
    entry->key = key;
    return entry;
}

int _map_key_index(map *m, uint64_t key){
    unsigned int hash, index;
    map_entry *entry;

    hash = m->type->hash_func(&key);
    if(_map_expand_if_needed(m) == ERROR)
        return -1;

    index = hash & m->ht[1].size_mask;
    entry = m->ht[1].buckets[index];
    while(entry){
        if(key == entry->key || (m->type->key_compare && m->type->key_compare(key, entry->key) == 0)){
            return -1;
        }
        entry = entry->next;
    }

    return index;
}

int _map_expand_if_needed(map *m){
    /* threshold为size的0.75倍 */
    unsigned long threshold = (m->ht[1].size >> 1) + (m->ht[1].size >> 2);
    if(m->ht[1].used == threshold)
        return _map_expand(m, m->ht[1].size * 2);

    return SUCC;
}

int _map_expand(map *m, unsigned long new_size){
    map_ht new_ht;

    new_ht.size = new_size;
    new_ht.size_mask = new_size - 1;
    new_ht.buckets = calloc(new_size, sizeof(map_entry *));

    _map_rehash(m, &new_ht);

    m->ht[1] = new_ht;
    return SUCC;
}

map_entry *_map_find(map *m, uint64_t key){
    map_entry *entry;
    size_t hash, index;

    if(m->ht[0].used + m->ht[1].used == 0) return NULL;

    if(key < MAP_DIRECT_ACCESS)
        return m->ht[0].buckets[key];

    hash = m->type->hash_func(&key);
    index = hash & m->ht[1].size_mask;
    entry = m->ht[1].buckets[index];
    while(entry){
        if(key == entry->key || (m->type->key_compare && m->type->key_compare(key, entry->key) == 0))
            return entry;
        entry = entry->next;
    }
    return NULL;
}

map_entry *_map_rm_base(map *m, uint64_t key){
    size_t hash, index;
    map_entry *entry, *prev_entry;

    if(m->ht[0].used + m->ht[1].used == 0) return NULL;

    if(key < MAP_DIRECT_ACCESS){
        entry = m->ht[0].buckets[key];
        if(entry){
            if(m->type->value_free)
                m->type->value_free(entry);
            free(entry);
            m->ht[0].used--;
            return entry;
        }
        return NULL;
    }

    hash = m->type->hash_func(&key);
    index = hash & m->ht[1].size_mask;
    entry = m->ht[1].buckets[index];
    prev_entry = NULL;
    while(entry){
        if(key == entry->key || (m->type->key_compare && m->type->key_compare(key, entry->key) == 0)){
            if(prev_entry)
                prev_entry->next = entry->next;
            else
                m->ht[1].buckets[index] = entry->next;
            if(m->type->value_free)
                m->type->value_free(entry->v.val);
            free(entry);
            m->ht[1].used--;
            return entry;
        }
        prev_entry = entry;
        entry = entry->next;
    }
    return NULL;
}

void _map_rehash(map *m, map_ht *new_ht){
    size_t hash, index;
    unsigned long i;

    for(i = 0; i < m->ht[1].size; i++){
        map_entry *entry, *next_entry;
        if((entry = m->ht[1].buckets[i]) != NULL){
            while(entry){
                next_entry = entry->next;
                entry->next = NULL;
                hash = m->type->hash_func(&entry->key);
                index = hash & new_ht->size_mask;
                if(new_ht->buckets[index])
                    entry->next = new_ht->buckets[index];
                new_ht->buckets[index] = entry;
                entry = next_entry;
            }
        }
    }
    new_ht->used = m->ht[1].used;
    free(m->ht[1].buckets);
}