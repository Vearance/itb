quiz_pokemon(NamaPokemon) :-
    nl, nl,
    write('Apa tipe element dari pokemon yang akan kamu lawan?'), nl, write('>> '),
    read(Answer), nl,
    type(X, NamaPokemon),
    ( X == Answer ->
        write('Jawaban benar!'), nl,
        statusKita(_, _, _, _, Nama, _, _, Slot),
        retract(level(Level, Nama, Slot, Exp, 1)),
        random_between(1, 100, R),
        (R =< 95 ->
            Exp1 is Exp + 1000000
        ;
            Exp1 is Exp + 25
        ),
        assertz(level(Level, Nama, Slot, Exp1, 1)),
        write('Exp '), write(Nama), write(' bertambah 25 exp! ('), write(Exp), write(' -> '), write(Exp1), write(')'), nl
    ;
        write('Jawaban salah. Tipe yang benar adalah: '), write(X), nl
    ).
