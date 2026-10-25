---
title: "Phase 3 — Navigation intelligente"
labels: ["phase", "P2", "navigation", "autonomie"]
milestone: "Phase 3"
state: open
blocked_by: "Phase 2"
---

## 🎯 Objectif

Passer du « robot téléguidé » au « robot qui nettoie seul » : déplacements asservis, réactions de sécurité intégrées, stratégie de couverture, retour au dock autonome.

## 📋 Checklist

- [ ] **3.1 — Remplacer `cord_move`/`cord_move_arc`**
  - Vrais déplacements asservis en distance/angle via odométrie
  - Événements `move-finished` fiables
- [ ] **3.2 — Réactions sécurité intégrées**
  - Bumper, chute, roue soulevée, surintensité → arrêt + événement CORD
  - Compléter `cord_event_bumper`
- [ ] **3.3 — Stratégie de couverture**
  - Spirale puis boustrophedon (lignes)
  - Wall-following en secours
- [ ] **3.4 — Homing / retour au dock**
  - Guidage par récepteurs IR dock + odométrie
  - Déclenché sur batterie faible ou commande explicite
- [ ] **3.5 — Machine de modes**
  - Activer `MODE_IDLE/WORKING/SLEEPING/DOCKING` de cordlib
  - Transitions propres entre modes

## ✅ Critères d'acceptation

- [ ] Le robot nettoie une pièce entière sans intervention (couverture > 90 %)
- [ ] Retour au dock autonome quand batterie < seuil
- [ ] Toute réaction de sécurité (bumper, chute, roue soulevée) arrête le robot en < 100 ms
- [ ] `make test` vert + CI 4 jobs verts
- [ ] Démonstration vidéo du cycle complet (nettoyage → dock)

## 🔗 Références

- `docs/audit.md` §9 (niveaux de navigation)
- `docs/plan.md` Phase 3
- `cordlib/cord_cordlib.c` (modes), `cordlib/cord_motors.c`

## 📝 Notes

- Bloqué par la **Phase 2** (odométrie indispensable)
- La stratégie de couverture peut être portée sur le co-processeur (Phase 1) qui envoie des consignes de trajectoire — à évaluer en 3.3
