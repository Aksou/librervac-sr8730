# Plan de développement — LibreRVAC sr8730

Objectif : mener le firmware à complétion — base temps-réel solide, **WiFi**, **cartographie permanente**, **navigation intelligente**.

Priorité retenue : **WiFi / appli de contrôle d'abord**, autonomie intelligente ensuite.

Légende : `[ ]` à faire · `[~]` en cours · `[x]` fait · `P1/P2/P3` priorité (P1 = bloquant pour la suite).

---

## Phase 0 — Fondations build & test (P1)

| # | Tâche | Détail |
|---|-------|--------|
| 0.1 | `[ ]` Créer le `Makefile` racine | Build SDCC croisé PIC32MX360F256L : sources racine + `cordlib/*.c` + `jsmn/jsmn.c`, flags `-fgnu89-inline`, sortie `.hex`. |
| 0.2 | `[ ]` Harnais de test host | Compiler les modules testables avec `gcc -DTESTING` (pattern déjà utilisé par cordlib). Rafraîchir `cordlib/t/` (API obsolète). |
| 0.3 | `[ ]` Fixer les bugs notés | `uart.c` : workaround `ticks = 0` toutes les 5 s ; `timing.c` : `UpdateCoreTimer(FOSC/1000/2)` suspect ; debounce dock dans `main.c`. |
| 0.4 | `[ ]` README sérieux | Architecture, build, flash, protocole CORD, état d'avancement. |

## Phase 1 — WiFi & contrôle à distance (P1, priorité porteur)

Deux pistes documentées ci-dessous ; décision d'implémentation après prototypage.

| # | Tâche | Détail |
|---|-------|--------|
| 1.1 | `[ ]` Étendre le protocole CORD | Nouvelles commandes : `{"c":"wifi"}`, `{"c":"map"}`, `{"c":"navigate"}`, `{"c":"dock"}`, `{"c":"pause"}` ; événements : position, batterie, carte partielle. Augmenter `PROTOCOL_BUFFER_SIZE`/`PROTOCOL_TOKENS` si besoin (budget RAM !). |
| 1.2 | `[ ]` Piste A — co-processeur WiFi/Linux | ESP32 / ESP8266 / RPi Zero W maître, relié au PIC32 par CORD (UART existant). Le co-processeur porte : WiFi, appli web/socket, carte en RAM, planification haut niveau. Le PIC32 reste temps-réel pur. **Recommandée** : zéro risque RAM/flash sur le PIC32, architecture d'origine du projet. |
| 1.3 | `[ ]` Piste B — WiFi sur le PIC32 | ESP8266 en commandes AT (ou SPI) directement sur l'UART/pins libres. Autonome mais très gourmand : pile TCP/IP + HTTP sur 32 Ko RAM. À réserver en fallback. |
| 1.4 | `[ ]` Application de contrôle minimale | Interface web/socket : joystick moteurs, start/stop nettoyage, retour dock, état batterie/capteurs, flux événements CORD. |
| 1.5 | `[ ]` Sécurité de la liaison | Watchdog de lien (si pas de commande pendant N s → arrêt moteurs), gestion déconnexion. |

## Phase 2 — Capteurs et odométrie (P1)

Prérequis de toute navigation intelligente.

| # | Tâche | Détail |
|---|-------|--------|
| 2.1 | `[ ]` Lire les encodeurs de roues | `wheels.c` : change-notice/interruptions sur `PIN_WHEEL_*_ENCODER_1/2`, comptage ticks, vitesse par roue. |
| 2.2 | `[ ]` Odométrie | `odometry.c` (nouveau) : intégration différentielle (avance + rotation) avec `WHEEL_GAP`/`WHEEL_RADIUS` de `params.h` (à calibrer). |
| 2.3 | `[ ]` Lire les capteurs de chute | `PIN_SENSOR_BOTTOM_*` (3 capteurs) : sécurité anti-chute, arrêt immédiat. |
| 2.4 | `[ ]` Lire les capteurs de roue soulevée | `PIN_WHEEL_LIFT_*` : arrêt des roues concernées. |
| 2.5 | `[ ]` Lire les récepteurs IR dock | `PIN_DOCKING_RECEIVER_LEFT/RIGHT` + `PIN_IR_RECEIVER_REAR_*` + `PIN_MID_RECEIVER_R` : base du homing. |
| 2.6 | `[ ]` Driver IMU (si présent) | Via `spi.c` (existant, inutilisé) sur `PIN_ACCELGYRO_POWER` : gyroscope pour la dérive angulaire. À confirmer sur le hardware. |
| 2.7 | `[ ]` Asservissement vitesse roues | Remplacer le placeholder `cord_set_speed_goal` par un vrai PI/PID sur consigne encodeurs. |

## Phase 3 — Navigation intelligente (P2)

| # | Tâche | Détail |
|---|-------|--------|
|3.1 | `[ ]` Remplacer `cord_move`/`cord_move_arc` | Vrais déplacements asservis en distance/angle via odométrie, événements `move-finished` fiables. |
| 3.2 | `[ ]` Réactions sécurité intégrées | Bumper, chute, roue soulevée, surintensité → arrêt + événement CORD (compléter `cord_event_bumper`). |
| 3.3 | `[ ]` Stratégie de couverture | Spirale puis boustrophedon (lignes), wall-following en secours. |
| 3.4 | `[ ]` Homing / retour au dock | Guidage par récepteurs IR dock + odométrie ; déclenché sur batterie faible ou commande. |
| 3.5 | `[ ]` Machine de modes | Activer `MODE_IDLE/WORKING/SLEEPING/DOCKING` de cordlib, transitions propres. |

## Phase 4 — Cartographie permanente (P2)

| # | Tâche | Détail |
|---|-------|--------|
| 4.1 | `[ ]` Modèle de carte | Grille d'occupation (résolution 5–10 cm/cellule, bits par cellule), stockée en mémoire externe ou flash. |
| 4.2 | `[ ]` Stockage — décision retenue | **Hybride** : settings/calibration (odométrie, seuils batterie) en **flash interne NVM** du PIC32 (writer/lecteur NVM à écrire, zone réservée en fin de flash) ; **carte complète sur le co-processeur** (piste A) ou **EEPROM I2C/SPI externe** (piste B). |
| 4.3 | `[ ]` Sérialisation carte | Format compact binaire sur CORD (événements `map-chunk`), reconstruction côté appli. |
| 4.4 | `[ ]` Fusion carte + odométrie | Mise à jour des cellules visitées pendant le nettoyage ; recalage simple (pas de SLAM complet sur 32 Ko RAM). |
| 4.5 | `[ ]` Persistance inter-sessions | Sauvegarde auto en fin de nettoyage / batterie faible ; restauration au boot. |

## Phase 5 — Finalisation (P3)

| # | Tâche | Détail |
|---|-------|--------|
| 5.1 | `[ ]` Mode basse consommation | `MODE_SLEEPING` : périphériques arrêtés, réveil sur bouton/événement. |
| 5.2 | `[ ]` Calibration assistée | Procédure guidée (encodeurs, batterie, boussole) via CORD, stockée en NVM. |
| 5.3 | `[ ]` Télémétrie complète | Batterie, capteurs, position, carte — flux périodique vers l'appli. |
| 5.4 | `[ ]` Documentation utilisateur | Flash, appli, protocole CORD pour développeurs. |
| 5.5 | `[ ]` CI | Build SDCC en CI (compile-only) + tests host. |

---

## Pistes d'implémentation détaillées

### WiFi

**Piste A — co-processeur WiFi/Linux (recommandée)**
- Hardware : ESP32 (~4 €, WiFi+BT, UART), ESP8266 (moins cher, moins puissant), ou RPi Zero W (Linux complet, appli riche).
- Liaison : UART existant (`PIN_CONNECTION_RX/TX`) — CORD est déjà le protocole.
- Rôle du co-processeur : WiFi, serveur web/WebSocket, stockage carte, planification haut niveau, envoi de commandes CORD haut niveau (`navigate`, `dock`).
- Rôle du PIC32 : temps réel inchangé (moteurs, capteurs, sécurité).
- Avantages : pas de pression RAM/flash sur le PIC32 ; appli riche ; mise à jour OTA du co-processeur.
- Inconvénients : coût + câblage ; dépendance au boot du co-processeur.

**Piste B — WiFi sur le PIC32**
- Hardware : ESP8266 (ESP-01) en commandes AT sur UART, ou module SPI.
- Contrainte : pile TCP/IP légère (ex. uIP) + HTTP — très serré sur 32 Ko RAM, probablement WebSocket exclu.
- Avantages : autonome, un seul board.
- Inconvénients : développement long, risque de blocage du temps réel, appli limitée.

### Cartographie permanente

| Option | Capacité | Persistance | Complexité | Recommandation |
|--------|----------|-------------|------------|----------------|
| Flash interne NVM (PIC32) | ~quelques Ko réservables | Oui | Moyenne (writer NVM, wear-leveling simple) | **Settings/calibration uniquement** |
| EEPROM I2C/SPI externe (ex. 24LC256) | 32 Ko | Oui | Faible | Carte si piste B |
| Co-processeur (piste A) | Illimitée (SD/RAM) | Oui | Faible côté PIC32 | **Carte complète si piste A** |

### Navigation intelligente

- Minimum viable : odométrie encodeurs + couverture boustrophedon + homing IR dock.
- Niveau suivant : IMU (gyro) pour corriger la dérive angulaire, mur-following, évitement dynamique.
- Ambitieux (hors scope PIC32 seul) : SLAM sur le co-processeur (lidar ou caméra) — le PIC32 n'exécute que les consignes de trajectoire.

## Suivi

Cocher les cases au fur et à mesure. Toute tâche commencée passe en `[~]`, terminée en `[x]` avec le commit/PR associé.
