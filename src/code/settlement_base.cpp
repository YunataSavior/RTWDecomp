#line 2 "E:\\Perforce\\Rome\\Rome,BI-1.5,1.6\\Dev\\barbi_daily\\code\\settlement_base.cpp"
// (A guess at the original's name: these sit between fort.cpp and garrison.cpp.)
static char THIS_FILE[] = __FILE__;

// What settlements and forts share.
#include "unsorted.h"

#include "character.h"

// Gives the settlement or fort plague, if it hasn't got it, and every character on its tile catches it too.
void SettlementBase::catch_plague()
{
    // NONMATCHING - the locals' stack slots differ
    Character* character = 0;
    TilePosition position = *this->position();
    StrategyMap* map = lbl_02921A34;
    Tile tile = *map->tile(position);
    StrategyObject* object = tile.m_objects;
    if (!m_plague) {
        m_plague_ticks = 0;
        m_plague = true;
        if (settlement()) {
            Settlement* settlement = this->settlement();
            settlement->m_plague_deaths = 0;
        }
        for (; object; object = object->m_next) {
            if (object->object_type() == OBJECT_CHARACTER) {
                character = object->unk_28();
                character->catch_plague();
            }
        }
    }
}

// One turn of plague: the fourth ends it.
void SettlementBase::plague_tick()
{
    if (m_plague) {
        m_plague_ticks++;
        bool over = false;
        if (m_plague_ticks >= 4)
            over = true;
        if (over) {
            m_plague = false;
            m_plague_ticks = 0;
            if (settlement()) {
                Settlement* settlement = this->settlement();
                settlement->m_plague_deaths = 0;
            }
        }
    }
}
