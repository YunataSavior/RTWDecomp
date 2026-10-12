#line 2 "E:\\Perforce\\Rome\\Rome,BI-1.5,1.6\\Dev\\barbi_daily\\code\\character_traits_db.cpp"
// The character traits: export_descr_character_traits.txt, with their texts from export_VnVs.txt. (The original's
// path, which its __FILE__ gives.)
static char THIS_FILE[] = __FILE__;

#include <stdio.h>
#include <string.h>

#include "character_traits_db.h"

#include "unsorted.h"

extern "C" {
int fn_00E3B72D(const char* name);      // a character type from its name
int fn_00E3C474(const char* name);      // a culture from its name (7 for none)
int fn_00E41B13(const char* name);      // an attribute from its name (55 for none)
}

bool read_triggers(ScriptReader& reader);  // the triggers that follow (code\triggers.cpp)

Array<TraitDescription> TraitDatabase::s_traits;
static StringTable* s_text;

TraitDefinition::TraitDefinition()
    : m_index(-1), m_num_levels(0), m_anti_traits(0), m_num_anti_traits(0), m_no_going_back_level(0), m_hidden(false)
{
    m_exclude_cultures.clear();
}

static bool read_traits(ScriptReader& reader);
static void resolve_anti_traits();

bool load_traits()
{
    s_text = fn_00D95980(-1, "export_VnVs.txt", true, fn_0040E567("export_Descr_VnVs_enums.txt"), true, true, false);
    ScriptReader reader(fn_0040E567("export_descr_character_traits.txt"), 0);
    if (!reader.is_open())
        return false;
    if (!read_traits(reader))
        return false;
    if (!read_triggers(reader))
        return false;
    s_text->unk_00D9405A();
    return true;
}

static bool read_traits(ScriptReader& reader)
{
    unsigned int index = 0;
    while (!reader.at_end() && reader.match("Trait")) {
        TraitDatabase::s_traits.add();
        TraitDescription* trait = &TraitDatabase::s_traits[index];
        trait->m_index = index;
        reader.skip_word();
        const char* text = reader.word();
        trait->m_name = text;
        reader.next_line();
        if (!reader.match("Characters")) {
            SCRIPT_ERROR2(reader, "Unknown identifier for trait(%s) level(%s) when expecting Characters.",
                          trait->m_name.c_str(), text);
            return false;
        }
        reader.skip_word();
        text = reader.word();
        if (reader.match("all")) {
            trait->m_characters.set_all();
        } else {
            int type = fn_00E3B72D(reader.word());
            trait->m_characters.set(type);
        }
        reader.next_line();
        if (reader.match("Hidden")) {
            trait->m_hidden = true;
            reader.next_line();
        }
        if (reader.match("ExcludeCultures")) {
            trait->m_exclude_cultures.clear();
            while (reader.skip_word()) {
                text = reader.word();
                int culture = fn_00E3C474(text);
                if (culture == 7) {
                    SCRIPT_ERROR(reader, "%s", "don't recognise this culture type");
                    return false;
                }
                trait->m_exclude_cultures.set(culture);
            }
            reader.next_line();
        }
        if (reader.match("NoGoingBackLevel")) {
            reader.skip_word();
            trait->m_no_going_back_level = reader.read_int();
            reader.next_line();
        }
        trait->m_num_anti_trait_names = 0;
        if (reader.match("AntiTraits")) {
            while (reader.skip_word()) {
                text = reader.word();
                trait->m_anti_trait_names[trait->m_num_anti_trait_names] = text;
                trait->m_num_anti_trait_names++;
            }
            reader.next_line();
        }
        trait->m_num_levels = 0;
        trait->m_levels[trait->m_num_levels].m_level = 0;
        trait->m_levels[trait->m_num_levels].m_threshold = 0;
        trait->m_num_levels = 1;
        while (reader.match("Level")) {
            trait->m_levels[trait->m_num_levels].m_level = trait->m_num_levels;
            reader.skip_word();
            text = reader.word();
            trait->m_levels[trait->m_num_levels].m_name = s_text->lookup_text(text);
            reader.next_line();
            if (!reader.match("Description")) {
                SCRIPT_ERROR2(reader, "Unknown identifier for trait(%s) level(%s) when expecting Description.",
                              trait->m_name.c_str(), text);
                return false;
            }
            reader.skip_word();
            text = reader.word();
            trait->m_levels[trait->m_num_levels].m_description = s_text->lookup_text(text);
            reader.next_line();
            if (!reader.match("EffectsDescription")) {
                SCRIPT_ERROR2(reader, "Unknown identifier for trait(%s) level(%s) when expecting Description.",
                              trait->m_name.c_str(), text);
                return false;
            }
            reader.skip_word();
            text = reader.word();
            trait->m_levels[trait->m_num_levels].m_effects_description = s_text->lookup_text(text);
            reader.next_line();
            if (reader.match("GainMessage")) {
                reader.skip_word();
                text = reader.word();
                trait->m_levels[trait->m_num_levels].m_gain_message = s_text->lookup_text(text);
                reader.next_line();
            }
            if (reader.match("LoseMessage")) {
                reader.skip_word();
                text = reader.word();
                trait->m_levels[trait->m_num_levels].m_lose_message = s_text->lookup_text(text);
                reader.next_line();
            }
            if (reader.match("Epithet")) {
                reader.skip_word();
                text = reader.word();
                trait->m_levels[trait->m_num_levels].m_epithet = s_text->lookup_text(text);
                reader.next_line();
            }
            if (!reader.match("Threshold")) {
                SCRIPT_ERROR2(reader, "Unknown identifier for trait(%s) level(%s) when expecting Threshold.",
                              trait->m_name.c_str(), text);
                return false;
            }
            reader.skip_word();
            trait->m_levels[trait->m_num_levels].m_threshold = reader.read_int();
            reader.next_line();
            int num_effects = 0;
            while (reader.match("Effect") || reader.match("Religious_Belief") || reader.match("Religious_Order")) {
                if (reader.match("Effect")) {
                    reader.skip_word();
                    trait->m_levels[trait->m_num_levels].m_effects.add();
                    TraitEffect* effect = &trait->m_levels[trait->m_num_levels].m_effects[num_effects];
                    effect->m_attribute = fn_00E41B13(reader.word());
                    if (effect->m_attribute == 55) {
                        SCRIPT_ERROR(reader, "Unknown attribute type(%s).", reader.word());
                        return false;
                    }
                    reader.skip_word();
                    effect->m_value = reader.read_int();
                    num_effects++;
                    reader.next_line();
                }
                if (reader.match("Religious_Belief")) {
                    reader.skip_word();
                    trait->m_levels[trait->m_num_levels].m_religious_belief = read_religion(reader);
                    reader.next_line();
                }
                if (reader.match("Religious_Order")) {
                    reader.skip_word();
                    trait->m_levels[trait->m_num_levels].m_religious_order = read_religion(reader);
                    reader.next_line();
                }
            }
            trait->m_num_levels++;
            if (trait->m_num_levels > MAX_TRAIT_LEVELS) {
                SCRIPT_ERROR2(reader,
                              "Too many levels for trait(%s). Max is %d. \nIf required, we can increase this in "
                              "character_traits_db.h",
                              trait->m_name.c_str(), MAX_TRAIT_LEVELS);
                return false;
            }
        }
        index++;
    }
    resolve_anti_traits();
    return true;
}

static void resolve_anti_traits()
{
    for (unsigned int i = 0; i < TraitDatabase::s_traits.size(); i++) {
        for (unsigned int j = 0; j < TraitDatabase::s_traits[i].m_num_anti_trait_names; j++) {
            String name = TraitDatabase::s_traits[i].m_anti_trait_names[j];
            TraitDatabase::s_traits[i].m_anti_traits[j] = find_trait(name);
            TraitDatabase::s_traits[i].m_num_anti_traits++;
        }
    }
}

void unload_traits()
{
    fn_00D95AB2(s_text);
    s_text = 0;
    while (TraitDatabase::s_traits.size() != 0)
        TraitDatabase::s_traits.pop_back();
}

TraitDescription* find_trait(const char* name)
{
    for (unsigned int i = 0; i < TraitDatabase::s_traits.size(); i++)
        if (!_stricmp(TraitDatabase::s_traits[i].m_name.c_str(), name))
            return &TraitDatabase::s_traits[i];
    return 0;
}

TraitDescription* find_trait(String name)
{
    for (unsigned int i = 0; i < TraitDatabase::s_traits.size(); i++)
        if (TraitDatabase::s_traits[i].m_name == name)
            return &TraitDatabase::s_traits[i];
    return 0;
}
