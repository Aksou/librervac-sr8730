# LibreRVAC sr8730

Firmware alternatif libre (GPLv3) pour les aspirateurs robots **Samsung Navibot
SR8730 / SR8750 / SR8751**. Projet d'origine : [LibreRVAC](https://librervac.org).

Ce dépôt contient le firmware bas niveau pour le **PIC32MX360F256L** (80 MHz,
256 Ko flash / 32 Ko RAM). Il dialogue avec une carte Linux/WiFi embarquée via
le **protocole CORD** (JSON sur liaison série) fourni par la bibliothèque
[`cordlib`](https://github.com/LibreRVAC/librervac-cordlib) (submodule).

## Architecture

```
┌──────────────────────────────┐  WiFi / appli de contrôle
│  Carte Linux / ESP32 (à venir)│  ← piste WiFi (voir docs/plan.md)
└──────────────┬───────────────┘
               │ UART — protocole CORD (JSON, séparateur 0x1E)
┌──────────────▼───────────────┐
│  cordlib (submodule)         │  protocole, callbacks, commandes motor/move/beep
│  + jsmn (submodule)          │  parser JSON
└──────────────┬───────────────┘
               │ callbacks hw_*
┌──────────────▼───────────────┐  ← ce dépôt
│  Firmware bas niveau (PIC32) │  moteurs, capteurs, batterie, charge, UART bit-bang
└──────────────────────────────┘
```

## Build

Le firmware se compile avec le compilateur **MPLAB XC32** (Microchip) :

```sh
# Prérequis : XC32 installé (par défaut /opt/microchip/xc32/current)
make                # build dist/firmware.hex + .elf + .map
make XC32_DIR=/chemin/vers/xc32/v4.45   # version spécifique
```

Flash du `.hex` avec un programmateur PIC32 (Pickit, ICD, ...).

## Tests (host)

Les modules du firmware sont testés sur PC (gcc) grâce à des stubs qui
simulent les registres du PIC32 (`tests/stubs/xc.h`) :

```sh
make test           # compile et exécute tous les tests unitaires
```

Couverture : moteurs (PWM/phase), batterie (tension/courant/température),
charge (dock, détection fin de charge), protocole CORD (JSON, commandes,
trames), bumper (debounce), UART bit-bang (auto-baud, décodage trames).

**Règle** : aucun flash sur le hardware sans que `make test` soit vert.

## Lint

```sh
make lint           # cppcheck (warning, performance, portability)
```

## Package

```sh
make package        # archive dist/librervac-sr8730-firmware.tar.gz
```

## CI

Une CI GitHub Actions (`.github/workflows/ci.yml`) valide chaque push/PR :

| Job | Rôle |
|-----|------|
| `lint` | cppcheck sur tout le code |
| `test` | tests unitaires host (gcc) |
| `build` | compilation croisée XC32 (PIC32MX360F256L) + artefacts `.hex`/`.elf` |
| `package` | archive du firmware téléchargeable |

## Protocole CORD

Communication JSON sur UART, trames séparées par `0x1E` (SEP), débit
auto-détecté. Commandes supportées :

```json
{"c":"motor","motor":"wheel left","throttle":0.5}
{"c":"move","length":1.5}
{"c":"move","length":0.5,"radius":0.3}
{"c":"beep","pitch":440,"duration":0.1,"volume":0.5}
```

Événements émis par le robot : `{"e":"bump",...}`, `{"e":"move-finished"}`,
infos périodiques (`info` : version, batterie, boucle principale).

## Feuille de route

Voir [`docs/plan.md`](docs/plan.md) pour le plan complet jusqu'à la
complétion du projet : WiFi, cartographie permanente, navigation
intelligente.

Voir [`AGENTS.md`](AGENTS.md) pour les instructions de développement.

## Licence

GPLv3 — voir [LICENSE](LICENSE).
