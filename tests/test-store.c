#include "ldinternal.h"
#include "ldstore.h"

static struct LDStore *
prepareEmptyStore()
{
    struct LDStore *store = NULL; struct LDJSON *sets = NULL, *tmp = NULL;

    LD_ASSERT(store = makeInMemoryStore());
    LD_ASSERT(!LDStoreInitialized(store));

    LD_ASSERT(sets = LDNewObject());

    LD_ASSERT(tmp = LDNewObject());
    LD_ASSERT(LDObjectSetKey(sets, "segments", tmp));

    LD_ASSERT(tmp = LDNewObject());
    LD_ASSERT(LDObjectSetKey(sets, "flags", tmp));

    LD_ASSERT(LDStoreInit(store, sets));
    LD_ASSERT(LDStoreInitialized(store));

    return store;
}

static void
allocateAndFree()
{
    struct LDStore *const store = prepareEmptyStore();

    LDStoreDestroy(store);
}

static struct LDJSON *
makeVersioned(const char *const key, const unsigned int version)
{
    struct LDJSON *feature = NULL, *tmp = NULL;

    LD_ASSERT(feature = LDNewObject());

    LD_ASSERT(tmp = LDNewText(key));
    LD_ASSERT(LDObjectSetKey(feature, "key", tmp));

    LD_ASSERT(tmp = LDNewNumber(version));
    LD_ASSERT(LDObjectSetKey(feature, "version", tmp));

    LD_ASSERT(tmp = LDNewBool(false));
    LD_ASSERT(LDObjectSetKey(feature, "deleted", tmp));

    return feature;
}

static struct LDJSON *
makeDeleted(const char *const key, const unsigned int version)
{
    struct LDJSON *feature = NULL, *tmp = NULL;

    LD_ASSERT(feature = makeVersioned(key, version));

    LD_ASSERT(tmp = LDNewBool(true));
    LD_ASSERT(LDObjectSetKey(feature, "deleted", tmp));

    return feature;
}

static void
deletedOnly()
{
    struct LDStore *store = NULL; struct LDJSON *feature = NULL, *lookup = NULL;

    LD_ASSERT(store = prepareEmptyStore());

    LD_ASSERT(feature = makeDeleted("abc", 123));

    LD_ASSERT(LDStoreUpsert(store, "flags", feature));

    LD_ASSERT(!(lookup = LDStoreGet(store, "abc", "flags")));

    LDStoreDestroy(store);
}

static void
basicExists()
{
    struct LDStore *store = NULL; struct LDJSON *feature = NULL, *lookup = NULL;

    LD_ASSERT(store = prepareEmptyStore());

    LD_ASSERT(feature = makeVersioned("my-heap-key", 3))

    LD_ASSERT(LDStoreUpsert(store, "flags", feature));

    LD_ASSERT((lookup = LDStoreGet(store, "my-heap-key", "flags")));

    /* LD_ASSERT(lookup->data == segment); NEED compare */

    LDJSONFree(lookup);

    LDStoreDestroy(store);
}

static void
basicDoesNotExist()
{
    struct LDStore *store = NULL; struct LDJSON *lookup = NULL;

    LD_ASSERT(store = prepareEmptyStore());

    LD_ASSERT(!(lookup = LDStoreGet(store, "abc", "flags")));

    LDStoreDestroy(store);
}

static void
upsertNewer()
{
    struct LDStore *store = NULL; struct LDJSON *feature = NULL, *lookup = NULL;

    LD_ASSERT(store = prepareEmptyStore());

    LD_ASSERT(feature = makeVersioned("my-heap-key", 3))
    LD_ASSERT(LDStoreUpsert(store, "segments", feature));

    LD_ASSERT(feature = makeVersioned("my-heap-key", 5))
    LD_ASSERT(LDStoreUpsert(store, "segments", feature));

    LD_ASSERT((lookup = LDStoreGet(store, "my-heap-key", "segments")));

    /* LD_ASSERT(lookup->data == segment); requires deep compare */

    LDJSONFree(lookup);

    LDStoreDestroy(store);
}

static void
upsertOlder()
{
    struct LDStore *store = NULL; struct LDJSON *feature1 = NULL, *feature2 = NULL, *lookup = NULL;

    LD_ASSERT(store = prepareEmptyStore());

    LD_ASSERT(feature1 = makeVersioned("my-heap-key", 5))
    LD_ASSERT(LDStoreUpsert(store, "segments", feature1));

    LD_ASSERT(feature2 = makeVersioned("my-heap-key", 3))
    LD_ASSERT(LDStoreUpsert(store, "segments", feature2));

    LD_ASSERT((lookup = LDStoreGet(store, "my-heap-key", "segments")));

    /* LD_ASSERT(lookup->data == segment2); requires deep compare */

    LDJSONFree(lookup);

    LDStoreDestroy(store);
}

static void
upsertDelete()
{
    struct LDStore *store = NULL; struct LDJSON *feature = NULL, *lookup = NULL;

    LD_ASSERT(store = prepareEmptyStore());

    LD_ASSERT(feature = makeVersioned("my-heap-key", 3))
    LD_ASSERT(LDStoreUpsert(store, "segments", feature));

    LD_ASSERT(feature = makeDeleted("my-heap-key", 5))
    LD_ASSERT(LDStoreUpsert(store, "segments", feature));

    LD_ASSERT(!(lookup = LDStoreGet(store, "my-heap-key", "segments")));

    LDStoreDestroy(store);
}

static void
conflictDifferentNamespace()
{
    struct LDStore *store = NULL; struct LDJSON *feature1 = NULL, *feature2 = NULL, *lookup = NULL;

    LD_ASSERT(store = prepareEmptyStore());

    LD_ASSERT(feature1 = makeVersioned("my-heap-key", 3));
    LD_ASSERT(LDStoreUpsert(store, "segments", feature1));

    LD_ASSERT(feature2 = makeVersioned("my-heap-key", 3));
    LD_ASSERT(LDStoreUpsert(store, "flags", feature2));

    LD_ASSERT((lookup = LDStoreGet(store, "my-heap-key", "segments")));
    /* LD_ASSERT(lookup->data == feature1); requires deep compare */
    LDJSONFree(lookup);

    LD_ASSERT((lookup = LDStoreGet(store, "my-heap-key", "flags")));
    /* LD_ASSERT(lookup->data == feature2); requires deep compare */
    LDJSONFree(lookup);

    LDStoreDestroy(store);
}

int
main()
{
    LDConfigureGlobalLogger(LD_LOG_TRACE, LDBasicLogger);

    allocateAndFree();
    deletedOnly();
    basicExists();
    basicDoesNotExist();
    upsertNewer();
    upsertOlder();
    upsertDelete();
    conflictDifferentNamespace();

    return 0;
}
