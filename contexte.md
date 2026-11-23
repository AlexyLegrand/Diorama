# Diorama — Contexte du projet

## Le principe

**Diorama** est un jeu de réflexion en 3D voxel, écrit en C99 avec [raylib](https://www.raylib.com/).

Le joueur incarne **Sprout**, un petit personnage-pousse, qui explore de minuscules îles flottantes
au-dessus d'une mer de nuages. Chaque niveau est un **diorama** : une petite scène en blocs, vue
de l'extérieur comme une maquette posée sur une table. L'objectif est toujours le même :
**atteindre la graine dorée** placée quelque part dans le niveau.

Le défi ne vient pas de l'adresse mais de la réflexion : il faut comprendre comment
utiliser les éléments du décor (escaliers, échelles, caisses, manivelles, plateformes) pour
se frayer un chemin jusqu'à la graine.

## Boucle de jeu

1. **Écran titre** : Jouer (ou Continuer et Nouvelle partie quand une sauvegarde existe), Paramètres,
   Quitter, avec le logo 3D posé au-dessus de l'île. Nouvelle partie demande confirmation avant d'effacer.
2. **Carte du monde** : un diorama « hub » où Sprout marche librement entre les entrées de niveaux
   (dalles numérotées). Se placer sur une dalle et valider lance le niveau.
3. **Niveau** : résoudre le puzzle pour atteindre la graine dorée.
4. **Victoire** : Sprout fait son animation de victoire, la graine est collectée, le niveau est
   marqué comme terminé (dalle dorée sur la carte) et le niveau suivant est débloqué.
5. Menu de victoire : **Niveau suivant** (s'il en reste dans le monde) ou **Retour à la carte**.

À tout moment sur la carte ou dans un niveau, **Échap** ouvre le **menu pause** : Reprendre,
Recommencer (dans un niveau), Paramètres, puis Retour à la carte (niveau) ou Menu principal (carte).
6. Quand tous les niveaux d'un monde sont terminés, une **fusée** apparaît sur le portail de la carte :
   en validant dessus, Sprout monte dedans et décolle vers le monde suivant (animation et son synchronisés).

Les niveaux se débloquent dans l'ordre : le niveau *N* s'ouvre quand le niveau *N − 1* est terminé.
Les mondes aussi : un monde s'ouvre quand tous les niveaux du précédent sont terminés. On peut
revenir à tout moment au monde précédent par son portail.
Les transitions entre la carte et les niveaux se font par un fondu au noir.

## Les mécaniques

### Monde en grille

Tout le monde est une grille 3D de cellules (au maximum 12 de large × 10 de haut × 12 de profondeur). Sprout se déplace **case par case**,
dans les quatre directions. Il ne peut se tenir que sur une case vide posée sur un bloc
« marchable » (sol, caisse, plateforme). Il n'y a ni saut ni chute libre pour le joueur.

### Déplacement relatif à la caméra

Les touches de déplacement sont interprétées **par rapport à la caméra** : « avancer » envoie
Sprout vers le fond de l'écran, quelle que soit l'orientation de la scène. La caméra tourne
par crans de 45° et la direction est arrondie à l'axe de la grille le plus proche.

### Escaliers

Un escalier (`^ > v <`, orienté) permet de monter ou descendre d'un étage en une seule action,
en marchant dans le sens de la marche.

### Échelles

Une échelle (`n e s w`, collée à un mur) permet de grimper plusieurs étages d'un coup.
Sprout monte jusqu'en haut puis descend sur la case derrière le sommet. On peut aussi
redescendre une échelle en marchant vers elle depuis le haut.

### Caisses

Marcher contre une caisse la **pousse** d'une case, si la case derrière est libre.
Une caisse poussée dans le vide **tombe** jusqu'au premier support : elle sert ainsi
à combler un trou. Elle ne peut pas servir de marche, car Sprout ne monte que par un escalier ou une échelle.

### Manivelle et plateformes

Pousser contre une **manivelle** (`k`) la fait tourner. Cela fait **pivoter de 90°** toutes les
plateformes (`p`) du niveau autour de leur centre. C'est le moyen de construire un pont
ou de dégager un passage. La rotation est bloquée si Sprout se tient sur la plateforme
ou si une case d'arrivée est occupée.

### Graine dorée

La graine (`*`) est l'objectif. Il suffit de s'arrêter dessus pour terminer le niveau.

## Ce qui est disponible aujourd'hui

### Contenu

Les mondes sont listés dans `levels/worlds.txt`, un dossier par monde.

**Monde 1 « Prairie »** (tutoriel, une mécanique par niveau)

| Niveau | Mécanique principale |
|---|---|
| Niveau 1 | Échelle : grimper sur un pilier pour atteindre la graine |
| Niveau 2 | Escalier et échelle combinés |
| Niveau 3 | Caisse à pousser dans un trou pour faire un pont |
| Niveau 4 | Manivelle qui fait pivoter une plateforme pour créer un pont |

**Monde 2 « Ravins »** (les mécaniques se compliquent)

| Niveau | Mécanique principale |
|---|---|
| Niveau 1 | Deux tranchées, deux caisses : aligner la première caisse avant de la pousser |
| Niveau 2 | Tourner le pont avant de pousser la caisse dessus, sinon le niveau est bloqué |
| Niveau 3 | Niveau de test à la taille maximale (12 × 10 × 12) : longue traversée puis grande échelle |

En plus des niveaux : une **carte** par monde, reliées par des portails, et une **scène de titre**.

### Visuel

- Modèles 3D `.glb` faits dans Blender : Sprout, blocs, caisse, escalier, échelle, manivelle, graine.
- Sprout est animé : marche, escalade, poussée, manivelle, victoire et attente (idle).
- Végétation décorative animée par le vent : arbres ronds et pins, buissons, fougères, herbe,
  fleurs, champignons, lierre, lianes.
- Décor de ciel propre à chaque niveau (fichier `.sky`) : mer de nuages, soleil, nuages,
  îles flottantes, arc-en-ciel, oiseaux.
- Éclairage et ombrage des blocs.

### Caméra

- Rotation autour du diorama par crans de 45° (flèches gauche et droite).
- Trois inclinaisons : rasante, 45° et vue de dessus (flèches haut et bas).
- Zoom à la molette de la souris.

### Audio

- `diorama_ambiance.wav` : musique de l'écran titre et des paramètres.
- `diorama_ambiance_version_longue.wav` : musique de la carte et des niveaux. Elle continue
  sans repartir de zéro entre la carte et les niveaux.
- Les musiques baissent en fondu pendant les transitions.
- Effets sonores (`assets/music/player`, `world`, `ui`) : pas (4 variantes tirées au hasard), échelle
  (un son par étage), début de poussée et frottement continu de la caisse, chute de caisse, manivelle et rotation continue des plateformes, butée contre un obstacle,
  victoire et graine récoltée, navigation et validation dans les menus, volume, touche assignée,
  entrée de niveau, niveau verrouillé, transition, recommencer, monde débloqué.
- Ambiance de vent en boucle, discrète, sur la carte et dans les niveaux.
- Pas encore utilisés : `push.mp3`, `seed_idle.mp3` (scintillement de la graine) et `bird.mp3` (oiseau).

### Menus et paramètres

- Menus utilisables au clavier, et à la souris (survol, clic, clic droit pour revenir dans les paramètres).
- Progression (niveaux terminés, monde en cours) sauvegardée dans `progress.dat`.
- Paramètres sauvegardés dans `settings.dat` : volume de la musique, volume des effets sonores, plein écran, affichage des FPS,
  souris activée ou non, touches de déplacement et de recommencement modifiables
  (avec retour aux touches par défaut).

### Technique

- Compilation avec `make` sous Linux et sous Windows (MSYS2 UCRT64).
- Tests unitaires (`make test`) : déplacement, solution et pièges de chaque niveau du monde 2,
  et vérification que chaque monde a sa carte, ses portails et ses niveaux.
- Niveaux décrits en fichiers texte (voir le README), faciles à créer et à modifier.

## Limites actuelles et pistes

- Seulement 7 niveaux sur 2 mondes. Ajouter un niveau ou un monde ne demande que des fichiers (voir le README).
- Les thèmes des mondes se limitent au ciel : pas encore de blocs ni de végétation propres à chaque monde.
- La souris ne sert que dans les menus et pour le zoom : pas de clic sur la carte, ni de caméra à la souris.
- Pas d'annulation de coup : seul « recommencer le niveau » existe.
