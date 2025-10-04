/* Visual map */
:- dynamic(map/1).

/* Pokemon location map */
:- dynamic(pokemap/1).

/* Initiating map and poke map */
init_map(N, X):- generate_matrix(N, _), bushes(X), add_pokes, place_random_p, place_random_h.

/* showMap; Print map to console */
showMap :-
    map(Matrix),
    print_matrix_rows(Matrix, 0).

print_matrix_rows([], _).
print_matrix_rows([Row|T], I) :-
    print_row(Row, I, 0),
    nl,
    I1 is I + 1,
    print_matrix_rows(T, I1).

print_row([], _, _).
print_row([H|T], RowIdx, ColIdx) :-
    /* Print C if there is common and not a bush */
    (pokemap(PokeList), member((common, (RowIdx, ColIdx)), PokeList), H == ' ' -> write('C'); write(H)),
    write(' '),
    C1 is ColIdx + 1,
    print_row(T, RowIdx, C1).

generate_matrix(N, Matrix) :- retractall(map(_)),
    generate_rows(1, N, N, Matrix), 
    assertz(map(Matrix)).

/* generate_rows: Create row */
generate_rows(I, N, _, []) :- I > N.
generate_rows(I, N, Size, [Row|T]) :-
    I =< N,
    generate_row(I, Size, Row),
    I1 is I + 1,
    generate_rows(I1, N, Size, T).

/* generate_row: Create column per Row */
generate_row(RowIndex, Size, Row) :-
    /* Create line (-----) */
    1 is RowIndex mod 2, 
    line(Size, Row).

generate_row(RowIndex, Size, Row) :-
    /* Create tiles (-0-0-) */
    0 is RowIndex mod 2,
    tile(Size, Row).

line(0, []).
line(N, ['+'|T]) :-
    N > 0,
    N1 is N - 1,
    line_zero(N1, T).

line_zero(0, []).
line_zero(N, ['-'|T]) :-
    N > 0,
    N1 is N - 1,
    line(N1, T).

tile(0, []).
tile(N, ['|'|T]) :-
    N > 0,
    N1 is N - 1,
    tile_zero(N1, T).

tile_zero(0, []).
tile_zero(N, [' '|T]) :-
    N > 0,
    N1 is N - 1,
    tile(N1, T).

/* shuffle: Shuffle the order of list */
shuffle([], []).
shuffle(List, [Elem|Shuffled]) :-
    length(List, Length), Length > 0,
    random(0, Length, Index),
    /* nth0 return Elem of List at Index */
    nth0(Index, List, Elem),
    delete(List, Elem, Tail),
    shuffle(Tail, Shuffled).

/* bushes: Make N bushes */
bushes(N) :-
    map(Matrix),
    /* findall with 0 (tile) */
    findall((X,Y), (nth0(X, Matrix, Row), nth0(Y, Row, ' ')), Count0),
    length(Count0, Max0),
    MinN is min(N, Max0),
    shuffle(Count0, Shuffled),
    length(ToChange, MinN),
    append(ToChange, _, Shuffled),
    replace_positions(Matrix, ToChange, NewMatrix, '#'),
    retractall(map(_)),
    assertz(map(NewMatrix)).

/* replace_positions: Create NewMatrix based on Matrix and change Element of Position with Elmt */
replace_positions(Matrix, Positions, NewMatrix, Elmt) :-
    maplist_index(with_index, Matrix, 0, MatrixIndex),
    replace_positions_rows(Positions, MatrixIndex, ResultIndex, Elmt),
    maplist(remove_index, ResultIndex, NewMatrix).

replace_positions_rows([], MatrixIndex, MatrixIndex, _).
replace_positions_rows([Pos|Tail], MatrixIndex, Result, Elmt) :-
    replace_pos_in_matrix(Pos, MatrixIndex, TempMatrix, Elmt),
    replace_positions_rows(Tail, TempMatrix, Result, Elmt).

replace_pos_in_matrix(_, [], [], _).
replace_pos_in_matrix((XIdx, YIdx), [(XIdx, Row)|T], [(XIdx, NewRow)|T], Elmt) :-
    replace_in_list(YIdx, Elmt, Row, NewRow), !.
replace_pos_in_matrix(Pos, [H|T], [H|R], Elmt) :-
    replace_pos_in_matrix(Pos, T, R, Elmt).

replace_in_list(0, Elem, [_|T], [Elem|T]).
replace_in_list(I, Elem, [H|T], [H|R]) :-
    I > 0,
    I1 is I - 1,
    replace_in_list(I1, Elem, T, R).

with_index(Row, Index, (Index, Row)).
remove_index((_, Row), Row).

maplist_index(_, [], _, []).
maplist_index(Pred, [X|Xs], I, [Y|Ys]) :-
    call(Pred, X, I, Y),
    I1 is I + 1,
    maplist_index(Pred, Xs, I1, Ys).

take(N, List, Taken, Rest) :-
    length(Taken, N),
    append(Taken, Rest, List).

print_pokes :-
    pokemap(Pokes),
    forall(member((Type, (X,Y)), Pokes), (format("~w at (~d,~d)~n", [Type, X, Y]))).

map_pokes(_, [], []).
map_pokes(Type, [Pos|T], [(Type, Pos)|Rest]) :-
    map_pokes(Type, T, Rest).

exclude_positions([], _, []).
exclude_positions([H|T], Exclude, Result) :-
    member(H, Exclude), !,
    exclude_positions(T, Exclude, Result).
exclude_positions([H|T], Exclude, [H|R]) :-
    exclude_positions(T, Exclude, R).

add_pokes :-
    map(Matrix),
    findall((X,Y), (nth0(X, Matrix, Row), nth0(Y, Row, Tile), Tile = '#'), HashPositions),
    findall((X,Y), (nth0(X, Matrix, Row), nth0(Y, Row, Tile), (Tile = ' '; Tile = '#')), CommonCandidates),
    shuffle(HashPositions, ShuffledHash),
    take(1, ShuffledHash, LegendaryPos, Tail1),
    take(3, Tail1, EpicPos, Tail2),
    take(5, Tail2, RarePos, _),
    append(LegendaryPos, EpicPos, Temp1),
    append(Temp1, RarePos, TakenPositions),
    exclude_positions(CommonCandidates, TakenPositions, CommonAvailable),
    shuffle(CommonAvailable, ShuffledCommon),
    take(10, ShuffledCommon, CommonPos, _),
    map_pokes('legendary', LegendaryPos, L1),
    map_pokes('epic', EpicPos, L2),
    map_pokes('rare', RarePos, L3),
    map_pokes('common', CommonPos, L4),
    append(L1, L2, TempList1),
    append(TempList1, L3, TempList2),
    append(TempList2, L4, AllPokes),
    replace_positions(Matrix, TakenPositions, TempMatrix, '#'),
    replace_common_positions(TempMatrix, CommonPos, NewMatrix),
    retractall(map(_)),
    assertz(map(NewMatrix)),
    retractall(pokemap(_)),
    assertz(pokemap(AllPokes)).

replace_common_positions(Matrix, [], Matrix).
replace_common_positions(Matrix, [(X,Y)|T], ResultMatrix) :-
    nth0(X, Matrix, Row),
    nth0(Y, Row, Tile),
    ( Tile = ' ' -> replace_positions(Matrix, [(X,Y)], TempMatrix, 'C');   TempMatrix = Matrix),
    replace_common_positions(TempMatrix, T, ResultMatrix).

random_player_pos((X, Y)) :-
    map(Matrix),
    findall((I, J), (nth0(I, Matrix, Row), nth0(J, Row, ' ')), ZeroTiles), random_member((X, Y), ZeroTiles),
    update_player(X,Y).

place_random_p :-
    random_player_pos((X, Y)),
    map(Matrix),
    replace_in_matrix(Matrix, (X, Y), 'P', NewMatrix),
    retractall(map(_)),
    assertz(map(NewMatrix)).

replace_in_matrix(Matrix, (X, Y), Elem, NewMatrix):-
    nth0(X, Matrix, Row),
    replace_in_list(Y, Elem, Row, NewRow),
    replace_in_list(X, NewRow, Matrix, NewMatrix).

random_member(X, List):-
    length(List, Length),
    Length > 0,
    random(0, Length, Index),
    nth0(Index, List, X).

/* Additional Rules for PokeCenter */
random_pokecenter_pos((X, Y)) :-
    map(Matrix),
    findall((I, J), (nth0(I, Matrix, Row), nth0(J, Row, ' ')), ZeroTiles), random_member((X, Y), ZeroTiles).

place_random_h :-
    random_pokecenter_pos((X, Y)),
    map(Matrix),
    replace_in_matrix(Matrix, (X, Y), 'H', NewMatrix),
    retractall(map(_)),
    assertz(map(NewMatrix)).

/* Additional Information for Player and PokeCenter */
print_info_p :-
    map(Matrix),
    player(_, _, _, _, _, PX, PY),
    findall((I,J), (nth0(I, Matrix, Row), nth0(J, Row, Tile), Tile = 'H'), PcenterPositions),
    format("Player at (~d,~d)~n", [PX,PY]),
    forall(member((HX,HY), PcenterPositions), format("PokeCenter at (~d,~d)~n", [HX,HY])).

update_player(X, Y) :-
    retract(player(A, B, C, D, E, _, _)),        
    assertz(player(A, B, C, D, E, X,Y)).  