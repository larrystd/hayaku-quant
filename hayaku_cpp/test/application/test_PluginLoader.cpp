/*
 * Copyright (c) 2026 hikyuu.org
 */

#include <string>

#include "application/plugins/PluginLoader.h"
#include "application/plugins/PluginManager.h"
#include "doctest/doctest.h"
#include "plugin_fixtures/VersionTwoPluginInterface.h"

using namespace hayaku;

/**
 * @defgroup test_plugin_loader_suite test_plugin_loader_suite
 * @ingroup test_hayaku_utilities
 * @{
 */

namespace {

class MissingPluginInterface : public PluginBase {};

void* openFixture(const std::string& filename) {
#if HAYAKU_OS_WINDOWS
  return reinterpret_cast<void*>(LoadLibraryA(filename.c_str()));
#else
  return dlopen(filename.c_str(), RTLD_NOW);
#endif
}

void closeFixture(void* handle) {
  if (!handle) {
    return;
  }
#if HAYAKU_OS_WINDOWS
  FreeLibrary(reinterpret_cast<HMODULE>(handle));
#else
  dlclose(handle);
#endif
}

int fixtureCount(void* handle, const char* symbol) {
#if HAYAKU_OS_WINDOWS
  auto function = reinterpret_cast<int (*)()>(
      GetProcAddress(reinterpret_cast<HMODULE>(handle), symbol));
#else
  auto function = reinterpret_cast<int (*)()>(dlsym(handle, symbol));
#endif
  return function ? function() : -1;
}

class FixtureHandle {
 public:
  explicit FixtureHandle(const std::string& filename)
      : m_handle(openFixture(filename)) {}
  ~FixtureHandle() { closeFixture(m_handle); }

  FixtureHandle(const FixtureHandle&) = delete;
  FixtureHandle& operator=(const FixtureHandle&) = delete;

  void* get() const { return m_handle; }

 private:
  void* m_handle{nullptr};
};

}  // namespace

TEST_CASE("test_PluginLoader_getFileName") {
  PluginLoader loader("/missing/hayaku-plugins");
#if HAYAKU_OS_WINDOWS
  /** @arg Windows plugins use the DLL filename convention. */
  CHECK_EQ(loader.getFileName("absent"), "/missing/hayaku-plugins/absent.dll");
#elif HAYAKU_OS_LINUX
  /** @arg Linux plugins use the lib*.so filename convention. */
  CHECK_EQ(loader.getFileName("absent"),
           "/missing/hayaku-plugins/libabsent.so");
#elif HAYAKU_OS_OSX
  /** @arg macOS plugins use the lib*.dylib filename convention. */
  CHECK_EQ(loader.getFileName("absent"),
           "/missing/hayaku-plugins/libabsent.dylib");
#endif
}

TEST_CASE("test_PluginLoader_load_missing") {
  PluginLoader loader("/missing/hayaku-plugins");

  /** @arg A missing plugin fails even when logging is disabled. */
  CHECK_UNARY(!loader.load("absent", false));
  CHECK_EQ(loader.instance<MissingPluginInterface>(), nullptr);

  /** @arg Retrying a failed load leaves no stale instance or handle. */
  CHECK_UNARY(!loader.load("absent", false));
  CHECK_EQ(loader.instance<MissingPluginInterface>(), nullptr);
}

TEST_CASE("test_PluginManager_getPlugin_missing") {
  PluginManager manager("/missing/hayaku-plugins");

  /** @arg A failed load is not cached as an instance. */
  CHECK_EQ(manager.getPlugin<MissingPluginInterface>("absent", false), nullptr);
  CHECK_EQ(manager.getPlugin<MissingPluginInterface>("absent", false), nullptr);
}

TEST_CASE("test_PluginLoader_load_v1_and_destroy") {
  PluginLoader loader(getCurrentDir());
  FixtureHandle fixture(loader.getFileName("hayaku_abi_valid"));
  REQUIRE_NE(fixture.get(), nullptr);
  int createCount = fixtureCount(fixture.get(), "hayakuTestCreateCount");
  int destroyCount = fixtureCount(fixture.get(), "hayakuTestDestroyCount");
  REQUIRE_GE(createCount, 0);
  REQUIRE_GE(destroyCount, 0);

  /** @arg ABI v1, matching ID, and destroy function permit loading. */
  REQUIRE_UNARY(loader.load("hayaku_abi_valid", false));
  REQUIRE_NE(loader.instance<PluginBase>(), nullptr);
  CHECK_UNARY(loader.supportsInterfaceVersion(1));
  CHECK_UNARY(!loader.supportsInterfaceVersion(2));
  CHECK_EQ(fixtureCount(fixture.get(), "hayakuTestCreateCount"),
           createCount + 1);
  CHECK_EQ(fixtureCount(fixture.get(), "hayakuTestDestroyCount"), destroyCount);

  /** @arg Reload destroys the prior instance before creating a new one. */
  REQUIRE_UNARY(loader.load("hayaku_abi_valid", false));
  CHECK_EQ(fixtureCount(fixture.get(), "hayakuTestCreateCount"),
           createCount + 2);
  CHECK_EQ(fixtureCount(fixture.get(), "hayakuTestDestroyCount"),
           destroyCount + 1);

  /** @arg A subsequent failed load unloads the current instance. */
  CHECK_UNARY(!loader.load("absent", false));
  CHECK_EQ(fixtureCount(fixture.get(), "hayakuTestDestroyCount"),
           destroyCount + 2);
  CHECK_EQ(loader.instance<PluginBase>(), nullptr);
  CHECK_UNARY(!loader.supportsInterfaceVersion(1));
}

TEST_CASE("test_PluginLoader_load_legacy") {
  PluginLoader loader(getCurrentDir());
  FixtureHandle fixture(loader.getFileName("hayaku_abi_legacy"));
  REQUIRE_NE(fixture.get(), nullptr);
  int createCount = fixtureCount(fixture.get(), "hayakuTestCreateCount");
  int destroyCount = fixtureCount(fixture.get(), "hayakuTestDestroyCount");
  REQUIRE_GE(createCount, 0);
  REQUIRE_GE(destroyCount, 0);

  /** @arg Existing createPlugin-only binaries still load and use the legacy
   * delete path. */
  REQUIRE_UNARY(loader.load("hayaku_abi_legacy", false));
  REQUIRE_NE(loader.instance<PluginBase>(), nullptr);
  CHECK_EQ(fixtureCount(fixture.get(), "hayakuTestCreateCount"),
           createCount + 1);
  CHECK_UNARY(!loader.load("absent", false));
  CHECK_EQ(fixtureCount(fixture.get(), "hayakuTestDestroyCount"),
           destroyCount + 1);

  /** @arg Legacy binaries cannot claim an unverified interface version. */
  CHECK_UNARY(!loader.load("hayaku_abi_legacy", false, 2));
  CHECK_EQ(fixtureCount(fixture.get(), "hayakuTestCreateCount"),
           createCount + 1);
}

TEST_CASE("test_PluginManager_cached_interface_version") {
  PluginManager manager(getCurrentDir());

  /** @arg The v1 metadata can load the fixture through the base interface. */
  auto* plugin = manager.getPlugin<PluginBase>("hayaku_abi_valid", false);
  REQUIRE_NE(plugin, nullptr);
  REQUIRE_NE(dynamic_cast<VersionTwoPluginInterface*>(plugin), nullptr);

  /** @arg A cached object must not satisfy a request for a different interface
   * version, even when its C++ dynamic type is convertible to that interface.
   */
  CHECK_EQ(
      manager.getPlugin<VersionTwoPluginInterface>("hayaku_abi_valid", false),
      nullptr);
}

TEST_CASE("test_PluginLoader_reject_incompatible_abi") {
  PluginLoader loader(getCurrentDir());
  for (const char* name :
       {"hayaku_abi_wrong_version", "hayaku_abi_wrong_id",
        "hayaku_abi_no_destroy", "hayaku_abi_wrong_interface"}) {
    FixtureHandle fixture(loader.getFileName(name));
    REQUIRE_NE(fixture.get(), nullptr);
    int createCount = fixtureCount(fixture.get(), "hayakuTestCreateCount");
    REQUIRE_GE(createCount, 0);

    /** @arg Invalid ABI, ID, interface version, or missing destroy is rejected
     * before construction. */
    CHECK_UNARY(!loader.load(name, false));
    CHECK_EQ(fixtureCount(fixture.get(), "hayakuTestCreateCount"), createCount);
    CHECK_EQ(loader.instance<PluginBase>(), nullptr);
  }
}

/** @} */
