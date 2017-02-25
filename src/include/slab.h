#ifndef SLAB_H_INCLUDED_
#define SLAB_H_INCLUDED_

typedef struct slab_page_s {
    uintptr_t slab;
    struct slab_page_s* next;
    uintptr_t prev;
} slab_page_t;

typedef struct slab_pool_s {
    shmtx_sh_t lock;

    size_t min_size;
    size_t min_shift;

    slab_page_t *pages;
    slab_page_t *last;
    slab_page_t free;

    u_char *start;
    u_char *end;

    shmtx_t mutex;

    u_char *log_ctx;
    u_char zero;

    unsigned log_nomem:1;

    void *data;
    void *addr;
} slab_pool_t;

#ifdef __cplusplus
extern "C" {
#endif

void slab_init(slab_pool_t *pool);
void *slab_alloc(slab_pool_t *pool, size_t size);
void *slab_alloc_locked(slab_pool_t *pool, size_t size);
void *slab_calloc(slab_pool_t *pool, size_t size);
void *slab_calloc_locked(slab_pool_t *pool, size_t size);
void slab_free(slab_pool_t *pool, void *p);
void slab_free_locked(slab_pool_t *pool, void *p);

#ifdef __cplusplus
}
#endif

#endif

