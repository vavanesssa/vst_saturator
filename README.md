🎛️ PRD — vst_saturator

(VST Saturator pédagogique, document vivant)

⸻

🎯 Vision & intention du projet

Le projet vst_saturator a pour objectif de créer un plugin audio de saturation extrêmement simple, destiné avant tout à l’apprentissage du développement de VST.
Ce plugin n’a pas vocation, dans sa première version, à être “parfait”, optimisé ou commercialisable. Il sert de terrain de jeu contrôlé pour comprendre :
	•	comment fonctionne un VST dans un DAW (Ableton)
	•	comment le son est traité en temps réel
	•	comment l’UI est reliée au moteur audio
	•	comment structurer un projet audio proprement
	•	comment itérer sans casser l’existant

Le projet doit rester lisible, modifiable et didactique à chaque étape.

⸻

🧠 Philosophie de développement

Ce projet est pensé comme un projet d’apprentissage long, pas comme un sprint.

Principes clés :
	•	le code doit être compréhensible avant d’être performant
	•	chaque fichier doit expliquer son rôle
	•	chaque fonction importante doit être commentée
	•	les variables clés doivent être facilement tweakables
	•	le projet doit encourager l’expérimentation (UI, paramètres, DSP)

Le document PRD est un document vivant :
👉 il doit être mis à jour au fur et à mesure de l’implémentation, avec :
	•	ce qui est fait
	•	ce qui reste à faire
	•	les décisions techniques prises
	•	les problèmes rencontrés et leurs solutions

⸻

📦 Description fonctionnelle du plugin

vst_saturator est un effet audio (pas un instrument).

Fonction principale

Appliquer une saturation soft au signal audio entrant afin d’ajouter de la chaleur, de la densité et de la distorsion harmonique légère.

Paramètres initiaux (v1)
	•	Drive
Contrôle le niveau de gain appliqué avant la saturation.
	•	Output
Contrôle le niveau de sortie après saturation.

Ces paramètres doivent être :
	•	automatisables dans le DAW
	•	visibles et modifiables via l’UI
	•	documentés clairement dans le code

⸻

🧱 Stack technique cible
	•	Langage : C++
	•	Framework : JUCE
	•	Format plugin : VST3
	•	Plateformes :
	•	macOS Apple Silicon + Intel (Universal Binary)
	•	Windows x64

⸻

💻 Environnement de développement (MacBook M3 Pro)

Le développement se fait sur macOS, avec comme objectif une compatibilité maximale.

Le projet doit être structuré pour permettre plus tard :
	•	un build Windows
	•	une extension AU (optionnelle)
	•	des évolutions DSP sans refonte complète

⸻

🗂️ Structure du projet (intentionnelle)

La structure du projet doit être simple, logique et commentée.

Chaque dossier et fichier doit :
	•	avoir un rôle clair
	•	contenir un commentaire de tête expliquant sa responsabilité
	•	éviter les “magical values” non expliquées

Les constantes importantes (plages de paramètres, couleurs UI, comportements DSP) doivent être centralisées et faciles à modifier.

⸻

🎛️ UI — intention et liberté créative

L’interface graphique est volontairement :
	•	simple dans sa structure
	•	libre dans son style

L’objectif est que tu puisses :
	•	changer les couleurs
	•	modifier les tailles
	•	expérimenter des layouts
	•	tester des looks (clean, dirty, cyberpunk, etc.)

👉 Le code UI doit être écrit de manière explorable, pas opaque.

⸻

🔊 DSP — règles fondamentales

Le traitement audio doit respecter strictement les règles du temps réel :
	•	aucune allocation mémoire dans le callback audio
	•	aucun log dans le thread audio
	•	aucun mutex / lock
	•	traitement clair, lisible, commenté

La saturation utilisée au départ est volontairement simple (ex : tanh) afin de se concentrer sur la compréhension plutôt que la sophistication.

⸻

🧪 Objectifs pédagogiques explicites

À la fin de la v1, le projet doit t’avoir permis de comprendre :
	•	comment un DAW appelle un plugin audio
	•	comment les buffers audio sont traités
	•	comment un paramètre UI impacte le DSP
	•	comment structurer un plugin proprement
	•	comment tester et déboguer un VST

⸻

🗺️ Plan d’action détaillé

Phase 0 — Préparation 🧱

Objectif : environnement prêt et projet compilable
Tâches :
	•	installer les outils nécessaires
	•	créer le projet JUCE
	•	configurer le format VST3
	•	compiler un plugin vide

⸻

Phase 1 — Audio pass-through 🔊

Objectif : vérifier que le son traverse le plugin sans modification
Tâches :
	•	implémenter processBlock
	•	s’assurer que le son est inchangé
	•	charger le plugin dans Ableton
	•	documenter le flux audio

⸻

Phase 2 — Paramètres 🎚️

Objectif : contrôler le son via des paramètres
Tâches :
	•	créer Drive et Output
	•	relier les paramètres au moteur audio
	•	vérifier l’automation
	•	commenter le système de paramètres

⸻

Phase 3 — Saturation 🔥

Objectif : ajouter une saturation audible et contrôlable
Tâches :
	•	implémenter la fonction de saturation
	•	gérer le gain staging
	•	tester à différents niveaux
	•	documenter le DSP

⸻

Phase 4 — UI minimale 🎨

Objectif : interface fonctionnelle et modifiable
Tâches :
	•	créer les sliders
	•	relier UI ↔ paramètres
	•	structurer le code UI
	•	commenter les choix visuels

⸻

Phase 5 — Stabilisation 🧪

Objectif : plugin stable et compréhensible
Tâches :
	•	tests à différents sample rates
	•	tests de buffer size
	•	nettoyage du code
	•	mise à jour complète de la documentation

⸻

📝 Documentation & commentaires (obligatoire)

Tout le code doit être :
	•	commenté de manière pédagogique
	•	lisible sans connaissance préalable
	•	structuré pour être relu plus tard

Chaque fichier doit expliquer :
	•	ce qu’il fait
	•	pourquoi il existe
	•	comment il interagit avec les autres

⸻

📌 État du document

Ce PRD :
	•	doit être mis à jour en continu
	•	doit refléter l’état réel du projet
	•	sert de référence centrale

Il est normal qu’il évolue avec :
	•	de nouvelles idées
	•	des contraintes découvertes
	•	des ajustements techniques

⸻

🚀 Prochaine étape

👉 Générer la checklist d’installation exacte + le squelette JUCE minimal + la structure de fichiers commentée