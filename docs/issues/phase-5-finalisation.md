---
title: "Phase 5 — Finalisation"
labels: ["phase", "P3", "finalisation", "polish"]
milestone: "Phase 5"
state: open
blocked_by: "Phases 1-4"
---

## 🎯 Objectif

Livrer un firmware **fini, documenté et agréable à utiliser** : basse consommation, calibration guidée, télémétrie complète, documentation utilisateur.

## 📋 Checklist

- [ ] **5.1 — Mode basse consommation**
  - `MODE_SLEEPING` : périphériques arrêtés, réveil sur bouton/événement
  - Consommation en veille < 1 mA (à mesurer)
- [ ] **5.2 — Calibration assistée**
  - Procédure guidée (encodeurs, batterie, boussole) via CORD
  - Stockage en NVM (voir Phase 4.2)
- [ ] **5.3 — Télémétrie complète**
  - Batterie, capteurs, position, carte — flux périodique vers l'appli
- [ ] **5.4 — Documentation utilisateur**
  - Flash, appli, protocole CORD pour développeurs
  - Guide de dépannage
- [ ] **5.5 — CI durcie**
  - Job `test-flash` (smoke test sur hardware via banc de test)
  - Badge de couverture dans le README

## ✅ Critères d'acceptation

- [ ] Autonomie batterie documentée (temps de nettoyage réel)
- [ ] Procédure de calibration réalisable par un utilisateur non-développeur
- [ ] Documentation à jour (README + `docs/`)
- [ ] CI 4 jobs verts + smoke test hardware
- [ ] Release v1.0 taguée

## 🔗 Références

- `docs/plan.md` Phase 5
- `docs/audit.md` §10

## 📝 Notes

- Dernière phase avant la **release v1.0**
- La télémétrie (5.3) dépend de la Phase 1 (WiFi) et de la Phase 4 (carte)
