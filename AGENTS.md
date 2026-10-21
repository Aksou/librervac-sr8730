# LibreRVAC sr8730 — Instructions agent

Firmware alternatif GPLv3 pour l'aspirateur robot **Samsung SR8730**.
Firmware d'origine du projet : [LibreRVAC](https://github.com/LibreRVAC).

## Cible hardware

- MCU : **PIC32MX360F256L**, 80 MHz (`FOSC 80000000`, `FPB_DIV 1`, voir `settings.h`)
- Ressources : **256 Ko flash / 32 Ko RAM** — extrêmement serrées. Toute feature doit tenir dans le budget RAM.
- Compilateur : **MPLAB XC32** (Microchip, gcc-based). Obligatoire : `-fgnu89-inline` (sinon la plib legacy ne compile pas, voir `main.h`). Le Makefile cible `/opt/microchip/xc32/current` par défaut (`XC32_DIR` surchargeable).
- **Legacy plib** : Microchip ne la livre plus avec XC32 (retirée depuis v1.40). Elle est **vendored** dans `pic32libs/` (headers `plib.h` + `peripheral/*.h` + `libmchp_peripheral_32MX360F256L.a`), extraite de MPIDE 0150 — voir `pic32libs/README.md`. Le Makefile l'utilise par défaut (`PLIB_DIR` surchargeable).
- Pas de RTOS : boucle coopérative, callbacks enregistrés dans `main.c` (`set_callback_*`), `step()` appelé en continu par `cord_init.c`.

## Architecture (3 couches)

1. **Ce dépôt** — drivers bas niveau : `pinout.h` (toutes les broches), `gpio.h`, `multiplexer.h`/`mulpinout.h`, `timing.c`, `uart.c` (UART bit-bang), `motors.c` (PWM 20 kHz), `sound.c`, `leds.c`, `battery.c`, `charging.c`, `sensors.c`, `bumper.c`, `wheels.c` (quasi vide).
2. **`cordlib/`** (vendored — fork de `LibreRVAC/librervac-cordlib`, avec le fix `motor`/`move` pour XC32) — framework : protocole **CORD** (JSON sur UART, séparateur `0x1E`, auto-détection de débit), machine à callbacks, commandes `{"c":"motor"|"move"|"beep"}`, événements, rapports périodiques, modes `MODE_IDLE/WORKING/SLEEPING/DOCKING` (déclarés, pas encore utilisés).
3. **`jsmn/`** (vendored) — parser JSON embarqué.
4. **`pic32libs/`** (vendored) — legacy peripheral library Microchip pour le PIC32MX360F256L.

Esprit du projet : le PIC32 dialogue avec une **carte Linux/WiFi embarquée** via CORD (voir `PIN_LEGACY_LINUX_BOARD_*` dans `pinout.h`).

## État des lieux (audit initial)

### Fonctionnel
- Pinout complet, GPIO, multiplexeurs (IC16 + IC_SENSORS)
- Timing 1 ms (core timer)
- UART bit-bang : RX par interruption change-notice + auto-baud, TX par Timer4
- Moteurs : PWM 20 kHz via OC1/OC2/OC4 + Timer3, deadband 0.65, inversion de sens par broche PHASE
- Buzzer (OC5 + Timer2), LEDs + afficheur 7 segments (tickés dans l'ISR Timer1)
- Batterie : tension (avant/après shunt), courant, température thermistance, calibration
- Détection station de charge (`PIN_ADAPTER_DETECT`) + signal IR « charge me » 38 kHz généré quand docké (`sensors.c`)
- Bumper gauche/droit avec debounce 100 ms → événement CORD
- Protocole CORD fonctionnel (commandes motor/move/beep, événements)

### Manquant / TODO (≈50 TODO|FIXME|XXX hors submodules)
- **Pas de lecture des encodeurs de roues** → pas d'odométrie (`wheels.c` à remplir)
- `cord_move`/`cord_move_arc` = placeholders (arrêt fixe 200 ms, pas de PID)
- Machine de modes déclarée mais inutilisée
- **Pas de WiFi, pas de cartographie, pas de navigation intelligente, pas de persistance**
- Capteurs IR dock L/R, capteurs de chute « bottom », roue soulevée : définis dans `pinout.h`, **non lus**
- IMU : alimentation prévue (`PIN_ACCELGYRO_POWER`), aucun driver ; `spi.c` existe mais inutilisé
- Bugs notés : reset `ticks = 0` toutes les 5 s dans `uart.c` (« SHITTY WORKAROUND »), `UpdateCoreTimer(FOSC/1000/2)` suspect dans `timing.c`, debounce dock perfectible

## Conventions de code

- C89/C90 + extensions GNU tolérées ; SDCC. Pas de C99 moderne (déclarations mixtes limitées).
- Fichiers par module : `truc.c` + `truc.h`, en-tête GPLv3 standard (copier le bloc d'un fichier existant).
- Macros GPIO : `CONFIGURE_PIN(PIN_X, INPUT|OUTPUT)`, `DIGPIN(PIN_X, on|off)`, `READPIN(PIN_X)`, `MULSEL`/`MULPIN` pour les multiplexeurs.
- `#pragma once` dans les headers. `#pragma config` uniquement dans `main.c`.
- Style : accolades K&R, 4 espaces, variables `snake_case`.
- **Ne pas ajouter de dépendances** sauf si le dépôt les utilise déjà (cordlib, jsmn, pic32libs uniquement).
- `cordlib/`, `jsmn/` et `pic32libs/` sont **vendored** (plus de submodules) : les modifier directement dans ce dépôt.

## Build et test

- **Build cible** : `make` → XC32 (`xc32-gcc`), produit `dist/firmware.hex/.elf/.map`.
- **Tests host** : `make test` — gcc + stubs PIC32 (`tests/stubs/xc.h`), 6 suites (motors, battery, charging, protocol, bumper, uart). **Jamais de flash sans `make test` vert.**
- **Lint** : `make lint` — cppcheck (warning, performance, portability).
- **Package** : `make package` — archive `dist/librervac-sr8730-firmware.tar.gz`.
- **CI** : `.github/workflows/ci.yml` — jobs `lint`, `test`, `build` (XC32 via `usimd/setup-xc-action`), `package`, sur push/PR.
- Le sandbox n'a **pas** XC32 installé : la compilation croisée se fait chez l'utilisateur ou en CI.

## Objectif du projet

Mener à complétion le firmware alternatif : base temps-réel solide (odométrie, asservissement, sécurité), **WiFi**, **cartographie permanente**, **navigation intelligente** (couverture + retour au dock).

**Priorité retenue par le porteur** : WiFi / application de contrôle d'abord, puis autonomie intelligente.

Voir `docs/plan.md` pour le plan de tâches détaillé et les pistes d'implémentation.
