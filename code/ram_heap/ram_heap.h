#ifndef _RAM_HEAP_H__
#define _RAM_HEAP_H__

/*
 * Lightweight UI & Widget RAM Pool Manager for GPCE500A (u'nSP 16-bit word-addressed architecture):
 *
 * Why a custom heap on a 4K-word MCU?
 * 1. Standard C malloc/free carries 4~8 words of metadata per block and easily fragments small RAM.
 * 2. Here, every block uses ONLY ONE 16-bit header word:
 *      - Bit 15     (0x8000): Allocated flag (1 = In Use, 0 = Free)
 *      - Bits 14..0 (0x7FFF): Total block size in 16-bit words (including the 1-word header)
 * 3. Supports TWO complementary allocation patterns in the same pool:
 *      (A) Dynamic Alloc/Free (`heap_alloc`, `heap_calloc`, `heap_free`):
 *          - First-fit search with block splitting and immediate forward/backward coalescing
 *            on `heap_free()` so adjacent free blocks merge back into one contiguous block.
 *      (B) Page Watermark / Arena Scope (`heap_get_mark`, `heap_release_to_mark`, `heap_reset`):
 *          - Record a watermark before entering a UI menu/dialog (`mark = heap_get_mark()`),
 *            allocate all widgets for that page, and when exiting the page call
 *            `heap_release_to_mark(mark)` to reclaim the entire page's widgets in O(1) with
 *            ZERO fragmentation!
 */

// clang-format off
#define HEAP_POOL_WORDS         512U    /* 512 words = 1KB UI widget pool (adjust as needed) */
#define HEAP_USED_FLAG          0x8000U
#define HEAP_SIZE_MASK          0x7FFFU
#define HEAP_MIN_SPLIT_WORDS    3U      /* 1 header word + at least 2 payload words */

/* Type-safe helper macros for allocating widget structures */
#define HEAP_NEW(type)              ((type *)heap_calloc((u16)sizeof(type)))
#define HEAP_NEW_ARRAY(type, count) ((type *)heap_calloc((u16)((u16)sizeof(type) * (u16)(count))))
// clang-format on

typedef u16 HEAP_Mark;

typedef struct
{
    u16 total_words;        /* Total pool capacity in words */
    u16 used_words;         /* Currently allocated words (including 1-word headers) */
    u16 free_words;         /* Total free words remaining */
    u16 largest_free_words; /* Largest single contiguous free payload (fragmentation indicator) */
    u16 alloc_block_count;  /* Number of active allocated blocks */
} HEAP_Stats;

void heap_init_module(void);
void heap_reset(void);

void *heap_alloc(u16 size_words);
void *heap_calloc(u16 size_words);
void heap_free(void *ptr);

HEAP_Mark heap_get_mark(void);
void heap_release_to_mark(HEAP_Mark mark);

void heap_get_stats(HEAP_Stats *stats);
u16 heap_get_free_words(void);

#endif /* _RAM_HEAP_H__ */
