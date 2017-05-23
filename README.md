# posix-type-fast

## Short description

POSIX-compliant game _posix-type-fast_ is a simple game based on server-client architecture using `pthreads` and `TCP` protocol. Server handles connections and disconnections from many players, finds new games and send the results to players. This game is written to achieve as broad compliance with POSIX standards as possible. 

The aim of this game is to type words presented by server faster than the opponent. No indicator is presented whether opponent is faster or not so do your best! Use `telnet` to connect to server and play. Port is one of arguments to `server` application, e.g. `./server 2000 input_file.txt`.

## Technical info

All the parameters, e.g. `WORDS_GAME` - count of words to rewrite during a single game, or `MAX_CLIENTS` - maximum number of clients connected simultaneously can be changed in source code. Feel free to change this code to suit your needs.

The path to input file is a second argument to `server` application. Provide `server` with any file you like but check if every word is in a separate line. The number of words being used in program is also a parameter in program, called `WORDS_CNT`.

If client disconnects from server, e.g. `telnet` quits, especially the client starts any game, this client may be visible in ranking but the disconnection will be detected after pairing with an opponent - and opponent wins automatically. Disconnection _during the game_ is detected instantaneously. No timeouts are imposed.

## Additional info

Code is documented in English, as well as this repository is maintained in English, although the UI is translated to Polish. There are no plans to translate it to English as this game was created as a final project for UNIX cource on Warsaw University of Technology.

## In-game screenshots

Server's view:

![server]

Winner's view:

![client1]

Disconnected client:

![client2]

Client won by opponent's disconnection:

![client3]

[server]: imgs/server.png "Server's window"
[client1]: imgs/client1.png "Client's view"
[client2]: imgs/client2.png "Client's view"
[client3]: imgs/client3.png "Client's view"

## Technologies / parts of language used:

* C language, especially:
    * pthreads - application is multi-threaded,
        * mutexes,
        * conditional variables,
    * sockets, especially `TCP` protocol,
    * signal handling,
    * structures, 
    * and more.

## Author

Created by: Grzegorz Czarnocki  
Warsaw University of Technology  
**Final project for UNIX course**  
