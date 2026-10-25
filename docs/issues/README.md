# Suivi des phases par issues

Le tracker d'issues GitHub du dépôt (`Aksou/librervac-sr8730`) est **indisponible dans l'environnement agent** (l'API de mutation retourne 403 — création d'issues et activation du tracker bloquées). En attendant une réactivation manuelle côté GitHub (Settings → Features → Issues), le suivi est assuré par les fichiers Markdown de ce répertoire, au format issue (frontmatter + checklist).

Dès que le tracker est réactivé, convertir chaque fichier en issue GitHub (titre, labels, checklist) et fermer ce répertoire.

## Index

| Phase | Fichier | Priorité | Bloquée par | État |
|---|---|---|---|---|
| 0.5 — Assainissement & sécurité | [`phase-0.5-assainissement.md`](phase-0.5-assainissement.md) | **P1** | Phase 0 ✅ | À faire |
| 1 — WiFi & contrôle à distance | [`phase-1-wifi-controle.md`](phase-1-wifi-controle.md) | **P1** (priorité porteur) | Phase 0.5 | À faire |
| 2 — Capteurs et odométrie | [`phase-2-capteurs-odometrie.md`](phase-2-capteurs-odometrie.md) | **P1** | Phase 0.5 (0.5.6) | À faire |
| 3 — Navigation intelligente | [`phase-3-navigation.md`](phase-3-navigation.md) | P2 | Phase 2 | À faire |
| 4 — Cartographie permanente | [`phase-4-cartographie.md`](phase-4-cartographie.md) | P2 | Phases 1 + 2 | À faire |
| 5 — Finalisation | [`phase-5-finalisation.md`](phase-5-finalisation.md) | P3 | Phases 1-4 | À faire |

## Ordre recommandé

```
Phase 0 ✅ → Phase 0.5 (assainissement) → Phase 1 (WiFi) ─┐
                                                        ├→ Phase 4 (carte) → Phase 5 (finalisation)
                     Phase 2 (capteurs/odométrie) ──→ Phase 3 (navigation) ─┘
```

La Phase 0.5 est le **levier stratégique** : watchdog + robustesse avant toute extension. La Phase 1 est la **priorité porteur** (WiFi/appli d'abord). Les Phases 2→3→4 mènent à l'autonomie intelligente.
