:- dynamic(mewtwo_defeated/0).

/* Pembuka cerita di final stage */
print_end_game_opening :-
    nl,
    write('The air crackles with psychic energy...'), nl,
    write('A powerful presence approaches!'), nl, nl,
    write('Suddenly, a vortex of dark energy swirls before you,'), nl,
    write('and from it emerges...'), nl, nl,
    pokemon_ascii(10),
    nl, nl, nl,
    write('=========================================================='), nl,
    write('                |||    FINAL BATTLE    |||                '), nl,
    write('=========================================================='), nl, nl,
    write('I see now that the circumstances of one`s birth are irrelevant.'), nl,
    write('It is what you do with the gift of life that determines who you are.'), nl,
    write('My powers are far beyond yours!'), nl,
    write('The ultimate life-form... MEWTWO!'), nl, nl.

/* buat mewtwo */
buat_lawan_mewtwo :-
    Nama = mewtwo,
    Level = 20,
    Level1 is Level -1,
    base_stats(HPBase, ATKBase, DEFBase, Nama),
    MaxHP is HPBase + 2 * Level1,
    ATK is ATKBase + 1 * Level1,
    DEF is DEFBase + 1 * Level1,
    type(Type, Nama),
    retractall(statusLawan(_, _, _, _, _, _, _)),
    assertz(statusLawan(MaxHP, MaxHP, ATK, DEF, Nama, 99, Type)),
    retractall(enemy_level(_)),
    assertz(enemy_level(Level)),
    /* pokemon(ID, Nama, _), */
    /* pokemon_ascii(ID), */
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

/* check keadaan end game */
check_endgame :-
    ( situation(win) -> print_win_message
    ; situation(lose) -> print_lose_message
    ; true
    ).


/* print pesan kekalahan */
print_lose_message :-
    nl,
    write('========================================================'), nl,
    write('                    YOU HAVE FALLEN...                  '), nl,
    write('========================================================'), nl, nl,
    write('... to the mighty MEWTWO!'), nl,
    write('Your journey ends here, brave trainer.'), nl, nl,
    write('Return stronger. A true Master never gives up!'), nl.

/* print pesan kemenangan */
print_win_message :-
    nl,
    write('========================================================'), nl,
    write('                CONGRATULATIONS, CHAMPION!              '), nl,
    write('========================================================'), nl, nl,
    write('MEWTWO slowly rises, a newfound respect in its eyes:'), nl,
    write('"I have never encountered a trainer with such strength'), nl,
    write('and bond with their POKeMON. You have proven yourself'), nl,
    write('worthy. The world is safe in your hands."'), nl, nl,
    write('You have defeated the legendary MEWTWO and proven yourself'), nl,
    write('as the true POKeMON Master of this region!'), nl, nl,
    write('Until we meet again in your next adventure...'), nl, nl.
