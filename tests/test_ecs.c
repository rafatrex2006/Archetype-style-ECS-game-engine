#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "data_structures/World.h"
#include <stdalign.h>
#include "data_structures/archetypes.h"
#include "components/transform.h"
#include "components/velocity.h"
#include "components/health.h"
#include "components/acceleration.h"
#include "metadata/metadata.h"
#include "systems/movement.h"

static int failures = 0;
#define CHECK(cond) do { if(!(cond)){ printf("  FAIL line %d: %s\n", __LINE__, #cond); failures++; } } while(0)

/* column of a component inside an entity's archetype, same popcount rule the ECS uses */
#define COMP(ent, comp, type) ((type*)(ent)->arch->data[__builtin_popcountll((ent)->arch->sig & ((1ULL << (comp)) - 1))].data + (ent)->index)

/* Health gets a destructor here so we can count how many times it runs. */
static int destroyed = 0;
static void countDestroy(void *p){ (void)p; destroyed++; }

static int rowsInQuery(archetype_array *q){
    int total = 0;
    for(int i = 0; i < q->n; i++) total += q->arcs[i]->entityCount;
    return total;
}

/* ---------------- ECS core ---------------- */

static void test_add_remove(void){
    printf("add/remove component\n");
    metadata *m = createMetadata(64);
    World *W = createWorld(20, 4, m, NULL);
    Transform t = {1.0f, 2.0f};

    entity *e = CreateEntity(W);
    add_component(W, e, TRANSFORM, &t);
    add_component(W, e, VELOCITY, NULL);
    CHECK(e->arch->sig == ((1ULL << TRANSFORM) | (1ULL << VELOCITY)));

    Transform *got = (Transform*)e->arch->data[0].data + e->index;
    CHECK(got->x == 1.0f && got->y == 2.0f);   /* data survived the move */

    add_component(W, e, TRANSFORM, NULL);        /* duplicate add must be a no-op */
    CHECK(e->arch->entityCount == 1);

    remove_component(W, e, VELOCITY);
    CHECK(e->arch->sig == (1ULL << TRANSFORM));
    got = (Transform*)e->arch->data[0].data + e->index;
    CHECK(got->x == 1.0f && got->y == 2.0f);

    remove_component(W, e, HEALTH);              /* removing a missing component must be a no-op */
    CHECK(e->arch->sig == (1ULL << TRANSFORM));

    entity *u = CreateEntity(W);
    add_component(W, u, HEALTH, NULL);
    Health *h = (Health*)u->arch->data[0].data + u->index;
    CHECK(h->hp == 100);

    freeWorld(W); freeMetadata(m);
}

static void test_swap_remove_keeps_indices(void){
    printf("swap-remove keeps entity indices consistent\n");
    metadata *m = createMetadata(64);
    World *W = createWorld(20, 4, m, NULL);
    enum { N = 50 };
    entity *ents[N];
    for(int i = 0; i < N; i++){
        ents[i] = CreateEntity(W);
        Transform t = {(float)i, (float)i * 2};
        add_component(W, ents[i], TRANSFORM, &t);
    }
    /* destroy every third entity, then verify the survivors still own their own data */
    for(int i = 0; i < N; i += 3){ DestroyEnt(W, ents[i]); ents[i] = NULL; }
    for(int i = 0; i < N; i++){
        if(!ents[i]) continue;
        Transform *t = (Transform*)ents[i]->arch->data[0].data + ents[i]->index;
        CHECK(t->x == (float)i && t->y == (float)i * 2);
        CHECK(ents[i]->arch->entities[ents[i]->index] == ents[i]);
    }
    freeWorld(W); freeMetadata(m);
}

static void test_destructors(void){
    printf("destructors run exactly once per destroyed component\n");
    metadata *m = createMetadata(64);
    assign_new_component(m, sizeof(Health), HEALTH, alignof(Health), createHealth, countDestroy, NULL);
    World *W = createWorld(20, 4, m, NULL);
    destroyed = 0;

    entity *a = CreateEntity(W), *b = CreateEntity(W), *c = CreateEntity(W);
    add_component(W, a, HEALTH, NULL);
    add_component(W, b, HEALTH, NULL);
    add_component(W, c, HEALTH, NULL);

    DestroyEnt(W, a);                 /* a is NOT the last row: the old code destroyed the wrong row */
    CHECK(destroyed == 1);
    remove_component(W, b, HEALTH);
    CHECK(destroyed == 2);

    freeWorld(W);                     /* c still alive: world teardown destroys it */
    CHECK(destroyed == 3);
    freeMetadata(m);
}

static void test_growth_and_queries(void){
    printf("archetype growth (5000 entities) and cached queries\n");
    metadata *m = createMetadata(64);
    World *W = createWorld(20, 4, m, NULL);   /* tiny initial capacity forces many reallocations */

    Component in[]  = { TRANSFORM, VELOCITY };
    Component out[] = { HEALTH };
    archetype_array *moving   = createCustomQueue(W, in, 2, NULL, 0);
    archetype_array *movingNH = createCustomQueue(W, in, 2, out, 1);   /* created BEFORE entities exist */

    enum { N = 5000 };
    entity **ents = malloc(N * sizeof(entity*));
    for(int i = 0; i < N; i++){
        ents[i] = CreateEntity(W);        /* 5000 > capacity of archetype 0 */
        add_component(W, ents[i], TRANSFORM, NULL);
        add_component(W, ents[i], VELOCITY, NULL);
        if(i % 2) add_component(W, ents[i], HEALTH, NULL);
    }
    CHECK(rowsInQuery(moving)   == N);        /* archetypes created later must join existing queries */
    CHECK(rowsInQuery(movingNH) == N / 2);    /* exclusion filter works */

    createQuery(W, 1ULL << TRANSFORM);
    CHECK(createQuery(W, 1ULL << TRANSFORM) == createQuery(W, 1ULL << TRANSFORM));  /* cache hit */

    free(ents);
    freeWorld(W); freeMetadata(m);
}

/* ---------------- systems ---------------- */

/* Probe systems record when they are initialized, run and destroyed.
   Each probe gets an id from its init call, so ids follow init order. */
typedef struct Probe { int id; } Probe;

static int inits, next_id;
static int run_log[256], run_n;
static int destroy_log[64], destroy_n;
static float last_dt;

static void reset_probes(void){
    inits = next_id = run_n = destroy_n = 0;
    last_dt = -1.0f;
}

static bool probe_init(World *W, void **state){
    (void)W;
    Probe *p = malloc(sizeof(Probe));
    p->id = next_id++;
    inits++;
    *state = p;
    return true;
}

static void probe_update(World *W, float dt, void *state){
    (void)W;
    last_dt = dt;
    if(run_n < 256) run_log[run_n++] = ((Probe*)state)->id;
}

static void probe_destroy(World *W, void *state){
    (void)W;
    if(destroy_n < 64) destroy_log[destroy_n++] = ((Probe*)state)->id;
    free(state);
}

static int add_probe(World *W, SystemPhase phase){
    return register_system_world(W, &(SystemDesc){ .name = "probe", .phase = phase, .init = probe_init, .update = probe_update, .destroy = probe_destroy });
}

static void noop_update(World *W, float dt, void *state){ (void)W; (void)dt; (void)state; }

static void test_register_validation(void){
    printf("register_system_world rejects bad systems and returns ids\n");
    metadata *m = createMetadata(64);
    World *W = createWorld(20, 4, m, NULL);
    reset_probes();

    CHECK(register_system_world(W, NULL) == -1);
    CHECK(register_system_world(W, &(SystemDesc){ .name = "no update", .phase = PHASE_UPDATE }) == -1);
    CHECK(register_system_world(W, &(SystemDesc){ .name = "bad phase", .phase = PHASE_COUNT, .update = noop_update }) == -1);
    CHECK(W->systems->n == 0);                   /* rejected systems are not stored */

    CHECK(register_system_world(W, &(SystemDesc){ .name = "a", .phase = PHASE_UPDATE, .update = noop_update }) == 0);
    CHECK(register_system_world(W, &(SystemDesc){ .name = "b", .phase = PHASE_RENDER, .update = noop_update }) == 1);

    world_init_systems(W);
    UpdateWorld(W, 0.016f);                      /* stateless systems (no init) run with NULL state */

    freeWorld(W); freeMetadata(m);
}

static void test_init_timing(void){
    printf("init runs at world_init_systems, once, and right away after that\n");
    metadata *m = createMetadata(64);
    World *W = createWorld(20, 4, m, NULL);
    reset_probes();

    add_probe(W, PHASE_UPDATE);
    add_probe(W, PHASE_UPDATE);
    CHECK(inits == 0);                           /* registering only records the system */

    world_init_systems(W);
    CHECK(inits == 2);
    world_init_systems(W);                       /* calling it again must not re-init */
    CHECK(inits == 2);

    add_probe(W, PHASE_UPDATE);
    CHECK(inits == 3);                           /* late registration inits immediately */

    UpdateWorld(W, 0.25f);
    CHECK(run_n == 3);                           /* every system got its own state */
    CHECK(run_log[0] == 0 && run_log[1] == 1 && run_log[2] == 2);
    CHECK(last_dt == 0.25f);                     /* dt reaches the update unchanged */

    freeWorld(W); freeMetadata(m);
}

static void test_phase_order(void){
    printf("phases run in order, registration order inside a phase\n");
    metadata *m = createMetadata(64);
    World *W = createWorld(20, 4, m, NULL);
    reset_probes();

    /* registered out of phase order on purpose, ids are 0..4 in this order */
    add_probe(W, PHASE_RENDER);      /* 0 */
    add_probe(W, PHASE_PRE_UPDATE);  /* 1 */
    add_probe(W, PHASE_UPDATE);      /* 2 */
    add_probe(W, PHASE_PRE_UPDATE);  /* 3 */
    add_probe(W, PHASE_POST_UPDATE); /* 4 */
    world_init_systems(W);

    UpdateWorld(W, 0.0f);
    int expected[] = { 1, 3, 2, 4, 0 };
    CHECK(run_n == 5);
    CHECK(memcmp(run_log, expected, sizeof(expected)) == 0);

    UpdateWorld(W, 0.0f);                        /* same order every frame */
    CHECK(run_n == 10);
    CHECK(memcmp(run_log + 5, expected, sizeof(expected)) == 0);

    freeWorld(W); freeMetadata(m);
}

static void test_enable_disable(void){
    printf("disabled systems are skipped\n");
    metadata *m = createMetadata(64);
    World *W = createWorld(20, 4, m, NULL);
    reset_probes();

    int a = add_probe(W, PHASE_UPDATE);
    add_probe(W, PHASE_UPDATE);
    world_init_systems(W);

    set_system_enabled(W->systems, a, false);
    UpdateWorld(W, 0.0f);
    CHECK(run_n == 1 && run_log[0] == 1);

    set_system_enabled(W->systems, a, true);
    UpdateWorld(W, 0.0f);
    CHECK(run_n == 3 && run_log[1] == 0 && run_log[2] == 1);

    set_system_enabled(W->systems, -1, false);   /* bad ids are ignored */
    set_system_enabled(W->systems, 99, false);
    UpdateWorld(W, 0.0f);
    CHECK(run_n == 5);

    freeWorld(W); freeMetadata(m);
}

/* destroy reads the World to prove freeWorld tears systems down before the ECS data,
   ASan reports a use-after-free if that order ever flips */
static int queries_seen_in_destroy;
static bool query_init(World *W, void **state){
    (void)state;
    createCustomQueue(W, (Component[]){ TRANSFORM }, 1, NULL, 0);
    return true;
}
static void query_destroy(World *W, void *state){
    (void)state;
    queries_seen_in_destroy = W->QueryArr->n;
}

static bool plain_init(World *W, void **state){ (void)W; *state = malloc(32); return true; }   /* no destroy: registry must free() it */

static void test_destroy(void){
    printf("freeWorld destroys systems in reverse order, before the ECS data\n");
    metadata *m = createMetadata(64);
    World *W = createWorld(20, 4, m, NULL);
    reset_probes();
    queries_seen_in_destroy = -1;

    add_probe(W, PHASE_UPDATE);
    add_probe(W, PHASE_RENDER);
    add_probe(W, PHASE_PRE_UPDATE);
    register_system_world(W, &(SystemDesc){ .name = "plain", .phase = PHASE_UPDATE, .init = plain_init, .update = noop_update });
    register_system_world(W, &(SystemDesc){ .name = "query", .phase = PHASE_UPDATE, .init = query_init, .update = noop_update, .destroy = query_destroy });
    world_init_systems(W);

    freeWorld(W);
    int expected[] = { 2, 1, 0 };                /* reverse registration order, not phase order */
    CHECK(destroy_n == 3);
    CHECK(memcmp(destroy_log, expected, sizeof(expected)) == 0);
    CHECK(queries_seen_in_destroy >= 1);         /* the World was still alive */
    freeMetadata(m);                             /* plain_init's block is checked by LeakSanitizer */
}

/* fails to start: the registry must never run or destroy it */
static int broken_updates, broken_destroys;
static bool broken_init(World *W, void **state){ (void)W; (void)state; return false; }
static void broken_update(World *W, float dt, void *state){ (void)W; (void)dt; (void)state; broken_updates++; }
static void broken_destroy(World *W, void *state){ (void)W; (void)state; broken_destroys++; }

static int add_broken(World *W){
    return register_system_world(W, &(SystemDesc){ .name = "broken", .phase = PHASE_UPDATE, .init = broken_init, .update = broken_update, .destroy = broken_destroy });
}

static void test_init_failure(void){
    printf("a failed init disables only that system and is reported\n");
    metadata *m = createMetadata(64);
    World *W = createWorld(20, 4, m, NULL);
    reset_probes();
    broken_updates = broken_destroys = 0;

    add_probe(W, PHASE_UPDATE);                  /* id 0 */
    int broken = add_broken(W);                  /* id 1, stored, fails at init */
    add_probe(W, PHASE_UPDATE);                  /* id 2 */
    CHECK(broken == 1);

    CHECK(world_init_systems(W) == false);       /* failure is reported */
    CHECK(inits == 2);                           /* the others still started */

    set_system_enabled(W->systems, broken, true);   /* a failed system cannot be switched back on */
    UpdateWorld(W, 0.0f);
    CHECK(broken_updates == 0);
    CHECK(run_n == 2);

    CHECK(add_broken(W) == -1);                  /* late registration reports the failure through the id */
    UpdateWorld(W, 0.0f);
    CHECK(broken_updates == 0);

    freeWorld(W);
    CHECK(broken_destroys == 0);                 /* init cleans up after itself, destroy is never called */
    CHECK(destroy_n == 2);
    freeMetadata(m);

    /* all systems fine: true */
    W = createWorld(20, 4, m = createMetadata(64), NULL);
    add_probe(W, PHASE_UPDATE);
    CHECK(world_init_systems(W) == true);
    freeWorld(W); freeMetadata(m);
}

static void test_registry_growth(void){
    printf("registry grows past its initial size\n");
    metadata *m = createMetadata(64);
    World *W = createWorld(2, 4, m, NULL);       /* registry starts with room for 2 systems */
    reset_probes();

    enum { N = 40 };
    for(int i = 0; i < N; i++){
        CHECK(add_probe(W, PHASE_UPDATE) == i);
    }
    world_init_systems(W);
    UpdateWorld(W, 0.0f);
    CHECK(inits == N && run_n == N);
    for(int i = 0; i < N; i++) CHECK(run_log[i] == i);

    freeWorld(W);
    CHECK(destroy_n == N);
    freeMetadata(m);
}

static void test_movement_systems(void){
    printf("accelerate + move systems update components through UpdateWorld\n");
    metadata *m = createMetadata(64);
    World *W = createWorld(20, 4, m, NULL);

    /* registered move first: accelerate still runs first because PRE_UPDATE comes before UPDATE */
    register_system_world(W, &MOVE_SYSTEM);
    register_system_world(W, &ACCELERATE_SYSTEM);

    entity *falling = CreateEntity(W);           /* Transform + Velocity + Acceleration */
    add_component(W, falling, TRANSFORM, NULL);
    add_component(W, falling, VELOCITY, NULL);
    add_component(W, falling, ACCELERATION, &(Acceleration){ 2.0f, 0.0f });

    entity *drifting = CreateEntity(W);          /* Transform + Velocity, constant speed */
    add_component(W, drifting, TRANSFORM, &(Transform){ 10.0f, 10.0f });
    add_component(W, drifting, VELOCITY, &(Velocity){ 3.0f, -4.0f });

    entity *still = CreateEntity(W);             /* no Velocity, must not move */
    add_component(W, still, TRANSFORM, &(Transform){ 5.0f, 5.0f });

    world_init_systems(W);

    /* archetype created after the queries: must still be picked up */
    entity *late = CreateEntity(W);
    add_component(W, late, TRANSFORM, NULL);
    add_component(W, late, VELOCITY, &(Velocity){ 1.0f, 1.0f });
    add_component(W, late, HEALTH, NULL);

    /* dt = 0.5 is exact in binary, so == comparisons are safe
       frame 1: v = 0 + 2*0.5 = 1, x = 0 + 1*0.5 = 0.5
       frame 2: v = 1 + 2*0.5 = 2, x = 0.5 + 2*0.5 = 1.5 */
    UpdateWorld(W, 0.5f);
    UpdateWorld(W, 0.5f);

    Velocity *fv = COMP(falling, VELOCITY, Velocity);
    Transform *ft = COMP(falling, TRANSFORM, Transform);
    CHECK(fv->x == 2.0f && fv->y == 0.0f);
    CHECK(ft->x == 1.5f && ft->y == 0.0f);

    Transform *dt = COMP(drifting, TRANSFORM, Transform);
    CHECK(dt->x == 13.0f && dt->y == 6.0f);      /* 10 + 3*1s, 10 - 4*1s */

    Transform *st = COMP(still, TRANSFORM, Transform);
    CHECK(st->x == 5.0f && st->y == 5.0f);

    Transform *lt = COMP(late, TRANSFORM, Transform);
    CHECK(lt->x == 1.0f && lt->y == 1.0f);

    freeWorld(W); freeMetadata(m);
}

int main(void){
    test_add_remove();
    test_swap_remove_keeps_indices();
    test_destructors();
    test_growth_and_queries();

    test_register_validation();
    test_init_timing();
    test_phase_order();
    test_enable_disable();
    test_destroy();
    test_init_failure();
    test_registry_growth();
    test_movement_systems();

    if(failures){ printf("\n%d check(s) FAILED\n", failures); return 1; }
    printf("\nall tests passed\n");
    return 0;
}
