#line 2 "E:\\Perforce\\Rome\\Rome,BI-1.5,1.6\\Dev\\barbi_daily\\code\\romans_file.cpp"
// Saving and loading the campaign: the save game's header, the table that gives each saved pointer its loaded object,
// and the markers that check a part of the save was read as it was written. (The original's path, which its
// __FILE__s give.)
static char THIS_FILE[] = __FILE__;

#include "unsorted.h"
#include "map_constants.h"

#include "ai/ai_cheat_manager.h"

static const ConstInt s_unknown_9(9);
static const float k_quarter_pi = PI * 0.25f;

extern "C" {
const char* fn_0040E497(const char* name);                              // a folder's path (a guess)
void fn_00F3B090(const unsigned short* path);                           // makes a folder (a guess)
void fn_00426624(SaveStream& stream, VersionRegistry* registry);         // (code\version_registry.cpp)
void fn_004267B1(SaveStream& stream, VersionRegistry* registry);         // (code\version_registry.cpp)
void fn_007E599C();
void fn_0072781F();
void fn_005EF510();
void fn_00714529();
void fn_0059655D();
void fn_005D3C90();
void fn_00987FC8();
void fn_0097915C();
void fn_0097C824();
void fn_00985408();
void fn_004E8F26();
extern int lbl_012CB0D0;            // 3 skips the checks of saved positions
extern bool lbl_014D6058;
extern int lbl_018CD7FC;
}

class Unk029219D4 {
public:
    void unk_004BE36D();            // 0x004BE36D
};

struct MapCorner {
    MapCorner(int x, int y) : x(x), y(y) {}

    int x;
    int y;
};

class Unk029219DC {
public:
    void unk_00C098B3(const MapCorner& a, const MapCorner& b);   // 0x00C098B3
};

class Unk014D6710 {
public:
    void unk_008C9797(int, int, int);   // 0x008C9797
};

extern "C" Unk029219D4* lbl_029219D4;
extern "C" Unk029219DC* lbl_029219DC;
extern "C" Unk014D6710 lbl_014D6710;


// A saved pointer: its type, the address it was saved with, and the object loaded in its place.
struct SavedPointer {
    SavedPointer() : m_type(0), m_key(0), m_object(0) {}

    int m_type;
    int m_key;
    void* m_object;
};

struct SavedPointerTable {
    SavedPointer m_items[0x10000];
};

// The saved pointers, by their addresses.
class SavedPointers {
public:
    SavedPointers() {}
    ~SavedPointers() {}
    unsigned short hash(unsigned int key) const;
    void* lookup(int type, int key);
    void* lookup(int type, int other_type, int key);
    void insert(int type, void* object, int key);
    void add(int type, void* object, int key) { insert(type, object, key); }
    void* find(int type, void* saved) { return lookup(type, (int)saved); }
    void* find(int type, int other_type, void* saved) { return lookup(type, other_type, (int)saved); }

    SavedPointerTable m_table;
};

static SavedPointers* s_saved_pointers;

inline SavedPointers* saved_pointers()
{
    return s_saved_pointers;
}

unsigned short SavedPointers::hash(unsigned int key) const
{
    return (unsigned short)((key >> 16) & 0xffff) ^ (unsigned short)key;
}

void* SavedPointers::lookup(int type, int key)
{
    if (!key)
        return 0;
    unsigned int index = hash(key);
    SavedPointer* entry = &m_table.m_items[index];
    while (entry->m_key && (entry->m_key != key || entry->m_type != type)) {
        if (index >= 0x10000)
            return 0;
        index = (index + 1) % 0x10000;
        entry = &m_table.m_items[index];
    }
    return entry->m_object;
}

void* SavedPointers::lookup(int type, int other_type, int key)
{
    if (!key)
        return 0;
    unsigned int index = hash(key);
    SavedPointer* entry = &m_table.m_items[index];
    while (entry->m_key && (entry->m_key != key || (entry->m_type != type && entry->m_type != other_type))) {
        if (index >= 0x10000)
            return 0;
        index = (index + 1) % 0x10000;
        entry = &m_table.m_items[index];
    }
    return entry->m_object;
}

void SavedPointers::insert(int type, void* object, int key)
{
    unsigned int index = hash(key);
    SavedPointer* entry = &m_table.m_items[index];
    while (entry->m_key) {
        index = (index + 1) % 0x10000;
        entry = &m_table.m_items[index];
    }
    entry->m_type = type;
    entry->m_object = object;
    entry->m_key = key;
}

extern "C" {
void fn_0041358D(void* object, int key) { saved_pointers()->add(0, object, key); }
void fn_004135AB(void* object, int key) { saved_pointers()->add(1, object, key); }
void fn_004135C9(void* object, int key) { saved_pointers()->add(2, object, key); }
void fn_004135E7(void* object, int key) { saved_pointers()->add(3, object, key); }
void fn_00413605(void* object, int key) { saved_pointers()->add(4, object, key); }
void fn_00413623(void* object, int key) { saved_pointers()->add(5, object, key); }
void fn_00413641(void* object, int key) { saved_pointers()->add(6, object, key); }
void fn_0041365F(void* object, int key) { saved_pointers()->add(7, object, key); }
void fn_0041367D(void* object, int key) { saved_pointers()->add(8, object, key); }
void fn_0041369B(void* object, int key) { saved_pointers()->add(9, object, key); }
void fn_004136B9(void* object, int key) { saved_pointers()->add(10, object, key); }
void fn_004136D7(void* object, int key) { saved_pointers()->add(11, object, key); }
void fn_004136F5(void* object, int key) { saved_pointers()->add(12, object, key); }
void fn_00413713(void* object, int key) { saved_pointers()->add(13, object, key); }
void fn_00413731(void* object, int key) { saved_pointers()->add(14, object, key); }

void* fn_0041374F(void* saved) { return saved_pointers()->find(0, saved); }
void* fn_0041376A(void* saved) { return saved_pointers()->find(2, saved); }
void* fn_00413785(void* saved) { return saved_pointers()->find(3, saved); }
void* fn_004137A0(void* saved) { return saved_pointers()->find(4, saved); }
void* fn_004137BB(void* saved) { return saved_pointers()->find(1, saved); }
void* fn_004137D6(void* saved) { return saved_pointers()->find(1, 2, saved); }
void* fn_004137F3(void* saved) { return saved_pointers()->find(5, saved); }
void* fn_0041380E(void* saved) { return saved_pointers()->find(6, saved); }
void* fn_00413829(void* saved) { return saved_pointers()->find(7, saved); }
void* fn_00413844(void* saved) { return saved_pointers()->find(7, saved); }
void* fn_0041385F(void* saved) { return saved_pointers()->find(8, saved); }
void* fn_0041387A(void* saved) { return saved_pointers()->find(9, saved); }
void* fn_00413895(void* saved) { return saved_pointers()->find(10, saved); }
void* fn_004138B0(void* saved) { return saved_pointers()->find(11, saved); }
void* fn_004138CB(void* saved) { return saved_pointers()->find(12, saved); }
void* fn_004138E6(void* saved) { return saved_pointers()->find(13, saved); }
void* fn_00413901(void* saved) { return saved_pointers()->find(14, saved); }

// Writes the position, for fn_0041393B to check when it is read.
void fn_0041391C(SaveStream* stream)
{
    int position = stream->tell();
    stream->write(&position, 4);
}

bool fn_0041393B(SaveStream* stream, int)
{
    int position = stream->offset();
    int saved = stream->read_int();
    if (lbl_012CB0D0 == 3)
        return true;
    if (position != saved)
        return false;
    return true;
}
}

// Marks a part of the save: written, it records the part's size at its end; read, it checks the part was read to
// the size written, and skips to its end if not.
class IntegrityMarker {
public:
    IntegrityMarker(SaveStream* stream, const char* name);
    IntegrityMarker(SaveStream& stream, const char* name);
    void end();

    SaveStream* m_in;
    SaveStream* m_out;
    const char* m_name;
    int m_start;
    int m_size;
    bool m_ok;
};

IntegrityMarker::IntegrityMarker(SaveStream* stream, const char* name)
{
    m_in = 0;
    m_out = stream;
    m_name = name;
    m_start = stream->tell();
    m_size = 0;
    m_ok = true;
    stream->write(&m_start, 4);
    stream->write(&m_size, 4);
}

IntegrityMarker::IntegrityMarker(SaveStream& stream, const char* name)
{
    m_in = &stream;
    m_out = 0;
    m_name = name;
    m_start = stream.offset();
    m_size = 0;
    m_ok = true;
    int start = stream.read_int();
    m_size = stream.read_int();
    if (start != m_start) {
        m_ok = false;
        if (m_name) {
            CA_CHECK_FAILED1("Integrity marker %s failed", m_name);
        } else {
            CA_CHECK_FAILED("Integrity marker failed");
        }
    }
}

void IntegrityMarker::end()
{
    if (m_out) {
        SaveStream* out = m_out;
        int position = out->tell();
        int length = position - m_start;
        out->seek_write(m_start + 4);
        out->write(&length, 4);
        out->seek_write(position);
    } else {
        SaveStream* in = m_in;
        int position = in->offset();
        if (position - m_start != m_size) {
            if (m_name) {
                CA_CHECK_FAILED1("Integrity marker %s failed", m_name);
            } else {
                CA_CHECK_FAILED("Integrity marker failed");
            }
            in->seek(m_start + m_size);
        }
    }
}

extern "C" void fn_00413BBD()
{
    lbl_014D6710.unk_008C9797(lbl_018CD7FC, 0, 0x15);
}

bool load_game(SaveStream& stream)
{
    bool result = false;
    if (stream.is_open()) {
#line 572
        s_saved_pointers = NEW SavedPointers;
        if (!s_saved_pointers)
            return false;
        unsigned short magic = 0;
        unsigned short version = 4;
        lbl_018CD818->unk_00452674();
        lbl_029219E0->unk_004A4E73();
        fn_007E599C();
        fn_0072781F();
        fn_005EF510();
        fn_00714529();
        fn_0059655D();
        fn_005D3C90();
        lbl_029219D4->unk_004BE36D();
        fn_00987FC8();
        fn_0097915C();
        fn_0097C824();
        fn_00985408();
        magic = stream.read_ushort();
        if (magic != 0xbbfc)
            return false;
        fn_004267B1(stream, VersionRegistry::instance());
        version = stream.read_ushort();
        if (version != 4)
            return false;
        result = lbl_018CD818->unk_00454CE1(stream);
        if (!result)
            return result;
        lbl_018CD818->unk_004562B8();
        lbl_029219E0->unk_004B3011();
        lbl_014D6058 = true;
        lbl_029219E0->unk_004AFADF();
        fn_004E8F26();
        lbl_029219DC->unk_00C098B3(MapCorner(0, lbl_018CD80C->height() - 1),
                                   MapCorner(lbl_018CD80C->width() - 1, 0));
        if (s_saved_pointers) {
#line 641
            CA_DELETE(s_saved_pointers);
            s_saved_pointers = 0;
        }
        for (unsigned int i = 0; i < lbl_018CD818->num_factions(); i++) {
            FactionAI* ai = lbl_018CD818->faction_at(i)->m_ai;
            if (ai)
                ai->unk_1c();
        }
        if (!lbl_018CD818->unk_22c()->unk_178()) {
            FactionAI* ai = lbl_018CD818->unk_22c()->m_ai;
            lbl_018CD818->unk_22c()->unk_004CB391();
            ai->unk_10();
        }
        AICheatManager::update();
    }
    return result;
}

// The path of a file in the saves folder, which it makes if needed.
UniString save_path(const UniString& name)
{
    const char* folder = fn_0040E497("saves");
    {
        UniString path(folder);
        fn_00F3B090(path.chars());
    }
    return UniString(folder) + UniChar('/') + name;
}

bool save_game(SaveStream& stream)
{
    bool result = false;
    if (stream.unk_00F3D990()) {
        unsigned short magic = 0xbbfc;
        unsigned short version = 4;
        stream.write(&magic, 2);
        fn_00426624(stream, VersionRegistry::instance());
        stream.write(&version, 2);
        result = lbl_018CD818->unk_00455C12(stream);
    }
    return result;
}

// The path of a file in the preferences folder, which it makes if needed.
UniString preferences_path(const UniString& name)
{
    const char* folder = fn_0040E497("preferences");
    {
        UniString path(folder);
        fn_00F3B090(path.chars());
    }
    {
        UniChar ch('/');
        UniString slash(ch);
        UniString prefix = UniString(folder) + slash;
        return prefix + name;
    }
}
