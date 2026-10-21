// The save game's part versions (code\version_registry.cpp).
#ifndef VERSION_REGISTRY_H
#define VERSION_REGISTRY_H

// Each part of a save game has a version, registered at startup (code\version_registry.cpp).
class VersionRegistry {
public:
    static VersionRegistry* instance();                     // 0x0042654A
    static void add(const char* name, int version);         // 0x004265D2
    int version(const char* name);                          // 0x00426599: -1 if not registered
};

inline int saved_version(const char* name)
{
    return VersionRegistry::instance()->version(name);
}

struct VersionRegistrar {
    VersionRegistrar(const char* name, int version) { VersionRegistry::add(name, version); }
};

#endif
