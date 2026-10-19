#line 2 "E:\\Perforce\\Rome\\Rome,BI-1.5,1.6\\Dev\\barbi_daily\\code\\landmark.cpp"
// (The original's path, which its __FILE__s give.)
static char THIS_FILE[] = __FILE__;

#include "landmark.h"

#include "unsorted.h"
#include "map_constants.h"

static const ConstInt NUM_LANDMARK_TYPES(9);  // the wonders' type IDs are 1 to 9
static const float QUARTER_PI = PI * 0.25f;

// Something a landmark keeps on the fog of war (code\fog_of_war.cpp).
class FogOfWarEntry {
public:
    FogOfWarEntry(Landmark* landmark);  // 0x00495A93

    char m_pad0[0x1c];
};

// An effect played at a point of the map (not identified).
class MapEffect {
public:
    MapEffect();                        // 0x0048FB22
    void set_position(const Vector3& position);  // 0x004960B8

    char m_pad0[0x40];
};

class UnkF6B3D0 {
public:
    bool unk_00F6B3D0(const Vector3& point, float radius);  // 0x00F6B3D0
};

class UnkA62E78 {
public:
    void unk_00A62E78(Model* model, MapEffect& at);  // 0x00A62E78: draws model
};

extern "C" {
Vector3 fn_009C90D2(float x, float z, bool, bool, bool);  // (C4190: C linkage returning a struct)
extern UnkB4CFDA* lbl_029219D8;
extern UnkF6B3D0 lbl_017B1C48;
extern UnkA62E78 lbl_016E84A0;
extern Campaign* lbl_02921AC0;
}

inline unsigned char visibility(const TilePosition& position)
{
    return fn_009CB2A7(position.x, position.y);
}

// Gives the landmark to faction (0 for none). When it is captured, the faction's people celebrate for four turns
// (Faction::m_wonder_order).
void Landmark::set_faction(Faction* faction, bool captured)
{
    if (faction)
        notify(LandmarkEvent(faction, this));
    if (captured) {
        faction->m_wonder_order = 4;
        if (faction == lbl_029219D8->unk_00B4CFDA()) {
            fn_00CB5468(faction, this, position(), 0);
            fn_00CB54E7(m_faction, this, position());
        }
        Port* port = lbl_018CD80C->region(region())->port();
        if (port->unk_a98())
            fire_triggers(UnkMessage(port->unk_a98()->unk_78()));
    }
    m_faction = faction;
}

FogOfWarEntry* Landmark::unk_74()
{
#line 185
    return NEW FogOfWarEntry(this);
}

void LandmarkManager::add(Landmark* landmark)
{
    const String& name = landmark->m_description->name();
    landmark->m_type = fn_00E3C9F6(name.c_str());
    m_landmarks.push_back(landmark);
    Tile* tile = lbl_018CD80C->tile(*landmark->position());
    tile->m_landmark = 1;
    tile->m_25 = 0xff;
    int region = lbl_018CD80C->tile(*landmark->position())->m_region;
    lbl_018CD80C->add_landmark(region, landmark);
}

void LandmarkManager::remove(Landmark* landmark)
{
    for (unsigned short i = 0; i < size(); i++) {
        if (m_landmarks[i] == landmark) {
            m_landmarks.erase(i);
#line 223
            CA_DELETE(landmark);
        }
    }
}

int LandmarkManager::num_held(int faction)
{
    int count = 0;
    for (unsigned int i = 0; i < size(); i++)
        if (m_landmarks[i]->faction() && m_landmarks[i]->faction()->id() == faction)
            count++;
    return count;
}

Landmark* LandmarkManager::find(int type)
{
    for (unsigned short i = 0; i < size(); i++)
        if (m_landmarks[i]->m_type == type)
            return m_landmarks[i];
    return 0;
}

void LandmarkManager::unk_0049D091()
{
}

void LandmarkManager::clear()
{
    while (size() != 0) {
#line 261
        CA_DELETE(m_landmarks.back());
        m_landmarks.pop_back();
    }
}

// Plays each landmark's effect when it is on screen.
void LandmarkManager::update()
{
    float x = 0.0f;
    float z = 0.0f;
    MapEffect effect;
    for (unsigned short i = 0; i < size(); i++) {
        if (visibility(*m_landmarks[i]->position()) == 1) {
            x = m_landmarks[i]->position()->x + 0.5f;
            z = m_landmarks[i]->position()->y + 0.5f;
            Vector3 point(x, fn_009C90D2(x, z, true, true, false).m_y, z);
            if (!lbl_017B1C48.unk_00F6B3D0(point, 3.0f)) {
                if (!lbl_02921AC0->unk_00452977() || lbl_029219E0->unk_004B3124(*m_landmarks[i]->position()) >= 2) {
                    effect.set_position(point);
                    lbl_016E84A0.unk_00A62E78(m_landmarks[i]->m_description->m_model, effect);
                }
            }
        }
    }
}

static VersionRegistrar s_landmark_manager_version("LANDMARK_MANAGER", 1);

bool LandmarkManager::load(SaveStream& stream)
{
    if (!stream.is_open())
        return false;
    switch (saved_version("LANDMARK_MANAGER")) {
    case 1: {
        String name;
        TilePosition position;
        unsigned short count = 0;
        Faction* saved_pointer;
        clear();
        count = stream.read_ushort();
        for (unsigned short i = 0; i < count; i++) {
            name.load(stream);
            stream.read(&position, 8);
            stream.read(&saved_pointer, 4);
            Landmark* landmark = create_landmark(name);
            landmark->m_faction = saved_pointer;
            lbl_029219E0->unk_004ABED9(landmark, &position);
            add(landmark);
            lbl_018CD80C->tile(position.x, position.y)->m_landmark = 1;
        }
        return true;
    }
    }
    stream.fail();
    return false;
}

bool LandmarkManager::save(SaveStream& stream)
{
    if (!stream.unk_00F3D990())
        return false;
    TilePosition position;
    unsigned short count = size();
    stream.write(&count, 2);
    for (unsigned short i = 0; i < count; i++) {
        m_landmarks[i]->m_description->m_name.save(stream);
        position = *m_landmarks[i]->position();
        stream.write(&position, 8);
        stream.write_value(m_landmarks[i]->m_faction);
    }
    return true;
}

void LandmarkManager::resolve_factions()
{
    unsigned short count = size();
    for (unsigned short i = 0; i < count; i++)
        m_landmarks[i]->m_faction = (Faction*)fn_0041374F(m_landmarks[i]->m_faction);
}
