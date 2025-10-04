:- dynamic(base_stats/4).
:- dynamic(poke_stats/6).
:- dynamic(level/5).

/* Pokemon */
/* List pokemon */
/* pokemon(ID, Nama_pokemon, Rarity) */
pokemon(1, charmander, common).
pokemon(2, squirtle, common).
pokemon(3, pidgey, common).
pokemon(4, charmeleon, common).
pokemon(5, wartortle, common).
pokemon(6, pikachu, rare).
pokemon(7, geodude, rare).
pokemon(8, snorlax, epic).
pokemon(9, articuno, legendary).
pokemon(10, mewtwo, legendary).

/* Type */
/* type(Type, Nama_pokemon) */
type(flying, pidgey).
type(ice, articuno).
type(electric, pikachu).
type(normal, snorlax).
type(rock, geodude).
type(fire, charmander).
type(water, squirtle).
type(fire, charmeleon).
type(water, wartortle).
type(psychic, mewtwo).

starter(1, charmander).
starter(2, squirtle).
starter(3, pidgey).

canevolve(charmander).
canevolve(squirtle).

/* Base stats */
/* base_stats(HP, ATK, DEF, Nama_pokemon) */
base_stats(30, 14, 10, pidgey).
base_stats(60, 28, 35, articuno).
base_stats(30, 16, 10, pikachu).
base_stats(70, 30, 20, snorlax).
base_stats(30, 20, 25, geodude).
base_stats(35, 15, 10, charmander).
base_stats(40, 12, 15, squirtle).
base_stats(35, 15, 10, charmeleon).
base_stats(40, 12, 15, wartortle).
base_stats(250, 1, 250, mewtwo).

/* level Pokemon */
/* level(Level, Nama_pokemon, Slot_Inventory, EXP_Counter, Boolean_party) */
init_starter:- asserta(level(1,pidgey,0, 0, -1)), asserta(level(1,charmander,0, 0, -1)), asserta(level(1,squirtle,0, 0, -1)).

/* Poke stats */
/* poke_stats(HP, ATK, DEF, Nama_pokemon, Slot_Inventory, Boolean_party) */

/* modifier */
/* superEffective(T1,T2) : T1 is super effective against T2 */
/* notEffective(T1,T2) : T1 is not too effective against T2*/
superEffective(fire, ice).
superEffective(water, fire).
superEffective(water, rock).
superEffective(rock, fire).
superEffective(rock, flying).
superEffective(rock, ice).
superEffective(ice, flying).

notEffective(fire, water).
notEffective(fire, rock).
notEffective(fire, fire).
notEffective(water, electric).
notEffective(water, water).
notEffective(electric, electric).
notEffective(electric, rock).
notEffective(flying, electric).
notEffective(flying, rock).
notEffective(flying, ice).
notEffective(rock, water).
notEffective(rock, rock).
notEffective(ice, fire).
notEffective(ice, rock).
notEffective(ice, water).
notEffective(ice, ice).
notEffective(normal, rock).

/* skills */
/* skills(Nama_skills, Type, Power, Ability, Ability_chance) */
skills(tackle, normal, 35, none, 0).
skills(scratch, normal, 35, none, 0).
skills(ember, fire, 40, burn(2, 3), 1.0).
skills(water_gun, water, 40, none, 0).
skills(gust, flying, 30, none, 0).
skills(fire_spin, fire, 35, burn(2, 5), 1.0).
skills(bubble, water, 30, lower_atk(3), 1.0).
skills(thunder_shock, electric, 40, paralyze, 0.2).
skills(quick_attack, normal, 30, none, 0).
skills(rock_throw, rock, 50, none, 0).
skills(rest, normal, 0, heal(1.0, 2), 1.0).
skills(ice_shard, ice, 40, none, 0).
skills(psychic_blast, psychic, 1, paralyze, 0.2).
skills(mind_shock, psychic, 1, area, 1.0).

/* special trait mewtwo dengan id 10*/
/* immune_status(id pokemon) */
immune_status(10).

/* Rarity */
/* rarity(Rarity, BaseEXP, BaseEXPGiven, CatchRate) */
rarity(common, 20, 10, 40).
rarity(rare, 30, 20, 30).
rarity(epic, 40, 30, 25).
rarity(legendary, 50, 40, 20).

catch_rate(common, 40).
catch_rate(rare, 30).
catch_rate(epic, 25).
catch_rate(legendary, 10).

% pokeSkill(Nama, Skill1, Skill2)
pokeSkill(charmander, scratch, ember).
pokeSkill(squirtle, tackle, water_gun).
pokeSkill(pidgey, tackle, gust).
pokeSkill(charmeleon, ember, fire_spin).
pokeSkill(wartortle, water_gun, bubble).
pokeSkill(pikachu, thunder_shock, quick_attack).
pokeSkill(geodude, tackle, rock_throw).
pokeSkill(snorlax, tackle, rest).
pokeSkill(articuno, gust, ice_shard).
pokeSkill(mewtwo, psychic_blast, mind_shock).