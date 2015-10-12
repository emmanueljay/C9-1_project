Remarque preliminaire:
=========
si le code vous paraît trop lourd au début,
n'hésitez pas à le tailler copieusement au début pour le
réduire à sa version minimale,
et après l'avoir bien compris, 
vous pourrez l'épaissir à nouveau.


Fichiers de code fournis:
=========

1- src/data.h
-------------
Définition de la classe Data et ses méthodes.

2- src/gotic.h
--------------
Essentiellement, classe Solution et ses méthodes.
Contient aussi la définition de solve_frontal et 
solve_lagrangian qui devront résoudre le problème
GOTIC.

3- src/data.cpp
------------
Instantiation des méthodes de la classe Data

4- src/solution.cpp
----------------
Instantiation des méthodes de la classe Solution

5- src/main.cpp
------------
Appels au solveur et aux fonctions de lecture de donnée
et de validateur de solutions.
Switch entre différentes possibilités du programme.


Méthodes à utiliser dans les classes Data et Solution
=========

1. Data::Data(std::string filename);
>> génère une instance à partir d'un fichier d'instance standard.
>> une fois créée, l'instance Data ne change pas.
On accède à ses éléments via des méthodes get

2. Toutes les méthodes de type Data::get...,
ainsi que job_job_dist(j1,j2) et home_job_dist(w, j)
>> compréhension facile.

3. Data::print_data(std::ofstream & out_stream);
>> dirige vers le flux out_stream la description de 
l'instance Data au format standard.

4. Solution::Solution(Data & data);
>> Initialise un objet Solution. Préliminaire à l'appel
d'un solveur comme solve_frontal(data,sol)

5. Solution::add_job_safe(int w, int j, int tstart);
>> Étend la solution en assignant le job j au tic w,
début du job au temps tstart. Utilisé par le validateur
au fur et à mesure de la lecture d'un fichier solution,
mais aussi à utiliser pour le solveur.
>> retourne 0 si tout se passe bien.

6. Solution::add_starting_time(int w, int tstart);
>> Facultatif ou debugage : si vous voulez connaître 
l'heure exacte de départ du tic calculée par le solveur.
>> retourne 0 si tout se passe bien.

7. Solution::are_jobs_done();
>> retourne 0 ou 1.

8. Solution::compute_cost();
>> compréhension facile

9. Solution::print_solution(std::ofstream & out_stream);
>> dirige vers le flux out_stream la solution au format
standard.

10. Solution::print_detail(std::ofstream & out_stream);
>> dirige vers le flux out_stream une version détaillée
de la solution. 
>> Les # en tête de chaque ligne permettent d'ajouter 
cette version détaillée à la fin d'un fichier solution.
>> utile pour le débugage.

11. Solution::read_solution(std::string filename);
>> validateur : lit une solution écrite au format
standard dans le fichier filename et vérifie que la 
solution est admissible. 

12. solve_frontal(Data& data, Solution& sol);
>> Résout le problème frontal associé à l'instance data.
>> Met à jour la solution sol avec la résolution.

13. solve_lagrangian(Data& data, Solution& sol);
>> Résout le problème associé à l'instance data par 
descente de gradient.
>> Met à jour la solution sol avec la résolution.


3 exemples de résolution par le frontal
=========
Pour vérifier vos codes :
gotic_1      0.08 sec.      520      Optimal
gotic_2      0.49 sec.      443      Optimal
gotic_4   3603.78 sec.      539      Feasible (gap 15.34%)

