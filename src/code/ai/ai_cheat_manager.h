// The campaign AI's difficulty "cheats": hidden bonuses for AI factions and penalties for the human player, set from
// the first human faction's campaign difficulty (0 easy, 1 medium, 2 hard, 3 very hard).
#ifndef AI_CHEAT_MANAGER_H
#define AI_CHEAT_MANAGER_H

class SaveStream;

class AICheatManager {
public:
    static void reset_war_turns();                        // 0x004FC042
    static void update_war_turns();                       // 0x004FC04E
    static void update();                                 // 0x004FC0A8

    static int ai_population_growth(int faction);         // 0x004FC8BE
    static int ai_public_order(int faction);              // 0x004FC8E8
    static int ai_extra_count(int faction);               // 0x004FC912
    static int ai_value_against(int faction, int other);  // 0x004FC931
    static int ai_turn_cash(int faction);                 // 0x004FC961
    static int boom_threshold(int faction);               // 0x004FCA93
    static float unused_scale();                          // 0x004FCAB3
    static float brigand_spawn_scale(int faction);        // 0x004FCABE
    static float tax_income_scale(int faction);           // 0x004FCB01
    static float farm_income_scale(int faction);          // 0x004FCB2B

    static bool may_target(int faction, int target);              // 0x004FCB55
    static bool punish_long_war(int faction, int player);         // 0x004FCC97
    static bool flag_war(int faction, int enemy);                 // 0x004FCCDE
    static bool target_player_bonus(int faction, int target);     // 0x004FCD41
    static bool unknown_10da998(int faction);                     // 0x004FCD79
    static bool unknown_12d2f92();                                // 0x004FCDAE
    static bool unknown_4fcdb8();                                 // 0x004FCDB8
    static bool small_ai_chance(int faction, int other);          // 0x004FCDBF
    static bool not_small_ai(int faction);                        // 0x004FCE2F
    static float player_target_value(int faction);                // 0x004FCE46
    static bool save(SaveStream& stream);                         // 0x004FCE6A
    static bool load(SaveStream& stream);                         // 0x004FCE80

    // Read by 0x004EA38A (an AI objective's bonus against targets the player owns).
    static bool player_objective_bonus() { return s_player_objective_bonus; }

private:
    static bool s_player_objective_bonus;                         // 0x012D2F98: hard and very hard
};

#endif
