#line 2 "E:\\Perforce\\Rome\\Rome,BI-1.5,1.6\\Dev\\barbi_daily\\code\\landmark_db.cpp"
// The wonders' types: descr_sm_landmarks.txt and the text and models it names. (The original's path, which its
// __FILE__s give.)
static char THIS_FILE[] = __FILE__;

#include <stdio.h>

#include "landmark.h"

#include "unsorted.h"
#include "angle_constants.h"

// The 10 wonder types' descriptions.
struct LandmarkDescriptions {
    LandmarkDescription m_items[10];
};

// (A static member: the destructor run at exit guards against running twice, as VC6 makes it for those.)
class LandmarkDatabase {
public:
    static LandmarkDescriptions s_descriptions;
};

LandmarkDescriptions LandmarkDatabase::s_descriptions;
static unsigned int s_type;
static StringTable* s_text;

bool load_landmark_descriptions()
{
    s_text = fn_00D95980(-1, "landmarks.txt", true, fn_0040E567("descr_landmarks_lookup.txt"), true, true, false);
    ScriptReader reader(fn_0040E567("descr_sm_landmarks.txt"), 0);
    if (!reader.is_open())
        return false;
    while (!reader.at_end() && reader.match("type")) {
        if (!reader.match("type")) {
            SCRIPT_ERROR(reader, "%s", "unknown identifier when new type expected.\n");
            return false;
        }
        reader.skip_word();
        int type = fn_00E3C9F6(reader.cur());
        if (type == 10) {
            SCRIPT_ERROR(reader, "dont recognise this wonder type %s", reader.cur());
            return false;
        }
        LandmarkDescription* description = &LandmarkDatabase::s_descriptions.m_items[type];
        description->m_type.assign(reader.cur(), reader.word_length());
        const char* name = description->m_type.c_str();
        char text[256];
        sprintf(text, "%s_title", name);
        description->m_title = s_text->lookup(text);
        sprintf(text, "%s_short_descr", name);
        description->m_short_description = s_text->lookup(text);
        sprintf(text, "%s_long_descr", name);
        description->m_long_description = s_text->lookup(text);
        sprintf(text, "%s_effects", name);
        description->m_effects = s_text->lookup(text);
        reader.next_line();
        while (!reader.match("type") && !reader.at_end()) {
            if (reader.match("item")) {
                reader.skip_word();
                description->m_item.assign(reader.cur(), reader.word_length());
            } else if (reader.match("image")) {
                reader.skip_word();
                description->m_image.assign(reader.cur(), reader.word_length());
            } else if (reader.match("julii_rtm")) {
                reader.skip_word();
                description->m_julii_rtm.assign(reader.cur(), reader.word_length());
            } else if (reader.match("brutii_rtm")) {
                reader.skip_word();
                description->m_brutii_rtm.assign(reader.cur(), reader.word_length());
            } else if (reader.match("scipii_rtm")) {
                reader.skip_word();
                description->m_scipii_rtm.assign(reader.cur(), reader.word_length());
            } else if (reader.match("movie")) {
                reader.skip_word();
                description->m_movie.assign(reader.cur(), reader.word_length());
            }
            reader.next_line();
        }
    }
    return true;
}

bool load_landmark_models()
{
    ModelFlags flags = MODEL_FLAG_4 | MODEL_FLAG_18 | MODEL_FLAG_100 | MODEL_FLAG_800;
    for (s_type = 0; s_type < 10; s_type++) {
        if (LandmarkDatabase::s_descriptions.m_items[s_type].m_item.c_str()) {
            LandmarkDatabase::s_descriptions.m_items[s_type].load_model(
                LandmarkDatabase::s_descriptions.m_items[s_type].m_item.c_str(), flags);
            LandmarkDatabase::s_descriptions.m_items[s_type].model()->m_11d = true;
        }
    }
    return true;
}

void unload_landmark_descriptions()
{
    for (unsigned int i = 0; i < 10; i++) {
        LandmarkDatabase::s_descriptions.m_items[i].m_type.clear();
        LandmarkDatabase::s_descriptions.m_items[i].m_item.clear();
    }
    fn_00D95AB2(s_text);
    s_text = 0;
}

Landmark* create_landmark(const String& type)
{
    for (unsigned int i = 0; i < 10; i++) {
        if (LandmarkDatabase::s_descriptions.m_items[i].m_type == type) {
#line 176
            Landmark* landmark = NEW Landmark;
            landmark->m_description = &LandmarkDatabase::s_descriptions.m_items[i];
            landmark->m_description->m_name = type;
            landmark->m_settlement = 0;
            landmark->m_type = i;
            return landmark;
        }
    }
    return 0;
}

Model*& landmark_model(int type)
{
    return LandmarkDatabase::s_descriptions.m_items[type].m_model;
}
