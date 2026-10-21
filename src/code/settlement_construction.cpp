// A settlement's buildings.
#include "unsorted.h"
#include "building.h"
#include "settlement.h"
#include "world.h"

// Damages the buildings in a riot or a revolt. Each building other than the walls (and, with skip_outside, other than
// the port and the hinterland buildings, such as roads and farms) is damaged with a chance of m_building_chance
// percent: it loses m_building_min to m_building_max health, or m_heavy_min to m_heavy_max when the roll is also below
// m_heavy_chance. A building already at 0 health is left alone.
void SettlementBuildings::damage(SettlementDamageParams* params, SettlementDamage* damage, bool skip_outside)
{
    // NONMATCHING - objdiff gives 99.9% with no instruction differing (a relocation)
    for (unsigned int i = 0; i < count(); i++) {
        Building* building = unk_005C2B69(i);
        if (building->type()->defenses())
            continue;
        if (skip_outside && building->type()->port())
            continue;
        if (skip_outside && building->type()->hinterland())
            continue;
        unsigned int roll = lbl_018CD818->m_random.percent();
        if (roll < params->building_chance()) {
            int low;
            int high;
            if (roll < params->heavy_chance()) {
                low = params->m_heavy_min;
                high = params->m_heavy_max;
            } else {
                low = params->m_building_min;
                high = params->m_building_max;
            }
            if (building->health() > 0) {
                unsigned int amount = lbl_018CD818->m_random.range(low, high);
                building->damage(amount);
                damage->add_building(building);
            }
        }
    }
}
