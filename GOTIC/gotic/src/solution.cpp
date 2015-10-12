#include "gotic.h"

namespace{
Schedule_node *
last_node (Circuit  & c)
{
        Schedule_node *node = c.schedule;
        Schedule_node *prev = node;

        while (node != NULL) {
                prev = node;
                node = node->next;
        }
        return prev;
}

int
last_job (Circuit & c)
{
        assert (last_node (c) != NULL);
        return last_node (c)->job;
}


void
destroy_circuit (Circuit & c)
{
        Schedule_node *n    = c.schedule;
        Schedule_node *prev = c.schedule;
        
        while (n != NULL) {
                n = prev->next;
                free (prev);
                prev = n;
        }
}

void
print_time (std::ostream & out_stream, int t)
{
        out_stream << "#" << std::setw(5) <<  t << " ";
}

}



Solution::
Solution (Data & d)
{
    int w;
    circuits = std::vector<Circuit> (d.get_wmax());
    job_done = std::vector<int>     (d.get_jmax());

    worker_max = d.get_wmax();
    job_max    = d.get_jmax();

    for (w=0; w< worker_max; ++w) {
        circuits[w].schedule    = NULL;
        circuits[w].location.x  = d.get_home_x (w);
        circuits[w].location.y  = d.get_home_y (w);
        circuits[w].time        = d. get_start_time (w);
        circuits[w].length      = 0;
        circuits[w].home_length = 0;
        circuits[w].start_work_time = 480;

    }

    data = &d;
}


Solution::~Solution ()
{
    for (int w=0; w < worker_max; ++w) {
        destroy_circuit (circuits[w]);
    }
}

// gives the length from the last element of the circuit to j. 
//If there is no job in the circuit, length from w start to job j.
int Solution::
length_to (int w, int j) 
{
    Circuit & c = circuits[w];
    if (c.schedule == NULL) {
        assert (c.length == 0);
        return data->home_job_dist (w, j);
    } else {
        return data->job_job_dist (last_job (c), j);
    }
}

int Solution::
add_job (int w, int j, int tstart)
{
    Schedule_node *p = last_node (circuits[w]);
    Schedule_node * n = new Schedule_node;

    assert (w < worker_max);
    assert (w >= 0);
    assert (j < job_max);
    assert (j >= 0);
    assert (tstart >= 0);
    
    circuits[w].location.x   = data->get_job_x (j);
    circuits[w].location.y   = data->get_job_y (j);
    circuits[w].time         = tstart + data->get_job_duration (j);
    circuits[w].length      += this->length_to (w, j);
    circuits[w].home_length  = data->home_job_dist (w, j);
    
    // s'il y a un last job, alors n est le suivant
    if (p) {
        p->next   = n;
    } else { // sinon, il n'y a aucun job : n est le premier schedule.
        assert (circuits[w].schedule == NULL) ;
        circuits[w].schedule = n;
    }
    n->next   = NULL;
    n->tstart = tstart;
    n->job    = j;
    
    assert (job_done [j] == 0);
    job_done [j] = 1;

    return 0;
}


int Solution::
add_job_safe (int w, int j, int tstart)
{
    int go_len    = this->length_to (w, j);
    int back_len  = data->home_job_dist (w, j);
    int back_time = data->travel_time (back_len);
    int go_time   = data->travel_time (go_len);
    
    if (data->has_appointment(j)) {
        if ((tstart < data->get_start_appnt(j)) || (tstart > data->get_end_appnt(j) )) {
            std::cerr << "Job " << j << " started by worker " << w << " at time " << tstart;
            std::cerr << " out of appointment schedule [" << data->get_start_appnt(j);
            std::cerr << ", " << data->get_end_appnt(j) << "]" << std::endl;
        }
    }

    if (circuits[w].time + go_time > tstart) {
        std::cerr << "Worker " << w << " does not have the time to go to the job ";
        std::cerr << j << " before " << tstart << "!" << std::endl;
        std::cerr << "The minimum starting time would be " << (circuits[w].time + go_time);
        std::cerr << " for this job." << std::endl;
        return -1;
    }

    if (tstart + data->get_job_duration(j) + back_time > data->get_end_time (w)) {
        std::cerr << "Worker " << w << " does not have the time to go back home ";
        std::cerr << "if he does the job " << j << " at " << tstart << "!" << std::endl;
        return -1;
    }
    if (! data->has_skill ( w, j)) {
        std::cerr << "Worker " << w << " does not have the skill ";
        std::cerr << "to do the job " << j << "!" << std::endl;
        return -1;
    }
    return this->add_job (w, j, tstart);
}

int 
Solution::add_starting_time (int w, int tstart)
{
    assert (tstart >= data->get_start_time (w));
    assert(tstart <= data->get_end_time (w));

    circuits[w].start_work_time = tstart;
    return 0;
}

int Solution::
compute_cost ()
{
    int w;
    cost = 0;
    for (w=0; w < worker_max; ++w) {
            cost += circuits[w].length + circuits[w].home_length;
    }
    // version avec penalites.
    // for (int j=0; j < data->get_job_max (); ++j) {
    //         if (! job_done [j])
    //                 cost += data->penalty (j);
    // }

    //debug
    // std::cout << "cost computed : " << cost << std::endl;
    //end debug
    return cost;
}


void Solution::
print_solution (std::ostream & out_stream)
{
    int w;
    
    out_stream << "#            instance    cost" << std::endl;
    out_stream << "solution " << std::setw(12) << data->get_name() << std::setw(8) << cost << std::endl;

    for (w = 0; w < worker_max; ++w) {
        Schedule_node * n = circuits[w].schedule;
        while (n != NULL) {
            out_stream << "tic_" << std::setfill('0') << std::setw(2) << w << std::setfill(' ');
            out_stream << "  job_" << std::setfill('0') << std::setw(3) << n-> job << std::setfill(' ');
            out_stream << std::setw(6) << n->tstart << std::endl;
            n = n->next;
        }
    }
}


void Solution::
print_detail (std::ostream & out_stream)
{
    int w;
    int t;
    int dist;
    int ccost;
    
    int ccost_tot = 0;
    
    for (w = 0; w < worker_max; ++w) {
        Schedule_node const * n  = circuits[w].schedule;
        Schedule_node const * pn = NULL;
        ccost = 0;

        out_stream << "#" << std::endl;
        out_stream << "#Circuit for tic_" << std::setfill('0') << std::setw(2) << w << std::setfill(' ');
        out_stream << ", cost " <<  (circuits[w].length + circuits[w].home_length) << std::endl;
        t = data->get_start_time (w);
        print_time (out_stream, t);
        out_stream << "day starts" << std::endl;
        t = circuits[w].start_work_time;
        print_time (out_stream, t);
        out_stream << "starts at home (" << data->get_home_x(w) << ", ";
        out_stream << data->get_home_y(w) << ")"<< std::endl;
        while (n != NULL) {
            if (pn == NULL)
                dist = data->home_job_dist (w, n->job);
            else
                dist = data->job_job_dist  (n->job, pn->job);
                            
            ccost += dist;
            t += data->travel_time (dist);
            print_time (out_stream, t);
            out_stream << "arrives at (" << data->get_job_x (n->job) << ", " << data->get_job_y (n->job);
            out_stream << "): " << dist << "km in " << data->travel_time(dist) << "min" << std::endl; 
            
            assert (n->tstart >= t);
            
            t = n->tstart;
            print_time(out_stream, t);
            out_stream << "starts job job_" << std::setfill('0') << std::setw(3) << n->job;
            out_stream << std::setfill(' ') << std::endl;
            t += data->get_job_duration (n->job);
            print_time(out_stream, t);
            out_stream << "job finished" << std::endl;
            
            pn = n;
            n  = n->next;
        }
        if (pn != NULL) /* There was at least one job */
                dist = data->home_job_dist (w, pn->job);
        else
                dist = 0;
        
        assert (ccost == circuits[w].length);
        assert (dist == circuits[w].home_length);
        
        t += data->travel_time (dist);
        print_time(out_stream, t);
        ccost += dist;
        out_stream << "arrives home (" << data->get_home_x (w) << ", " << data->get_home_y (w) << "): ";
        out_stream << data->travel_time (dist) << "km in " << dist << "min" << std::endl;
        
        assert (data->get_end_time (w) >= t);
        
        print_time (out_stream, data->get_end_time (w));
        out_stream << "days ends" << std::endl;

        ccost_tot += ccost;
    }

    out_stream << std::endl;
    out_stream << "#Total cost " << std::setw(7) << ccost_tot << std::endl;
}

int Solution::are_jobs_done () 
{
    for (int j = 0; j<job_max; j++) {
        if (!job_done[j]) {
            std::cerr << "Job " << j << " was not completed" << std::endl;
            return 0;
        }
    }
    return 1;
}

int Solution::
read_and_validate (std::string inc_filename)  // mostly C code
{
    char const *filename = inc_filename.c_str();
    char       *line     = NULL;  /* line buffer */
    size_t      lsize    = 0;     /* alloc size of the line buffer */
    char const *lpos;             /* position in the line buffer */
    int         lineno   = 0;     /* line number */
    char const *errmsg   = NULL;  /* error message */
    
    /*  The 'parser' is implemented as a very simple
     *  state machine */
    enum {
            st_solution,
            st_line
    } state = st_solution;
    
    int       cost;
    char      name  [NAME_SIZE];  /* worker, job, or instance name */
    int       w;
    int       j;
    int       t;
    
    FILE     *f = fopen (filename, "r");

    if (!f) {
        std::cerr << "Unable to open data file \"" << filename << "\" (for reading)" << std::endl;
        return 0;
    }
    
    while (getline (&line, &lsize, f) != -1) {
        
        lineno++;
        lpos = line;
        while (((lpos - line) != lsize) && (isspace (*lpos)))
            lpos++;
        
        if ((*lpos == '\0') || (*lpos == '#')) {
            continue;  /* empty or comment */
        }
        
        /* scanf does not add the trailing '\0' with '%c' */
        memset (name, 0, 256);
        
        switch (state) {
                
        case st_solution:
            if (sscanf (lpos, "solution "NAME_SCANF" %d", name, &cost) != 2) {
                errmsg = "solution initial line";
                goto parse_error;
            }
            if (strcmp (name, (data->get_name()).c_str()) != 0) {
                std::cerr << "solution instance name (" << name << ")" << std::endl;
                std::cerr << "does not match current instance name " << data->get_name() << std::endl;
                errmsg = "invalid solution file";
                goto parse_error;
            }
            state = st_line;
            break;
                
        case st_line:

            if (sscanf (lpos, "tic_%d job_%d %d", &w, &j, &t) != 3) {
                errmsg = "Expected a line of the form 'tic job time'";
                goto parse_error;
            }

            if (this->add_job_safe (w, j, t)) {
                errmsg = "Unable to add this job";
                goto parse_error;
            }
            
            break;
                
        default:                
            assert (0 && "invalid code path");
        }
    }

    if (!are_jobs_done())
        return 0;


    if (cost != this->compute_cost ()) {
        std::cerr << "Error, the indicated cost (" << cost << ") does not match" << std::endl;
        std::cerr << "the actual cost " << this->compute_cost() << " of the solution." << std::endl;
        return 0;
    }
    return 1;

parse_error:
        assert (errmsg);
        std::cerr << "invalid line " << lineno << " (" << errmsg << "):" << std::endl << line << std::endl;
        return 0;
}
