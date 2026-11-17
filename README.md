# Diorama

Jeu de puzzle 3D en voxels, écrit en C99 avec [raylib](https://www.raylib.com/).
Guidez Sprout à travers de petits dioramas flottants : poussez des caisses, grimpez
escaliers et échelles, actionnez des manivelles pour déplacer des plateformes,
et atteignez la graine dorée.

## Compiler et lancer

Prérequis : `gcc`, `make` et raylib installé dans `/usr/local` (Linux / X11).

```sh
make        # compile ./diorama
make run    # compile puis lance
make test   # tests de mouvement
make clean  # supprime obj/ (fclean supprime aussi le binaire)
```

Le jeu se lance depuis la racine du projet (il lit `levels/` et `assets/` en chemins relatifs).

## Contrôles

| Action | Touche par défaut |
|---|---|
| Se déplacer | W A S D (Z Q S D en AZERTY) |
| Recommencer le niveau | R |
| Tourner la caméra / naviguer dans les menus | Flèches |
| Valider / entrer dans un niveau | Entrée ou Espace |
| Retour | Échap |

Les touches de déplacement et de redémarrage se changent dans **Paramètres**.
Les réglages sont sauvegardés dans `settings.dat`.

## Structure

```
src/
  core/       caméra orbitale, paramètres
  world/      chargement des niveaux, mouvement, plateformes, types de blocs
  render/     blocs, éclairage, végétation, ciel, décor de fond
  entities/   joueur, manivelle
  game/       états du jeu, mise à jour et rendu d'une frame
  ui/         menus, HUD, écran des paramètres
levels/       niveaux (.txt) et décor du ciel (.sky)
assets/       modèles .glb et sources Blender
tests/        tests unitaires
```

## Format des niveaux

Un niveau `levels/<nom>.txt` empile des couches horizontales séparées par une ligne
vide, de bas en haut. Chaque couche est une grille de caractères (max 8×6×8).

| Car. | Bloc | Car. | Végétation |
|---|---|---|---|
| `.` | vide | `T` | arbre rond |
| `@` | départ du joueur | `P` | sapin |
| `#` | sol | `B` | buisson |
| `c` | caisse | `F` | fougère |
| `*` | graine dorée (objectif) | `g` | herbe |
| `k` | manivelle | `f` | fleurs |
| `p` | plateforme | `m` | champignons |
| `^ > v <` | escalier (N E S O) | `I` | lierre (sur sol) |
| `n e s w` | échelle (N E S O) | `V` | liane (sur sol) |
| `1`–`9` | entrée de niveau (carte du monde) | | |

Le fichier `.sky` du même nom place le décor : une ligne par objet,
`type x y z échelle yaw` (position relative au centre du niveau), `#` pour les commentaires.

Pour ajouter un niveau : créer `level_N.txt` (+ `.sky`), l'ajouter à `LEVEL_PATHS`
dans `src/game/game.c`, incrémenter `LEVEL_COUNT` dans `src/game/game.h` et placer l'entrée `N` sur `levels/world_map.txt`.
