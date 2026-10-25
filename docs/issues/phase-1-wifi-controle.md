---
title: "Phase 1 — WiFi & contrôle à distance"
labels: ["phase", "P1", "wifi", "priorité-porteur"]
milestone: "Phase 1"
state: open
blocked_by: "Phase 0.5"
---

## 🎯 Objectif

**Priorité porteur** : donner au robot une connexion WiFi et une application de contrôle à distance. C'est la première brique visible pour l'utilisateur.

## 🧭 Stratégie retenue

**Piste A — Co-processeur WiFi/Linux (recommandée)** + **Piste B — ESP8266 sur le MCU (fallback)**. Les deux pistes sont documentées dans `docs/audit.md` §7.

| | Piste A ⭐ | Piste B |
|---|---|---|
| Hardware | ESP32 (~4 €) / ESP8266 / RPi Zero W | ESP8266 (ESP-01) en commandes AT |
| Liaison | UART existant (`PIN_CONNECTION_RX/TX`) | UART ou SPI |
| Rôle co-pro | WiFi, web/WebSocket, carte, planification | — |
| Rôle PIC32 | Temps réel inchangé | + pile TCP/IP uIP (très serré sur 32 Ko RAM) |
| Avantage clé | Zéro pression RAM/flash sur le PIC32, OTA du co-pro | Autonome, un seul board |

Le câblage `PIN_LEGACY_LINUX_BOARD_*` est **déjà prévu sur le PCB d'origine** — c'est l'architecture native du robot.

## 📋 Checklist

- [ ] **1.1 — Étendre le protocole CORD**
  - Nouvelles commandes : `{"c":"wifi"}`, `{"c":"map"}`, `{"c":"navigate"}`, `{"c":"dock"}`, `{"c":"pause"}`
  - Événements : position, batterie, carte partielle
  - Augmenter `PROTOCOL_BUFFER_SIZE`/`PROTOCOL_TOKENS` si besoin (budget RAM !)
  - **Versionner le protocole CORD v1** (contrat figé entre les deux cerveaux)
- [ ] **1.2 — Piste A : co-processeur WiFi/Linux**
  - Choix hardware : ESP32 vs ESP8266 vs RPi Zero W (benchmarker)
  - Câblage UART vers `PIN_LEGACY_LINUX_BOARD_*`
  - Boot, handshake CORD, watchdog de lien
- [ ] **1.3 — Piste B : ESP8266 sur le PIC32 (fallback)**
  - ESP-01 en commandes AT sur UART
  - Évaluer uIP + HTTP sur 32 Ko RAM (WebSocket probablement exclu)
  - Ne démarrer que si la Piste A échoue
- [ ] **1.4 — Application de contrôle minimale**
  - Interface web/socket : joystick moteurs, start/stop nettoyage, retour dock
  - État batterie/capteurs, flux d'événements CORD
  - Langage : Rust (expérimentation) ou C/Go (vitesse) — au choix
- [ ] **1.5 — Sécurité de la liaison**
  - Watchdog de lien : si pas de commande pendant N s → arrêt moteurs
  - Gestion déconnexion/reconnexion propre

## ✅ Critères d'acceptation

- [ ] On peut piloter le robot (avancer, tourner, stop, dock) depuis un téléphone/PC en WiFi
- [ ] Les événements CORD (batterie, bumper, état) remontent en temps réel
- [ ] Déconnexion WiFi = arrêt sécurisé des moteurs (pas de robot fou)
- [ ] `make test` vert + CI 4 jobs verts
- [ ] Protocole CORD v1 documenté (`docs/cord-protocol.md`)

## 🔗 Références

- `docs/audit.md` §7 (pistes WiFi), §10 (architecture deux cerveaux)
- `docs/plan.md` Phase 1
- Protocole CORD existant : `cordlib/cord_protocol.c`

## 📝 Notes

- Bloqué par la **Phase 0.5** (watchdog + assainissement)
- Le daemon Linux est un **projet séparé** (autre dépôt) ou un sous-répertoire `host/` — à décider en 1.2
