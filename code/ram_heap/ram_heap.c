#include "..\Include\spinmaster.h"

/*
 * Memory pool layout inside `heap_pool[HEAP_POOL_WORDS]`:
 *
 *   +------------------+-------------------------------+------------------+-------------------+
 *   | Header (1 word)  | Payload (size - 1 words)      | Header (1 word)  | Payload ...       |
 *   | [Bit15|Size14:0] |  <-- pointer returned to user | [Bit15|Size14:0] |                   |
 *   +------------------+-------------------------------+------------------+-------------------+
 *   ^                                                  ^
 *   idx = 0                                            idx = block0_size
 *
 * Notice that in u'nSP (where `sizeof(u16) == 1` char-unit), `sizeof(GUI_Button)` is already
 * expressed in 16-bit words, and every pointer is naturally 2-word (32-bit) or 1-word aligned.
 * Keeping sizes in `u16` words means walking from one block to the next is simply:
 *     next_idx = idx + (heap_pool[idx] & HEAP_SIZE_MASK);
 */
static u16 heap_pool[HEAP_POOL_WORDS];
static bool heap_initialized = false;

/*
 *@ brief    : Verify that the heap has been initialized and that block headers walk cleanly
 *             to HEAP_POOL_WORDS.
 *@ type     : private
 *@ argu in  : None.
 *@ argu out : Halts on uninitialized state or corrupted block header.
 *@ theme    : Fail-fast integrity check against buffer overruns in UI code.
 */
static void _heap_check_ready(void)
{
    if (!heap_initialized)
    {
        bsp_halt_fault(BSP_Fault_HEAP_State);
    }
}

/*
 *@ brief    : Merge all adjacent free blocks across the entire pool.
 *@ type     : private
 *@ argu in  : None.
 *@ argu out : Contiguous free blocks coalesced into single larger free blocks.
 *@ theme    : Linear single-pass coalescing keeps fragmentation near zero.
 */
static void _heap_coalesce(void)
{
    u16 idx = 0U;

    while (idx < HEAP_POOL_WORDS)
    {
        u16 hdr = heap_pool[idx];
        u16 blk_size = hdr & HEAP_SIZE_MASK;
        u16 next_idx = (u16)(idx + blk_size);

        if (blk_size < 2U || next_idx > HEAP_POOL_WORDS)
        {
            bsp_halt_fault(BSP_Fault_HEAP_Corrupt);
        }

        /* If both current block and next block are free, merge them */
        if ((hdr & HEAP_USED_FLAG) == 0U && next_idx < HEAP_POOL_WORDS &&
            (heap_pool[next_idx] & HEAP_USED_FLAG) == 0U)
        {
            u16 next_size = heap_pool[next_idx] & HEAP_SIZE_MASK;
            if (next_size < 2U || (u16)(next_idx + next_size) > HEAP_POOL_WORDS)
            {
                bsp_halt_fault(BSP_Fault_HEAP_Corrupt);
            }
            heap_pool[idx] = (u16)(blk_size + next_size);
            /* Stay at `idx` in case a third consecutive free block follows */
            continue;
        }

        idx = next_idx;
    }
}

/*
 *@ brief    : Initialize the RAM heap pool as a single large free block.
 *@ type     : public
 *@ argu in  : None.
 *@ argu out : Pool ready for allocation.
 *@ theme    : Called once during bsp_init_global().
 */
void heap_init_module(void)
{
    heap_pool[0] = HEAP_POOL_WORDS & HEAP_SIZE_MASK;
    heap_initialized = true;
}

/*
 *@ brief    : Reset the entire heap pool back to a single empty block (O(1)).
 *@ type     : public
 *@ argu in  : None.
 *@ argu out : All allocations discarded and full capacity restored.
 *@ theme    : Instant full-pool reclamation when switching top-level desktop apps/menus.
 */
void heap_reset(void)
{
    _heap_check_ready();
    heap_pool[0] = HEAP_POOL_WORDS & HEAP_SIZE_MASK;
}

/*
 *@ brief    : Allocate `size_words` of uninitialized memory from the heap pool.
 *@ type     : public
 *@ argu in  : size_words: number of 16-bit words requested (e.g., sizeof(GUI_Button)).
 *@ argu out : Pointer to payload, or null if out of memory.
 *@ theme    : First-fit allocator with block splitting.
 */
void *heap_alloc(u16 size_words)
{
    u16 idx = 0U;
    u16 needed;

    _heap_check_ready();
    if (size_words == 0U || size_words >= HEAP_POOL_WORDS)
    {
        return null;
    }

    /* 1 word for the block header + requested payload words */
    needed = (u16)(size_words + 1U);

    while (idx < HEAP_POOL_WORDS)
    {
        u16 hdr = heap_pool[idx];
        u16 blk_size = hdr & HEAP_SIZE_MASK;

        if (blk_size < 2U || (u16)(idx + blk_size) > HEAP_POOL_WORDS)
        {
            bsp_halt_fault(BSP_Fault_HEAP_Corrupt);
        }

        if ((hdr & HEAP_USED_FLAG) == 0U && blk_size >= needed)
        {
            u16 rem = (u16)(blk_size - needed);
            if (rem >= HEAP_MIN_SPLIT_WORDS)
            {
                /* Split the free block: allocate front part, create free header for remainder */
                heap_pool[idx] = (u16)(HEAP_USED_FLAG | needed);
                heap_pool[idx + needed] = rem;
            }
            else
            {
                /* Remainder too small to split; hand the whole block to caller */
                heap_pool[idx] = (u16)(HEAP_USED_FLAG | blk_size);
            }
            return (void *)&heap_pool[idx + 1U];
        }

        idx = (u16)(idx + blk_size);
    }

    return null;
}

/*
 *@ brief    : Allocate `size_words` from the heap pool and zero-initialize the payload.
 *@ type     : public
 *@ argu in  : size_words: number of 16-bit words requested.
 *@ argu out : Zero-filled payload pointer, or null if out of memory.
 *@ theme    : Safe widget allocation so uninitialized struct fields default to 0/null.
 */
void *heap_calloc(u16 size_words)
{
    u16 *p = (u16 *)heap_alloc(size_words);
    u16 i;

    if (p != null)
    {
        for (i = 0U; i < size_words; ++i)
        {
            p[i] = 0U;
        }
    }
    return (void *)p;
}

/*
 *@ brief    : Free a previously allocated block and coalesce adjacent free blocks.
 *@ type     : public
 *@ argu in  : ptr: pointer returned by heap_alloc() or heap_calloc().
 *@ argu out : Block marked free and merged with any neighboring free blocks.
 *@ theme    : Safe individual widget deallocation with immediate defragmentation.
 */
void heap_free(void *ptr)
{
    u16 *p;
    u16 hdr_idx;
    u16 hdr;

    _heap_check_ready();
    if (ptr == null)
    {
        return;
    }

    p = (u16 *)ptr;
    if (p < &heap_pool[1] || p >= &heap_pool[HEAP_POOL_WORDS])
    {
        bsp_halt_fault(BSP_Fault_HEAP_Arg);
    }

    hdr_idx = (u16)(p - &heap_pool[0] - 1U);
    hdr = heap_pool[hdr_idx];

    /* Double-free or invalid pointer check */
    if ((hdr & HEAP_USED_FLAG) == 0U)
    {
        bsp_halt_fault(BSP_Fault_HEAP_State);
    }

    heap_pool[hdr_idx] = hdr & HEAP_SIZE_MASK;
    _heap_coalesce();
}

/*
 *@ brief    : Capture the current high-water mark (end of the last allocated block).
 *@ type     : public
 *@ argu in  : None.
 *@ argu out : Watermark index (0..HEAP_POOL_WORDS) representing current UI stack depth.
 *@ theme    : Save state before entering a sub-menu or popup dialog.
 */
HEAP_Mark heap_get_mark(void)
{
    u16 idx = 0U;
    u16 mark = 0U;

    _heap_check_ready();
    while (idx < HEAP_POOL_WORDS)
    {
        u16 hdr = heap_pool[idx];
        u16 blk_size = hdr & HEAP_SIZE_MASK;

        if (blk_size < 2U || (u16)(idx + blk_size) > HEAP_POOL_WORDS)
        {
            bsp_halt_fault(BSP_Fault_HEAP_Corrupt);
        }
        if ((hdr & HEAP_USED_FLAG) != 0U)
        {
            mark = (u16)(idx + blk_size);
        }
        idx = (u16)(idx + blk_size);
    }
    return mark;
}

/*
 *@ brief    : Release all blocks allocated at or after `mark` in a single operation.
 *@ type     : public
 *@ argu in  : mark: watermark previously obtained from heap_get_mark().
 *@ argu out : All allocations at/above `mark` collapsed back into one trailing free block.
 *@ theme    : Zero-fragmentation page/dialog exit cleanup.
 */
void heap_release_to_mark(HEAP_Mark mark)
{
    _heap_check_ready();
    if (mark > HEAP_POOL_WORDS)
    {
        bsp_halt_fault(BSP_Fault_HEAP_Arg);
    }
    if (mark == HEAP_POOL_WORDS)
    {
        return;
    }

    /* Collapse everything from `mark` to the end of the pool into one free block */
    heap_pool[mark] = (u16)(HEAP_POOL_WORDS - mark);
    _heap_coalesce();
}

/*
 *@ brief    : Query heap utilization and fragmentation metrics.
 *@ type     : public
 *@ argu in  : stats: output structure to populate.
 *@ argu out : Current used, free, largest contiguous free payload, and block count filled.
 *@ theme    : Runtime memory diagnostics for UI tuning.
 */
void heap_get_stats(HEAP_Stats *stats)
{
    u16 idx = 0U;

    _heap_check_ready();
    if (stats == null)
    {
        bsp_halt_fault(BSP_Fault_HEAP_Arg);
    }

    stats->total_words = HEAP_POOL_WORDS;
    stats->used_words = 0U;
    stats->free_words = 0U;
    stats->largest_free_words = 0U;
    stats->alloc_block_count = 0U;

    while (idx < HEAP_POOL_WORDS)
    {
        u16 hdr = heap_pool[idx];
        u16 blk_size = hdr & HEAP_SIZE_MASK;

        if (blk_size < 2U || (u16)(idx + blk_size) > HEAP_POOL_WORDS)
        {
            bsp_halt_fault(BSP_Fault_HEAP_Corrupt);
        }

        if ((hdr & HEAP_USED_FLAG) != 0U)
        {
            stats->used_words = (u16)(stats->used_words + blk_size);
            stats->alloc_block_count++;
        }
        else
        {
            u16 payload_free = (blk_size > 1U) ? (u16)(blk_size - 1U) : 0U;
            stats->free_words = (u16)(stats->free_words + blk_size);
            if (payload_free > stats->largest_free_words)
            {
                stats->largest_free_words = payload_free;
            }
        }

        idx = (u16)(idx + blk_size);
    }
}

/*
 *@ brief    : Return the largest single contiguous payload size (in words) currently allocatable.
 *@ type     : public
 *@ argu in  : None.
 *@ argu out : Maximum `size_words` that a single `heap_alloc()` call can succeed with.
 *@ theme    : Quick free-space check.
 */
u16 heap_get_free_words(void)
{
    HEAP_Stats st;
    heap_get_stats(&st);
    return st.largest_free_words;
}
