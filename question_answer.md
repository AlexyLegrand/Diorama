# Diorama — Questions et réponses de conception

Récapitulatif des décisions prises lors des séances de questions sur la direction du jeu.
Chaque question indique la proposition faite, la réponse donnée et son statut.

**Statuts** : ✅ décidé · 🕓 à revoir plus tard · ❓ sans réponse pour l'instant

---

## Vision du projet

### Q1 — Objectif du projet ✅
- **Proposé** : sortie gratuite sur itch.io, qui sert aussi de portfolio.
- **Réponse** : projet perso pour le portfolio, mais un **vrai jeu correct et fonctionnel**, jouable
  par toute personne qui veut tester un jeu, et pas un simple prototype.

### Q2 — Durée de jeu ✅
- **Proposé** : environ 1 h (4 mondes de 5 à 6 niveaux).
- **Réponse** : à long terme, une **vraie expérience de plus de 2 h**.

### Q3 — Joueur cible ✅
- **Proposé** : public détente, avec quelques niveaux bonus difficiles en option.
- **Réponse** : **public détente**, à l'image de *Captain Toad: Treasure Tracker*.

### Q6 — Distribution ✅
- **Proposé** : page itch.io avec des builds Windows et Linux prêts à télécharger.
- **Réponse** : oui, un jeu **pour tout le monde**, téléchargeable sans compiler.

---

## Contenu

### Q4 — Thèmes des mondes ✅
- **Proposé** : chaque monde a une identité visuelle et une nouvelle mécanique.
- **Réponse** : un **thème différent par monde** (par exemple monde 2 glace, monde 3 feu…),
  et **chaque monde a ses propres mécaniques**.

### Q7 — Taille du jeu ✅
- **Proposé** : 6 mondes de 8 niveaux, plus 1 bonus par monde (9 entrées par carte, la limite actuelle).
- **Réponse** : oui, **dans l'idéal on part sur ce principe**.

### Q9 — Collectibles 🕓
- **Proposé** : 3 pousses cachées par niveau, avec des bonus débloqués selon le total récolté.
- **Réponse** : les niveaux actuels **ne sont pas assez grands** (en hauteur et en largeur) pour cacher des collectibles.
- **Suite** : dépend de Q14 (agrandir la grille).

### Q10 — Liste des mondes 🕓
- **Proposé** :

  | Monde | Thème | Nouvelle mécanique |
  |---|---|---|
  | 1 | Prairie | les bases : escaliers, échelles, caisses, manivelle |
  | 2 | Glace | sol glissant : Sprout et les caisses glissent jusqu'à un obstacle |
  | 3 | Feu / volcan | lave infranchissable, refroidie par une caisse de pierre ou un interrupteur |
  | 4 | Forêt / jungle | lianes à escalader, plantes qui poussent quand on les active |
  | 5 | Ciel / nuages | téléporteurs, plateformes qui apparaissent et disparaissent |
  | 6 | Cristal / ruines | mélange de tout, plus des interrupteurs à combiner |

- **Réponse** : à garder en tête, **on verra au fur et à mesure de la création**.

### Q11 — Le monde 2 actuel (« Ravins ») 🕓
- **Proposé** : déplacer ses 2 niveaux (caisses et manivelle) à la fin du monde 1 et réserver le monde 2 à la glace.
- **Réponse** : le monde 2 est **en création**, il n'y a **rien de concret** pour l'instant.

### Q5 — Fin du jeu ✅
- **Proposé** : une scène où Sprout plante les graines dorées récoltées, puis les crédits.
- **Réponse** : une fin est **à créer**, par exemple **une scène 3D**.

### Q13 — Pourquoi des graines dorées ❓
- **Proposé** : Sprout récolte les graines pour faire repousser un grand arbre. L'arbre grandit sur
  l'écran titre à chaque monde terminé, et la scène finale le montre en fleurs. Sans dialogues.
- **Réponse** : —

---

## Gameplay

### Q8 — Annulation de coup ✅
- **Proposé** : annulation illimitée (Z / Ctrl+Z), pour qu'un joueur détente ne soit jamais coincé.
- **Réponse** : **non pour l'instant**, on garde le principe de **recommencer le niveau (R)**.

### Q15 — Indice quand le joueur est bloqué ❓
- **Proposé** : afficher « Bloqué ? Appuie sur R pour recommencer » après environ 20 s d'inactivité dans un niveau.
- **Réponse** : —

---

## Technique et accessibilité

### Q14 — Agrandir la grille ❓
- **Proposé** : relever la limite de 8×6×8 à 12×10×12 (constantes `LEVEL_MAX_*` dans `src/world/level.h`).
  Le coût est négligeable, et on laisse la porte ouverte à des niveaux plus grands et aux collectibles.
- **Réponse** : —

### Q16 — Manette ❓
- **Proposé** : support manette (stick ou croix pour marcher, caméra au stick droit ou aux gâchettes,
  A pour valider, B pour revenir). raylib le gère nativement.
- **Réponse** : —

### Q17 — Langue ❓
- **Proposé** : français et anglais au choix dans les paramètres, avec une vraie police qui gère les accents
  (aujourd'hui « Parametres », faute d'accents dans la police par défaut).
- **Réponse** : —

### Q18 — Plateformes ❓
- **Proposé** : builds Windows et Linux uniquement, macOS plus tard si quelqu'un peut le compiler.
- **Réponse** : —

### Q12 — Production des visuels par thème ❓
- **Proposé** : un dossier de modèles `.glb` par thème (par exemple `assets/themes/ice/block.glb`), avec les mêmes
  noms de fichiers d'un thème à l'autre. Le jeu charge le bon jeu de modèles selon le monde, et reprend
  le modèle de la Prairie s'il en manque un.
- **Réponse** : —

---

## Décisions déjà appliquées dans le jeu

- **Musique** : seulement 2 musiques. `diorama_ambiance.wav` joue sur l'écran titre et dans les paramètres,
  `diorama_ambiance_version_longue.wav` dans tout le reste du jeu.
- **Mondes** : plusieurs mondes reliés par des portails. Un monde s'ouvre quand tous les niveaux du précédent sont terminés.
- **Sauvegarde** : la progression est enregistrée dans `progress.dat`.
- **Transitions** : fondu au noir avec « Chargement... » entre l'écran titre, la carte et les niveaux.
