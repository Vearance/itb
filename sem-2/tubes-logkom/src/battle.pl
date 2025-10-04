:- dynamic(myTurn/0).
:- dynamic(situation/1).
:- dynamic(statusKita/8).
:- dynamic(statusLawan/7).
:- dynamic(defendStatus/2).
:- dynamic(statusEfekKita/2).
:- dynamic(statusEfekLawan/1).
:- dynamic(cooldown_kita/2).
:- dynamic(cooldown_lawan/2).
:- dynamic(player_level/1).
:- dynamic(enemy_level/1).
:- dynamic(atkindex/1).
:- dynamic(enemyskill/1).
:- dynamic(enemy_predefend/0).

random_between(Low, High, R) :-
    Range is High - Low + 1,
    random(XFloat),
    XInt is floor(XFloat * 1000000),
    R0 is XInt mod Range,
    R is Low + R0.

daftar_party :-
    findall(Index-Nama, party(Index, Nama), List),
    sort(List, Sorted),
    write('Daftar POKeMON dalam party kamu:'), nl,
    tampilkan_party(Sorted).

has_alive_pokemon :-
    party(Index, Name),
    curr_health(Index, Name, HP, 1),
    HP > 0, !.

tampilkan_party([]).
tampilkan_party([Index-Nama | T]) :-
    % Ambil data max HP
    poke_stats(HPMax, _, _, _, Index, 1),
    % Ambil data current HP
    (curr_health(Index, Nama, CurrHP, 1) -> true ; CurrHP = 0),
    % Ambil data lainnya
    % Tampilkan data
    format('~w: ~w (~w/~w HP)~n', [Index, Nama, CurrHP, HPMax]),
    tampilkan_party(T).


pilih_pokemon :-
    repeat,
    write('Masukkan indeks POKeMON yang ingin kamu gunakan: '), nl,
    write('>> '),
    read(Index),
    (
        valid_pokemon_choice(Index),
        party(Index, Name),
        curr_health(Index, Name, HP, 1),
        HP > 0 ->
            format('Kamu memilih ~w sebagai POKeMON utama!~n', [Name]),
            retractall(atkindex(_)), asserta(atkindex(Index)), init_poke(Index),!
        ;
        write('Pilihan tidak valid atau POKeMON sudah tumbang, silakan pilih lagi.'), nl,
        fail
    ).

valid_pokemon_choice(Index) :-
    party(Index, Name),
    curr_health(Index, Name, HP, 1),
    HP > 0.

init_poke(Index) :-
    atkindex(Index),
    party(Index, Nama),
    pokemon(ID, Nama, _),
    level(LevelKita,Nama,Index, _, 1),
    poke_stats(MaxHPKita, ATKKita, DEFKita, Nama, Index, 1),
    curr_health(Index, Nama, CurrHPKita, 1),
    type(Type, Nama),
    retractall(statusKita(_, _, _, _, _, _, _, Index)),
    assertz(statusKita(CurrHPKita, MaxHPKita, ATKKita, DEFKita, Nama, ID, Type, Index)),
    retractall(player_level(_)),
    assertz(player_level(LevelKita)).

buat_lawan(Rarity) :-
    pokeRandomizer(Rarity, Nama),
    
    % Ambil remaining moves
    remaining_moves(Remaining),

    % Hitung batas level berdasarkan remaining_moves
    LevelMin is max(1, min(15, round(1 + (20 - Remaining) / 3.5))),
    LevelMax is max(LevelMin, min(15, round(2 + (20 - Remaining) / 2.5))),
    random_between(LevelMin, LevelMax, Level),
    Level1 is Level - 1,
    base_stats(HPBase, ATKBase, DEFBase, Nama),
    MaxHP is HPBase + 2 * Level1,
    ATK is ATKBase + 1 * Level1,
    DEF is DEFBase + 1 * Level1,
    type(Type, Nama),
    retractall(statusLawan(_, _, _, _, _, _, _)),
    assertz(statusLawan(MaxHP, MaxHP, ATK, DEF, Nama, 99, Type)),
    retractall(enemy_level(_)),
    assertz(enemy_level(Level)),
    pokemon(ID, Nama, _),
    pokemon_ascii(ID),
    write('Kamu melawan '), write(Nama), write('.'), nl,
    write('Level: '), write(Level), nl,
    write('HP: '), write(MaxHP), nl,
    write('ATK: '), write(ATK), nl,
    write('DEF: '), write(DEF), nl, nl,
    write('Pilih POKeMON mu dari party!'), nl,
    daftar_party,
    pilih_pokemon,
    retractall(defendStatus(_, _)),
    assertz(defendStatus(1, 1)),
    true.

% Pilih pokemon secara acak berdasarkan rarity
pokeRandomizer(common, Nama) :-
    findall(N, pokemon(_, N, common), CommonList),
    random_member(Nama, CommonList).

pokeRandomizer(rare, Nama) :-
    findall(N, pokemon(_, N, rare), RareList),
    random_member(Nama, RareList).

pokeRandomizer(epic, Nama) :-
    findall(N, pokemon(_, N, epic), EpicList),
    random_member(Nama, EpicList).

pokeRandomizer(legendary, articuno).

battle(Rarity) :-
    retractall(situation(_)),
    assertz(situation(ongoing)),

    remaining_moves(SisaMove),
    (
        SisaMove =< 0 ->
            /* pasti lawan mewtwo */
            print_end_game_opening,
            buat_lawan_mewtwo
        ;
            /* lawan normal */
            buat_lawan(Rarity)
    ),

    retractall(myTurn),
    assertz(myTurn),

    random_between(1, 10, R),
    (R >= 9 ->
        statusLawan(_, _, _, _, NamaLawan, _, _),
        quiz_pokemon(NamaLawan)
    ; true),

    % Inisialisasi cooldown
    retractall(cooldown_kita(_, _)),
    retractall(cooldown_lawan(_, _)),
    assertz(cooldown_kita(0, 0)),
    assertz(cooldown_lawan(0, 0)),

    % Mulai giliran
    turn.

get_status(HP, MaxHP, ATK, DEF, Nama, ID) :-
    ( myTurn ->
        atkindex(Index),
        statusKita(HP, MaxHP, ATK, DEF, Nama, ID, Type, Index)
    ; 
        statusLawan(HP, MaxHP, ATK, DEF, Nama, ID, Type)
    ).

update_status(HP, MaxHP, ATK, DEF, Nama, ID) :-
    ( myTurn ->
        atkindex(Index),
        retractall(statusKita(_, _, _, _, _, ID, Type, Index)),
        assertz(statusKita(HP, MaxHP, ATK, DEF, Nama, ID, Type, Index))
    ; 
        retractall(statusLawan(_, _, _, _, _, ID, _)),
        assertz(statusLawan(HP, MaxHP, ATK, DEF, Nama, ID, Type))
    ).

ignore(Goal) :- call(Goal), !.
ignore(_).

turn :-
    situation(ongoing), !,
    atkindex(Index),
    ignore(apply_turn_effects),
    ignore(reset_defend),
    statusKita(HP1, _, _, _, Name1, _, _, Index),
    statusLawan(HP2, _, _, _, Name2, _, _),
    (
        HP1 =< 0 ->
            format('~w tumbang!~n', [Name1]),
            party(Index, Name1),
            curr_health(Index, Name1, 0, 1),
            retractall(statusEfekKita(_, _)),
            ( has_alive_pokemon ->
                write('Pilih POKeMON pengganti:\n'),
                retract(statusKita(0, _, _, _, Name1, _, _, _)),
                daftar_party,
                pilih_pokemon,
                retractall(statusEfekKita(_, _)),
                retractall(myTurn),
                assertz(myTurn),
                turn
            ;
                write('Semua POKeMON-mu sudah kalah. Kamu kalah total...\n'),
                retractall(situation(_)),
                assertz(situation(lose)),
                retractall(statusKita(_,_,_,_,_,_,_,_)),
                retractall(statusLawan(_,_,_,_,_,_,_)),
                remaining_moves(SisaMove),
                (SisaMove =< 0 -> check_endgame ; true)

            )
    ; HP2 =< 0 ->
        format('POKeMON ~w kalah! Kamu menang!~n~n', [Name2]),
        enemy_level(X),
        kalahkan_pokemon,
        forall(
            (party(Index1, Name), curr_health(Index1, Name, HP, 1)),
            (
                enemy_level(X), rarity(_, _, Y, _), Expgiven is Y + X*2,
                retractall(curr_health(Index1, Name, _,1)),
                assertz(curr_health(Index1, Name, HP, 1)),
                ignore(addExp(Expgiven, Index1, Name))
            )
        ),
        retractall(statusEfekKita(_, _)),
        retractall(statusEfekLawan(_)),
        retractall(situation(_)),
        assertz(situation(win)), 
        retractall(statusKita(_,_,_,_,_,_,_,_)),
        retractall(statusLawan(_,_,_,_,_,_,_)),
        remaining_moves(SisaMove),
        (SisaMove =< 0 -> check_endgame ; true),
        retractall(situation(_))
        
    ; myTurn ->
        nl, handle_player_turn, nl
    ; 
        handle_enemy_turn
    ).

turn :-
    situation(Status),
    format('Pertarungan selesai! Hasil: ~w~n', [Status]).

find_next_alive_pokemon(CurrentIndex, NextIndex, PokemonName) :-
    party(NextIndex, PokemonName),
    NextIndex \= CurrentIndex,
    curr_health(NextIndex, PokemonName, HP),
    HP > 0, !.  % ! agar ambil yang pertama ketemu

handle_player_turn :-
    statusEfekKita(_, sleep(T)), T > 0, !,
    NewT is T - 1,
    retract(statusEfekKita(_, sleep(T))),
    (NewT > 0 -> assertz(statusEfekKita(_, sleep(NewT))) ; true),
    statusKita(_, _, _, _, Name, _, _, _),
    write(Name), write(' sedang tidur... turn dilewati.'), nl, nl, toggle_turn, turn.

handle_player_turn :-
    write('Giliran kamu! Pilih aksi: attack. | defend. | skill(N).').

handle_enemy_turn :-
    statusEfekLawan(sleep(T)), T > 0, !,
    NewT is T - 1,
    retract(statusEfekLawan(sleep(T))),
    (NewT > 0 -> assertz(statusEfekLawan(sleep(NewT))) ; true),
    statusLawan(_, _, _, _, Name, _, _),
    write(Name), write(' sedang tidur... turn dilewati.'), nl, nl, toggle_turn, turn.

handle_enemy_turn :-
    situation(ongoing),  % penting: lanjut hanya jika battle belum selesai
    enemy_action.

reset_defend :-
    ( myTurn ->
        retract(defendStatus(DefMulKita, DefMulLawan)),
        assertz(defendStatus(DefMulKita, 1))
    ;
        retract(defendStatus(DefMulKita, DefMulLawan)),
        assertz(defendStatus(1, DefMulLawan))
    ).

toggle_turn :-
    ( myTurn -> retractall(myTurn) ; assertz(myTurn) ).

defend :-
    ( myTurn ->
        defendStatus(DefMulKita, DefMulLawan),
        retractall(defendStatus(_, _)),
        assertz(defendStatus(1.3, DefMulLawan)),
        statusKita(CurHP, _, _, _, Name, _, _, _),
        write(Name), write(' bertahan! DEF naik 30% untuk 1 turn.'), nl
    ;
        defendStatus(DefMulKita, DefMulLawan),
        retractall(defendStatus(_, _)),
        assertz(defendStatus(DefMulKita, 1.3)),
        statusLawan(CurHP, _, _, _, Name, _, _),
        write(Name), write(' bertahan! DEF naik 30% untuk 1 turn.'), nl
    ),
    toggle_turn, turn.

attack :-
    predict_enemy_defend,             % Prediksi dulu
    damage_skill(1, neutral),         % Baru serang
    defendStatus(DefMulKita, _),
    retract(defendStatus(_, _)),
    assertz(defendStatus(DefMulKita, 1)),
    (
        enemy_predefend ->
            retractall(enemy_predefend)
        ;
            toggle_turn
    ), turn.

skill(SkillNumber) :-
    myTurn,
    atkindex(Index),
    statusKita(_, _, _, _, NamaPokemon, _, _, Index),
    level(Level, NamaPokemon, Index, _, 1),
    cooldown_kita(CD1, CD2),
    pokeSkill(NamaPokemon, Skill1, Skill2),
    (
        SkillNumber =:= 1 ->
            ( CD1 > 0 ->
                write('Skill 1 masih cooldown '), write(CD1), write(' turn.'), nl, fail
            ;
                NamaSkill = Skill1,
                NewCD1 = 2,
                NewCD2 = CD2
            )
    ;
        SkillNumber =:= 2 ->
            ( Level < 5 ->
                write('Skill 2 hanya dapat digunakan jika level minimal 5!'), nl, fail
            ;
                ( CD2 > 0 ->
                    write('Skill 2 masih cooldown '), write(CD2), write(' turn.'), nl, fail
                ;
                    NamaSkill = Skill2,
                    NewCD1 = CD1,
                    NewCD2 = 3
                )
            )
    ;
        write('Skill tidak valid! Pilih 1 atau 2.'), nl, fail
    ),
    % Jalankan skill
    skills(NamaSkill, AtkType, Power, Ability, Chance),
    predict_enemy_defend,
    format('~w used ~w!~n', [NamaPokemon, NamaSkill]),

    ( Power > 0 ->
        damage_skill(Power, AtkType)
    ; true ),

    apply_ability(Ability, Chance),

    % Update cooldown setelah penggunaan
    retractall(cooldown_kita(_, _)),
    assertz(cooldown_kita(NewCD1, NewCD2)),
    defendStatus(DefMulKita, _),
    retract(defendStatus(_, _)),
    assertz(defendStatus(DefMulKita, 1)),
    (
        enemy_predefend ->
            retractall(enemy_predefend)
        ;
            toggle_turn
    ),
    turn.


damage_skill(Power, Elmt) :-
    calculate_damage(Power, Damage, Elmt),
    ( myTurn ->
        retract(statusLawan(CurHP, MaxHP, ATK, DEF, Defender, 99, Type)),
        NewHP is max(0, CurHP - Damage),
        assertz(statusLawan(NewHP, MaxHP, ATK, DEF, Defender, 99, Type))
    ; 
        atkindex(Index),
        retract(statusKita(CurHP, MaxHP, ATK, DEF, Defender, ID, Type, Index)),
        NewHP is max(0, CurHP - Damage),
        assertz(statusKita(NewHP, MaxHP, ATK, DEF, Defender, ID, Type, Index)),
        retract(curr_health(Index, Defender, _, 1)), assertz(curr_health(Index, Defender, NewHP, 1))
    ),
    format('Serangan memberikan ~w damage ke ~w. Sisa HP: ~w~n', [Damage, Defender, NewHP]), nl.

damage_skill_all_exceptatkindex(Power, Elmt) :-
    atkindex(AtkIndex),
    ( integer(Power) -> P = Power ; P is round(Power) ),
    statusLawan(_, _, ATK, _, _, _, _),
    defendStatus(DefMul, _),
    damage_loop(1, AtkIndex, P, Elmt, ATK, DefMul).

damage_loop(5, _, _, _, _, _) :- !.  % selesai iterasi sampai 4
damage_loop(I, AtkIndex, Power, Elmt, ATK, DefMul) :-
    ( I =\= AtkIndex ->
        (
            ( party(I, Defender),
              poke_stats(_, _, DEFD, Defender, I, 1),
              curr_health(_, Defender, CurHP, 1),
              type(Type, Defender),
              (
                  superEffective(Elmt, Type) -> Mult = 1.5,
                      write('It is very effective...'), nl
              ;   notEffective(Elmt, Type) -> Mult = 0.5,
                      write('It is not very effective...'), nl
              ;   Mult = 1
              ),
              Temp is float(DEFD) * float(DefMul),
              TempRounded is round(Temp),
              DEFAdj is max(1, TempRounded),
              DamageFloat is Mult * Power * ATK / float(DEFAdj) / 5,
              Damage is max(1, round(DamageFloat)),
              retract(curr_health(I, Defender, CurHP, 1)),
              NewHP is max(0, CurHP - Damage),
              assertz(curr_health(I, Defender, NewHP, 1)),
              format('Serangan memberikan ~w damage ke ~w. Sisa HP: ~w~n', [Damage, Defender, NewHP])
            ) -> true ; true
        )
    ; true),
    I1 is I + 1,
    damage_loop(I1, AtkIndex, Power, Elmt, ATK, DefMul).

calculate_damage(Power, Damage, Elmt) :-
    ( integer(Power) -> P = Power ; P is round(Power) ),
    ( myTurn ->
        statusKita(_, _, ATK, _, _, _, _, _),
        statusLawan(_, _, _, DEFD, _, _, Type),
        defendStatus(_, DefMul)
    ;
        statusLawan(_, _, ATK, _, _, _, _),
        statusKita(_, _, _, DEFD, _, _, Type, _),
        defendStatus(DefMul, _)
    ),
    ( superEffective(Elmt, Type) ->
        Mult = 1.5,
        write('It is very effective...'), nl
    ; notEffective(Elmt, Type) ->
        Mult = 0.5,
        write('It is not very effective...'), nl
    ;
        Mult = 1
    ),
    number(DEFD), number(DefMul),
    Temp is float(DEFD) * float(DefMul),
    TempRounded is round(Temp),
    DEFAdj is max(1, TempRounded),
    DamageFloat is Mult * P * ATK / float(DEFAdj) / 5,
    Damage is max(1, round(DamageFloat)).

apply_ability(none, _) :- !.
apply_ability(_, Chance) :- random(X), X > Chance, !.
apply_ability(burn(T, D), _) :-
    ( myTurn -> assertz(statusEfekLawan(burn(T, D))) ; atkindex(Index), assertz(statusEfekKita(Index, burn(T, D))) ),
    write('Efek burn diterapkan!'), nl.
apply_ability(lower_atk(N), _) :-
    ( myTurn ->
        statusLawan(HP, MaxHP, ATK, DEF, Nama, ID, Type),
        NewATK is max(1, ATK - N),
        retract(statusLawan(HP, MaxHP, ATK, DEF, Nama, ID, Type)),
        assertz(statusLawan(HP, MaxHP, NewATK, DEF, Nama, ID, Type)),
        write(Nama), write(' ATK berkurang sebanyak '), write(N), nl
    ;
        statusKita(HP, MaxHP, ATK, DEF, Nama, ID, Type, Index),
        NewATK is max(1, ATK - N),
        retract(statusKita(HP, MaxHP, ATK, DEF, Nama, ID, Type, Index)),
        assertz(statusKita(HP, MaxHP, NewATK, DEF, Nama, ID, Type, Index)),
        write(Nama), write(' ATK berkurang sebanyak '), write(N), nl
    ).
    
apply_ability(paralyze, _) :-
    ( myTurn -> assertz(statusEfekLawan(paralyze)) ; atkindex(Index), assertz(statusEfekKita(Index, paralyze)) ),
    write('Efek paralysis diterapkan! Mungkin gagal menyerang.'), nl.

apply_ability(heal(Ratio, Turns), _) :-
    ( myTurn ->
        statusKita(CurHP, MaxHP, ATK, DEF, Nama, ID, Type, Index),
        Heal is round(MaxHP * Ratio),
        NewHP is min(MaxHP, CurHP + Heal),
        retract(statusKita(CurHP, MaxHP, ATK, DEF, Nama, ID, Type, Index)),
        assertz(statusKita(NewHP, MaxHP, ATK, DEF, Nama, ID, Type, Index)),
        retract(curr_health(Index, Nama, CurHP, 1)),
        assertz(curr_health(Index, Nama, NewHP, 1)),
        write(Nama), write(' memulihkan '), write(Heal), write(' HP!'), nl,
        assertz(statusEfekKita(Index, sleep(Turns))),
        write('Efek sleep diterapkan ke '), write(Nama),
        write('! Akan tertidur selama '), write(Turns), write(' turn.'), nl
    ;   
        statusLawan(CurHP, MaxHP, ATK, DEF, Nama, ID, Type),
        Heal is round(MaxHP * Ratio),
        NewHP is min(MaxHP, CurHP + Heal),
        retract(statusLawan(CurHP, MaxHP, ATK, DEF, Nama, ID, Type)),
        assertz(statusLawan(NewHP, MaxHP, ATK, DEF, Nama, ID, Type)),
        write(Nama), write(' memulihkan '), write(Heal), write(' HP!'), nl,
        assertz(statusEfekLawan(sleep(Turns))),
        write('Efek sleep diterapkan ke '), write(Nama),
        write('! Akan tertidur selama '), write(Turns), write(' turn.'), nl
    ).
    

apply_ability(area, _) :-
    enemyskill(NamaSkill),
    skills(NamaSkill, AtkType, Power, _, _),
    damage_skill_all_exceptatkindex(Power, AtkType).

% ----------------------
% Efek Turn
% ----------------------

apply_turn_effects :-
    reduce_cooldown,
    ( myTurn ->
        statusLawan(_, _, _, _, _, ID, _),
        apply_burn(ID),
        apply_paralyze(ID)
    ;
        statusKita(_, _, _, _, _, ID, _, _),
        apply_burn(ID),
        apply_paralyze(ID)
    ).

reduce_cooldown :-
    ( myTurn ->
        \+ statusEfekKita(_, sleep(_)),
        cooldown_kita(CD1, CD2),
        NewCD1 is max(0, CD1 - 1),
        NewCD2 is max(0, CD2 - 1),
        retract(cooldown_kita(_, _)),
        assertz(cooldown_kita(NewCD1, NewCD2))
    ;
        \+ statusEfekLawan(sleep(_)),
        cooldown_lawan(CD1, CD2),
        NewCD1 is max(0, CD1 - 1),
        NewCD2 is max(0, CD2 - 1),
        retract(cooldown_lawan(_, _)),
        assertz(cooldown_lawan(NewCD1, NewCD2))
    ).

% Burn Effect
apply_burn(ID) :-
    ( myTurn ->
        situation(ongoing),
        \+ immune_status(ID),
        \+ statusEfekLawan(burn(0, 3)),
        statusEfekLawan(burn(T, D)),
        statusLawan(CurHP, MaxHP, ATK, DEF, Nama, 99, Type),
        NewHP is max(0, CurHP - D),
        retract(statusLawan(CurHP, MaxHP, ATK, DEF, Nama, 99, Type)),
        assertz(statusLawan(NewHP, MaxHP, ATK, DEF, Nama, 99, Type)),
        NewT is T - 1,
        retract(statusEfekLawan(burn(T, D))),
        (NewT > 0 -> assertz(statusEfekLawan(burn(NewT, D))) ; true),
        write(Nama), write(' terkena burn! -'), write(D), write(' HP'),
        format('. Sisa HP: ~w~n', [NewHP]), nl, nl
    ;
        situation(ongoing),
        \+ immune_status(ID), atkindex(Index), 
        \+ statusEfekKita(Index, burn(0, D)),
        statusEfekKita(Index, burn(T, D)),
        statusKita(CurHP, MaxHPKita, ATKKita, DEFKita, Nama, ID, Type, Index),
        NewHP is max(0, CurHP - D),
        retract(statusKita(CurHP, MaxHPKita, ATKKita, DEFKita, Nama, ID, Type, Index)),
        assertz(statusKita(CurHP, MaxHPKita, ATKKita, DEFKita, Nama, ID, Type, Index)),
        NewT is T - 1,
        retract(statusEfekKita(Index, burn(T, D))),
        (NewT > 0 -> assertz(statusEfekKita(Index, burn(NewT, D))) ; true),
        write(Nama), write(' terkena burn! -'), write(D), write(' HP'), 
        format('. Sisa HP: ~w~n', [NewHP]), nl, nl
    ).
    
apply_burn(_) :- true.  % fallback bila tidak ada efek burn

% Paralyze Effect
apply_paralyze(ID) :-
( myTurn ->
        situation(ongoing),
        \+ immune_status(ID), atkindex(Index), 
        statusEfekKita(Index, paralyze),
        statusKita(_, _, _, _, Nama, ID, _, Index),
        random_between(1, 10, X),
        ( X < 2 ->
        write(Nama), write(' terserang paralysis! Tidak bisa menyerang.'), nl, retractall(statusEfekKita(Index, paralyze)),
        toggle_turn  % menghentikan aksi berikutnya (misal: menyerang)
            ; true )
    ;
        situation(ongoing),
        \+ immune_status(ID),
        statusEfekLawan(paralyze),
        statusLawan(_, _, _, _, Nama, 99, _),
        random_between(1, 10, X),
        ( X < 2 ->
        write(Nama), write(' terserang paralysis! Tidak bisa menyerang.'), nl, retractall(statusEfekLawan(paralyze)),
        toggle_turn  % menghentikan aksi berikutnya (misal: menyerang)
    ; true )
    ), !.
apply_paralyze(_).  % fallback bila tidak ada efek paralyze

% Menggabungkan tiga list manual, menggantikan append/2
combine_actions([], [], L, L).
combine_actions([], L2, L3, R) :- combine(L2, L3, R).
combine_actions(L1, [], L3, R) :- combine(L1, L3, R).
combine_actions(L1, L2, L3, R) :-
    combine(L1, L2, T),
    combine(T, L3, R).

combine([], L, L).
combine([H|T], L, [H|R]) :- combine(T, L, R).

enemy_action :-
    statusLawan(_, _, _, _, NamaPokemon, _, _),
    enemy_level(Level),
    cooldown_lawan(CD1, CD2),
    pokeSkill(NamaPokemon, S1, S2),
    findall(A,
        ( member(A, [1,2,3,4]),
          ( A = 3 -> CD1 =:= 0 ; A = 4 -> CD2 =:= 0, Level >= 5 ; true )),
        Actions),
    ( Actions == [] -> Action = 2 ; random_member(Action, Actions) ),
    (
      Action = 1 ->
        (enemy_predefend ->
            true
        ;
            defend )% defend saja, selesai
    ; Action = 2 -> attack  % attack biasa
    ; Action = 3 -> 
        enemy_use_skill(S1),
        retractall(cooldown_lawan(_, _)),
        assertz(cooldown_lawan(2, CD2))
    ; Action = 4 ->
        enemy_use_skill(S2),
        retractall(cooldown_lawan(_, _)),
        assertz(cooldown_lawan(CD1, 3))
    ).

enemy_use_skill(NamaSkill) :-
    retractall(enemyskill(_)),
    statusLawan(_, _, _, _, Name, _, _),
    skills(NamaSkill, AtkType, Power, Ability, Chance),
    assertz(enemyskill(NamaSkill)),
    format('~w used ~w!~n', [Name, NamaSkill]),
    ( Power > 0 ->
        damage_skill(Power, AtkType)
    ; true ),
    apply_ability(Ability, Chance),
    retractall(enemyskill(_)),
    toggle_turn,
    turn.

predict_enemy_defend :-
    statusLawan(_, _, _, _, NamaPokemon, _, _),
    enemy_level(Level),
    cooldown_lawan(CD1, CD2),
    pokeSkill(NamaPokemon, _, _),
    findall(A,
        ( member(A, [1,2,3,4]),
          ( A = 3 -> CD1 =:= 0 ; A = 4 -> CD2 =:= 0, Level >= 5 ; true )),
        Actions),
    ( Actions == [] -> Action = 2 ; random_member(Action, Actions) ),
    (
        Action =:= 1 ->
            % Simulasi AI memilih defend
            defendStatus(DefMulKita, _),
            retract(defendStatus(_, _)),
            assertz(defendStatus(DefMulKita, 1.3)),
            statusLawan(_, _, _, _, Name, _, _),
            format('~w bersiap bertahan! DEF naik 30%% untuk turn ini.~n', [Name]),
            assertz(enemy_predefend)  % Tandai musuh sudah act
        ;
            true
    ).

kalahkan_pokemon :-
    handle_item_drop,
    party_slots_remaining(Remaining),
    statusLawan(_, HP, ATK, DEF, Pokemon, _, _),
    enemy_level(Level),
    (Remaining > 0 ->
        Idx is 5 - Remaining, 
        assertz(poke_stats(HP, ATK, DEF, Pokemon, Idx, 1)),
        assertz(level(Level, Pokemon, Idx, 0, 1)), 
        add_to_party(Idx, Pokemon),
        assertz(curr_health(Idx,Pokemon,HP, 1)),
        format('~w tertangkap dan masuk ke party!~n', [Pokemon])
    ;
        (   item_inventory(Index, pokeball(empty)) ->
            retract(item_inventory(Index, pokeball(empty))),
            assertz(poke_stats(HP, ATK, DEF, Pokemon, Index, 0)),
            assertz(level(Level, Pokemon, Index, 0, 0)), 
            assertz(item_inventory(Index, pokeball(filled(Pokemon)))),
            assertz(curr_health(Index,Pokemon,HP, 0)),
            format('~w tertangkap dan disimpan di Poke Ball slot ~w~n', [Pokemon, Index])
        ;   
            format('Tidak ada Poke Ball kosong! Gagal menangkap ~w~n', [Pokemon]), fail
        )
    ).
