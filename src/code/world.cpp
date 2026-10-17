// The campaign: its factions and its settings.
#include "unsorted.h"

// How much bigger or smaller units are than at the normal unit size, which the garrison's public order weighs the
// population by.
float Campaign::unit_scale()
{
    return settings()->m_unit_scale;
}

// Whether the region is the last one with the britain hidden resource that a major faction holds (other than this
// one, none is held by a faction with custom_battle_availability and without description+0xa7), while it isn't the
// Romano-British's and they haven't been spawned yet (m_2c4). A revolt there goes to the Romano-British (BI).
bool Campaign::last_britain_region(Region* region)
{
    // NONMATCHING - stack slot order differs
    if (!m_2c4) {
        int resource = lbl_014DA2B0.unk_008DB1B9(String("britain"));
        if (resource >= 0 && region->has_resource(resource)) {
            FactionDescription* description = region->m_owner->m_description;
            bool not_romano_british = description != fn_008F60D5(String("romano_british"));
            if (not_romano_british) {
                unsigned int unused = 0;
                unsigned int i;
                for (i = 0; i < lbl_02921A34->num_regions(); i++) {
                    Region* other = lbl_02921A34->region(i);
                    if (!(other->id() == region->id()) && other->m_owner && other->m_owner->unk_00445B2B()
                        && other->has_resource(resource))
                        break;
                }
                if (i == lbl_02921A34->num_regions())
                    return true;
            }
        }
    }
    return false;
}
