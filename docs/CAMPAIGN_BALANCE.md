# Campaign map and difficulty design

## Difficulty model

Campaign difficulty is not defined by enemy count alone. Every mission is reviewed across four pressures:

1. **Route pressure** — path length, number of turns and how long one defense can keep a target in range.
2. **Placement pressure** — number and distribution of build spots, including whether one position covers several route segments.
3. **Wave pressure** — enemy health, speed, spacing and the order in which archetypes arrive.
4. **Economy pressure** — whether the initial 120 gold is enough for the obvious answer and how quickly kills fund the next position.

All missions remain capped at 16 active wave enemies and 16 defenses. A later mission should introduce a harder spatial problem before it reduces spawn intervals.

## Enemy durability curve

The tutorial keeps the base enemy values (`Scout 2`, `Raider 3`, `Brute 5`). Later missions apply a campaign multiplier to maximum health while preserving movement speed and base damage:

| Mission | HP multiplier |
| --- | ---: |
| Straznica | 1.00x |
| Popielna Brama | 1.15x |
| Zniszczona Osada | 1.30x |
| Kamienny Most | 1.50x |
| Dolina Echa | 1.70x |
| Zatopiony Trakt | 1.90x |
| Zelazny Wawoz | 2.10x |
| Krag Burz | 2.30x |
| Ostatnia Cytadela | 2.50x |

Health is rounded upward. This keeps the first mission approachable while making later enemies require sustained fire, mixed tower roles and upgrades instead of dying to one or two basic hits.

## Campaign curve

| # | Mission | Size | Tactical lesson | Intended pressure |
| --- | --- | --- | --- | --- |
| 1 | Straznica | 12x12 | build and start a wave | very forgiving route and small wave |
| 2 | Popielna Brama | 12x12 | cover two turns | faster scouts, still generous positions |
| 3 | Zniszczona Osada | 12x12 | crossfire on a returning route | grouped raiders |
| 4 | Kamienny Most | 12x12 | exploit a choke point | first meaningful brute pressure |
| 5 | Dolina Echa | 12x12 | distribute range over a long path | mixed wave and coverage pressure |
| 6 | Zatopiony Trakt | 16x16 | defend separated islands | first rocket mission, long S route |
| 7 | Zelazny Wawoz | 16x16 | split defense over three chokes | alternating fast and heavy enemies |
| 8 | Krag Burz | 16x16 | cover an outer ring around ruins | dense mixed wave and open rocket lanes |
| 9 | Ostatnia Cytadela | 12x12 | combine every defense archetype | final compact mixed assault |

## Guided rocket role

The rocket launcher costs 120 gold and is intentionally not an opening default on early maps. It has:

- the longest range;
- the slowest firing cycle;
- area damage suitable for grouped enemies;
- a guided projectile with persistent velocity and limited turn rate;
- a climbing, offset launch vector that creates a visible hook before guidance converges.

Guidance rotates the current velocity toward the target by at most `turnRate * deltaSeconds`. It does not snap to the desired direction. The projectile remains inside the shared 32-projectile pool.

## Build-spot navigation

The D-pad continues to cycle all build spots. After a successful build, the cursor advances to the next unoccupied position. This keeps large 16x16 maps usable without requiring the player to traverse non-buildable grid cells.

## Physical validation still required

Automated tests validate file structure, unique layouts, geometry budgets, bounded projectile flight and deterministic progression. Final difficulty, route readability, rocket visibility and 30 FPS must still be confirmed on an original Nintendo 3DS XL.
