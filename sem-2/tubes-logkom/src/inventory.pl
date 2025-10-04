:- dynamic(item_inventory/2).

max_party_size(4).

/* Inisialisasi inventori */
initialize_inventory :-
    initialize_inventory(0).

initialize_inventory(40) :- !.
initialize_inventory(Index) :-
    (Index < 20 -> Item = pokeball(empty); Item = empty),
    assertz(item_inventory(Index, Item)),
    NextIndex is Index + 1,
    initialize_inventory(NextIndex).

/* Menambahkan item */
add_item(Item) :-
    (   item_inventory(Index, empty) ->
        retract(item_inventory(Index, empty)),
        assertz(item_inventory(Index, Item)),
        format('Item ~w ditambahkan ke slot ~w~n', [Item, Index])
    ;   write('Inventori penuh! Tidak bisa menambahkan item.'), nl
    ).

/* Menggunakan Poke Ball */
use_pokeball :-
    (   item_inventory(Index, pokeball(empty)) ->
        retract(item_inventory(Index, pokeball(empty))),
        assertz(item_inventory(Index, empty)),
        write('Poke Ball digunakan.'), nl
    ;   write('Tidak ada Poke Ball kosong!'), nl
    ).

/* Menangkap Pokemon */
catch_pokemon(Pokemon) :-
    party_slots_remaining(Remaining),
    remaining_moves(Remaining1),
    LevelMin is min(14, max(2, round(2 + (20 - Remaining1) / 2))),
    LevelMax is min(14, max(LevelMin, round(4 + (20 - Remaining1) / 1.5))),
    random_between(LevelMin, LevelMax, Level),
    base_stats(HPBase, ATKBase, DEFBase, Pokemon),
    HP is HPBase + 2 * Level,
    ATK is ATKBase + 1 * Level,
    DEF is DEFBase + 1 * Level,
    Leve1 is Level + 1,
    (Remaining > 0 ->
        Idx is 5 - Remaining, 
        assertz(poke_stats(HP, ATK, DEF, Pokemon, Idx, 1)),
        assertz(level(Leve1, Pokemon, Idx, 0, 1)), 
        add_to_party(Idx, Pokemon),
        assertz(curr_health(Idx,Pokemon,HP, 1)),
        format('~w tertangkap dan masuk ke party!~n', [Pokemon])
    ;
        (   item_inventory(Index, pokeball(empty)) ->
            retract(item_inventory(Index, pokeball(empty))),
            assertz(poke_stats(HP, ATK, DEF, Pokemon, Index, 0)),
            assertz(level(Leve1, Pokemon, Index, 0, 0)), 
            assertz(item_inventory(Index, pokeball(filled(Pokemon)))),
            assertz(curr_health(Index,Pokemon,HP, 0)),
            format('~w tertangkap dan disimpan di Poke Ball slot ~w~n', [Pokemon, Index])
        ;   
            format('Tidak ada Poke Ball kosong! Gagal menangkap ~w~n', [Pokemon]), fail
        )
    ).

/* Menampilkan inventori */
showBag :-
    nl, write('=== Isi Inventory (40 slot) ==='), nl,
    show_inventory(0).

show_inventory(40) :- !.
show_inventory(Index) :-
    (   item_inventory(Index, Item) -> true ; Item = kosong ),
    Index1 is Index + 1,
    format('Slot ~w : ', [Index1]),
    (Item == empty -> write('Kosong') ; write(Item)),
    nl,
    Next is Index + 1,
    show_inventory(Next).

/* Handle item drop setelah pertarungan */
handle_item_drop :-
    random(0, 100, RandInt),
    Rand is RandInt / 100.0,
    (   Rand =< 0.75 ->
        random_item(Item),
        add_item(Item),
        format('Item ~w didapatkan!~n', [Item])
    ;   true
    ).

/* Item yang mungkin didapat */
random_item(Item) :-
    Items = [potion, super_potion, hyper_potion, pokeball(empty)],
    random_memberinven(Item, Items).

/* Memilih elemen acak dari list */
random_memberinven(Item, List) :-
    length(List, Length),
    random(0, Length, Index),
    nth0(Index, List, Item).


add_to_party(Index, Pokemon) :-
    findall(P, party(_, P), List),
    length(List, Len),
    max_party_size(Max),
    (Len < Max ->
        assertz(party(Index, Pokemon))
    ;
        write('Party penuh!'), nl, fail
    ).

party_slots_remaining(Remaining) :-
    findall(P, party(_, P), List),
    length(List, Len),
    max_party_size(Max),
    Remaining is Max - Len.

setParty(Inven, Party):-
    Party1 is Party + 1,
    poke_stats(HPi, ATKi, DEFi, Pokemoni, Inven, 0),
    poke_stats(HPp, ATKp, DEFp, Pokemonp, Party1, 1),
    curr_health(Inven, Pokemoni, CurrHpi, 0),
    curr_health(Party1, Pokemonp, CurrHpp, 1),
    level(Leveli, Pokemoni, Inven, Counteri, 0),
    level(Levelp, Pokemonp, Party1, Counterp, 1),
    party(Party1,Pokemonp),
    retract(item_inventory(Inven, pokeball(filled(Pokemoni)))),

    /* Ambil pokemon inven */
    retract(poke_stats(HPi, ATKi, DEFi, Pokemoni, Inven, 0)),
    retract(curr_health(Inven, Pokemoni, CurrHpi, 0)),
    retract(level(Leveli, Pokemoni, Inven, Counteri, 0)),

    /* replace pokemon party ke inven */
    assertz(poke_stats(HPp, ATKp, DEFp, Pokemonp, Inven, 0)),
    assertz(curr_health(Inven, Pokemonp, CurrHpp, 0)),
    assertz(level(Levelp, Pokemonp, Inven, Counterp, 0)),
    assertz(item_inventory(Inven, pokeball(filled(Pokemonp)))),   

    /* Ambil pokemon party */
    retract(poke_stats(HPp, ATKp, DEFp, Pokemonp, Party1, 1)),
    retract(curr_health(Party1, Pokemonp, CurrHpp, 1)),
    retract(level(Levelp, Pokemonp, Party1, Counterp, 1)),
    retract(party(Party1,Pokemonp)),

    /* replace pokemon inven ke party */
    assertz(poke_stats(HPi, ATKi, DEFi, Pokemoni, Party1, 1)),
    assertz(curr_health(Party1, Pokemoni, CurrHpi, 1)),
    assertz(level(Leveli, Pokemoni, Party1, Counteri, 1)),
    assertz(party(Party1,Pokemoni)).

:- dynamic(inv/1).
:- dynamic(hw/2).

/* Initiating inv and poke inv */
init_bag(Width, Height) :- generate_bag(Width, Height), assertz(hw(Width,Height)).

/* showBag: Print inv to console */
showBag1 :-
    inv(Bag),!,
    nl,nl, write('=========================== My Inventory ============================'),nl,nl,
    print_bag_rows(Bag, 0).

print_bag_rows([], _).
print_bag_rows([Row|T], I) :-
    print_row_b(Row, I, 0),
    nl,
    I1 is I + 1,
    print_bag_rows(T, I1).

print_row_b([], _, _).
print_row_b([H|T], RowIdx, ColIdx) :-
    write(H), write(' '),
    C1 is ColIdx + 1,
    print_row_b(T, RowIdx, C1).

generate_bag(Width, Height) :-
    retractall(inv(_)),
    generate_row_bs_b(1, Height, Width, Bag),
    assertz(inv(Bag)).

/* generate_row_bs_b: Create row */
generate_row_bs_b(I, Height, _, []) :- I > Height.
generate_row_bs_b(I, Height, Width, [Row|T]) :-
    I =< Height,
    generate_row_b(I, Width, Row),
    I1 is I + 1,
    generate_row_bs_b(I1, Height, Width, T).

/* generate_row_b: Create column per Row */
generate_row_b(RowIndex, Size, Row) :-
    Size1 is Size / 2,
    Size2 is ceiling(Size1),
    ( 0 is RowIndex mod 2 ->
        line_b(Size, Row)      % even rows → lines
    ;   tile_b(Size2, RowIndex, Row)  % odd rows → items
    ).


line_b(0, []).
line_b(N, ['+'|T]) :-
    N > 0,
    N1 is N - 1,
    line_b_zero_b(N1, T).

line_b_zero_b(0, []).
line_b_zero_b(N, ['--------------'|T]) :-
    N > 0,
    N1 is N - 1,
    line_b(N1, T).

tile_b(0, _, []).
tile_b(N, RowIdx, ['|'|T]) :-
    N > 0,
    N1 is N - 1,
    tile_b_zero(N1, RowIdx, 0, T).
    
tile_b_zero(0, _, _, []).
tile_b_zero(N, RowIdx, ColIdx, [Item|T]) :-
    columns(Columns),  
    Index is ((RowIdx-1)//2) * Columns + ((ColIdx-1)//2),
    ( item_inventory(Index, Item) -> true ; Item = empty),
    Col1 is ColIdx + 1,
    N1 is N - 1,
    tile_b_zero(N1, RowIdx, Col1, T).


columns(8). /* Assume always 8 * 5 just like in the specs :p */