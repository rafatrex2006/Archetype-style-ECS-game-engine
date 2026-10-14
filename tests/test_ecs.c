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

static int failures = 0;
#define CHECK(cond) do { if(!(cond)){ printf("  FAIL line %d: %s\n", __LINE__, #cond); failures++; } } while(0)

/* Health gets a destructor here so we can count how many times it runs. */
static int destroyed = 0;
static void countDestroy(void *p){ (void)p; destroyed++; }

static int rowsInQuery(archetype_array *q){
    int total = 0;
    for(int i = 0; i < q->n; i++) total += q->arcs[i]->entityCount;
    return total;
}

static void test_add_remove(void){
    printf("add/remove component\n");
    metadata *m = createMetadata(64);
    World *W = createWorld(20, 4, m);
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
    World *W = createWorld(20, 4, m);
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
    World *W = createWorld(20, 4, m);
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
    World *W = createWorld(20, 4, m);   /* tiny initial capacity forces many reallocations */

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

int main(void){
    test_add_remove();
    test_swap_remove_keeps_indices();
    test_destructors();
    test_growth_and_queries();
    if(failures){ printf("\n%d check(s) FAILED\n", failures); return 1; }
    printf("\nall tests passed\n");
    return 0;
}
