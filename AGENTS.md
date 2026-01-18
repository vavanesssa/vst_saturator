# 🤖 AGENTS.md — Instructions pour les Agents IA

## 🎯 IMPORTANT : TOUJOURS COMMENCER ICI

**AVANT TOUT TRAVAIL, VOUS DEVEZ :**

1. ✅ Lire `README.md` entièrement
2. ✅ Consulter `improvements.md` pour les tâches proposées
3. ✅ Après avoir lu, informer l'utilisateur : `🐮🐷🐸🐱 : CONTEXT ACQUIRED BAWS ! proceeding...`
4. ✅ Seulement ensuite commencer la tâche

**NE PASSEZ PAS CETTE ÉTAPE. TOUJOURS LIRE LA DOCUMENTATION D'ABORD.**

---

## 📋 Résumé du Projet

**steverator** est un plugin VST3 audio built with JUCE 7.x.

**Stack Technique :**
- C++17, JUCE 7.x
- VST3 + Standalone
- macOS Universal Binary (ARM64 + x86_64)

**Fonctionnalités Clés :**
- 3-bandes de saturation (Low/Mid/High)
- 58 algorithmes de waveshape
- 20 paramètres automatisables
- Système de visualiseurs temps réel (5 panneaux)
- 76 presets d'usine
- Interface à onglets (Knobs + Visualizers)

---

## 🔨 Build & Deploy Workflow

### Quand l'utilisateur demande des modifications

Toujours suivre ce workflow :

1. **Lire et comprendre** les fichiers source pertinents (toujours utiliser l'outil Read)
2. **Planifier les changements** si la tâche est complexe
3. **Appliquer les changements** avec l'outil Edit
4. **Build et deployer** avec le script
5. **Vérifier** que les changements fonctionnent

### Commande Build

Après tout changement de code, TOUJOURS exécuter :

```bash
cd /Users/vava/Documents/GitHub/vst_saturator
./build_and_deploy.sh
```

**Ce script fait :**
- Génère un hash unique (5 caractères, ex: `09FA1`)
- CMake compilation (Release)
- Signe le plugin
- Copie les assets
- Nettoie le cache Ableton
- Déploie dans `/Library/Audio/Plug-Ins/VST3/steverator.vst3`
- Lance le standalone pour test

### Informer l'utilisateur après le build

Toujours inclure le hash build :

```
✅ Build réussi !
🔑 Build hash: 09FA1

Pour tester dans Ableton :
1. Redémarrer Ableton (Cmd+Q)
2. Rouvrir Ableton
3. Rescanner les plugins : Preferences → File/Folder → Rescan
4. Rechercher "steverator" (Manufacturer: NeiXXa)
5. Vérifier le hash en bas à droite de l'interface

Le standalone a été lancé automatiquement.
```

---

## 📁 Structure du Projet

```
steverator/
├── CMakeLists.txt              # Configuration build JUCE VST3
├── build_and_deploy.sh         # Script de build et déploiement ⭐ À TOUJOURS LANCER APRÈS CHANGEMENTS
├── build_and_test.sh          # Build rapide sans déploiement
├── README.md                   # Documentation complète (LA SEULE SOURCE DE VÉRITÉ)
├── AGENTS.md                   # Ce fichier - instructions pour agents
├── TUTORIAL.md                 # Guide d'apprentissage
├── improvements.md              # Améliorations proposées
├── Assets/                     # Assets du thème
│   ├── steve.png              # Mascotte "Steve le Poisson" (bouche fermée)
│   ├── steve2.png             # Mascotte (bouche ouverte)
│   ├── indicator.png           # Indicateur de knob
│   ├── NanumPenScript-Regular.ttf # Police manuscrite
│   ├── background.png          # (Non utilisé)
│   └── version.txt            # Hash + timestamp (généré au build)
├── Source/
│   ├── PluginProcessor.h       # Interface audio processor
│   ├── PluginProcessor.cpp     # DSP temps réel : 3-bandes, 58 waveshapes
│   ├── PluginEditor.h          # Déclarations UI
│   ├── PluginEditor.cpp        # Layout UI, 76 presets
│   ├── CustomLookAndFeel.h     # Interface styling custom
│   ├── CustomLookAndFeel.cpp   # Dessin procédural (knobs, boutons)
│   ├── VisualizerAnalysis.h    # Structures données visualiseur
│   ├── VisualizerAnalysis.cpp  # FFT, ring buffer, métriques
│   ├── VisualizerComponents.h  # Composants UI visualiseurs
│   └── VisualizerComponents.cpp # Rendu des panneaux
├── libs/
│   └── JUCE/                   # Framework JUCE (submodule - NE PAS ÉDITER)
└── build/                      # Artefacts de build (généré - NE PAS ÉDITER)
```

---

## 💾 Fichiers Clés

| Fichier | Purpose | Éditer? |
|----------|----------|----------|
| `Source/PluginProcessor.cpp` | DSP audio | ✅ Oui |
| `Source/PluginEditor.cpp` | UI, paramètres, presets | ✅ Oui |
| `Source/CustomLookAndFeel.cpp` | Styling, dessin procédural | ✅ Oui |
| `Source/VisualizerAnalysis.h/cpp` | Moteur analyse visualiseur | ✅ Oui |
| `Source/VisualizerComponents.h/cpp` | Composants UI visualiseur | ✅ Oui |
| `CMakeLists.txt` | Configuration build | ⚠️ Avec prudence |
| `build_and_deploy.sh` | Build & déploiement | ⚠️ Avec prudence |
| `Assets/steve.png` | Image mascotte | ✅ Oui - Remplacer |
| `README.md` | Documentation complète | ✅ Oui - Tenir à jour |
| `TUTORIAL.md` | Guide apprentissage | ✅ Oui - Tenir à jour |
| `improvements.md` | Améliorations proposées | ✅ Oui - Référence |
| `libs/JUCE/` | Framework JUCE | ❌ JAMAIS éditer |
| `build/` | Artefacts build | ❌ JAMAIS éditer |

---

## 🐛 Dépannage

### Le plugin n'apparaît pas dans Ableton

1. Lancer `./build_and_deploy.sh`
2. Fermer complètement Ableton (Cmd+Q)
3. Rouvrir Ableton
4. Rescanner les plugins
5. Chercher "steverator"
6. Vérifier le hash en bas à droite de l'interface

### L'interface ne s'anime pas

- Vérifier que `timerCallback()` est appelé
- Vérifier que l'audio passe
- Vérifier le timer rate (~30-60fps)

### Build échoue

- Vérifier les warnings du compilateur
- Initialiser JUCE submodule : `git submodule update --init --recursive`
- Vérifier CMake : `cmake --version` (doit être 3.15+)
- Vérifier OpenSSL pour le hash build : `openssl version`

### CPU trop élevé

- Désactiver les visualiseurs si pas nécessaires
- Réduire oversampling (4x → 2x)
- Simplifier les waveshapes coûteuses

---

## 📝 Style de Code

- **Langage :** C++17
- **Comments :** Expliquer le "pourquoi", pas juste le "quoi"
- **Naming :** camelCase pour variables, PascalCase pour classes
- **Temps réel :** PAS d'allocation dans `processBlock()`
- **Dépendances :** Seulement JUCE

---

## 🚀 Règles Temps Réel

### ❌ INTERDIT dans `processBlock()` :

- `new` / `delete` (allocation heap)
- `std::vector::resize()` (potential realloc)
- `std::make_unique()` (allocation heap)
- Opérations fichier (`juce::File`)
- `DBG()`, `std::cout` (logging)
- `std::mutex`, `juce::CriticalSection` (bloquant)
- Toute opération bloquante

### ✅ AUTORISÉ dans `processBlock()` :

- Opérations atomiques (`load`, `store`)
- Allocation stack (tableaux sur stack)
- Opérations math (`std::tanh`, `std::exp`)
- Opérations buffer (`copy`, `add`, `applyGain`)
- JUCE DSP (filtres, oversampling)
- Buffers membres pré-alloués

---

## 📚 Références

- JUCE : https://juce.com/
- VST3 : https://www.steinberg.net/vst3
- CMake : https://cmake.org/

---

**Dernière mise à jour :** Janvier 2026
