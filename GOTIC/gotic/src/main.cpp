#include "gotic.h"
// #include "data.h" // en cas d'utilisation minimale : option -d.
#include <cassert>
#include <iostream>
#include <sstream>
#include <cstring>
#include <ctime>

#define PROG_NAME "gotic"


char const *usage_msg =
        "Usage:\n"
        // "   "PROG_NAME" -g  W J S V P\n"
        "   "PROG_NAME" -f  FILE\n"
        "   "PROG_NAME" -l  FILE\n"
        "   "PROG_NAME" -d <data>\n"
        "   "PROG_NAME" -v  <data> <sol>\n"
        "   "PROG_NAME" -h\n"
        ;
       
char const *help_msg =
        "The first option is the action.\n\n"
        "  -f FILE\n"
        "     Solve the instance in the file FILE with frontal PLNE.\n"
        "\n"
        "  -l FILE\n"
        "     Solve the instance in the file FILE with gradient descent.\n"
        "\n"
        "  -v <data> <sol>\n"
        "     Read the instance in the file <data> and check\n"
        "     that the solution <sol> is valid.\n"
        "     Also print the details of the schedules (for debugging).\n"
        "\n"
        "  -d Reading and writing of a data file \n"
        "  -t Temporary option.\n"
        "     (Tests of the program.)\n"
        ;
        

int
usage ()
{
    std::cerr << usage_msg << std::endl;
    std::cerr << "Run `"PROG_NAME" -h` for help." << std::endl;
    return 1;
}

int
main (int argc, char **argv)
{
    if (argc < 2) {
        return usage ();
#if 1
    } else if (strcmp (argv[1], "-f") == 0) {
        if (argc != 3)
            return usage ();
        try {
            Data d1 = Data(argv [2]);
            d1.print_data(std::cout);
            // d = &d1;
            try{
                Solution s = Solution(d1); 
                time_t const tstart = time (NULL);
                int has_solution = solve_frontal (d1,s);
                time_t tend   = time (NULL);

                double const tdiff = difftime (tend, tstart);
                if (has_solution) {
                    std::cout << "Frontal resolution succeeded in " << tdiff << std::endl;
                    std::cout << "#  Existing solution from frontal method." << std::endl;
                    std::cout << "#  instance file \"" << argv[2] << "\"" << std::endl;
                    std::cout << "#  resolution time " << tdiff << "s" << std::endl << std::endl;
                    
                    std::ostringstream oss;
                    std::string folder = "sol/";
                    std::string filetype = ".sol";
                    oss << folder << d1.get_name() << filetype;
                    std::ofstream fluxO(oss.str().c_str());
                    s.print_solution (fluxO);
                    // FILE *f = fopen ("sols/mysol", "r"); 
                    s.print_detail   (fluxO);
                    fluxO.close();
                    std::cout << "Created : solution file " << oss.str() << std::endl << std::endl;
                }
                else {
                    std::cout << "Frontal resolution unsucceeded in " << tdiff << std::endl;
                }
                return 0;
            } catch (...) {
                std::cerr << "Frontal resolution failed." << std::endl << std::endl;
                return -1;
            } 
        } catch (...) {
            std::cerr << "Invalid data file \"" << argv[2] <<"\"" << std::endl;
            return -1;
        }
#endif /* if 0-1 */
#if 1
    } else if (strcmp (argv[1], "-l") == 0) {
        if (argc != 3)
            return usage ();
        try {
            Data d1 = Data(argv [2]);
            // d = &d1;
            try{
                Solution s = Solution(d1); 
                time_t const tstart = time (NULL);
                int has_solution = solve_lagrangian (d1,s);
                time_t tend   = time (NULL);

                double const tdiff = difftime (tend, tstart);
                if (has_solution) {
                    std::cout << "Gradient descent succeeded in " << tdiff << std::endl;
                    std::cout << "#  Existing solution from gradient descent." << std::endl;
                    std::cout << "#  instance file \"" << argv[2] << "\"" << std::endl;
                    std::cout << "#  resolution time " << tdiff << "s" << std::endl << std::endl;
                    
                    std::ostringstream oss;
                    std::string folder = "sol/";
                    std::string filetype = ".sol";
                    oss << folder << d1.get_name() << filetype;
                    std::ofstream fluxO(oss.str().c_str());
                    s.print_solution (fluxO);
                    // FILE *f = fopen ("sols/mysol", "r"); 
                    s.print_detail   (fluxO);
                    fluxO.close();
                    std::cout << "Created : solution file " << oss.str() << std::endl << std::endl;
                }
                else {
                    std::cout << "Gradient descent unsucceeded in " << tdiff << std::endl;
                }
                return 0;
            } catch (...) {
                std::cerr << "Gradient descent failed." << std::endl << std::endl;
                return -1;
            } 
        } catch (...) {
            std::cerr << "Invalid data file \"" << argv[2] <<"\"" << std::endl;
            return -1;
        }
#endif
#if 1
    } else if (strcmp (argv[1], "-d") == 0) {
        if (argc != 3)
            return usage ();
        try {
            Data d1 = Data(argv[2]);
            std::string dataFile = "sol/tiny.dat";
            std::ofstream fluxO;
            fluxO.open(dataFile.c_str());
            d1.print_data (fluxO);
            std::cout << "Program ended normally." << std::endl;
            fluxO.close();
            return 0;
        } catch (const char* msg) {
            std::cerr << msg << std::endl;
        }
#endif
    } else if (strcmp (argv[1], "-v") == 0) {
        if (argc != 4)
            return usage ();
        try{
            Data *d;
            Data d1 = Data(argv [2]);
            d = &d1;
            try {
                Solution s = Solution (d1);
                if (s.read_and_validate(argv[3]))
                {
                    std::string solutionFile = "sol/new_validated_solution.sol";
                    std::ofstream fluxO;
                    fluxO.open(solutionFile.c_str());
                    s.print_solution (fluxO);
                    s.print_detail (fluxO);
                    fluxO.close();
                    std::cout << "Program ended normally. Valid solution." << std::endl;
                    return 0;
                }
                else
                {
                    std::cerr << "Invalid solution file \"" << argv[3] << "\"" << std::endl;
                    return -1;
                }

            } catch (...) {
                std::cerr << "Invalid solution file \"" << argv[3] << "\"" << std::endl;
                return -1;
            }   
        } catch (...) {
            std::cerr << "Invalid data file \"" << argv[2] << "\"" << std::endl;
        return -1;
        }
    } else if (strcmp (argv[1], "-h") == 0) {
        fprintf (stderr, "%s", help_msg);
        return 0;
    } else {
        fprintf (stderr, "Invalid first argument: \"%s\"\n", argv[1]);
        return usage ();
    }

    assert (0);
    return usage ();
        
}
