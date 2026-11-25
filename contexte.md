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
- Logo 3D (`assets/logo/logo.glb`) posé au-dessus de l'île de l'écran titre, éclairé comme le décor.
- Fusée (`assets/rocket/rocket.glb`) animée avec feu et fumée pour voyager entre les mondes ;
  Sprout a une animation dédiée pour y monter et décoller.
- Jeu disponible en **français et en anglais** : tous les textes sont dans `src/core/i18n.c`, et les noms
  des mondes sont bilingues dans `levels/worlds.txt` (`Prairie|Meadow`).
- Police **Lilita One** (`assets/fonts/LilitaOne-Regular.ttf`, licence OFL dans `OFL.txt`) pour tous
  les textes, avec les accents français. Sans le fichier, le jeu reprend la police par défaut.

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
- Menu pause (Échap) sur la carte et dans les niveaux, avec accès aux paramètres sans quitter la partie.
- Écran titre : Continuer / Nouvelle partie (avec confirmation) quand une sauvegarde existe.
  Le menu titre est une rangée horizontale sous l'île (`TITLE_MENU_ANCHOR` dans `src/game/game.c`,
  `MENU_ANCHOR_CENTER` pour revenir à l'ancien menu centré).
- Fenêtre redimensionnable, interface qui s'adapte à la taille de la fenêtre, anticrénelage (MSAA 4×)
  et icône de fenêtre (`assets/logo/icon.png`).
- Menu de victoire : Niveau suivant ou Retour à la carte.
- Progression (niveaux terminés, monde en cours) sauvegardée dans `progress.dat`.
- Paramètres sauvegardés dans `settings.dat` : langue (français ou anglais), volume de la musique, volume des effets sonores, plein écran, affichage des FPS,
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

## Plan de développement

Les prochaines étapes, dans l'ordre :

1. **Collectibles**
   - Un collectible que l'on peut placer plusieurs fois dans un niveau.
   - Le même modèle pour tous les mondes, avec une couleur différente selon le monde.
   - **Utilité retenue : personnaliser Sprout.** Les collectibles débloqueront des accessoires
     (chapeau, écharpe de couleur…). Les graines dorées serviront plus tard au lore du jeu : faire
     repousser le grand arbre (écran titre et scène de fin).
   - **Fait** :
     - placement dans les niveaux avec le caractère `o` (jusqu'à 32 par niveau), récolte en passant dessus ;
     - la sauvegarde retient chaque collectible récolté, compte les graines (`seeds`, une par niveau
       terminé) et les collectibles (`collectibles`) ; un collectible ne compte qu'une fois ;
     - comme dans *Captain Toad*, un collectible déjà récolté lors d'une partie précédente reste visible
       en transparent ;
     - HUD : dans un niveau, « ● récoltés / total » dans la couleur du monde ; sur la carte, le total
       de graines dorées et de collectibles.
   - **Provisoire** : le collectible est une sphère de la couleur du monde, en attendant le modèle Blender ;
     le son `assets/music/world/collectible.mp3` est prévu mais pas encore créé (récolte silencieuse).
   - Reste à faire : le modèle, le son, et l'utilisation des compteurs (personnalisation de Sprout, arbre).
   - **Le collectible est le lotus de cristal** (`assets/collectibles/crystal_lotus.glb`), animé (Idle en
     boucle, Collect au ramassage). Ses parties en cristal prennent la teinte du monde (vert, bleu, orange,
     violet, jaune, turquoise) ; le pollen et les feuilles gardent leurs couleurs. Un lotus déjà récupéré
     lors d'une partie précédente reste visible en transparent.
   - **HUD (inspiré de *Captain Toad*, d'après `assets/hud/hud_mockup.png`)** :
     - **dans un niveau**, en haut à gauche : portrait animé de Sprout (planches `sprout_portrait_idle_sheet.png`
       et `sprout_portrait_happy_sheet.png`, l'animation joyeuse se joue au ramassage d'un lotus et à la
       victoire) avec « ×3 » vies en doré ; en dessous, une icône par lotus du niveau, en silhouette
       (`crystal_lotus_icon_empty.png`) tant qu'il n'est pas récupéré, en cristal (`crystal_lotus_icon.png`,
       recolorée selon le monde) une fois récupéré. Le titre du niveau est en haut au centre ;
     - **sur la carte du monde**, sous le nom du monde : le total de graines dorées et de lotus.
   - Sur la carte, la graine dorée utilise l'icône `assets/hud/golden_seed_icon.png`.
   - Reste à faire : le son de ramassage (`assets/music/world/collectible.mp3`) et l'utilisation des compteurs.
   - **Vies** : 3 vies à chaque entrée dans un niveau ; `game_lose_life()` en retire une, et à 0 le
     niveau recommence (pas de *game over*). Rien ne fait encore perdre de vie : les ennemis arriveront
     dans les prochains mondes.
2. **Distribution**
   - Builds Windows et Linux prêts à télécharger.
   - Page itch.io.
3. **Manette**
   - Jouer et naviguer dans les menus à la manette.
4. **Monde 2**
   - Commencer le contenu du monde 2 : thème, mécanique et niveaux.

Reporté à plus tard : écran de crédits (le jeu n'est pas encore partagé), noms des niveaux,
sons non utilisés (`push.mp3`, `seed_idle.mp3`, `bird.mp3`).
