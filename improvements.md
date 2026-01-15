# Améliorations proposées pour la qualité, la stabilité et la performance

Ce document regroupe **20 corrections/améliorations possibles** observées dans la codebase pour améliorer la qualité globale, la stabilité et la rapidité du VST. Les points ci-dessous sont classés pour couvrir **DSP temps réel**, **qualité audio**, **performance UI/visualizers**, et **maintenabilité**.

## 1) DSP / Temps réel (stabilité + performance)

1. **Éviter les allocations en audio thread** : `dryBuffer.makeCopyOf()` alloue potentiellement à chaque bloc. Stocker un buffer “dry” membre, dimensionné dans `prepareToPlay()`, puis utiliser `copyFrom()` pour éviter des allocations et pics CPU.
2. **Éviter les copies inutiles des bandes** : `lowBuffer.makeCopyOf()`, `midBuffer.makeCopyOf()`, `highBuffer.makeCopyOf()` peuvent réallouer si la taille change. Pré-dimensionner en `prepareToPlay()` et utiliser `copyFrom()`/`copyFromWithRamp()` pour limiter la charge.
3. **Supprimer `rand()` dans `processBlock()`** : le `rand()` (case “Crackle”) n’est pas RT-safe et peut provoquer jitter/latence. Utiliser un PRNG léger (xorshift/LFSR) maintenu comme état membre.
4. **Lissage des paramètres** : appliquer des `juce::SmoothedValue` pour `drive`, `mix`, `inputGain`, `output`, `deltaGain`, `lowFreq`, `highFreq`. Cela réduit le zipper noise lors d’automations rapides.
5. **Mise à jour de `deltaSmoothed` hors boucle channel** : le lissage delta est mis à jour pour chaque canal et chaque sample, ce qui accélère artificiellement la transition. Le calcul devrait être par sample (une seule fois), puis appliqué à tous les canaux.
6. **Gestion mono/stéréo pour l’oversampling** : `juce::dsp::Oversampling<float>` est instancié avec 2 canaux. Adapter dynamiquement au nombre réel de canaux (mono/stéréo) afin d’éviter d’éventuels mismatches de canaux.
7. **Pré-calcul des pointeurs de paramètres** : récupérer `getRawParameterValue()` à chaque bloc est ok, mais on peut stocker des pointeurs atomiques en membre pour réduire overhead et clarifier le code.
8. **Réduction du coût des `std::pow`/`std::exp`/`std::tanh`** : certaines waveshapes utilisent plusieurs fonctions coûteuses par échantillon. On peut proposer des versions approximées (fast tanh / exp) ou des LUTs pour réduire la charge CPU.
9. **Limiter/Bypass avec crossfade** : pour éviter les “clicks” quand on active `bypass`, `limiter` ou `prePost`, utiliser une rampe de crossfade (Dry/Wet mixer) au lieu d’un switch immédiat.
10. **Vérification de `fftSize` power-of-two** : `VisualizerAnalysisEngine::setFftSize()` accepte 1024-4096, mais ne force pas la puissance de 2. Ajouter un “round to next power of two” pour garantir la stabilité FFT.

## 2) Qualité audio & DSP (résultats sonores)

11. **Normaliser l’énergie dans les bandes** : après recombinaison Low/Mid/High, appliquer une compensation d’énergie ou une correction de gain (Linkwitz-Riley peut nécessiter une calibration selon la courbe).
12. **Saturation “shape” dépendante de la fréquence** : si l’objectif est un son plus musical, appliquer une légère pondération fréquentielle ou une dynamique sur `shape` pour éviter un haut du spectre trop agressif.
13. **Mise à l’échelle cohérente des waveshapes** : certains algorithmes peuvent clipper plus fort que d’autres. Implémenter une normalisation de sortie par algorithme pour éviter des sauts de gain énormes lors du changement de waveshape.
14. **Optimisation du mix Wet/Dry** : `mix` est appliqué après tout traitement, mais en delta mode il est ignoré. Prévoir un mix dédié pour delta ou une “safe mode” pour éviter un signal delta trop fort.

## 3) Visualizers & UI (performance + stabilité)

15. **Éviter allocations dans `VisualizerAnalysisEngine::updateFrame()`** : les `assign()` et `resize()` provoquent des reallocations fréquentes. Préallouer les buffers et utiliser des copies directes (`std::copy`) pour réduire la charge UI.
16. **Remplacer le ring buffer maison par `juce::AbstractFifo`** : plus robuste pour multi-thread (audio/UI), réduit le risque de race condition et simplifie la logique.
17. **Limiter le FPS quand les visualizers ne sont pas visibles** : désactiver le timer si le tab n’est pas visible ou si la fenêtre n’est pas active, pour réduire l’usage CPU.
18. **Optimiser `heatHistory`** : utiliser un ring buffer fixe au lieu d’un `deque<vector<float>>`, afin d’éviter copies/allocation à chaque frame.

## 4) Maintenabilité & robustesse globale

19. **Éliminer les chemins absolus hardcodés** : `CustomLookAndFeel::ensureImageLoaded()` et `ensureFontLoaded()` font référence à `/Users/...`. Remplacer par `BinaryData` uniquement ou chemins relatifs pour un build portable.
20. **Structurer les waveshapes en stratégie** : déplacer la logique de waveshape dans une classe/fonctions dédiées, avec tests unitaires. Cela rend le code plus lisible, testable, et réduit la taille de `processBlock()`.

---

Si tu veux, je peux transformer ces 20 points en un plan d’implémentation détaillé (priorisé par impact CPU/qualité audio), ou démarrer directement sur un sous-ensemble ciblé.
