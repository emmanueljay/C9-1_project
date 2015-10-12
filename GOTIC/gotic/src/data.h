/** \file
 *  \brief Definition of the model main data structures.
 */

#ifndef TIC_DATA_H_INCLUDED
#define TIC_DATA_H_INCLUDED

#include <fstream>
#include <iostream>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <cmath> /* sqrt */
#include <iomanip>  /*setw, setfill*/ 


#define NAME_SIZE    256
#define NAME_SCANF  "%256[a-zA-Z0-9_]"

typedef unsigned long ulong;   /* used for skills */

struct Location {
        int x;
        int y;
};

/**
 * Information associated with each each job.
 */
struct Job {
        Location   location;   /**< Location of the job */
        int        skill;      /**< Skill required to perform the job */
        int        duration;   /**< Job duration, in minutes */
        int        penalty;    /**< Penalty if the job is not completed */
        int        appointment; /**< 1 if the job has to start in a defined schedule */
        int        tmin,tmax;   /**< interval in which the job has to start */
};

struct Worker {
        Location  home;        /**< Position at the beginning and at the end of the day */
        int       tstart;      /**< Start time of the day of work (in minutes) */
        int       tend;        /**< End time of the day of work (in minutes) */
        ulong     skills;      /**< Bitset of the worker skills */
};


class Data
{
public:
    Data();
    Data(std::string inc_name, int nw, int nj, int ns, int c);
    Data(std::string filename);

    int           get_wmax          ();
    int           get_jmax          ();
    std::string   get_name          ();
    const int     has_skill         (int w,  int j);
    int           get_job_duration  (int j);
    int           get_penalty       (int j);
    int           get_end_time      (int w);
    int           get_start_time    (int w);
    int           has_appointment   (int j);
    int           get_start_appnt   (int j);
    int           get_end_appnt     (int j);
    int           get_home_x        (int w);
    int           get_home_y        (int w);
    int           get_job_x         (int j);
    int           get_job_y         (int j);
    int           job_job_dist      (int j1, int j2);
    int           home_job_dist     (int w,  int j);
    int           travel_time       (int dist);   

    // Add a template for thoses functions
    void          print_data       (std::ofstream& out_stream);
    void          print_data       (std::ostream& out_stream);

private:
    std::string      name;
    int              worker_max;
    int              job_max;
    int              skill_max;
    int              speed;
    std::vector<Worker>   workers;      
    std::vector<Job>      jobs;

    /* data */
};

#endif /* TIC_DATA_H_INCLUDED */
