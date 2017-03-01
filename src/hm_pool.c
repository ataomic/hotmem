#include <mm_pool.h>

void mm_pool_fini(mm_pool* pool)
{
}

void* mm_pool_alloc_mem(mm_pool* pool, size_t size, int* ret)
{
	size_t tot_size = MM_SIZE(size+sizeof(mm_hdr));
	mm_hdr* hdr = (mm_hdr*)pool->ops->alloc(tot_size);
	if(hdr) {
		hdr->size = (u32)size;
		hdr->psize = 0;
		hdr->pool = pool;
		return (hdr+1);
	}
	*ret = MM_ERR_NO_MEM;
	return NULL;
}

static __inline void mm_pool_free_mem(mm_pool* pool, mm_hdr* hdr)
{
	pool->ops->free(hdr);
}

void* mm_pool_alloc_from_block(mm_pool* pool, size_t size, int* ret)
{
	mm_hdr* hdr;
	size_t tot_size = MM_SIZE(size+sizeof(mm_hdr));
	mm_block* block;

	if(tot_size > block->addr+pool->block_size-block->pos) {
		*ret = MM_ERR_SIZE_TOO_BIG;
		return mm_pool_alloc_mem(pool, size);
	}

	block = mm_pool_cur_block(pool, size);
	if(!block && (block = mm_pool_add_block(pool)) == NULL) {
		*ret = MM_ERR_ALLOC_BLOCK;
		return NULL;
	}
	pool->ops->lock(&block->lock);
	hdr = (mm_hdr*)block->pos;
	hdr->size = size;
	hdr->pool = pool;
	/* hdr->psize = ? don't touch this field */
	block->pos += tot_size;
	pool->ops->unlock(&block->lock);
}

static __inline void* mm_pool_alloc_from_head(mm_pool* pool,
	mm_head* head, size_t size)
{
	pool->ops->lock(&head->lock);
	mm_pool_do_alloc_from_head(pool, head, size);
	pool->ops->unlock(&head->lock);
}

void* mm_pool_alloc(mm_pool* pool, size_t size)
{
	mm_head* head = mm_head_find(&pool->root, size);
	if(head)
		return mm_pool_alloc_from_head(pool, head, size);
	return mm_pool_alloc_from_block(pool, size);
}

void mm_pool_free(void* ptr)
{
	mm_hdr* hdr;
	mm_head* head;

	if(!ptr)
		return;
	hdr = (mm_hdr*)(ptr)-1;
	
	if(hdr->size < 0) {
		/* double free !!! */
		assert(0);
		return;
	}

	if(hdr->pool->magic != MM_POOL_MAGIC) {
		/* wrong pool */
		assert(0);
		return;
	}
	
	if(hdr->size >= pool->block_size) {
		mm_pool_free_mem(pool, hdr);
		return;
	}

	if(hdr->psize)
		hdr = mm_pool_merge(pool, hdr);
	if(hdr->type == MM_HDR_BLOCK) {
		mm_pool_free_block(pool, (mm_block*)(hdr+1));
		return;
	}

	head = mm_head_find(&pool->root, hdr->size);
	if(!head)
		head = mm_pool_add_head(pool, hdr->size);
	mm_pool_free_to_head(pool, head, hdr);
}

