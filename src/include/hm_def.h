#ifndef HM_DEF_H
#define HM_DEF_H

/* the object aligned with 16 bytes */
#define HM_OBJ_MIN_SIZE	16
#define HM_ZONE_MAX 4
#define HM_POOL_MAGIC	0x1234abcd 
#define HM_BITS 22

#define HM_OK			0
#define HM_ERROR		-1

/* for each kind of pool, there are 4 types of zone */
#define HM_POOL_1K  { 16, 32, 64, 128, }
#define HM_POOL_4K  { 64, 128, 256, 512, }
#define HM_POOL_16K { 256, 512, 1024, 2048, }
#define HM_POOL_64K { 1024, 2048, 4096, 8192, }

#define HM_POOL_1K_MAX	1024*1024 /* 1GB: for at least 1M sessions */
#define HM_POOL_4K_MAX	1024*128  /* 512MB */
#define HM_POOL_16K_MAX	1024*32   /* 512MB */	
#define HM_POOL_64K_MAX 1024*8    /* 512MB */

enum {
	HM_ERR_NO_MEM = 1,
	HM_ERR_ALLOC_BLOCK,
	HM_ERR_ALLOC_HEAD,
	HM_ERR_SIZE_TOO_BIG,
};

enum {
	HM_HDR_BLOCK = 1,
	HM_HDR_HEAD,
	HM_HDR_ALLOCATED,
};

#endif

