/* Variable dynamic */
/* init: kondisi start game */
:- dynamic(init/0).
/* player/7: (nama, pokemon 1, pokemon 2, pokemon 3, pokemon 4, X_pos, Y_pos) */
:- dynamic(player/7).
/* mengatur inisialisasi banyak putaran yang bisa dilakukan player */

/* File lain */
:- include('interaction-map.pl').
:- include('ascii.pl').
:- include('variable.pl').
:- include('player.pl').
:- include('help.pl').
:- include('map.pl').
:- include('battle.pl').
:- include('inventory.pl').
:- include('end-game.pl').
:- include('side-quest.pl').

/* Start game*/
start :-
    ( current_predicate(init/0), init ->
        write('Game already started.'), nl
    ;
        init_starter, 
        assertz(init),
        asserta(pcentervisit(0)),
        init_moves(20),
        assertz(player(ash, 0, 0, 0, 0, 0, 0)),
        title, created_by, startgame(0)
    ).

exit :-
    ( current_predicate(init/0), init ->
        retractall(init),
        resetall,
        write('Game exited successfully.'), nl
    ;
        write('Game is not started.'), nl
    ).

resetall :-
        retractall(init),
        retractall(pcentervisit(_)),
        retractall(myTurn),
        retractall(situation(_)),
        retractall(statusKita(_,_,_,_,_,_,_,_)),
        retractall(statusLawan(_,_,_,_,_,_,_)),
        retractall(defendStatus(_,_)),
        retractall(statusEfekKita(_,_)),
        retractall(statusEfekLawan(_)),
        retractall(cooldown_kita(_,_)),
        retractall(cooldown_lawan(_,_)),
        retractall(player_level(_)),
        retractall(enemy_level(_)),
        retractall(atkindex(_)),
        retractall(enemyskill(_)),
        retractall(mewtwo_defeated),
        retractall(remaining_moves(_)),
        retractall(map(_)),
        retractall(last_player_tile(_)),
        retractall(item_inventory(_,_)),
        retractall(init),
        retractall(player(_,_,_,_,_,_,_)),
        retractall(pokemap(_)),
        retractall(party(_,_)),
        retractall(curr_health(_,_,_,_)),
        retractall(poke_stats(_,_,_,_,_,_)),
        retractall(level(_,_,_,_,_)),
        retractall(skill(_,_,_,_,_)).

/* update_name: input name and change the dynamic variable */
update_name(NewName):- player(_, Poke1, Poke2, Poke3, Poke4, X_pos, Y_pos),
    retract(player(_, Poke1, Poke2, Poke3, Poke4, X_pos, Y_pos)),
    assertz(player(NewName, Poke1, Poke2, Poke3, Poke4, X_pos, Y_pos)).

/* print_name: output Name on console */
print_name:- player(Name, _, _, _, _, _, _),
    write(Name).

/* startgame: intiating sequences of game intro */
startgame(X) :-
    step(X),
    wait_enter,
    X1 is X + 1,
    (X1 =< 10 -> startgame(X1); help, showMap).

/* step: sequences */
step(0) :- nl.
step(1) :- oak, nl, nl, write('|    Hello there welcome to the World of POKeMON!').
step(2) :- write('|    My name is OAK').
step(3) :- write('|    People call me the POKeMON PROF!').
step(4) :- pokemon_ascii(6), nl, nl, write('|    This world is inhabited by creatures called POKeMON!').
step(5) :- write('|    For some people POKeMON are pets.').
step(6) :- write('|    Others use them for fights.').
step(7) :- write('|    Myself...').
step(8) :- write('|    I study POKeMON as a profession.').
step(9) :- set_name, wait_enter. 
step(10) :- starter_pokemon, initialize_inventory, init_map(17,32), assertz(pcentervisit(0)).

/* wait_enter: press ENTER to continue */
wait_enter :- nl, nl, write('press [ENTER] to continue'), get_char(_), nl.

/* set_name: asking for name input */
set_name:- red, nl, nl, write('|    First, what is your name?'), nl, write('>> '), read(X), nl, update_name(X), 
    write('|    Right! So your name is '), print_name, write('!'), nl.

/* starter_pokemon: asking to choose starter pokemon */
starter_pokemon:- write('|    Choose your starter POKeMON. '), print_name, write('!'), nl, chooseStarter,!.

