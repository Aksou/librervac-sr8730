---
title: "Phase 0.5 — Assainissement & sécurité du MCU"
labels: ["phase", "P1", "sécurité", "refactor"]
milestone: "Phase 0.5"
state: open
---

## 🎯 Objectif

Rendre le PIC32MX360F256L **robuste et maintenable** avant d'y brancher le WiFi (Phase 1). C'est le levier stratégique identifié dans l'audit (`docs/audit.md` §5, §10) : on ne construit pas sur une fondation fragile.

## 📋 Checklist

- [ ] **0.5.1 — Activer le watchdog**
  - `FWDTEN = ON` + `SWDTEN` dans `main.c`
  - Kicker (`ClearWDT()`) dans la boucle principale
  - Ajuster le postscaler (actuellement `PS1048576` → viser ~1-2 s de timeout)
  - Test host : vérifier que le kicker est appelé à chaque itération
- [ ] **0.5.2 — Corriger `process_beep()`**
  - Même bug que `motor`/`move` : `#ifndef __SDCC` dans `cordlib/cord_sound.c`
  - La commande CORD `beep` est un no-op sur XC32
  - Gérer la dépendance `atof` (float interdit sur cible `-mno-float`)
- [ ] **0.5.3 — Séparer HAL et protocole**
  - `cord_hw.c` touche directement aux registres PIC32
  - Extraire une couche HAL propre → cordlib devient testable/portable
- [ ] **0.5.4 — Moderniser C11**
  - `stdint.h` / `stdbool.h` systématiques
  - `_Static_assert` sur les tailles de buffers
  - Nettoyer les 39 `goto` restants
- [ ] **0.5.5 — Nettoyer `cordlib/t/`**
  - Tests Perl 6 obsolètes (référencent `hw_set_motor_utilization`, API morte)
  - Supprimer ou porter sur le harnais host `tests/`
- [ ] **0.5.6 — Audit capteurs hardware**
  - Confirmer sur le robot : encodeurs roues ? IMU (accéléro/gyro) ? Récepteurs IR dock ?
  - Bloquant pour la Phase 2 — documenter les résultats dans `docs/audit-hardware.md`

## ✅ Critères d'acceptation

- [ ] `make test` vert (toutes les suites + nouvelles)
- [ ] `make lint` 0 finding
- [ ] CI 4 jobs verts
- [ ] Watchdog actif et kické (vérifié sur hardware si possible)
- [ ] `docs/audit.md` mis à jour avec le résultat de l'audit hardware (0.5.6)

## 🔗 Références

- `docs/audit.md` §5 (qualité), §10 (orientation)
- `docs/plan.md` Phase 0.5
- Bugs corrigés en Phase 0 : voir `docs/audit.md` §4

## 📝 Notes

- Règle d'or : **jamais de flash sans `make test` vert**
- Chaque tâche = un commit séparé, testée individuellement
