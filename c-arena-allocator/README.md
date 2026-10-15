# An example of C arena allocator

Based on https://eliasebner.com/blog/guides/arena-allocators-in-c/

```
gcc main.c && ./a.out
```

Checking for issues:

```
gcc main.c -std=c11 -pedantic
clang main.c -std=c11 -pedantic
```
