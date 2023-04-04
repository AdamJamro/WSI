//
// Created by adame on 4/4/2023.

#include "Solver.h"



const std::vector<Solver::Node::direction> Solver::Node::all_directions = {Solver::Node::direction::up, Solver::Node::direction::down, Solver::Node::direction::left, Solver::Node::direction::right};


Solver::Solver(char* _init_state) noexcept {
    solution.clear();
    init_state = _init_state;
}

std::vector<Solver::Node*> Solver::solve() {
    if (solution.empty()) solution = Solver::find_feasible_solution();
    return solution;
}

bool Solver::is_solvable(Node* game_node) {
    int num_of_inversions = 0;
    for ( int i = 0 ; i < Node::grid_size * Node::grid_size ; i++ )
        for ( int j = i+1 ; j < Node::grid_size * Node::grid_size ; j++ ) {
            if (game_node->game_state[i] == 0 || game_node->game_state[j] == 0)
                continue;
            if (game_node->game_state[i] > game_node->game_state[j])
                num_of_inversions++;
        }

    Node::Point blank = Node::to_coordinate(game_node->current);
//        int manhattan_distance_between_blank_and_top_left_corner = blank.x + blank.y;

    return ((num_of_inversions + blank.y) % 2 == 1);
}

std::vector<Solver::Node*> Solver::find_feasible_solution(){
    //// setup
    int num_of_iterations = 0;
    Node* base_node = new Node(init_state);

    if ( !is_solvable(base_node) ) {
        delete base_node;
        throw std::runtime_error("given starting permutation is not solvable!\n");
    }

    //// begin A*
    open.push(base_node);
    std::vector<Node*> feasible_solutions;
    short current_min_val = INT16_MAX;

    std::ofstream result_dump("../algorithm_logs.txt");

    while (!open.empty()) {

        num_of_iterations++;
        auto* current_node = open.top();
        open.pop();
        visited.insert(current_node);

        if (current_min_val < current_node->f_cost - 1) {
            continue;
        }

        result_dump << "\n";
        result_dump << "current_node->h_cost " << current_node->h_cost << "\n";
        result_dump << "current_node->g_cost " << current_node->g_cost << "\n";
        result_dump << "current_node->f_cost " << current_node->f_cost << "\n";
        result_dump << "\n";


        if (current_node->get_heuristic_cost() == 0) {
            bool feasible = true;
            for (int i = 0 ; i < Node::grid_size * Node::grid_size - 1; i++) {
                if (current_node->game_state[i] != static_cast<char>(i+1)) {
                    feasible = false;
                    break;
                }
            }
            if (feasible){
                feasible_solutions.push_back(current_node);
                current_min_val = std::min(current_min_val, current_node->get_distance_cost());

                auto finish = std::chrono::high_resolution_clock::now();
                std::cout<<"i found a solution candidate! distance:" << current_node->g_cost <<"\n";
            }
        }


        for (int direction : Solver::Node::all_directions) {
            if (current_node->get_direction_towards_parent() == direction) {
                continue;
            }

            Node* new_node;
            try {
                new_node = Node::create_new_node(current_node, direction);
            }
            catch (const std::invalid_argument & e) {
//                std::cout << "couldnt make new node\n";
                continue;
            }

            if (visited.count(new_node)){
                auto visited_node_pointer = visited.find(new_node);
                if( (*visited_node_pointer)->g_cost <= new_node->get_distance_cost() ) {
                    delete new_node;
                    continue;
                }
                visited.erase(visited_node_pointer);
                delete *visited_node_pointer;
                visited.insert(new_node);
            }
            open.push(new_node);
        }
    }
    return feasible_solutions;
}


Solver::~Solver() {
    for ( auto* node : visited ) {
        delete node;
    }
}

bool Solver::is_valid_move(int origin, int destination){
    return destination >= 0
           && destination < Node::grid_size * Node::grid_size
           && (abs((destination % Node::grid_size) - (origin % Node::grid_size)) == 1 || abs(destination - origin) != 1);
}

int Solver::find_current_blank_space_index(const char* game_state) {
    int current = -1;
    for (int i = 0; i < Node::grid_size * Node::grid_size; ++i ) {
        if (game_state[i] == 0){
            current = i;
            break;
        }
    }
    if (current == -1) {
        throw std::invalid_argument("game_state is invalid, no blank space found!");
    }

    return current;
}

short Solver::heuristic_function_manhattan_with_linear_conflict(char* game_state) {
    int sum = 0;
    for ( int i = 0 ; i < Node::grid_size*Node::grid_size ; i++ ) {
        if ( game_state[i] == 0 ) {
            continue; // ignonre blank space
        }
        Node::Point current_coordinate = Node::to_coordinate(i);
        Node::Point destination_coordinate = Node::to_coordinate(game_state[i] - 1); // game_state includes value from 1 to grid_size^2 not - to grid_size^2 - 1
        sum +=  abs(current_coordinate.x - destination_coordinate.x)
                + abs(current_coordinate.y - destination_coordinate.y); // manhattan distance
    }
    sum /= 2;

    for ( int row = 0 ; row < Node::grid_size*Node::grid_size ; row+=Node::grid_size) {
        for ( int i = 0 ; i < Node::grid_size ; i++) {
            for ( int j = i + 1 ; j < Node::grid_size ; j++) {
                if (row < game_state[row + i] < row + Node::grid_size && row < game_state[row + j] < row + Node::grid_size && game_state[row + i] > game_state[row + j])
                    sum+=2;
            }
        }
    }

    for ( int column = 0 ; column < Node::grid_size ; column++) {
        for ( int i = 0 ; i < Node::grid_size * Node::grid_size ; i += Node::grid_size ) {
            for ( int j = i + Node::grid_size ; j < Node::grid_size * Node::grid_size ; j += Node::grid_size ) {
                if (game_state[column + i] % Node::grid_size == column == game_state[column + j] % Node::grid_size && game_state[column + i] > game_state[column + j])
                    sum+=2;
            }
        }
    }
    return static_cast<short>(sum);
}

short Solver::heuristic_function_manhattan(char* game_state) {
    int sum = 0;
    for ( int i = 0 ; i < Solver::Node::grid_size*Solver::Node::grid_size ; i++ ) {
        if ( game_state[i] == 0 ) {
            continue; // ignonre blank space
        }
        Node::Point current_coordinate = Node::to_coordinate(i);
        Node::Point destination_coordinate = Node::to_coordinate(game_state[i] - 1); // game_state includes value from 1 to grid_size^2 not - to grid_size^2 - 1
        sum +=  abs(current_coordinate.x - destination_coordinate.x)
                + abs(current_coordinate.y - destination_coordinate.y); // manhattan distance
    }
    return static_cast<short>((sum) / 2);
}

short Solver::heuristic_function_walking_distance(char* game_state, int current) {
    int result_sum = 0;
    if (current == -1) {
        current = find_current_blank_space_index(game_state);
    }
    Node::Point current_blank_coordinate = Node::to_coordinate(current);

    for ( int i = 0 ; i < Node::grid_size * Node::grid_size ; i++ ) {

        if ( i == current ) {
            continue; // omit blank space
        }

        Node::Point source_coordinate = Node::to_coordinate(i);
        Node::Point destination_coordinate = Node::to_coordinate(static_cast<int>(game_state[i] - 1)); //minus one since game_state values begin at 1 not 0
        int vertical_distance = std::abs(destination_coordinate.y - source_coordinate.y);
        int horizontal_distance = std::abs(destination_coordinate.x - source_coordinate.x);

        if ( vertical_distance != 0 ) {
            result_sum += vertical_distance * 3;
            +std::abs(source_coordinate.y - current_blank_coordinate.y);
            if ((destination_coordinate.y - current_blank_coordinate.y)*(destination_coordinate.y - source_coordinate.y) < 0)
                if (std::abs(destination_coordinate.y - current_blank_coordinate.y) >= vertical_distance )
                    result_sum += 2;

        }
        if ( horizontal_distance != 0 ) {
            result_sum += horizontal_distance * 3;
            + std::abs(source_coordinate.x - current_blank_coordinate.x);
            if ((destination_coordinate.x - current_blank_coordinate.x)*(destination_coordinate.x - source_coordinate.x) > 0)
                if (std::abs(destination_coordinate.x - current_blank_coordinate.x) >= horizontal_distance )
                    result_sum += 2;
        }
    }

//    return static_cast<short>((result_sum - 1) / 2 + 1);
    return static_cast<short>(result_sum / 2);
}



short Solver::heuristic_function_inversion_distance(const char* game_state) {
    int num_of_inversions = 0;
    for ( int i = 0 ; i < Node::grid_size * Node::grid_size ; i++ )
        for ( int j = i+1 ; j < Node::grid_size * Node::grid_size ; j++ ) {
            if (game_state[i] == 0 || game_state[j] == 0)
                continue;
            if (game_state[i] > game_state[j])
                num_of_inversions++;
        }

//        int manhattan_distance_between_blank_and_top_left_corner = blank.x + blank.y;

    return std::ceil(num_of_inversions / 3.0);

}


int Solver::do_move(char* game_state, int direction) {
    int current = find_current_blank_space_index(game_state);
    return do_move(game_state, direction, current);
}

// returns current index of 0 in the puzzle
int Solver::do_move(char* game_state, int direction, int current){

    int destination = current + direction;

    if (!is_valid_move(current, destination)){
        throw std::invalid_argument("invalid move direction");
    }

    game_state[current] = game_state[destination];
    game_state[destination] = 0;
    current = destination;

    return current;
}


bool Solver::Compare::operator()(Solver::Node *a, Solver::Node *b) {
    if (a->f_cost > b->f_cost) {
        return true;
    } else if (a->f_cost == b->f_cost && a->h_cost > b->h_cost) {
        return true;
    }
    return false;
}



size_t Solver::game_state_hasher::operator()(const Node *node) const {
    std::hash<char> hasher;
    size_t seed = 0;
    for (int i = 0 ; i < Node::grid_size * Node::grid_size; i++) {
        seed ^= hasher(node->game_state[i]) + 0x9e3779b9 + (seed<<6) + (seed>>2);
    }
    return seed;
}