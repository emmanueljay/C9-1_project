#ifndef TIC_GOTIC_H_INCLUDED
#define TIC_GOTIC_H_INCLUDED

#include "data.h"

struct Schedule_node {
        Schedule_node *next;
        int            tstart;
        int            job;
};

struct Circuit {
        Schedule_node *schedule;
        Location       location;    /* current location */
        int            time;        /* current time (last job finished) */
        int            length;      /* never include the length to 'go back' at the end */
        int            home_length; /* length to 'go back' to the home at the end */
        int            start_work_time; /* beginning of the work */
};


class Solution{
public:
    Solution(Data & data);
    // Solution(Data const *data,char const *filename);
    ~Solution();

    int     add_job_safe      (int w, int j, int tstart);
    int     add_starting_time (int w, int tstart);
    int     are_jobs_done     ();
    int     compute_cost      ();
    void    print_solution    (std::ostream & out_stream);
    void    print_detail      (std::ostream & out_stream);
    int     read_and_validate (std::string inc_filename);

private:
    std::vector<Circuit>  circuits;
    std::vector<int>      job_done;
    int                   cost;
    Data                 *data;
    int                   worker_max;
    int                   job_max;
     
    int  length_to (int w, int j);
    int  add_job   (int w, int j, int tstart);

};

int   solve_frontal    (Data& data, Solution& sol);
int   solve_lagrangian (Data& data, Solution& sol);


#endif /* TIC_GOTIC_H_INCLUDED */