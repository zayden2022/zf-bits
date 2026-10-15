// types.c
#ifndef TYPES_C
#define TYPES_C

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

#define KiB(x) x * 1024
#define MiB(x) KiB(x) * 1024
#define GiB(x) MiB(x) * 1024

#define MIN(a, b) a < b ? a : b
#define MAX(a, b) a > b ? a : b

#endif // TYPES_C

#ifndef ARENA_C
#define ARENA_C

// #include "types.c"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

struct arena {
    void *data;
    usize size;
    usize used;
};

struct arena_create_out {
    struct arena arena;
    bool success;
};
[[nodiscard]]
struct arena_create_out arena_create(usize size) {
    struct arena_create_out out = {0};
    out.arena.data = malloc(size);
    if (out.arena.data == NULL)
        return out;
    out.arena.size = size;
    out.arena.used = 0;
    out.success = true;
    return out;
}

#define ARENA_ALIGN 8

struct arena_alloc_out {
    void *ptr;
    bool success;
};
[[nodiscard]]
struct arena_alloc_out arena_alloc(struct arena *arena, usize size) {
    struct arena_alloc_out out = {0};
    usize alignment = (ARENA_ALIGN - (arena->used % ARENA_ALIGN)) % ARENA_ALIGN;
    out.ptr = arena->data + arena->used + alignment;
    if (out.ptr + size > arena->data + arena->size)
        return out;
    arena->used += size + alignment;
    out.success = true;
    return out;
}

struct arena_realloc_out {
    void *ptr;
    bool success;
};
[[nodiscard]]
struct arena_realloc_out arena_realloc(struct arena *arena, usize new_size, void *src, usize old_size) {
    struct arena_realloc_out out = {0};
    struct arena_alloc_out alloc_out = arena_alloc(arena, new_size);
    if (!alloc_out.success)
        return out;

    out.ptr = memcpy(alloc_out.ptr, src, MIN(old_size, new_size));
    out.success = true;
    return out;
}

void arena_clear(struct arena *arena) { arena->used = 0; }

void arena_free(struct arena *arena) { free(arena->data); }

#endif // ARENA_C

#include <stdio.h>

int main(void) {
    printf("hello\n");

    struct arena_create_out arena_out = arena_create(1024);
    if (!arena_out.success) {
        printf("failed to create the arena\n");
        return 1;
    }

    struct arena arena = arena_out.arena;

    while(true) {
        usize alloc_size = 100;
        struct arena_alloc_out alloc_res = arena_alloc(&arena, alloc_size);
        if(alloc_res.success) {
            printf("successfully allocated 0x%x bytes, address: 0x%x\n", alloc_size, alloc_res.ptr);
        } else {
            printf("failed to allocated 0x%x bytes\n", alloc_size);
            break;
        }
    }

    arena_free(&arena);
    return 0;
}
