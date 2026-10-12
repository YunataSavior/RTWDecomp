# Findings

Gameplay mechanics of `RomeTW.exe` (Steam Rome: Total War Gold), worked out from the decompilation and from disassembly. Each file says which parts come from matching source in [`src/`](../src/), which from disassembly alone, and what's still unverified. Claims from code that isn't decompiled yet are marked **NYD** (Not Yet Decompiled).

| File | Subject | Decompiled source |
|---|---|---|
| [`difficulty_findings.md`](game_mechanics/difficulty_findings.md) | What campaign difficulty changes: the AI's hidden bonuses, the player's income, and brigand and pirate spawning | [`code/ai/ai_cheat_manager.cpp`](../src/code/ai/ai_cheat_manager.cpp), [`code/romans_file.cpp`](../src/code/romans_file.cpp) |
| [`trait_effects_findings.md`](game_mechanics/trait_effects_findings.md) | How traits are read, and why character attributes drift when triggers remove traits | [`code/character_traits_db.cpp`](../src/code/character_traits_db.cpp), [`code/character_record.cpp`](../src/code/character_record.cpp), [`code/triggers.cpp`](../src/code/triggers.cpp) |
| [`wonder_bonus_findings.md`](game_mechanics/wonder_bonus_findings.md) | Whether wonder bonuses decay, and where each wonder's effect is applied | [`code/landmark.cpp`](../src/code/landmark.cpp), [`code/landmark_db.cpp`](../src/code/landmark_db.cpp) |
