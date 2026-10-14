# ECS-game-engine

An archetype-based Entity Component System (ECS) written in C, intended as the core of a small game engine. Built from scratch as a learning project, with SDL3 for windowing.

> **Status: early development.** The ECS core works and is tested. Systems, a scheduler, and the renderer are not built yet (see [Roadmap](#roadmap)).

## What it does today

- **Archetype storage.** Entities with the same set of component types share an archetype. Each archetype stores its components in contiguous per-component arrays (structure-of-arrays), so iterating one component type is cache-friendly.
- **Bitmask signatures.** An archetype is identified by a 64-bit signature, one bit per component type. Column lookup uses popcount on the bits below a component's bit.
- **Add/remove components at runtime.** Adding or removing a component moves the entity to the matching archetype and fills the hole it leaves with the last row (swap-remove), keeping every entity's stored index consistent.
- **Archetype hash table.** Archetypes are looked up by signature in an open-addressing hash table and created on demand.
- **Cached queries.** A query is a set of required components plus optional excluded components. Matching archetypes are cached, and new archetypes are added to existing queries when they are created.
- **Component metadata.** Each component type registers its size, alignment, and optional constructor, destructor, and move hooks. Plain structs need only a constructor.
- **No global state.** Everything hangs off a `World`.

## Project layout

```
data_structures/   world, archetypes, archetype hash table, archetype array, queries
metadata/          component registration (size, alignment, constructor/destructor/move)
components/        example components: Transform, Velocity, Health
renderer/          SDL3 window (work in progress)
tests/             test_ecs.c
main.c             SDL3 entry point
run.sh             builds and runs the SDL3 app
Makefile           builds and runs the tests
```

## Building

Run the tests (needs only a C compiler, builds with AddressSanitizer and UBSan):

```sh
make test
```

Build and run the SDL3 app (needs SDL3 and `pkg-config`):

```sh
./run.sh
```

## Usage sketch

```c
metadata *m = createMetadata(64);
World *W = createWorld(20, 100, m);

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

`tests/test_ecs.c` covers adding and removing components, swap-remove index consistency, destructor counts, growth to thousands of entities, and the query cache including exclusion filters. These test the ECS bookkeeping; they do not yet cover rendering.

## Known limitations

- At most 64 component types (one bit each in the signature).
- Entity IDs are not recycled and have no generation counter, so stale handles are not detected.
- Adding, removing, or destroying components while iterating a query is unsafe. A command buffer is planned.
- Not thread-safe.

## Roadmap

- [ ] Systems and a phase-based scheduler (startup / input / update / render)
- [ ] Modules: one function that registers a feature's components and systems
- [ ] Runtime component registration instead of a hard-coded list in `createMetadata`
- [ ] Command buffer for deferred structural changes
- [ ] Entity generations to detect stale handles
- [ ] Renderer driven by a `Renderable` component (layer, flags) and a `Hidden` tag
- [ ] Event bus
- [ ] Multithreaded system execution
- [ ] Demo and a benchmark (archetype iteration vs. array of pointers)

## Notes

<!-- Add design notes, things learned, and benchmark results here as the project grows. -->
