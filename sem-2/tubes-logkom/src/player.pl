
/* Variable dynamic */
/* party: (Inventory index (start from 1), Pokemon_name) */
:- dynamic(party/2).
/* curr_health : (Indeks, Nama, CurrHP, Bool_party)*/
:- dynamic(curr_health/4).

/* Initiating to choose starter */
chooseStarter:-  findall(NamaX,starter(_, NamaX),ListStarter),
    retractall(curr_health(_, _, _, _)),
    writeList(ListStarter),
    repeat,
    write('Choose your POKeMON'),nl,
    /* Choosing 2 starter */
    write('>> '), read(Starter1), write('>> '), read(Starter2),
    ( Starter1 >= 1, Starter1 =< 3, Starter2 >= 1, Starter2 =< 3 -> !
    ;
        write('Indeks Pokemon yang Kamu Pilih Ada yang Tidak Valid'), nl,
        fail
    ),
    starterToInventory(Starter1, Starter2),!.

/* Putting starter to inventory */
starterToInventory(X, Y) :- 
    /* Adding choosen starter as new level dynamic variable */
    starter(X, NamaX), starter(Y, NamaY), level(Lev, NamaX, 0, 0, -1), asserta(level(Lev, NamaX, 1, 0, 1)),
    level(Lev, NamaY, 0, 0, -1), asserta(level(Lev, NamaY, 2, 0, 1)),
    /* Setting stats for choosen starter 1 (X) */
    base_stats(HP1, ATK1, DEF1, NamaX),
    asserta(poke_stats(HP1, ATK1, DEF1, NamaX, 1, 1)),
    assertz(curr_health(1, NamaX, HP1, 1)),
    add_to_party(1, NamaX),
    /* Setting stats for choosen starter 2 (Y) */
    base_stats(HP2, ATK2, DEF2, NamaY),
    asserta(poke_stats(HP2, ATK2, DEF2, NamaY, 2, 1)),
    assertz(curr_health(2, NamaY, HP2, 1)),
    add_to_party(2, NamaY),
    write(NamaX), write(' & '), write(NamaY), write(' is now your partner!'),nl, !.

/* Print all starter to console */
/* Basis */
writeList([]) :- nl,!.
/* Procedure */
writeList([H|T]) :-
    pokemon(N, H, _),
    pokemon_ascii(N),
    write('|    '),
    write(N),
    write('. '), write(H),nl,
    type(X,H),
    write('|    Type: '), write(X), nl, nl, nl,
    writeList(T).

/* initiating rint status of pokemon in inventory */
status :-
    write('Your Pokemon:'), nl,
    findall([N,X],party(N,X),ListInventory),
    /* Sorting by N (1-4) */
    sort(ListInventory,SortedList),
    showStatusList(SortedList),!. 

showStatusList([]) :- !.
/* Print status */
showStatusList([[No_invenH, XH]|T]) :-
    No_invenH > 0,
    party(No_invenH, XH),      % Jika predicate party dipakai seperti ini
    poke_stats(HP, _, _, _, No_invenH, 1),
    curr_health(No_invenH, XH, CurrHP, 1),
    pokemon(_, XH, Rarity),
    rarity(Rarity, BaseEXP, _, _),
    type(Type, XH),
    level(Lev, XH, No_invenH, Exp, 1),
    write('Name : '), write(XH), nl,
    write('Health: '), write(CurrHP), write('/'), write(HP), nl,
    write('Type: '), write(Type), nl,
    write('Level: '), write(Lev), nl,
    ExpCap is BaseEXP * Lev,
    write('Exp: '), write(Exp), write('/'), write(ExpCap), nl, nl,
    showStatusList(T).

/* Level up X at No_inven */
/* Procedure */
levelUp(X, No_inven) :-
    party(No_inven, X),
    pokemon(_, X, Rarity), level(Lev,X, No_inven, Exp, _),
    rarity(Rarity, BaseEXP, _, _), !,
    Exp >= (BaseEXP*Lev), statsUp(Lev, X, No_inven, BaseEXP),
    addExp(0, No_inven, X).

/* Evolve pokemon */
evolve(No_inven) :-
    retract(poke_stats(HP, ATK, DEF, X, No_inven, 1)),
    retract(level(Lev, X, No_inven, Counter, 1)),
    retract(curr_health(No_inven, X, CurrHp, 1)),
    retract(party(No_inven, X)),
    pokemon(ID, X, common),
    IdNew is ID + 3,
    pokemon(IdNew, NamaBaru, common),
    assertz(poke_stats(HP, ATK, DEF, NamaBaru, No_inven, 1)),
    assertz(level(Lev, NamaBaru, No_inven, Counter, 1)),
    assertz(curr_health(No_inven, NamaBaru, CurrHp, 1)),
    assertz(party(No_inven, NamaBaru)),
    format('Selamat ~w telah berhasil evolve menjadi ~w!~n', [X, NamaBaru]).
    

/* Changing the level and stats, reduce CurrentEXP by EXPCap */
statsUp(Lev, X, No_inven, BaseEXP):-
    /* Taking old dynamic variable and remove it */
    retract(level(Lev, X, No_inven, Exp, Boolean_party)),
    retract(poke_stats(HP, ATK, DEF, X, No_inven, Y)),
    /* Adding new dynamic variable */
    ExpCap is BaseEXP*Lev,
    NewExp is Exp - ExpCap,
    NewLev is Lev+1,
    assertz(level(NewLev, X, No_inven, NewExp, Boolean_party)),
    ATK1 is ATK+1,
    HP1 is HP+2,
    DEF1 is DEF+1,
    assertz(poke_stats(HP1, ATK1, DEF1, X, No_inven, Y)),
    write('Your '),write(X),write(' has leveled up!'),nl,
    write('Health: '), write(HP1),nl,
    write('ATK: '), write(ATK1),nl,
    write('DEF: '), write(DEF1), nl,
    ignore((NewLev >= 10 -> canevolve(X), evolve(No_inven); true)).

addExp(X, Idx, Nama) :- 
    level(Lev,Nama,Idx, Exp, 1),
    retract(level(Lev,Nama,Idx, Exp, 1)),
    Expnew is Exp + X, 
    assertz(level(Lev,Nama,Idx, Expnew, 1)),
    pokemon(_, Nama, Rarity),
    isLevelUp(Rarity,Lev, Expnew, Nama, Idx).

isLevelUp(Rarity, Lev, Counter, Nama, Idx) :-
    rarity(Rarity, BaseEXP, _, _),
    Counter >= BaseEXP * Lev,
    ( levelUp(Nama, Idx) -> true ; true ).
