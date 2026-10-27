# ECS-game-engine

An archetype-based Entity Component System (ECS) written in C, intended as the core of a small game engine. Built from scratch as a learning project, with SDL3 for windowing and rendering.

> **Status: early development.** The ECS core works and is tested, systems run through a phase-based scheduler owned by the `World`, and a first renderer system draws shapes and PNG images (see [Roadmap](#roadmap)).

## What it does today

- **Archetype storage.** Entities with the same set of component types share an archetype. Each archetype stores its components in contiguous per-component arrays (structure-of-arrays), so iterating one component type is cache-friendly.
- **Bitmask signatures.** An archetype is identified by a 64-bit signature, one bit per component type. Column lookup uses popcount on the bits below a component's bit.
- **Add/remove components at runtime.** Adding or removing a component moves the entity to the matching archetype and fills the hole it leaves with the last row (swap-remove), keeping every entity's stored index consistent.
- **Archetype hash table.** Archetypes are looked up by signature in an open-addressing hash table and created on demand.
- **Cached queries.** A query is a set of required components plus optional excluded components. Matching archetypes are cached, and new archetypes are added to existing queries when they are created.
- **Component metadata.** Each component type registers its size, alignment, and optional constructor, destructor, and move hooks. Plain structs need only a constructor.
- **Systems and phases.** The `World` owns a system registry. Systems run once per frame, phase by phase.
- **Renderer.** A render system draws every entity that has a `Transform` plus a `Shape` (rectangles, lines, points) or a `Surface` (an image, such as a PNG).
- **No global state.** Everything hangs off a `World`.

## Systems

A system is a function that runs every frame and works on the world, usually by iterating a query. Systems are registered with a `SystemDesc`:

- `name`: used in error messages.
- `phase`: when the system runs (see below).
- `init` (optional): runs once and returns the system's own state, such as its queries or settings. Return `NULL` if the system needs none.
- `update`: runs every frame and receives the world, the frame's `dt` (seconds since the last frame), and the state from `init`.
- `destroy` (optional): frees the state. If it is left out, the registry calls `free(state)`.

Each frame, the phases run in order: `PRE_UPDATE`, `UPDATE`, `POST_UPDATE`, `RENDER`. Inside a phase, systems run in the order they were registered. A system can be switched off and on with `set_system_enabled`.

The registry lives in the `World`, so you go through the World's functions:

```c
void move_update(World *W, float dt, void *state) {
    // iterate a Transform + Velocity query and apply velocity * dt
}

int id = register_system_world(W, &(SystemDesc){
    .name = "move", .phase = PHASE_UPDATE, .update = move_update,
});                                // returns the system id, or -1 on bad input

world_init_systems(W);             // once, before the main loop: runs every init
UpdateWorld(W, dt);                // every frame: runs every phase
freeWorld(W);                      // runs destroy on every system first, in reverse order
```

Registering a system only records it. Its `init` runs in `world_init_systems`, so every system's `init` can rely on the World being fully set up and on earlier systems already being initialized. A system registered after `world_init_systems` has run is initialized right away.

## Rendering

The renderer in `renderer/` is a regular system in the `RENDER` phase. `main` creates the SDL window and passes it to `createWorld`. The renderer's `init` creates the `SDL_Renderer` for that window and two queries:

- **`Transform + Shape`.** A `Shape` is a rectangle (`rect.w`, `rect.h`, filled or outlined), a line (`line.end`, an offset from the Transform), or a point, drawn in its `color`.
- **`Transform + Surface`.** A `Surface` holds an `SDL_Surface` loaded from an image file. The first time the renderer draws it, the renderer turns it into a GPU texture and keeps that texture in the component, so later frames don't upload the image again. Images are drawn at their native size.

Each frame the renderer clears the screen, draws shapes, then draws surfaces on top, and presents.

```c
SDL_Surface *img = SDL_LoadPNG("fade.png");   // load once; entities can share it

entity *box = CreateEntity(W);
add_component(W, box, TRANSFORM, &(Transform){50, 50});
add_component(W, box, SHAPE, &(Shape){
    .type = SHAPE_RECT, .color = {1, 0, 0, 1}, .filled = true, .rect = {200, 100},
});

entity *sprite = CreateEntity(W);
add_component(W, sprite, TRANSFORM, &(Transform){100, 100});
add_component(W, sprite, SURFACE, &(Surface){ .surface = img });

// ... main loop ...

freeWorld(W);              // destroys the renderer, and with it every texture
SDL_DestroySurface(img);   // the surface belongs to you; free it after the World
```

## Adding a component

1. Write the struct and a constructor (`void createX(void *dst)`) in `components/`.
2. Add an id to the `Component` enum in `metadata/metadata.h`, before `COMPONENT_COUNT`, and include the component's header there.
3. Register it in `createMetadata` (`metadata/metadata.c`) with `assign_new_component`.

## Project layout

```
data_structures/   world, archetypes, archetype hash table, archetype array, queries
metadata/          Component enum and component registration (size, alignment, hooks)
System_Handler/    system registry and phase scheduler
components/        Transform, Velocity, Acceleration, Health, Shape, Surface
renderer/          render system: SDL renderer, shape and surface drawing
tests/             test_ecs.c
main.c             SDL3 entry point: window, World, main loop
run.sh             builds and runs the SDL3 app
Makefile           builds and runs the tests
```

## Building

Run the tests (needs a C compiler and the SDL3 headers; builds with AddressSanitizer and UBSan):

```sh
make test
```

Build and run the SDL3 app (needs SDL 3.4 or newer for `SDL_LoadPNG`, and `pkg-config`). Run it from the project root so relative image paths like `fade.png` resolve:

```sh
./run.sh
```

## Usage sketch

```c
metadata *m = createMetadata(64);
World *W = createWorld(20, 100, m, window);   // window can be NULL when nothing renders, e.g. in tests

entity *e = CreateEntity(W);
Transform t = {10.0f, 20.0f};
add_component(W, e, TRANSFORM, &t);   // copy initial data in
add_component(W, e, HEALTH, NULL);    // NULL runs the component's constructor

remove_component(W, e, HEALTH);
DestroyEnt(W, e);

freeWorld(W);
freeMetadata(m);
```

Querying:

```c
Component in[]  = { TRANSFORM, VELOCITY };
Component out[] = { HEALTH };
archetype_array *q = createCustomQueue(W, in, 2, out, 1);   // has Transform + Velocity, no Health
// q->arcs[i] are the matching archetypes; iterate their component columns.
```

## Tests

`tests/test_ecs.c` covers adding and removing components, swap-remove index consistency, destructor counts, growth to thousands of entities, and the query cache including exclusion filters. These test the ECS bookkeeping; they do not yet cover the scheduler or rendering.

## Known limitations

- At most 64 component types (one bit each in the signature).
- Entity IDs are not recycled and have no generation counter, so stale handles are not detected.
- Adding, removing, or destroying components while iterating a query is unsafe. A command buffer is planned.
- The main loop passes `dt = 0` for now; a real frame time is not measured yet.
- Each `Surface` gets its own texture, even when entities share the same image. Textures are only freed when the renderer is destroyed, so removing a `Surface` component or destroying its entity keeps the texture alive until shutdown.
- Images are drawn at native size; there is no scaling, rotation, or draw order beyond "shapes, then surfaces".
- Not thread-safe.

## Roadmap

- [x] Systems and a phase-based scheduler (pre-update / update / post-update / render)
- [x] Systems owned by the `World` (`register_system_world`, `world_init_systems`, `UpdateWorld`)
- [x] Basic renderer: shapes and PNG images through `Shape` and `Surface` components
- [ ] Real `dt` from the frame clock in the main loop
- [ ] Shared texture cache so entities using the same image share one texture
- [ ] Renderable component (layer, size, flags) and a `Hidden` tag
- [ ] Tests for the scheduler
- [ ] Modules: one function that registers a feature's components and systems
- [ ] Runtime component registration instead of a hard-coded list in `createMetadata`
- [ ] Command buffer for deferred structural changes
- [ ] Entity generations to detect stale handles
- [ ] Event bus
- [ ] Multithreaded system execution
- [ ] Demo and a benchmark (archetype iteration vs. array of pointers)

## Notes

<!-- Add design notes, things learned, and benchmark results here as the project grows. -->
