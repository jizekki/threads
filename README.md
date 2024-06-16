# Système exploitation s8

Membres du projet :
- Saad Abidi
- David Gond
- Jalal Izekki
- Eloi Magon De La Villehuchet
- Vincent Ridacker

Ce projet de système d'exploitation réalisé à l'ENSEIRB-MATMECA consiste à implémenter une bibiliothèque de gestion de threads en espace utilisateur.

Pour commencer le processus de compilation, il suffit de créer un dossier `build`, s'y déplacer puis lancer `cmake ..` afin de démarrer l'outil CMake.

Dans ce dossier, il est possible de faire appel à Make afin de réaliser les tâches suivantes :

- `make` compile les versions des tests utilisant notre bibliothèque et `pthread`
- `make check` compile les tests utilisant notre bibliothèque
- `make install` compile comme `make check`, crée un dossier `install` à la racine du projet et y crée 3 dossiers :
  - `bin` contient les exécutables des tests utilisant notre librairie
  - `lib` contient notre libraire partagée
  - `include` contient les headers `queue.h` des listes BSD et `threads.h` de notre interface
- `make valgrind` compile puis exécute l'outil `valgrind` sur les tests utilisant notre bibliothèque
- `make pthreads` compile les tests utilisant `pthread`
- `make graph` compile puis lance notre script python générant les graphiques d'exécution afin de comparer les performances entre notre bibliothèque et `pthread`

