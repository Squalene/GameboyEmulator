Commentaires généraux:

1) Nous avons dû baisser GB_CYCLES_PER_S de 2^20 à 2^19 (macro OUR_GB_CYCLES_PER_S dans gameboy.h) car avec 2^20, notre gameboy n'avait pas le temps de dessiner l'image et laggait énormément. Nous pensons que notre erreur vient de bit_vector_extract_wrap_ext qui n'est pas assez optimisée, car nous avons utilisé de nombreuses autres méthodes de manipulation de bit_vector pour pouvoir gérer les cas où la size n'est pas un multiple de 32 (nous avions une méthode efficace pour ce cas, fonctionnant sur le modèle de bit_vector_extract_zero_ext, mais nous l'avons recodée car nous ne voyions pas comment l'adapter aux cas dont la size est plus petite par exemple et nous préférions avoir une méthode fonctionnant dans tous les cas testés).

2) Notre makefile produit 4 exécutable/test unitaire en trop (unit-test-old-bit-vector, test-image, unit-test-cpu-dispatch et unit-test-alu_ext), que nous avons décidé de laisser dans notre makefile puisqu'ils nous avaient été utiles tout au long du projet.

3) Le test pour Fibonnaci (test-gameboy) fonctionne mais avec un peu plus de cycles (~2'180'000 au lieu de 2'165'183). Il fonctionnait correctement avant la dernière étape et nous avons aucune idée d'où cela peut venir puisqu'il nous semble pas avoir touché à des parties en rapport avec ça (et le test de blargg instr_timing.gb fonctionne correctement).



FAQ:

1) Jusqu’où avez-vous été ? Jusqu'à la fin. Nous avons tout codé mais pas réoptimisé bit_vector_extract_wrap_ext car nous avions déjà passé de nombreuses heures dessus.

2) Combien d’heures en moyenne par personne estimez vous avoir passé par semaine sur le projet (dans son ensemble, y.c. premières semaines individuelles) ? Environ 7h15 par semaine par personne.
