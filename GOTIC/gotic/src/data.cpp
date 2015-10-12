#include "data.h"


namespace{
int
round_nearest (double d)
{
        return (int)(d + 0.5);
}

#define SQUARE(x) ((x)*(x))
int
location_dist (Location & l1, Location & l2)
{
        return round_nearest (std::sqrt (SQUARE(l1.x - l2.x) + SQUARE(l1.y - l2.y)));
}
}

Data::Data(std::string inc_name, int nw, int nj, int ns, int c)
{
    name = inc_name;
    worker_max = nw;
    job_max = nj;
    skill_max = ns;
    speed = c;

    workers = std::vector<Worker> (worker_max);
    jobs = std::vector<Job> (job_max);
}

Data::Data (std::string inc_filename) /* warning: mostly C code */
{
    char const *filename = inc_filename.c_str();
    char       *line   = NULL;  /* line buffer */
    size_t      lsize  = 0;     /* alloc size of the line buffer */
    char const *lpos;           /* position in the line buffer */
    int         lineno = 0;     /* line number */
    int         offset;         /* output of scanf (%n) */
    char       *endpos;         /* output of strtod */
    char const *errmsg = NULL;  /* error message */
    
    /*  The 'parser' is implemented as a very simple
     *  state machine */
    enum {
        st_instance,
        st_worker,
        st_job,
        st_end
    } state = st_instance;
    
    int       nw, nj, ns, v;     /* see `alloc_and_init_data` */
    int       skill;             /* current skill of a worker */
    char      inc_name [NAME_SIZE];  /* worker, job, or instance name */
    int   w = 0;          /* current worker */
    int   j = 0;          /* current job */
    // Worker & worker;
    // Job & job;

    FILE     *f = fopen (filename, "r");

    if (!f) {
        std::cerr << "Unable to open data file \"" << filename <<"\" (for reading)" << std::endl;
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
        memset (inc_name, 0, 256);
        
        switch (state) {
                   
        case st_instance:
            if (sscanf (lpos, "instance "NAME_SCANF" %d %d %d %d", inc_name, &nw, &nj, &ns, &v) != 5) {
                    errmsg = "instance initial line";
                    goto parse_error;
            }

            name = std::string(inc_name);
            workers = std::vector<Worker> (nw);
            jobs = std::vector<Job> (nj);

            worker_max = nw;
            job_max    = nj;
            skill_max  = ns;
            speed      = v;
            // stop.

            state = st_worker;
            break;
                
        case st_worker:
            assert (w < worker_max);
            
            if (sscanf (lpos, "tic "NAME_SCANF" %d %d %d %d%n",
                        inc_name,
                        &(workers[w].home.x),
                        &(workers[w].home.y),
                        &(workers[w].tstart),
                        &(workers[w].tend),
                        &offset)
                != 5) {
                    errmsg = "Expected a 'worker' line";
                    goto parse_error;
            }
            
            lpos += offset;
            for (;;) {
                    skill = strtol (lpos, &endpos, 10);
                    if (endpos == lpos)
                            break;
                    
                    workers[w].skills |= (1 << skill);
                    lpos = endpos;
            }
            
            ++w;
            if (w == worker_max)
                    state = st_job;

            break;
                
        case st_job:  
            assert (j < job_max);
            
            if (sscanf (lpos, "job "NAME_SCANF" %d %d %d %d %d %d %d %d",
                        inc_name,
                        &(jobs[j].location.x),
                        &(jobs[j].location.y),
                        &(jobs[j].tmin),
                        &(jobs[j].tmax),
                        &(jobs[j].skill),
                        &(jobs[j].duration),
                        &(jobs[j].appointment),
                        &(jobs[j].penalty))
                != 9) {
                    errmsg = "expected a 'job' line";
                    goto parse_error;
            }
            
            ++j;
            if (j == job_max)
                    state = st_end;

            break;
                
        case st_end:
            if (strncmp (lpos, "end", 3) != 0) {
                    errmsg = "expected 'end'";
                    goto parse_error;
            }

            goto initialization_done;
                
        default:
            assert (0 && "invalid code path");
                
        }
    }
        // return 0;
        
parse_error:
    assert (errmsg);
    std::cerr << "invalid line " << lineno << " ("<< errmsg << "):" << std::endl;
    std::cerr << line << std::endl;

initialization_done:
    // std::cout << "Initialization done." << std::endl;
    return;   
}

int
Data::get_wmax ()
{
    return worker_max;
}

int
Data::get_jmax ()
{
    return job_max;
}

std::string
Data::get_name ()
{
    return name;
}

const int
Data::has_skill (int w,  int j)
{
    assert (w < worker_max);
    assert (w >= 0);
    assert (j < job_max);
    assert (j >= 0);
    
    return (workers[w].skills) & (1 << (jobs[j].skill));  /* is the bit #jobs[j].skill equal to 1? */
}

int
Data::get_job_duration (int j)
{
    assert (j < job_max);
    assert (j >= 0);
    
    return jobs[j].duration;

}

int
Data::get_penalty (int j)
{
    assert (j < job_max);
    assert (j >= 0);
    return jobs[j].penalty;
}

int
Data::get_end_time (int w)
{
    assert (w < worker_max);
    assert (w >= 0);
    return workers[w].tend;
}

int
Data::get_start_time (int w)
{
    assert (w < worker_max);
    assert (w >= 0);
    return workers[w].tstart;
}

int
Data::has_appointment (int j)
{
    assert (j < job_max);
    assert (j >= 0);
    return jobs[j].appointment;
}

int
Data::get_start_appnt (int j)
{
    assert (j < job_max);
    assert (j >= 0);
    return jobs[j].tmin;
}

int
Data::get_end_appnt (int j)
{
    assert (j < job_max);
    assert (j >= 0);
    return jobs[j].tmax;
}

int
Data::get_home_x (int w)
{
    assert (w < worker_max);
    assert (w >= 0);
    return workers[w].home.x;
}

int
Data::get_home_y (int w)
{
    assert (w < worker_max);
    assert (w >= 0);
    return workers[w].home.y;
}

int
Data::get_job_x  (int j)
{
    assert (j < job_max);
    assert (j >= 0);
    return jobs[j].location.x;
}

int
Data::get_job_y  (int j)
{
    assert (j < job_max);
    assert (j >= 0);
    return jobs[j].location.y;
}


int
Data::job_job_dist (int job1, int job2)
{
    assert (job1 < job_max);
    assert (job1 >= 0);
    assert (job2 < job_max);
    assert (job2 >= 0);
    return location_dist ((jobs[job1].location), (jobs[job2].location));
}

int
Data::home_job_dist (int w, int j)
{
    assert (w < worker_max);
    assert (w >= 0);
    assert (j < job_max);
    assert (j >= 0);
    return location_dist ((jobs[j].location),(workers[w].home));
}

int
Data::travel_time (int dist)
{
    double const hours = ((double)dist) / ((double)(speed));
    return round_nearest (hours * 60.0);
}


// Add a template for this function
void
Data::print_data (std::ofstream& out_stream)
{
    assert (job_max < 1000);
    assert (worker_max < 100);

    int i;
    std::string str1 = "#                        name    w   j   s   v";
    out_stream << str1 << std::endl;
    out_stream << "instance " << std::setw(20) << name << std::setw(5) << worker_max << std::setw(4) << job_max;
    out_stream << std::setw(4) << skill_max << std::setw(4) << speed << std::endl << std::endl;
    out_stream << "#         id   x   y     Ts    Te   skills" << std::endl;
    for (i=0; i < worker_max; ++i) {
        Worker & worker = workers[i];
        int s;
        
        out_stream << "tic   tic_" << std::setfill('0') << std::setw(2) << i << std::setfill(' ');
        out_stream << std::setw(4) << worker.home.x << std::setw(4) << worker.home.y;
        out_stream << std::setw(7) << worker.tstart << std::setw(7) << worker.tend << " ";
        for (s=0; s < skill_max; ++s) {
            if (worker.skills & (1 << s)) {
                out_stream << " " << s;
            }
        }
        out_stream << std::endl;
    }
    out_stream << std::endl;

    out_stream << "#          id    x   y   tmin   tmax    s    d   r      p" << std::endl;
    for (i=0; i < job_max; ++i) {
        Job & job = jobs[i];

        out_stream << "job   job_" << std::setfill('0') << std::setw(3) << i << std::setfill(' ');
        out_stream << std::setw(5) << job.location.x << std::setw(4) << job.location.y;
        out_stream << std::setw(7) << job.tmin << std::setw(7) << job.tmax;
        out_stream << std::setw(5) << job.skill;
        out_stream << std::setw(5) << job.duration;
        out_stream << std::setw(4) << job.appointment;
        out_stream << std::setw(7) << job.penalty;
        out_stream << std::endl;
    }
    out_stream << std::endl << "end" << std::endl;
}

void
Data::print_data (std::ostream& out_stream)
{
    assert (job_max < 1000);
    assert (worker_max < 100);

    int i;
    std::string str1 = "#                        name    w   j   s   v";
    out_stream << str1 << std::endl;
    out_stream << "instance " << std::setw(20) << name << std::setw(5) << worker_max << std::setw(4) << job_max;
    out_stream << std::setw(4) << skill_max << std::setw(4) << speed << std::endl << std::endl;
    out_stream << "#         id   x   y     Ts    Te   skills" << std::endl;
    for (i=0; i < worker_max; ++i) {
        Worker & worker = workers[i];
        int s;
        
        out_stream << "tic   tic_" << std::setfill('0') << std::setw(2) << i << std::setfill(' ');
        out_stream << std::setw(4) << worker.home.x << std::setw(4) << worker.home.y;
        out_stream << std::setw(7) << worker.tstart << std::setw(7) << worker.tend << " ";
        for (s=0; s < skill_max; ++s) {
            if (worker.skills & (1 << s)) {
                out_stream << " " << s;
            }
        }
        out_stream << std::endl;
    }
    out_stream << std::endl;

    out_stream << "#          id    x   y   tmin   tmax    s    d   r      p" << std::endl;
    for (i=0; i < job_max; ++i) {
        Job & job = jobs[i];

        out_stream << "job   job_" << std::setfill('0') << std::setw(3) << i << std::setfill(' ');
        out_stream << std::setw(5) << job.location.x << std::setw(4) << job.location.y;
        out_stream << std::setw(7) << job.tmin << std::setw(7) << job.tmax;
        out_stream << std::setw(5) << job.skill;
        out_stream << std::setw(5) << job.duration;
        out_stream << std::setw(4) << job.appointment;
        out_stream << std::setw(7) << job.penalty;
        out_stream << std::endl;
    }
    out_stream << std::endl << "end" << std::endl;
}
