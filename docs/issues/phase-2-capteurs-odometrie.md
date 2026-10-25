---
title: "Phase 2 — Capteurs et odométrie"
labels: ["phase", "P1", "capteurs", "odométrie"]
milestone: "Phase 2"
state: open
blocked_by: "Phase 0.5 (0.5.6 audit hardware)"
---

## 🎯 Objectif

Donner au robot la **perception de son mouvement**. Prérequis absolu de toute navigation intelligente (Phase 3).

## ⚠️ Dépendance

La tâche **0.5.6 (audit hardware)** doit confirmer quels capteurs sont réellement présents et câblés sur le SR8730 avant de commencer. Les pins sont documentés dans `params.h` mais leur présence physique est à vérifier.

## 📋 Checklist

- [ ] **2.1 — Lire les encodeurs de roues**
  - `wheels.c` : change-notice/interruptions sur `PIN_WHEEL_*_ENCODER_1/2`
  - Comptage ticks, vitesse par roue
- [ ] **2.2 — Odométrie**
  - `odometry.c` (nouveau) : intégration différentielle (avance + rotation)
  - Utiliser `WHEEL_GAP`/`WHEEL_RADIUS` de `params.h` (à calibrer sur hardware)
- [ ] **2.3 — Lire les capteurs de chute**
  - `PIN_SENSOR_BOTTOM_*` (3 capteurs) : sécurité anti-chute, arrêt immédiat
- [ ] **2.4 — Lire les capteurs de roue soulevée**
  - `PIN_WHEEL_LIFT_*` : arrêt des roues concernées
- [ ] **2.5 — Lire les récepteurs IR dock**
  - `PIN_DOCKING_RECEIVER_LEFT/RIGHT` + `PIN_IR_RECEIVER_REAR_*` + `PIN_MID_RECEIVER_R`
  - Base du homing (retour au dock, Phase 3.4)
- [ ] **2.6 — Driver IMU (si présent)**
  - Via `spi.c` (existant, inutilisé) sur `PIN_ACCELGYRO_POWER`
  - Gyroscope pour la dérive angulaire
  - **À confirmer sur le hardware** (dépend de 0.5.6)
- [ ] **2.7 — Asservissement vitesse roues**
  - Remplacer le placeholder `cord_set_speed_goal` par un vrai PI/PID sur consigne encodeurs

## ✅ Critères d'acceptation

- [ ] Le robot connaît sa position relative (x, y, θ) avec une dérive acceptable (< 5 % sur 5 m)
- [ ] Capteurs de chute et roue soulevée déclenchent un arrêt immédiat (< 50 ms)
- [ ] Asservissement : écart vitesse réelle/consigne < 5 % en régime établi
- [ ] `make test` vert (nouvelles suites odométrie + capteurs) + CI 4 jobs verts
- [ ] Calibration odométrie documentée (méthode + résultats)

## 🔗 Références

- `docs/audit.md` §9 (navigation), §3 (état des modules)
- `docs/plan.md` Phase 2
- `params.h` (pinout), `wheels.c` (placeholder encodeurs)

## 📝 Notes

- Commencer par 2.1 + 2.2 (odométrie) — c'est le cœur du sujet
- 2.3/2.4 sont des **sécurités** — prioritaires même avant l'odométrie
- L'IMU (2.6) est optionnel — ne pas bloquer la phase dessus
