# Améliorations proposées — plan d’actions IA (basé sur la codebase)

Ce document transforme les 20 points initiaux en **tâches actionnables pour l’IA** en s’appuyant sur l’état réel du code (DSP, oversampling, analyseurs, UI). Chaque point décrit **quoi changer**, **pourquoi c’est pertinent**, ainsi que **avantages / inconvénients** pour prioriser.

---

## 1) DSP / Temps réel (stabilité + performance)

### 1.1 Éviter les allocations dans `processBlock()` (dryBuffer)
**Constat codebase :** `dryBuffer.makeCopyOf(buffer)` alloue potentiellement à chaque bloc. `dryBuffer` est local, recréé à chaque appel. (`Source/PluginProcessor.cpp`).

**Tâche IA (actionnable)**
- Ajouter un `juce::AudioBuffer<float> dryBuffer;` membre.
- Le dimensionner dans `prepareToPlay()` (mêmes dimensions que `lowBuffer`/`midBuffer`/`highBuffer`).
- Dans `processBlock()`, remplacer `makeCopyOf()` par `copyFrom()`.

**Pourquoi c’est pertinent**
- Le thread audio doit éviter toute allocation pour préserver la stabilité RT.

**Avantages**
- Réduction des pics CPU et risques de glitch.
- Comportement plus déterministe sous automation lourde.

**Inconvénients**
- Un buffer membre augmente la mémoire permanente du plugin.

---

### 1.2 Éviter `makeCopyOf()` sur les buffers de bande
**Constat codebase :** `lowBuffer.makeCopyOf(audio)` / `midBuffer.makeCopyOf(audio)` / `highBuffer.makeCopyOf(audio)` sont appelés à chaque bloc. Les buffers sont pourtant dimensionnés dans `prepareToPlay()`.

**Tâche IA (actionnable)**
- Remplacer `makeCopyOf()` par `copyFrom()` pour chaque bande.
- Conserver la taille fixe en `prepareToPlay()`.

**Pourquoi c’est pertinent**
- `makeCopyOf()` peut réallouer si la taille change, ce qui est coûteux.

**Avantages**
- CPU plus stable.
- Moins de churn mémoire.

**Inconvénients**
- Attention aux changements de taille dynamiques (sample rate / buffer size). Il faut gérer un resize si la taille change.

---

### 1.3 Remplacer `rand()` dans la waveshape “Crackle”
**Constat codebase :** la waveshape `Crackle` utilise `rand()` dans `processBlock()`. (`Source/PluginProcessor.cpp`).

**Tâche IA (actionnable)**
- Implémenter un PRNG léger (xorshift) en membre.
- Utiliser une fonction `nextNoise()` RT-safe sans appels CRT.

**Pourquoi c’est pertinent**
- `rand()` est non déterministe et pas RT-safe. Il peut ajouter du jitter.

**Avantages**
- Améliore la stabilité temps réel.
- Rend le bruit reproductible (utile pour tests).

**Inconvénients**
- Plus de code et maintenance du PRNG.

---

### 1.4 Lissage des paramètres (SmoothedValue)
**Constat codebase :** plusieurs paramètres critiques (`drive`, `mix`, `inputGain`, `output`, `deltaGain`, `lowFreq`, `highFreq`) sont utilisés directement par bloc.

**Tâche IA (actionnable)**
- Ajouter `juce::SmoothedValue<float>` pour ces paramètres.
- Initialiser les temps de lissage dans `prepareToPlay()`.
- Remplacer les lectures directes par `getNextValue()` dans la boucle d’échantillons.

**Pourquoi c’est pertinent**
- Réduit le zipper noise lors d’automations rapides.

**Avantages**
- Transition plus musicale.
- Moins d’artefacts audibles.

**Inconvénients**
- Léger coût CPU (par échantillon).
- Introduit une latence de modulation (selon le temps de lissage).

---

### 1.5 Mettre à jour `deltaSmoothed` une seule fois par échantillon
**Constat codebase :** `deltaSmoothed` est mis à jour dans la boucle par canal. Cela accélère la transition (N canaux → N fois la vitesse).

**Tâche IA (actionnable)**
- Calculer la valeur lissée une fois par sample (avant la boucle de canaux), puis l’utiliser pour tous les canaux.

**Pourquoi c’est pertinent**
- L’enveloppe de crossfade est plus cohérente entre canaux.

**Avantages**
- Transition fidèle au temps de fade configuré.
- Stéréo mieux alignée.

**Inconvénients**
- Refactor nécessaire du flux de boucle (sample-first vs channel-first).

---

### 1.6 Oversampling : adapter au nombre de canaux
**Constat codebase :** l’oversampling est instancié avec 2 canaux fixes.

**Tâche IA (actionnable)**
- Remplacer la construction `oversampling(2, 2, ...)` par un initialiseur basé sur `getTotalNumOutputChannels()` (ou re-instancier dans `prepareToPlay()`).

**Pourquoi c’est pertinent**
- En mode mono, le setup 2 canaux peut provoquer des incohérences ou du travail inutile.

**Avantages**
- Moins de CPU en mono.
- Plus robuste pour formats multiples.

**Inconvénients**
- Le changement de canaux peut nécessiter une réallocation/instanciation d’oversampling (à gérer proprement).

---

### 1.7 Stocker les pointeurs de paramètres
**Constat codebase :** les `getRawParameterValue()` sont appelés à chaque bloc.

**Tâche IA (actionnable)**
- Stocker les pointeurs atomiques (`std::atomic<float>*`) en membre (dans le constructeur).
- Remplacer les appels par `param->load()`.

**Pourquoi c’est pertinent**
- Simplifie et accélère la récupération des valeurs.

**Avantages**
- Micro-optimisation CPU.
- Code plus lisible.

**Inconvénients**
- Risque de pointer obsolète si l’APVTS change (rare, mais à documenter).

---

### 1.8 Réduire le coût des fonctions coûteuses par sample
**Constat codebase :** beaucoup de waveshapes utilisent `std::tanh`, `std::exp`, `std::pow` en boucle d’échantillons.

**Tâche IA (actionnable)**
- Identifier 3–5 waveshapes les plus coûteuses.
- Ajouter des approximations rapides (fast tanh/exp) ou LUTs.
- Offrir un flag `highQualityShaping` pour basculer.

**Pourquoi c’est pertinent**
- Le coût CPU s’additionne avec oversampling (x4).

**Avantages**
- CPU réduit, surtout en mode oversampling.
- Maintien d’un mode “HQ” si nécessaire.

**Inconvénients**
- Approximation = potentiel changement subtil du son.
- LUT = mémoire + interpolation.

---

### 1.9 Crossfade pour bypass/limiter/prePost
**Constat codebase :** ces switches sont instantanés (clics possibles).

**Tâche IA (actionnable)**
- Ajouter un crossfade (Dry/Wet mixer) pour ces changements de mode.
- Appliquer une rampe courte (5–10 ms).

**Pourquoi c’est pertinent**
- Les changements brusques peuvent générer des clicks audibles.

**Avantages**
- UI/automation plus propre.

**Inconvénients**
- Légère complexité DSP.

---

### 1.10 Forcer une taille FFT power-of-two
**Constat codebase :** `VisualizerAnalysisEngine::setFftSize()` accepte 1024–4096 sans vérifier le power-of-two.

**Tâche IA (actionnable)**
- Ajouter un `roundToNextPowerOfTwo()` avant création de la FFT.
- Maintenir la plage 1024–4096.

**Pourquoi c’est pertinent**
- `juce::dsp::FFT` requiert un ordre qui correspond à une puissance de 2.

**Avantages**
- Prévention de comportements imprévisibles si la valeur change.

**Inconvénients**
- La taille réelle peut différer de la demande utilisateur.

---

## 2) Qualité audio & DSP (résultat sonore)

### 2.1 Normaliser l’énergie à la recombinaison des bandes
**Constat codebase :** la recombinaison Low/Mid/High est une simple somme (Linkwitz-Riley). Les gains peuvent dériver selon fréquences.

**Tâche IA (actionnable)**
- Mesurer le gain de recombinaison (générer un sweep interne en debug ou tests).
- Ajouter une compensation de gain global par bande si nécessaire.

**Pourquoi c’est pertinent**
- Réduit les variations de niveau selon le réglage des crossovers.

**Avantages**
- Mix plus cohérent.

**Inconvénients**
- Calibration délicate (peut dépendre du design du filtre).

---

### 2.2 Saturation “shape” dépendante de la fréquence
**Constat codebase :** `shape` est uniforme pour toutes les bandes.

**Tâche IA (actionnable)**
- Ajouter un facteur de pondération (ex: moins de shape sur les hautes fréquences).
- Introduire un paramètre interne (non exposé) pour ajuster la courbe.

**Pourquoi c’est pertinent**
- Evite l’agressivité dans le haut du spectre.

**Avantages**
- Son plus musical sur cymbales/airs.

**Inconvénients**
- Le comportement peut devenir moins prévisible pour l’utilisateur.

---

### 2.3 Normaliser les waveshapes entre elles
**Constat codebase :** certaines waveshapes produisent plus de gain perçu que d’autres.

**Tâche IA (actionnable)**
- Mesurer le gain RMS d’un signal test par waveshape.
- Appliquer un `makeupGain` par waveshape (table de normalisation).

**Pourquoi c’est pertinent**
- Evite les sauts de volume lors du changement de waveshape.

**Avantages**
- UX plus stable.

**Inconvénients**
- Le niveau perçu fait partie du “caractère” de certains modes (peut frustrer les power users).

---

### 2.4 Optimiser le mix Wet/Dry en mode Delta
**Constat codebase :** le mode Delta ignore le paramètre `mix`.

**Tâche IA (actionnable)**
- Ajouter un mix dédié pour Delta (ex: `deltaMix`).
- Option: limiter `deltaGain` automatiquement si `deltaMix` est élevé.

**Pourquoi c’est pertinent**
- Permet d’écouter la différence sans exploser le niveau.

**Avantages**
- Meilleur contrôle utilisateur.

**Inconvénients**
- Un paramètre en plus (complexité UI/UX).

---

## 3) Visualizers & UI (performance + stabilité)

### 3.1 Éviter allocations dans `VisualizerAnalysisEngine::updateFrame()`
**Constat codebase :** `deltaTemp.resize()` + `assign()` provoquent des allocations régulières.

**Tâche IA (actionnable)**
- Préallouer `deltaTemp` une fois dans `ensureBuffers()`.
- Remplacer `assign()` par `std::copy` vers des buffers fixes.

**Pourquoi c’est pertinent**
- Les visualizers tournent en timer UI. Les allocations peuvent créer des micro-freezes.

**Avantages**
- UI plus fluide.

**Inconvénients**
- Plus de gestion manuelle des tailles de buffers.

---

### 3.2 Remplacer le ring buffer maison par `juce::AbstractFifo`
**Constat codebase :** `AnalyzerTap` gère un ring buffer custom avec index atomique.

**Tâche IA (actionnable)**
- Utiliser `juce::AbstractFifo` pour simplifier la logique.
- Stocker des blocs audio pré/post dans des buffers circulaires.

**Pourquoi c’est pertinent**
- Réduit le risque d’erreur de synchronisation UI/audio.

**Avantages**
- Code plus robuste.
- Moins de logique maison.

**Inconvénients**
- Refactor plus important (structure de données différente).

---

### 3.3 Désactiver le timer des visualizers si non visible
**Constat codebase :** le timer tourne en permanence quand l’onglet visualizers est actif, même si la fenêtre est minimisée.

**Tâche IA (actionnable)**
- Ajouter un hook sur `visibilityChanged()` ou `parentHierarchyChanged()`.
- Stopper le timer si `!isShowing()`.

**Pourquoi c’est pertinent**
- Réduit la charge CPU quand l’UI n’est pas visible.

**Avantages**
- Économie CPU.

**Inconvénients**
- L’état des frames peut être moins “frais” au retour (OK en pratique).

---

### 3.4 Optimiser `heatHistory`
**Constat codebase :** `heatHistory` est un `deque<vector<float>>` qui alloue à chaque push.

**Tâche IA (actionnable)**
- Remplacer par un ring buffer fixe (vector 2D, index circulaire).
- Copier chaque spectre dans une “ligne” préallouée.

**Pourquoi c’est pertinent**
- Le heatmap est mis à jour fréquemment (UI). Les allocations sont coûteuses.

**Avantages**
- Plus de stabilité UI.

**Inconvénients**
- Complexité de gestion du ring buffer.

---

## 4) Maintenabilité & robustesse globale

### 4.1 Éliminer les chemins absolus hardcodés
**Constat codebase :** `CustomLookAndFeel::ensureImageLoaded()` et `ensureFontLoaded()` utilisent des paths `/Users/...`.

**Tâche IA (actionnable)**
- Supprimer les chemins absolus.
- Utiliser uniquement `BinaryData` + chemins relatifs dans l’app bundle.

**Pourquoi c’est pertinent**
- Les chemins absolus cassent les builds sur d’autres machines.

**Avantages**
- Build portable.
- Moins de comportement non déterministe.

**Inconvénients**
- Requiert que les assets soient toujours embarqués correctement.

---

### 4.2 Structurer les waveshapes en stratégie/testable
**Constat codebase :** l’énorme `switch` dans `processBlock()` est difficile à maintenir.

**Tâche IA (actionnable)**
- Extraire chaque waveshape dans des fonctions dédiées (`Waveshaper::applyTube`, etc.).
- Ajouter un test unitaire de niveau/clip pour quelques formes.

**Pourquoi c’est pertinent**
- Le code est long et difficile à déboguer.

**Avantages**
- Meilleure lisibilité.
- Permet des tests ciblés.

**Inconvénients**
- Refactor large, risque de régression sonore si erreurs.

---

## 5) Ordre de priorité suggéré (impact / effort)

1. **RT-safety** : 1.1, 1.2, 1.3, 1.5 (impact fort, effort faible).
2. **Stabilité UI** : 3.1, 3.4 (impact moyen, effort moyen).
3. **Qualité audio** : 2.3, 2.1 (impact perceptible, effort moyen).
4. **Maintenabilité** : 4.1, 4.2 (impact long terme, effort moyen/haut).
5. **Optimisations CPU fines** : 1.7, 1.8 (impact variable, effort moyen/haut).

---

Si tu veux, je peux maintenant convertir cette liste en un **plan d’implémentation priorisé**, ou attaquer un subset spécifique (par exemple “RT-safety + visualizers”).
