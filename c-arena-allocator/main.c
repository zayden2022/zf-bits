#include <stddef.h>
#include <stdint.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef float f32;
typedef double f64;

typedef size_t usize;

#define KiB(x) ((x) * 1024)
#define MiB(x) (KiB(x) * 1024)
#define GiB(x) (MiB(x) * 1024)

#define MIN(a, b) ((a < b) ? (a) : (b))
#define MAX(a, b) ((a > b) ? (a) : (b))

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

struct arena {
    u8 *data;
    usize size;
    usize used;
};

struct arena_create_out {
    struct arena arena;
    bool success;
};

struct arena_create_out arena_create(usize size) {
    struct arena_create_out out = {0};
    if (size == 0) {
        return out;
    }
    out.arena.data = malloc(size);
    if (out.arena.data == NULL) {
        return out;
    }

    // malloc returns an aligned memory block.  If malloc is raplaced with
    // something else, make sure that the first allocation will be aligned.

    out.arena.size = size;
    out.arena.used = 0;
    out.success = true;
    return out;
}

#define ARENA_ALIGN _Alignof(max_align_t)

struct arena_alloc_out {
    void *ptr;
    bool success;
};

struct arena_alloc_out arena_alloc(struct arena *arena, usize size) {
    struct arena_alloc_out out = {0};
    if (arena == NULL || arena->data == NULL || size == 0) {
        return out;
    }

    if (size > SIZE_MAX - ARENA_ALIGN) {
        // avoiding size overflow during alignment below
        return out;
    }

    assert(arena->used <= arena->size);

    if (size % ARENA_ALIGN != 0) {
        size += ARENA_ALIGN - (size % ARENA_ALIGN);
    }
    usize space_left = arena->size - arena->used;
    if (size > space_left) {
        return out;
    }
    out.ptr = arena->data + arena->used;
    arena->used += size;
    out.success = true;
    return out;
}

/**
 * Clear the arena, but not deallocate the underlying memory.
 */
void arena_clear(struct arena *arena) { arena->used = 0; }

/**
 * Free memory allocated by the arena.
 * All memory blocks allocated in this arena will become invalid.
 */
void arena_free(struct arena *arena) {
    free(arena->data);
    arena->data = 0;
    arena->size = 0;
    arena->used = 0;
}

#include <stdio.h>

int main(void) {
    printf("hello\n");
    printf("alignment: %lu\n", _Alignof(max_align_t));

    struct arena_create_out arena_out = arena_create(1024);
    if (!arena_out.success) {
        printf("failed to create the arena\n");
        return 1;
    }

    struct arena arena = arena_out.arena;
    printf("arena(address = %p, size = %zu, used = %zu)\n", arena.data, arena.size, arena.used);

    while(true) {
        usize alloc_size = 100;
        struct arena_alloc_out alloc_res = arena_alloc(&arena, alloc_size);
        if(alloc_res.success) {
            printf("successfully allocated 0x%zx bytes, address: 0x%p\n", alloc_size, alloc_res.ptr);
        } else {
            printf("failed to allocated 0x%zx bytes\n", alloc_size);
            break;
        }
        printf("arena(address = %p, size = %zu, used = %zu)\n", arena.data, arena.size, arena.used);
    }

    arena_free(&arena);
    return 0;
}
