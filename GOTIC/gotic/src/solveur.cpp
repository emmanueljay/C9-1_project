#include "gotic.h"
#include <ilcplex/ilocplex.h>
// #include <cmath> 
#include <limits> 


typedef IloArray<IloBoolArray>    BoolMatrix;
typedef IloArray<IloBoolVarArray> BoolVarMatrix;
typedef IloArray<IloIntArray>     IntMatrix;
typedef IloArray<IloIntVarArray>  IntVarMatrix;
typedef IloArray<IloNumArray>     NumMatrix;
typedef IloArray<IloNumVarArray>  NumVarMatrix;


void var_sum(
  IloExpr* expression_p, 
  IntVarMatrix const& variables_p,
  bool is_first_index_p,
  int other_index_p,
  int size_p,
  bool non_equal_p = false) 
{
  // std::cout << "Computing the sum of " << size_p
    // <<  " elements with index fixed :" << other_index_p << std::endl;
  for (int i = 0; i < size_p; ++i)
  {
    if (!non_equal_p || i!=other_index_p) {
      if (is_first_index_p) {
        (*expression_p) += variables_p[i][other_index_p];
        // std::cout << "Using Fst index : "<<i<<" - "<< other_index_p<< std::endl;
      }
      else {
        (*expression_p) += variables_p[other_index_p][i];
        // std::cout << "Using Snd index : "<<other_index_p<<" - "<<i<< std::endl;
      }
    }
  }
}

void add_easy_constraints (
  int w, 
  IloEnv* env,
  IloModel* model,
  Data* data,
  IntVarMatrix* n_ij,
  IloIntVarArray* f_i,
  IloIntVarArray* l_i,
  IloIntVarArray* x_i,
  IloNumVarArray* t_i) 
{
  std::cout << "- Addition of constraints for technician : "<< w << std::endl;

  int nb_places = data->get_jmax();
  // int max_time_day = std::numeric_limits<int>::max();
  int max_time_day = 10000;

  /** 1/ MODELISER LES TRAJETS */

  /** (1.1) Loi des noeuds 
   * Pour chaque technicien : si le trajet d’un technicien « entre » dans une 
   * tâche (un nœud), soit en tant que première tâche, soit après une 
   * autre, alors il doit en « sortir » (pour rentrer à sa base ou pour faire
   * une autre tâche).
   **/

  std::cout << "-- Constraint (1.1) : Loi des noeuds" << std::endl;

  for (int i = 0; i < nb_places ; ++i) {
    // std::cout << "Treating w = " << w << " -- i = " << i << std::endl;
    IloExpr sumOverFst(*env,0);
    var_sum(&sumOverFst,(*n_ij),true,i,nb_places,true);
    IloExpr sumOverSnd(*env,0);
    var_sum(&sumOverSnd,(*n_ij),false,i,nb_places,true);
  
    model->add((*f_i)[i] + sumOverFst == (*l_i)[i] + sumOverSnd);
  }

  /** (1.2) Egalité des flots 
   * si le technicien part de la base (c’est-à-dire qu’il réalise une 
   * première tâche), alors il doit également en réaliser une dernière.
   **/      

  std::cout << "-- Constraint (1.2) : Egalité des flots" << std::endl;

  model->add(IloSum(*f_i) == IloSum(*l_i));

  /** (1.3) Une seule boucle 
   * Chaque technicien réalise au plus une tâche initiale.
   **/      

  std::cout << "-- Constraint (1.3) : Une seule boucle" << std::endl;

  model->add(IloSum(*f_i) <= 1);



  /** 2/ REALISATION DES TACHES */

  /** (2.1) Valeur de x_wi */

  std::cout << "-- Constraint (2.1) : Valeur de x_i" << std::endl;

  for (int i = 0; i < nb_places ; ++i) { 
    IloExpr sumOverFst(*env,0);
    var_sum(&sumOverFst,(*n_ij),true,i,nb_places,true);

    model->add((*x_i)[i] == (*f_i)[i] + sumOverFst);
  }



  /** 3/ RESPECT DES CONTRAINTES DE COMPETENCES */

  std::cout << "-- Constraint (3.1) : Respect des compétences" << std::endl;
  
  for (int i = 0; i < nb_places ; ++i) { 

    model->add((*x_i)[i] <= data->has_skill(w,i));
  }



  /** 4/ RESPECT DES HORAIRES DU TECHNICIEN */

  std::cout 
    << "-- Constraint (4.1) : Horaires du technicien --"
    <<" si la tache est réalisée en premier" << std::endl;

  for (int i = 0; i < nb_places ; ++i) {

    model->add(
      data->get_start_time(w) 
      + data->travel_time(data->home_job_dist(w,i))
      <= -1 + (*t_i)[i] 
      + max_time_day*(1-(*f_i)[i]));
  }

  std::cout 
    << "-- Constraint (4.2) : Horaires du technicien --"
    <<" si la tache est réalisée après une tache j" << std::endl;

  for (int i = 0; i < nb_places ; ++i) 
  for (int j = 0; j < nb_places ; ++j)
  if (i != j) { 

    model->add(
      (*t_i)[i] 
      + data->get_job_duration(i) 
      + data->travel_time(data->job_job_dist(i,j))
      <= -1 + (*t_i)[j]
      + max_time_day*(1-(*n_ij)[i][j]));
  }

  std::cout 
    << "-- Constraint (4.2) : Horaires du technicien --"
    <<" Le technicien a le temps de rentrer à sa base" << std::endl;

  for (int i = 0; i < nb_places ; ++i){ 

    model->add(
      (*t_i)[i] 
      + data->get_job_duration(i) 
      + data->travel_time(data->home_job_dist(w,i))
      <= -1 + data->get_end_time(w)
      + max_time_day*(1-(*l_i)[i]));
  }
}

/**
 * Function that will solve the problem with a frontal aproach, using Cplex
 * In this function we will implement a set of variables, constraints, and
 * finaly launch Cplex
 * 
 * @param  data Context of the problem, already filled with the instance
 * @param  sol  Solution data structure to put the computed solution into
 * @return      Return if the problem went correctly
 */
int solve_frontal (Data & data, Solution & sol)
{
    IloEnv       env;
#ifdef NVERBOSE
    env.setOut (env.getNullStream ());   // no output on screen
#endif        

    try {

      /**************
       * CPLEX MODEL
       **************/
      std::cout << std::endl << "Creation of the Cplex Model" << std::endl;

      IloModel model(env);
      IloCplex cplex(env);

      /*******
       * DATA
       *******/

      int nb_workers = data.get_wmax();
      int nb_places = data.get_jmax();

      std::cout 
        << std::endl
        << "Data informations : " << std::endl
        << "-- Number of worker = " << nb_workers << std::endl
        << "-- Number of places = " << nb_places << std::endl;



      /************************** 
       * DEFINITION OF VARIABLES
       **************************/
      std::cout << std::endl << "Definition of variables" << std::endl;

      /** (1) Variables d'arc
       * Vaut 1 si le technicien w réalise la tâche j juste après la tâche i 
       **/

      std::cout << "-- Variables (1) : Variables d'arc n" << std::endl;
      
      IloArray<IntVarMatrix> n_wij(env);
      for (int w = 0; w < nb_workers ; ++w)
      {
        n_wij.add(IntVarMatrix(env));
        for (int i = 0; i < nb_places ; ++i)
          n_wij[w].add(IloIntVarArray(env,nb_places ,0,1));
      }

      /** (2) Variables de premières taches
       * Vaut 1 si le technicien w réalise la tâche i en premier 
       **/

      std::cout << "-- Variables (2) : Variables des premières taches f" 
        << std::endl;

      IntVarMatrix f_wi(env);
      for (int w = 0; w < nb_workers ; ++w)
        f_wi.add(IloIntVarArray(env,nb_places,0,1));

      /** (3) Variables de dernières taches taches
       * Vaut 1 si le technicien w réalise la tâche i en dernier 
       **/

      std::cout << "-- Variables (3) : Variables des dernières taches l" 
        << std::endl;

      IntVarMatrix l_wi(env);
      for (int w = 0; w < nb_workers ; ++w)
        l_wi.add(IloIntVarArray(env,nb_places,0,1));

      /** (4) Variables auxilières de réalisation 
       * Vaut 1 si et seulement si le technicien w réalise la tâche i
       */

      std::cout << "-- Variables (4) : Variables aux de réalisation x" 
        << std::endl;

      IntVarMatrix x_wi(env);
      for (int w = 0; w < nb_workers ; ++w)
        x_wi.add(IloIntVarArray(env,nb_places,0,1));

      /** (5) Variables auxilières continues de temps t 
       * Vaut 1 si et seulement si le technicien w réalise la tâche i
       */

      std::cout <<"-- Variables (5) : Variables auxilières continues de temps t" 
        << std::endl;

      IloNumVarArray t_i(env,nb_places,0,IloInfinity);


      /****************************
       * DEFINITION OF CONSTRAINTS
       ****************************/
      std::cout << std::endl << "Definition of constraints" << std::endl;

      /** (0) Essais */

      // IloExpr sum(env,0);
      // var_sum(&sum,f_wi,false,1,nb_places,false);
      // model.add(sum == 1);
      
      /** ADDITION OF THE EASY CONSTRAINTS (FOR LAGRANGIEN RELAXATION) */

      for (int w = 0; w < nb_workers ; ++w) {
        add_easy_constraints (
          w, 
          &env,
          &model,
          &data,
          &(n_wij[w]),
          &f_wi[w],
          &l_wi[w],
          &x_wi[w],
          &t_i);
      }

      std::cout << "- Addition of additional constraints" << std::endl;

      /** ADDITION OF THE BOUNDING CONSTRAINT (FOR LAGRANGIEN RELAXATION) */

      /** (2.2) Unicité de la réalisation des taches 
       * Chaque tâche est effectuée exactement une fois
       **/

      std::cout << "-- Constraint (2.2) : Unicité de la réalisation des taches "
        << std::endl;

      for (int i = 0; i < nb_places ; ++i) { 
        IloExpr sumOverFst(env,0);
        var_sum(&sumOverFst,x_wi,true,i,nb_workers,false);
        model.add(sumOverFst >= 1);
      }

      /** 5/ RESPECT DES FENETRES DE TEMPS D'INTERVENTION */
      /** Contrainte non liante, mais indépendante des techicien */
      std::cout << "-- Constraint (5.1) : Respect des fenetres" << std::endl;

      for (int i = 0; i < nb_places ; ++i)
      if (data.has_appointment(i) == 1) {

        model.add(data.get_start_appnt(i) <= t_i[i]);
        model.add(t_i[i] <= data.get_end_appnt(i));
      } 

      /******************************
       * DEFINITION OF THE OBJECTIVE
       ******************************/

      std::cout << std::endl << "Addition of the objective" << std::endl;


      IloExpr objective(env,0);
      for (int w = 0; w < nb_workers ; ++w)
      for (int i = 0; i < nb_places ; ++i) {
        objective += data.home_job_dist(w,i) * (f_wi[w][i] + l_wi[w][i]);
      }
      for (int i = 0; i < nb_places ; ++i)
      for (int j = 0; j < nb_places ; ++j) {
        if (j!=i) {
          IloExpr sum_n(env,0);
          for (int w = 0; w < nb_workers ; ++w) {
            sum_n += n_wij[w][i][j];
          }
          objective += data.job_job_dist(i,j) * sum_n;
        }
      }
      
      model.add(IloMinimize(env, objective));



      /******************************************
       * EXTRACTION AND EXPORTATION OF THE MODEL
       ******************************************/

      std::cout << std::endl;

      // Extract model
      cplex.extract(model);

      // Export model
      cplex.exportModel("lp/current.lp");


      /*************
       * RESOLUTION
       *************/

      // Solve
      if ( !cplex.solve() ) {
        env.error() << "Failed to optimize problem" << std::endl;
        cplex.out() << "Solution status " << cplex.getStatus() << std::endl;
        throw(-1);
      }
      
      cplex.out() << "Solution status " << cplex.getStatus() << std::endl;
      cplex.out() << "Objective value " 
        << cplex.getObjValue() << std::endl<< std::endl;




      /****************************************
       * EXTRATION OF THE SOLUTION INTO OBJECT
       ****************************************/


      for (int w = 0; w < nb_workers ; ++w) {
        int position = -1;
        bool continu = false;

        // Departure
        for (int i = 0; i < nb_places ; ++i) {
          if (cplex.getValue(f_wi[w][i]) > 0.5) {
            std::cout << "The worker " << w << " is starting the task " << i 
              << " at time : " << cplex.getValue(t_i[i]) << std::endl;
            sol.add_job_safe(w,i,cplex.getValue(t_i[i]));
            continu = true;
            position =i;
            break;
          }
        }
        // Iteration
        while (continu) {
          continu = false;
          for (int i = 0; i < nb_places ; ++i)
          if (i != position) {
            if (cplex.getValue(n_wij[w][position][i]) > 0.5) {
              std::cout << "The worker " << w << " is then doing the task " << i 
                << " at time : " << cplex.getValue(t_i[i]) << std::endl;
              sol.add_job_safe(w,i,cplex.getValue(t_i[i]));
              continu = true;
              position =i;
              break;
            }
          }
        }
      }
      
      // for (int w = 0; w < nb_workers ; ++w)
      // for (int i = 0; i < nb_places ; ++i) { 
      //   std::cout << "f_wi(" << w << "," << i << ") = " << cplex.getValue(f_wi[w][i]) << std::endl;
      //   std::cout << "l_wi(" << w << "," << i << ") = " << cplex.getValue(l_wi[w][i]) << std::endl;

      //   for (int j = 0; j < nb_places ; ++j) { 
      //     if (i !=j ) 
      //       std::cout << "n_wij(" << w << "," << i << "," << j << ") = " << cplex.getValue(n_wij[w][i][j]) << " || ";
      //   }
      //   std::cout << std::endl;
      // }


      std::cout << std::endl << "Are the job done : " << sol.are_jobs_done() << std::endl;
      std::cout << std::endl << "Cost of the solution : " << sol.compute_cost() << std::endl;

      std::cout << std::endl << "Print Solution :" << std::endl;
      sol.print_solution    (std::cout);
      std::cout << std::endl << "Print Detailed Solution :" << std::endl;
      sol.print_detail      (std::cout);
      // sol.read_and_validate (std::string inc_filename);

      return 1;


    }
    catch (IloException & e) 
    // Gestion des exceptions levées par Concert Technology
    {
      std::cerr << "Exception de la Concert Technology : " << e << std::endl;
    } 
    catch(...) 
    // Les autres exceptions éventuelles levées dans le programme
    {
      std::cerr << "Exception inconnue "<< std::endl;
    } 
    env.end(); 
    // toujours libérer l'environnement, même si on lève une exception

    return 0;
}






double d(
  IloEnv* env,
  std::vector<double> const& lambda,
  Data* data,
  Solution* sol,
  std::vector<int>* xres_i,
  int w,
  bool add_to_solution) 
{
  /**************
   * CPLEX MODEL
   **************/
  std::cout << std::endl << "Creation of the Cplex Model" << std::endl;

  IloModel model(*env);
  IloCplex cplex(*env);

  /*******
   * DATA
   *******/

  int nb_places = data->get_jmax();
  int nb_workers = data->get_wmax();

  std::cout 
    << std::endl
    << "Data informations : " << std::endl
    << "-- Number of worker = " << nb_workers << std::endl
    << "-- Number of places = " << nb_places << std::endl;

  /************************** 
   * DEFINITION OF VARIABLES
   **************************/
  std::cout << std::endl << "Definition of variables" << std::endl;

  /** (1) Variables d'arc
   * Vaut 1 si le technicien w réalise la tâche j juste après la tâche i 
   **/

  std::cout << "-- Variables (1) : Variables d'arc n" << std::endl;
  
  IntVarMatrix n_ij(*env);

  for (int i = 0; i < nb_places ; ++i)
    n_ij.add(IloIntVarArray(*env,nb_places ,0,1));
  

  /** (2) Variables de premières taches
   * Vaut 1 si le technicien w réalise la tâche i en premier 
   **/

  std::cout << "-- Variables (2) : Variables des premières taches f" 
    << std::endl;

  IloIntVarArray f_i(*env,nb_places,0,1);

  /** (3) Variables de dernières taches taches
   * Vaut 1 si le technicien w réalise la tâche i en dernier 
   **/

  std::cout << "-- Variables (3) : Variables des dernières taches l" 
    << std::endl;

  IloIntVarArray l_i(*env,nb_places,0,1);

  /** (4) Variables auxilières de réalisation 
   * Vaut 1 si et seulement si le technicien w réalise la tâche i
   */

  std::cout << "-- Variables (4) : Variables aux de réalisation x" 
    << std::endl;

  IloIntVarArray x_i(*env,nb_places,0,1);

  /** (5) Variables auxilières continues de temps t 
   * Vaut 1 si et seulement si le technicien w réalise la tâche i
   */

  std::cout <<"-- Variables (5) : Variables auxilières continues de temps t" 
    << std::endl;

  IloNumVarArray t_i(*env,nb_places,0,IloInfinity);


  /****************************
   * DEFINITION OF CONSTRAINTS
   ****************************/
  std::cout << std::endl << "Definition of constraints" << std::endl;

  
  /** ADDITION OF THE EASY CONSTRAINTS (FOR LAGRANGIEN RELAXATION) */

  add_easy_constraints (
    w, 
    env,
    &model,
    data,
    &n_ij,
    &f_i,
    &l_i,
    &x_i,
    &t_i);

  std::cout << "- Addition of additional constraints" << std::endl;

  /** 5/ RESPECT DES FENETRES DE TEMPS D'INTERVENTION */
  /** Contrainte non liante, mais indépendante des techicien */
  std::cout << "-- Constraint (5.1) : Respect des fenetres" << std::endl;

  for (int i = 0; i < nb_places ; ++i)
  if (data->has_appointment(i) == 1) {

    model.add(data->get_start_appnt(i) <= t_i[i]);
    model.add(t_i[i] <= data->get_end_appnt(i));
  } 

  /*****************************
   * DEFINITION OF THE OBJECTIVE
   *****************************/

  std::cout << std::endl << "Addition of the objective" << std::endl;


  IloExpr objective(*env,0);
  for (int w = 0; w < nb_workers ; ++w)
  for (int i = 0; i < nb_places ; ++i) {
    objective += data->home_job_dist(w,i) * (f_i[i] + l_i[i]);
  }
  for (int i = 0; i < nb_places ; ++i)
  for (int j = 0; j < nb_places ; ++j) {
    if (j!=i) {
      IloExpr sum_n(*env,0);
      for (int w = 0; w < nb_workers ; ++w) {
        sum_n += n_ij[i][j];
      }
      objective += data->job_job_dist(i,j) * sum_n;
    }
  }
  for (int i = 0; i < nb_places ; ++i) {
    objective -= lambda[i]*(x_i[i]-1/nb_workers);
  }

  model.add(IloMinimize(*env, objective));



  /******************************************
   * EXTRACTION AND EXPORTATION OF THE MODEL
   ******************************************/

  std::cout << std::endl;

  // Extract model
  cplex.extract(model);

  // Export model
  cplex.exportModel("lp/current.lp");


  /*************
   * RESOLUTION
   *************/

  // Solve
  if ( !cplex.solve() ) {
    env->error() << "Failed to optimize problem" << std::endl;
    cplex.out() << "Solution status " << cplex.getStatus() << std::endl;
    throw(-1);
  }
  
  cplex.out() << "Solution status " << cplex.getStatus() << std::endl;
  cplex.out() << "Objective value " 
    << cplex.getObjValue() << std::endl<< std::endl;


  /****************************************
   * EXTRATION OF THE SOLUTION INTO OBJECT
   ****************************************/

  std::cout << std::endl;
  for (int i = 0; i < nb_places ; ++i) {
    xres_i->at(i) += cplex.getValue(x_i[i]);
    std::cout << "x(" << i << ") = " << xres_i->at(i) << std::endl;
  }

  if (add_to_solution) {
    int position = -1;
    bool continu = false;

    // Departure
    for (int i = 0; i < nb_places ; ++i)
    if (cplex.getValue(f_i[i]) == 1) {
      std::cout << "The worker " << w << " is starting the task " << i 
        << " at time : " << cplex.getValue(t_i[i]) << std::endl;
      sol->add_job_safe(w,i,cplex.getValue(t_i[i]));
      continu = true;
      position =i;
      break;
    }

    // Iteration
    while (continu) {
      continu = false;
      for (int i = 0; i < nb_places ; ++i)
      if (i != position)
      if (cplex.getValue(n_ij[position][i]) == 1) {
        std::cout << "The worker " << w << " is then doing the task " << i 
          << " at time : " << cplex.getValue(t_i[i]) << std::endl;
        sol->add_job_safe(w,i,cplex.getValue(t_i[i]));
        continu = true;
        position =i;
        break;
      }
    }
  }

  //   // // Iteration over jobs
  //   // for (int i = 0; i < nb_places ; ++i)
  //   // if (cplex.getValue(x_i[w][i]) == 1) {
  //   //   std::cout << "The worker " << w << " is doing the task " << i 
  //   //     << " and starting at time : " << cplex.getValue(t_i[i]) << std::endl;
  //   //   sol.add_job_safe(w,i,cplex.getValue(t_i[i]));
  //   // }
  // }
  

    // IloNum cplex.getValue()
  // sol.add_job_safe      (int w, int j, int tstart);
  // sol.add_starting_time (int w, int tstart);

  // std::cout << std::endl << "Are the job done : " << sol.are_jobs_done() << std::endl;
  // std::cout << std::endl << "Cost of the solution : " << sol.compute_cost() << std::endl;

  // std::cout << std::endl << "Print Solution :" << std::endl;
  // sol.print_solution    (std::cout);
  // std::cout << std::endl << "Print Detailed Solution :" << std::endl;
  // sol.print_detail      (std::cout);
  // sol.read_and_validate (std::string inc_filename);

  return cplex.getObjValue();

}





int solve_lagrangian (Data & data, Solution & sol)
{
  IloEnv       env;
#ifdef NVERBOSE
  env.setOut (env.getNullStream ());   // no output on screen
#endif        

  // lambda[0] = 1500;
  // lambda[1] = 400;
  // lambda[2] = 1500;
  // lambda[3] = 250;
  // lambda[4] = 350;
  // lambda[5] = 750;
  // lambda[6] = 1000;

  /** FIND AN ESTIMATION OF RHO_0 */

  int k = 0;
  double rho = 1000.0;
  double epsilon = 0.001;
  double nb_max_iteration = 1000;
  std::vector<double> lambda(data.get_jmax(),rho);
  double diff = std::numeric_limits<double>::max();
  double val_opt_k = std::numeric_limits<double>::max();
  double val_opt_km1;
  /** ALGORITHME DE DECENTE DE GRADIENT */

  while (diff > epsilon && k < nb_max_iteration) {
    val_opt_km1 = val_opt_k;

    /** Computation of the solution */
    std::vector<int> xres_i(data.get_jmax(),0);
    val_opt_k = 0;
    for (int w = 0; w < data.get_wmax() ; ++w)
      val_opt_k += d(
        &env,
        lambda,
        &data,
        &sol,
        &xres_i,
        w,
        false);

    for (int i = 0; i < data.get_jmax() ; ++i) {
      lambda[i] = lambda[i] - rho/(k+1)*(xres_i[i]-1);
      if (lambda[i] < 0)
        lambda[i] = 0;
    }
    diff = std::abs(val_opt_k - val_opt_km1);

    k++;


    std::cout << std::endl << std::endl 
      << "Value of the optimiation program : "
      << val_opt_k  << std::endl;

    std::cout << std::endl;
    for (int i = 0; i < data.get_jmax() ; ++i) {
      std::cout << "x(" << i << ") = " << xres_i.at(i) << std::endl;
    }

    std::cout << std::endl;
    for (int i = 0; i < data.get_jmax() ; ++i) {
      std::cout << "lambda(" << i << ") = " << lambda.at(i) << std::endl;
    }
  }

  std::vector<int> xres_i(data.get_jmax(),0);
  for (int w = 0; w < data.get_wmax() ; ++w)
    val_opt_k += d(
      &env,
      lambda,
      &data,
      &sol,
      &xres_i,
      w,
      true);

  std::cout << std::endl << "Are the job done : " << sol.are_jobs_done() << std::endl;
  std::cout << std::endl << "Cost of the solution : " << sol.compute_cost() << std::endl;

  std::cout << std::endl << "Print Solution :" << std::endl;
  sol.print_solution    (std::cout);
  std::cout << std::endl << "Print Detailed Solution :" << std::endl;
  sol.print_detail      (std::cout);
  // sol.read_and_validate (std::string inc_filename);

  return 1;
}
