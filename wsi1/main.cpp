#include <iostream>
#include <cmath>
#include <random>
#include <list>
#include <algorithm>
#include "Solver.h"


char* generate_target();
char* generate_random_target();
void print_game_state(char*&);
// used to represent game state graph nodes


int main() {


    std::cout << "initial permutation:"<< std::endl;
    char* base_game_state = generate_random_target();
//    std::vector<char> vec;
//    vec = {2,       10,      8,       7,
//           1,       4,       14,      3,
//           6,       5,       0,       12,
//           9,       13,      15,      11};
//    std::copy (vec.begin(), vec.end(), base_game_state);
    print_game_state(base_game_state);


    /// start measuring time
    auto start = std::chrono::high_resolution_clock::now();

    //// here the search for solution
    //// (A* algorithm) begins
    auto solver = new Solver(base_game_state);
    auto feasible_solutions = solver->solve();

    //// stop measuring elapsed time
    auto finish = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = finish - start;


    int current_min_val = INT32_MAX;

    //// PRINT SOLUTION
    for (auto candidate : feasible_solutions) {
        Solver::Node* tmp_node = candidate;
        int num_of_steps = 0;
        std::cout<< "\nfeasible solution:\n";
        print_game_state(candidate->game_state);
        while (tmp_node->parent != nullptr) {
            num_of_steps++;
            print_game_state(tmp_node->parent->game_state);
            tmp_node = tmp_node->parent;
        }
        std::cout << "solution needed to perform: " << num_of_steps << " steps\n";

        current_min_val = std::min(current_min_val, num_of_steps);
    }

    std::cout << "\nshortest path consists of " << current_min_val << " steps" << std::endl;
//    std::cout << "number of iterations of this algorithm: " << num_of_iterations << " steps" << std::endl;
    std::cout << "time spent searching the solution: " << elapsed.count() << std::endl;

    return 0;
}

//// END OF MAIN //// ////
//// //// ///// //// //// //// //// ///// //// //// //// //// ///// //// //// //// //// ///// //// ////
//// //// ///// //// //// //// //// ///// //// //// //// //// ///// //// //// //// //// ///// //// ////
//// //// ///// //// //// //// //// ///// //// //// //// //// ///// //// //// //// //// ///// //// ////
//// //// ///// //// //// //// //// ///// //// //// //// //// ///// //// //// //// //// ///// //// ////
//// //// ///// //// //// //// //// ///// //// //// //// //// ///// //// //// //// //// ///// //// ////


char* generate_target() {
    char *target = static_cast<char *>(malloc(sizeof(char) * Solver::Node::grid_size * Solver::Node::grid_size));
    for (char i = 0; i < Solver::Node::grid_size * Solver::Node::grid_size; i++)
        target[i] = static_cast<char> (i + 1);

    target[ (Solver::Node::grid_size * Solver::Node::grid_size) - 1 ] = 0;

    return target;
}

char* generate_random_target() {
    char *target = generate_target();
    std::random_device rd;
    std::mt19937 mt(rd());

    std::shuffle(target, target + Solver::Node::grid_size * Solver::Node::grid_size, mt);

    return target;
}




void print_game_state(char*& game){
    int i = 0;
    while (i < Solver::Node::grid_size * Solver::Node::grid_size){
        std::cout<<static_cast<int>(*(game + i))<<"\t";
        if (i % Solver::Node::grid_size == Solver::Node::grid_size - 1)
            std::cout<<std::endl;
        i++;
    }
    std::cout<<"\n";
}

