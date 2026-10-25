# Audit du projet — LibreRVAC SR8730

Date : 2026-10-21 · Branche : `vibe/agent-docs-06c3a4` · PR : #1

Ce document répond en détail à la demande initiale : **état des lieux du projet**, **pistes pour le WiFi**, **cartographie permanente** et **navigation intelligente**, plus l'**avis sur la qualité du code** et l'**orientation technique recommandée**.

---

## 1. Vue d'ensemble

| Aspect | Détail |
|---|---|
| Projet | Firmware alternatif open-source pour robot aspirateur **Samsung Navibot SR8730** |
| Dépôt | `Aksou/librervac-sr8730` (fork/mirror de `LibreRVAC/librervac-sr8730`) |
| Licence | **GPLv3** |
| Cible | **PIC32MX360F256L** @ 80 MHz — MIPS32 M4K, **256 Ko flash / 32 Ko RAM** |
| Toolchain | **MPLAB XC32** (gcc-based) — *pas SDCC* (correction faite en Phase 0) |
| RTOS | Aucun — boucle coopérative par callbacks |
| Architecture | 3 couches : firmware (racine) ↔ **cordlib** (protocole CORD) ↔ **jsmn** (JSON) |
| État | Firmware fonctionnel (moteurs, charge, bumper, UART bit-bang), reverse-engineering en cours |

### Origine

Le projet est né du reverse-engineering du firmware d'origine Samsung. Le protocole **CORD** (JSON sur UART bit-bangé, séparateur `0x1E`, auto-baud) est le protocole que le robot utilisait pour parler à une carte Linux/WiFi optionnelle. L'objectif : recréer un firmware libre complet, avec WiFi, cartographie et navigation.

---

## 2. Architecture logicielle

```
┌─────────────────────────────────────────────────────────┐
│  CO-PROCESSEUR (futur) — WiFi / Linux / appli de contrôle │  ← Phase 1+
│  (pins PIN_LEGACY_LINUX_BOARD_* — câblage prévu d'origine)│
└──────────────────────────┬──────────────────────────────┘
                           │  CORD : JSON sur UART bit-bang
                           │  (SEP = 0x1E, auto-baud 0xAA)
┌──────────────────────────▼──────────────────────────────┐
│  FIRMWARE (racine, ~2 500 lignes C)                      │
│  main.c · motors.c · wheels.c · sensors.c · battery.c    │
│  charging.c · bumper.c · buttons.c · uart.c · timing.c   │
│  spi.c · sound.c · leds.c · params.h · main.h            │
├─────────────────────────────────────────────────────────┤
│  CORDLIB (vendored, ~1 700 lignes)                       │
│  cord_protocol.c · cord_cordlib.c · cord_hw.c            │
│  cord_motors.c · cord_sensors.c · cord_bumper.c          │
│  cord_buttons.c · cord_wheels.c · cord_sound.c           │
│  cord_errors.c · cord_timing.c                           │
├─────────────────────────────────────────────────────────┤
│  JSMN (vendored) — parser JSON minimal                   │
└─────────────────────────────────────────────────────────┘
         │
         ▼  registres PIC32 (plib legacy, vendored dans pic32libs/)
    PIC32MX360F256L @ 80 MHz
```

### Boucle principale

`main()` initialise les périphériques, enregistre des callbacks (`set_callback_*`), puis boucle sur `cord_event_init()` qui appelle `step()` — chaque module a son `step()` appelé périodiquement. Pas de préemption, pas de scheduler : la coopération est manuelle.

---

## 3. Audit module par module (firmware racine)

| Module | Rôle | État | Commentaire |
|---|---|---|---|
| `main.c` | Init, boucle, callbacks, debounce dock | ✅ corrigé | Debounce dock maintenant edge-triggered (était over-callback) |
| `motors.c` | PWM moteurs (OC1-OC4), phases, rampes | ✅ testé | 19 checks, logique saine |
| `wheels.c` | Placeholder encodeurs | ⚠️ incomplet | `PIN_WHEEL_*_ENCODER_*` câblés mais pas lus — **Phase 2.1** |
| `sensors.c` | ADC batterie, IR, charge | ✅ corrigé | Shift 33 bits UB fixé (`1ULL`, `0x1F8C30CC3ULL`) |
| `battery.c` | Mesure V/I/T batterie | ✅ testé | 7 checks |
| `charging.c` | Détection dock, fin de charge | ✅ testé | 14 checks, charge CC/CV gérée |
| `bumper.c` | Capteurs choc + debounce | ✅ corrigé | Debounce réécrit — l'ancien réémettait un event /100 ms |
| `buttons.c` | Boutons utilisateur | ✅ OK | — |
| `uart.c` | UART bit-bang, auto-baud, trames | ✅ corrigé | Workaround `ticks=0` /5 s supprimé, `min()` non-portable remplacé |
| `timing.c` | Timer core, `get_time_ms` | ✅ corrigé | `UpdateCoreTimer(FOSC/1000)` — le timer avançait 2× trop vite |
| `spi.c` | SPI (pour IMU futur) | ⚠️ inutilisé | Présent mais aucune périphérique SPI active — **Phase 2.6** |
| `sound.c` | Bips / mélodies | ✅ OK | — |
| `leds.c` | LEDs état | ✅ OK | — |
| `params.h` | Constantes hardware (pins, timings) | ✅ OK | Bien documenté, base du reverse-engineering |

### Audit cordlib

| Module | Rôle | État | Commentaire |
|---|---|---|---|
| `cord_protocol.c` | Parse/envoi CORD, dispatch commandes | ✅ **bug critique corrigé** | `motor`/`move` étaient `#ifdef __SDCC` — **no-op sur XC32** |
| `cord_cordlib.c` | Init, boucle `step()`, états | ✅ OK | — |
| `cord_hw.c` | HAL : accès registres PIC32 | ⚠️ couplé | Mélange protocole et hardware — candidat au refactor Phase 0.5 |
| `cord_motors.c` | Commandes moteurs CORD | ✅ OK | — |
| `cord_sensors.c` | Événements capteurs CORD | ✅ OK | — |
| `cord_bumper.c` | Événements bumper CORD | ✅ OK | — |
| `cord_buttons.c` | Événements boutons CORD | ✅ OK | — |
| `cord_wheels.c` | Placeholder roues | ⚠️ incomplet | — |
| `cord_sound.c` | Commandes son CORD | ⚠️ `process_beep` encore `#ifndef __SDCC` | À corriger en Phase 0.5 (utilise `atof`) |
| `cord_errors.c` | Codes d'erreur | ✅ OK | — |
| `cord_timing.c` | Temps cordlib | ✅ OK | — |

---

## 4. Bugs trouvés et corrigés (Phase 0)

| # | Fichier | Bug | Sévérité | Fix |
|---|---|---|---|---|
| 1 | `cordlib/cord_protocol.c` | `process_motor()`/`process_move()` compilés uniquement sous SDCC (`#ifdef __SDCC` / `#ifndef __SDCC`) — **les commandes `motor` et `move` ne faisaient RIEN sur XC32** | 🔴 **Critique** | Guards supprimés, test de régression ajouté |
| 2 | `bumper.c` | Debounce réémettait un événement toutes les 100 ms tant que le bumper restait pressé | 🟠 Majeur | Réécrit : latche sur front stable ≥ 100 ms, rejette les rebonds |
| 3 | `timing.c` | `UpdateCoreTimer(FOSC/1000/2)` — le timer core avançait **2× trop vite**, toutes les temporisations doublées | 🟠 Majeur | `FOSC/1000` |
| 4 | `main.c` | Debounce dock sur-callback (appelait `dock_connect`/`disconnect` en boucle) | 🟡 Mineur | Edge-triggered avec `static bool dock_pin_state` |
| 5 | `uart.c` | Workaround `ticks = 0` toutes les 5 s ("SHITTY WORKAROUND") + `min()` non-portable | 🟡 Mineur | Supprimé / remplacé par comparaison explicite |
| 6 | `sensors.c` | Shift de 33 bits sur un littéral 33 bits = **UB** | 🟠 Majeur | `CHARGE_ME_SIGNAL = 0x1F8C30CC3ULL`, shift `1ULL <<` |

**Méthode** : ces bugs ont été trouvés par le harnais de tests host (90 checks) et cppcheck — pas par lecture seule. C'est la preuve que le projet était **non testable** avant la Phase 0.

---

## 5. Qualité du code — diagnostic franc

### ✅ Points forts

- **Compact** : ~64 Ko flash utilisés sur 256 Ko (25 %), ~9,4 Ko RAM statique sur 32 Ko (29 %) — marge confortable
- **Pas d'allocation dynamique** : 100 % mémoire statique, déterministe — choix sain pour un robot autonome
- **Architecture 3 couches** conceptuellement propre (firmware / protocole / JSON)
- **Pinout documenté** dans `params.h` — le reverse-engineering est capitalisé
- **Depuis la Phase 0** : testable (90 checks), linté (cppcheck 0 finding), CI 4 jobs verts

### ⚠️ Dette technique (héritée du reverse-engineering)

- **Watchdog DÉSACTIVÉ** (`FWDTEN = OFF` dans `main.c`) — risque #1 pour un robot autonome. Un blocage = robot immobilisé au milieu du salon
- **UART bit-bangée en polling** — fonctionnelle mais fragile (pas de DMA, auto-baud artisanal)
- **cordlib couplé au hardware** : `cord_hw.c` touche directement aux registres PIC32 — pas de séparation protocole/HAL, ce qui complique les tests et la portabilité
- **Style C89 daté** : `goto`/`volatile` éparpillés (39 occurrences), pas de `stdint.h`/`stdbool.h` systématiques
- **Boucle coopérative sans protection** : un `step()` qui bloque bloque tout le robot (d'où l'importance du watchdog)
- **`process_beep()` encore `#ifndef __SDCC`** dans cordlib — la commande `beep` est aussi un no-op sur XC32 (même bug que #1, pas encore corrigé car il utilise `atof`)
- **`cordlib/t/`** : tests Perl 6 obsolètes référençant une API morte (`hw_set_motor_utilization`) — non câblés en CI

### Verdict

Projet de **reverse-engineering fonctionnel**, qualité « hack embarqué » : ni mauvais, ni propre. La base matérielle est validée, la logique métier compréhensible. La dette est **structurelle, pas catastrophique**.

### Faut-il repartir d'une base saine ?

**Non.** Trois raisons :

1. **La connaissance hardware est dans ce code** — timings, séquences d'init, quirks du SR8730 (charge, dock, IR). Un rewrite les perd.
2. **Seul le porteur a le hardware** — tout code non testé sur la cible est un pari dangereux.
3. **Le projet est maintenant testable** — on peut refactorer morceau par morceau, test après test, sans risque.

→ **Refactor incrémental**, pas big bang.

### Migration vers Rust ?

**Sur le PIC32MX360 : non.**

| Critère | Réalité |
|---|---|
| Cible Rust | Pas de target officiel pour MIPS32 M4K — il faudrait un target JSON custom + HAL from scratch |
| Toolchain | XC32 reste le seul supporté par Microchip (plib, linker scripts, flash PICkit) |
| Gain sécurité | Faible — le code n'a **aucun malloc**, la mémoire est 100 % statique. Le vrai risque = blocage (watchdog), pas les fuites |
| Coût | ~4 500 lignes de C validées hardware → mois de réécriture non testable sur cible sans écosystème |

**Rust a du sens ailleurs** : sur le **co-processeur Linux** (WiFi, appli, mapping, navigation) — async, sécurité, écosystème riche. C'est exactement la piste prioritaire choisie.

---

## 6. Contraintes hardware (budget)

| Ressource | Total | Utilisé (firmware actuel) | Marge |
|---|---|---|---|
| Flash | 256 Ko | ~64 Ko (25 %) | ✅ 192 Ko libres |
| RAM statique | 32 Ko | ~9,4 Ko (29 %) | ✅ ~22 Ko libres |
| Fréquence | 80 MHz | — | — |

**Conséquence** : le PIC32 peut absorber odométrie, asservissement, protocoles étendus — mais **pas** une pile TCP/IP complète, ni un SLAM, ni une carte haute résolution. D'où l'architecture deux cerveaux.

---

## 7. Piste WiFi — les deux options documentées

### Piste A — Co-processeur WiFi/Linux ⭐ **recommandée**

| | |
|---|---|
| Hardware | **ESP32** (~4 €, WiFi+BT, UART) · ESP8266 (moins cher, moins puissant) · **RPi Zero W** (Linux complet, appli riche) |
| Liaison | UART existant (`PIN_CONNECTION_RX/TX` / `PIN_LEGACY_LINUX_BOARD_*`) — **CORD est déjà le protocole** |
| Rôle co-pro | WiFi, serveur web/WebSocket, stockage carte, planification haut niveau, commandes CORD haut niveau (`navigate`, `dock`) |
| Rôle PIC32 | Temps réel inchangé (moteurs, capteurs, sécurité) |
| Avantages | Zéro pression RAM/flash sur le PIC32 · appli riche · OTA du co-pro · **architecture d'origine du projet** (les pins Linux board existent sur le PCB) |
| Inconvénients | Coût + câblage · dépendance au boot du co-pro |

### Piste B — WiFi sur le PIC32 (fallback)

| | |
|---|---|
| Hardware | ESP8266 (ESP-01) en commandes AT sur UART, ou module SPI |
| Contrainte | Pile TCP/IP légère (uIP) + HTTP — **très serré sur 32 Ko RAM**, WebSocket probablement exclu |
| Avantages | Autonome, un seul board |
| Inconvénients | Développement long, risque de blocage du temps réel, appli limitée |

**Décision retenue** : Piste A (ESP32 ou RPi Zero W), Piste B en fallback. Le câblage `PIN_LEGACY_LINUX_BOARD_*` est déjà prévu sur le PCB d'origine.

---

## 8. Piste stockage — cartographie permanente

**Décision retenue : hybride** (réponse à la question posée).

| Donnée | Stockage | Pourquoi |
|---|---|---|
| Settings / calibration (odométrie, seuils batterie) | **Flash interne NVM** du PIC32 | Petit volume, persistant, zéro hardware ajouté. Writer/lecteur NVM à écrire, zone réservée en fin de flash, wear-leveling simple. |
| **Carte complète** | **Co-processeur** (Piste A : SD/RAM) ou **EEPROM I2C/SPI externe** 24LC256 (Piste B : 32 Ko) | Une carte 5–10 cm/cellule sur 20 m² = ~40–160 Ko — **impossible en flash interne** avec le firmware. |

| Option | Capacité | Persistance | Complexité | Recommandation |
|---|---|---|---|---|
| Flash interne NVM | ~quelques Ko réservables | ✅ | Moyenne | **Settings/calibration uniquement** |
| EEPROM I2C/SPI (24LC256) | 32 Ko | ✅ | Faible | Carte si Piste B |
| Co-processeur (Piste A) | Illimitée (SD/RAM) | ✅ | Faible côté PIC32 | **Carte complète si Piste A** |

---

## 9. Piste navigation intelligente

| Niveau | Contenu | Où |
|---|---|---|
| **Minimum viable** | Odométrie encodeurs + couverture boustrophedon + homing IR dock | PIC32 |
| **Niveau suivant** | IMU (gyro) pour corriger la dérive angulaire, wall-following, évitement dynamique | PIC32 (+ SPI) |
| **Ambitieux** | SLAM (lidar ou caméra) | **Co-processeur uniquement** — le PIC32 n'exécute que les consignes de trajectoire |

Prérequis absolus (Phase 2) : encodeurs de roues, capteurs de chute, capteurs de roue soulevée, récepteurs IR dock, asservissement vitesse roues (PI/PID).

---

## 10. Orientation stratégique recommandée

Le levier du projet n'est **pas le langage du MCU** — c'est la **séparation des responsabilités entre les deux cerveaux** :

```
┌─────────────────────────────┐     CORD (UART)      ┌──────────────────────────┐
│  Co-processeur Linux (WiFi) │ ◄──────────────────► │  PIC32MX360F256L (MCU)   │
│  - Appli de contrôle        │   JSON, SEP=0x1E     │  - Moteurs, capteurs      │
│  - Mapping persistant       │                      │  - Charge, dock, bumper   │
│  - Navigation intelligente │                      │  - Protocole CORD (serveur)│
│  - OTA, cloud (optionnel)   │                      │                           │
│  → Rust ou Go, au choix     │                      │  → C, refactoré, testé    │
└─────────────────────────────┘                      └──────────────────────────┘
```

**Court terme — Phase 0.5 « Assainissement » (avant Phase 1) :**
1. Activer le **watchdog** + kicker dans la boucle (sécurité #1)
2. Corriger `process_beep()` (`#ifndef __SDCC` restant)
3. Séparer `cord_hw.c` (HAL PIC32) de la logique protocole pure
4. Moderniser C11 (`_Static_assert`, `stdbool.h`, `stdint.h`), nettoyer les `goto`
5. Étendre les tests host à chaque module touché

**Moyen terme — Phases 1-2 (priorité porteur : WiFi d'abord) :**
6. Stabiliser et **versionner le protocole CORD v1** (contrat entre les deux cerveaux)
7. Écrire le **daemon Linux** (WiFi, appli de contrôle) — Rust si expérimentation, C/Go pour vitesse

**Long terme — Phases 3-4 :**
8. Mapping persistant + navigation sur le Linux (stockage SD/eMMC du co-pro, jamais sur le MCU)

---

## 11. Synthèse des réponses aux 5 questions posées

| # | Question | Réponse retenue |
|---|---|---|
| Q1 | WiFi : co-processeur ou ESP8266 sur le MCU ? | **Documenter les deux pistes** → Piste A (co-pro, recommandée) + Piste B (ESP8266, fallback) — §7 |
| Q2 | Stockage : interne, EEPROM, co-pro ? | **Hybride** : settings/calibration en flash NVM interne, carte complète sur co-pro (A) ou EEPROM 24LC256 (B) — §8 |
| Q3 | Capteurs présents ? (encodeurs, IMU, IR dock) | **À vérifier sur le hardware** — pins documentés dans `params.h`, présence IMU à confirmer — Phase 2 |
| Q4 | Build/test avant flash ? | **Oui, tests host obligatoires** — harnais `tests/` + règle « jamais de flash sans `make test` vert » |
| Q5 | Priorité : WiFi d'abord ou autonomie d'abord ? | **WiFi / appli de contrôle d'abord**, autonomie intelligente ensuite |

---

## 12. État de la Phase 0 (référence)

| Livrable | État |
|---|---|
| Makefile (XC32, test, lint, package) | ✅ |
| Harnais tests host (6 suites, 90 checks) | ✅ |
| 6 bugs corrigés | ✅ |
| CI 4 jobs (lint/test/build/package) | ✅ tous verts (run 39687194480) |
| cordlib/jsmn/pic32libs vendored | ✅ |
| README, AGENTS.md, plan.md | ✅ |
