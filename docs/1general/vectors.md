## Vectors
Vectors are an essential part of every computer game. Because they are used in so many program modules they are
defined in the [shared.h](../../rogal/headers/shared.h) header file, which is available to almost all other source files.

Inspired by [Quake 2 source code](https://github.com/id-Software/Quake-2/blob/master/game/q_shared.h) the vector definition looks like this:

```c
typedef float	vec_t;
typedef vec_t	vec2_t[2];
```

Each vector is simply an array of two floats. For additional clarity when using vectors two macros are specified:

```c
#define VEC_X 0
#define VEC_Y 1
```

The [shared.h](../../rogal/headers/shared.h) header adds a bunch of macro tools for adding, substracting, copying and other operations on
vectors (again inspired by Quake 2 source).

#### Colours

```c
typedef vec_t color3_t[3];
```

A similar definition is made for colour storage. All colours are defined as arrays of three values (R, G, B) in range of 0 to 1.

- Previous article: [The main loop and GLUT callbacks](main_loop.md)
- Next article: [Textures](textures.md)