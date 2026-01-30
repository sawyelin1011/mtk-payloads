/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 * SPDX-FileCopyrightText: 2026 Shomy, R0rt1z2
 */

#include <stdint.h>
#include <stdbool.h>
#include <heap.h>
#include <debug.h>
#include <libc.h>

static int ptr_valid(uintptr_t p, uintptr_t base, uintptr_t end)
{
    return p >= base && p < end && (p & PTR_ALIGN_MASK) == 0;
}

void heap_dump(uintptr_t addr)
{
    struct heap *h = (struct heap *)addr;

    uintptr_t base = (uintptr_t)h->base;
    uintptr_t end = base + h->len;

    printf("heap @ 0x%lx\n", (unsigned long)addr);

#ifdef __aarch64__
    printf("  base=0x%lx len=0x%lx rem=0x%lx\n",
           (unsigned long)base,
           (unsigned long)h->len,
           (unsigned long)h->remaining);
#else
    printf("  base=0x%lx len=0x%lx\n",
           (unsigned long)base,
           (unsigned long)h->len);
#endif

    struct list_node *head = &h->free_list;
    struct list_node *node = head->next;

    printf("  head @ 0x%lx\n", (unsigned long)head);
    printf("  free_list.prev=0x%lx free_list.next=0x%lx\n",
           (unsigned long)head->prev,
           (unsigned long)head->next);

    if (node == head)
    {
        printf("  (free list empty)\n");
        return;
    }

    if (!ptr_valid((uintptr_t)node, (uintptr_t)head, end))
    {
        printf("  free_list.next 0x%lx OUTSIDE heap [0x%lx-0x%lx]\n",
               (unsigned long)node, (unsigned long)base, (unsigned long)end);
        return;
    }

    int i = 0;
    struct list_node *first = node;
    do
    {
        if (!ptr_valid((uintptr_t)node, (uintptr_t)head, end))
        {
            printf("  [%d] 0x%lx INVALID\n", i, (unsigned long)node);
            break;
        }

        struct free_heap_chunk *c = (struct free_heap_chunk *)node;
        printf("  [%d] 0x%lx len=0x%lx prev=0x%lx next=0x%lx\n",
               i, (unsigned long)c, (unsigned long)c->len, (unsigned long)c->node.prev, (unsigned long)c->node.next);

        node = c->node.next;
        i++;
    } while (node && node != first && i < 10);
}

void heap_dump_chunk(void *user_ptr)
{
    if (!user_ptr)
    {
        printf("chunk: NULL\n");
        return;
    }

    struct alloc_struct_begin *as = (struct alloc_struct_begin *)user_ptr;
    as--;

    printf("chunk @ 0x%lx\n", (unsigned long)user_ptr);

#ifndef __aarch64__
    printf("  magic = 0x%08x %s\n", as->magic,
           (as->magic == HEAP_MAGIC) ? "(ok)" : "(INVALID!)");
#endif

    printf("  ptr   = 0x%lx\n", (unsigned long)as->ptr);
    printf("  size  = 0x%lx\n", (unsigned long)as->size);
}

static void clear_dpc(uintptr_t corrupted_node)
{
    /*
     * The corrupted node is a fake "chunk" that's actually pointing
     * into the DPC region. Clear the key/cb/arg fields to prevent
     * code execution.
     */
    uintptr_t dpc_key_addr = corrupted_node + DPC_KEY_OFFSET;

    printf("  Clearing DPC @ 0x%lx (0x%x bytes from key field)\n",
           (unsigned long)dpc_key_addr, DPC_CLEAR_SIZE);
    memset((void *)dpc_key_addr, 0, DPC_CLEAR_SIZE);
}

void heap_fix(uintptr_t addr)
{
    struct heap *h = (struct heap *)addr;
    uintptr_t base = (uintptr_t)h->base;
    uintptr_t end  = base + h->len;
    struct list_node *head = &h->free_list;

    printf("Fixing heap @ 0x%lx\n", (unsigned long)addr);
    printf("  heap range: [0x%lx - 0x%lx]\n", (unsigned long)base, (unsigned long)end);

    struct list_node *last_valid = head;
    struct list_node *first_valid = NULL;
    struct list_node *curr = head->next;

    int iterations = 0;
    const int max_iter = 256;

    while (iterations++ < max_iter) {
        if (curr == head)
            break;
        if (first_valid != NULL && curr == first_valid)
            break;

        bool valid = ptr_valid((uintptr_t)curr, (uintptr_t)head, end);
        struct list_node *next = curr->next;

        if (valid) {
            last_valid->next = curr;
            curr->prev = last_valid;
            last_valid = curr;

            if (first_valid == NULL) {
                first_valid = curr;
                printf("  first valid @ 0x%lx\n", (unsigned long)curr);
            }
        } else {
            printf("  skipping invalid @ 0x%lx\n", (unsigned long)curr);
            if ((uintptr_t)curr <= base || (uintptr_t)curr > end) clear_dpc((uintptr_t)curr);
        }

        curr = next;
    }

    last_valid->next = head->next;
    head->prev = 0;

    printf("  last valid @ 0x%lx\n", (unsigned long)last_valid);
    printf("Heap fixed: head <-> 0x%lx ... 0x%lx <-> head\n",
           (unsigned long)(first_valid ? first_valid : head),
           (unsigned long)last_valid);
}
