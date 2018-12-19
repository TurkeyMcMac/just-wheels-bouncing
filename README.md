# Just Wheels Bouncing (JWB)
This is a simple physics engine for simulating frictionless circles colliding. I
hope to keep the user interface nice and simple. This is currently a
work-in-progress.

## Development
### Building the Library
Currently, only the shared library is buildable. Furthermore, as of now, it can
only be built on Linux. Here is how to build it:

```
make shared
```

### Building the Documentation
The documentation requires awk (not necessarily gawk) to build, and should be
buildable on any system satisfying this dependency. To build the documentation,
run:

```
make docs.md
```

## Implemented features
 * Perfectly elastic collisions
 * Toroidal universe
 * Entity resource management
 * Friction with the surface

## Features planned
 * Partially elastic collisions
 * Walled universe
 * Bindings to other languages
 * Friction between circles?
 * Move geometric data to separate shared structures?
