---
title: "Phase 4 — Cartographie permanente"
labels: ["phase", "P2", "mapping", "persistance"]
milestone: "Phase 4"
state: open
blocked_by: "Phase 2 (odométrie), Phase 1 (co-processeur)"
---

## 🎯 Objectif

Mémoriser l'environnement entre les sessions : le robot « connaît » la maison et ne repasse pas deux fois au même endroit.

## 🧭 Stratégie retenue — Hybride

| Donnée | Stockage | Justification |
|---|---|---|
| Settings / calibration | **Flash interne NVM** du PIC32 | Petit volume, persistant, zéro hardware ajouté |
| **Carte complète** | **Co-processeur** (Piste A : SD/RAM) ou **EEPROM I2C/SPI 24LC256** (Piste B : 32 Ko) | 5–10 cm/cellule sur 20 m² = 40–160 Ko — impossible en flash interne |

## 📋 Checklist

- [ ] **4.1 — Modèle de carte**
  - Grille d'occupation (résolution 5–10 cm/cellule, bits par cellule)
  - Stockage en mémoire externe ou flash
- [ ] **4.2 — Stockage (décision retenue : hybride)**
  - Writer/lecteur NVM flash interne (settings/calibration), zone réservée en fin de flash, wear-leveling simple
  - Carte complète sur co-processeur (Piste A) ou EEPROM I2C/SPI externe (Piste B)
- [ ] **4.3 — Sérialisation carte**
  - Format compact binaire sur CORD (événements `map-chunk`)
  - Reconstruction côté appli
- [ ] **4.4 — Fusion carte + odométrie**
  - Mise à jour des cellules visitées pendant le nettoyage
  - Recalage simple (pas de SLAM complet sur 32 Ko RAM)
- [ ] **4.5 — Persistance inter-sessions**
  - Sauvegarde auto en fin de nettoyage / batterie faible
  - Restauration au boot

## ✅ Critères d'acceptation

- [ ] La carte persiste après extinction du robot
- [ ] Le robot ne repasse pas sur les zones déjà nettoyées (couverture intelligente)
- [ ] La carte est visualisable dans l'appli de contrôle (Phase 1.4)
- [ ] `make test` vert + CI 4 jobs verts

## 🔗 Références

- `docs/audit.md` §8 (stockage), §6 (budget hardware)
- `docs/plan.md` Phase 4

## 📝 Notes

- Ne **jamais** stocker la carte en flash interne du PIC32 — 256 Ko ne tiennent pas firmware + carte
- Le SLAM complet (lidar/caméra) est **hors scope PIC32** — réservé au co-processeur si ambitieux
