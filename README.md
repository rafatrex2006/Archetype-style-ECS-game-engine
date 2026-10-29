# ECS-game-engine

An archetype-based Entity Component System (ECS) in C, built from scratch as a learning project and meant to become the core of a small game engine. SDL3 handles the window and rendering.

> **Status: early development.** The ECS core, the system scheduler and a basic renderer (shapes and PNG images) work and are tested.

## Features

- **Archetype storage:** entities with the same components share an archetype, and each component type is stored in its own contiguous array, so iterating it is cache-friendly.
- **Runtime add/remove:** adding or removing a component moves the entity to the matching archetype.
- **Cached queries:** "has these components, not those" queries stay up to date as new archetypes appear.
- **Systems:** functions that run every frame, in phases, owned by the `World`.
- **Renderer:** draws entities that have a `Transform` plus a `Shape` or a `Surface` (image).
- **No global state:** everything lives in a `World`.

## Build and run

You need a C compiler, `pkg-config` and SDL 3.4 or newer.

```sh
make test   # runs the tests with AddressSanitizer and UBSan
./run.sh    # builds and runs the app; run it from the project root so fade.png is found
```

## Quick start

```c
metadata *m = createMetadata(64);
World *W = createWorld(20, 100, m, window);   // window can be NULL if nothing is drawn

// systems
register_system_world(W, &ACCELERATE_SYSTEM);
register_system_world(W, &MOVE_SYSTEM);

// entities
entity *box = CreateEntity(W);
add_component(W, box, TRANSFORM, &(Transform){50, 50});          // copy initial data in
add_component(W, box, VELOCITY, &(Velocity){20, 0});
add_component(W, box, SHAPE, &(Shape){
    .type = SHAPE_RECT, .color = {1, 0, 0, 1}, .filled = true, .rect = {200, 100},
});
add_component(W, box, HEALTH, NULL);                             // NULL uses the default values

if (!world_init_systems(W)) { /* a system failed to start */ }

while (running) {
    UpdateWorld(W, dt);   // dt = seconds since the last frame
}

freeWorld(W);
freeMetadata(m);
```

## Systems

A system is described by a `SystemDesc`:

| Field | What it does |
|---|---|
| `name` | Shown in error messages. |
| `phase` | `PHASE_PRE_UPDATE`, `PHASE_UPDATE`, `PHASE_POST_UPDATE` or `PHASE_RENDER`. Phases run in that order, and systems in the same phase run in registration order. |
| `init` | Optional. `bool init(World *W, void **state)` sets up the system's state (usually its queries) and returns `false` if the system can't start. |
| `update` | `void update(World *W, float dt, void *state)` runs every frame. |
| `destroy` | Optional. Frees the state. If left out, `free(state)` is called. |

Things to know:

- `init` runs in `world_init_systems`, not when the system is registered. A system registered later is initialized right away.
- A system whose `init` fails is switched off: it never runs and is never destroyed, so `init` must clean up after itself.
- `freeWorld` destroys systems first, in reverse order, so `destroy` can still use the World.
- Queries belong to the World. Don't use a query by itself as a system's state, because the default `free(state)` would free it twice. Wrap it in a struct, as `systems/movement.c` does.

## Rendering

The renderer is a system in the `RENDER` phase. Each frame it clears the screen, draws shapes, draws images on top, and presents.

- **`Shape`:** a rectangle (filled or outlined), a line (`line.end` is an offset from the Transform) or a point, in a `color`.
- **`Surface`:** an image. Load it once (`SDL_LoadPNG`) and share it between entities. The renderer turns it into a texture the first time it draws it. The image is yours to free, after `freeWorld`.

```c
SDL_Surface *img = SDL_LoadPNG("fade.png");
add_component(W, sprite, SURFACE, &(Surface){ .surface = img });
// ...
freeWorld(W);
SDL_DestroySurface(img);
```

## Adding a component

1. Write the struct and a `createX(void *dst)` function that sets default values, in `components/`.
2. Add its id to the `Component` enum in `metadata/metadata.h` (before `COMPONENT_COUNT`) and include its header there.
3. Register it in `createMetadata` in `metadata/metadata.c`.

## Project layout

```
data_structures/   World, archetypes, hash table, queries
metadata/          Component enum and component registration
System_Handler/    system registry and phase scheduler
systems/           example systems: accelerate and move
components/        Transform, Velocity, Acceleration, Health, Shape, Surface
renderer/          render system
tests/             test_ecs.c: ECS core and scheduler tests
main.c             window, World and main loop
```

## Known limitations

- At most 64 component types.
- Entity handles are not checked: using a destroyed entity is not detected.
- Adding or removing components while iterating a query is unsafe.
- Each `Surface` gets its own texture, freed only when the renderer shuts down.
- Images are drawn at their original size, with no scaling, rotation or layers.
- Not thread-safe.

## Roadmap

- [x] Phase-based system scheduler owned by the `World`
- [x] Basic renderer: shapes and PNG images
- [x] Real frame time (`dt`) in the main loop
- [x] Scheduler tests
- [ ] Shared texture cache
- [ ] Renderable component (layer, size, flags) and a `Hidden` tag
- [ ] Modules: one function that registers a feature's components and systems
- [ ] Runtime component registration
- [ ] Command buffer for safe add/remove during iteration
- [ ] Entity generations to detect stale handles
- [ ] Event bus
- [ ] Multithreaded systems
- [ ] Demo and benchmark

## Notes

<!-- Add design notes, things learned, and benchmark results here as the project grows. -->
