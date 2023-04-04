//
// Created by adame on 4/4/2023.
//

#ifndef SOLVER_H
#define SOLVER_H
#include <iostream>
#include <cstddef>
#include <vector>
#include <queue>
#include <unordered_set>
#include <stdexcept>
#include <cstring>
#include <random>
#include <algorithm>
#include <fstream>
#include <chrono>



class Solver {
public:
    struct Node {
        static const int grid_size = 4;
        enum direction {
            up = -grid_size,
            down = grid_size,
            left = -1,
            right = 1
        };
//    std::vector<direction> all_directions;
        static const std::vector<direction> all_directions;


        Solver::Node* parent = nullptr;
        char* game_state = nullptr;
        int current = -1;
        short f_cost = -1;
        short h_cost = -1;
        short g_cost = -1;

        struct Point{
            int x, y;
        };

        static Point to_coordinate(int index){
            if (index < 0 || index >= grid_size*grid_size)
                throw std::runtime_error("Invalid index. Cannot convert to point.");

            Point result{ index % grid_size, index / grid_size };
            return result;
        }

        int calculate_f_cost() {
            return get_distance_cost() + get_heuristic_cost();
        }

        int calculate_heuristic_cost() const {
//            return Solver::heuristic_function_manhattan(game_state);
        return Solver::heuristic_function_walking_distance(game_state, current); //put -1 in place of current to calculate it automatically
//        return Solver::heuristic_function_manhattan_with_linear_conflict(game_state);
//        return Solver::heuristic_function_inversion_distance(game_state);
        }

        int calculate_distance_cost() const {
            if (parent == nullptr) {
                return 0;
            }

            return parent->g_cost + 1;
        }

        short get_heuristic_cost(){
            if (h_cost == -1) h_cost = calculate_heuristic_cost();
            return h_cost;
        }

        short get_distance_cost() {
            if (g_cost == -1) g_cost = calculate_distance_cost();
            return g_cost;
        }

        short get_f_cost() {
            if (f_cost == -1) f_cost = calculate_f_cost();
            return f_cost;
        }

        int get_direction_towards_parent() const{
            if (parent == nullptr) {
                return grid_size + 1; //any unfeasible dir
            }
            return parent->current - current;
        }

        ~Node() {
            delete[] game_state;
        }

        explicit Node(int shuffle_depth) {
            game_state = generate_target();
            shuffle(game_state, shuffle_depth);
            current = Solver::find_current_blank_space_index(game_state);
            parent = nullptr;
            get_f_cost(); // calculates h,g,f costs and sets them
        }

        explicit Node(char* _game_state) {
            game_state = _game_state;
            current = Solver::find_current_blank_space_index(game_state);
            parent = nullptr;
            get_f_cost(); // calculates h,g,f costs and sets them
        }

        Node(char* _game_state, Node* _parent){
            game_state = _game_state;
            current = Solver::find_current_blank_space_index(game_state);
            parent = _parent;
            get_f_cost(); // calculates h,g,f costs and sets them
        }

        Node(char* _game_state, Node* _parent, int _current){
            game_state = _game_state;
            current = _current;
            parent = _parent;
            get_f_cost(); // calculates h,g,f costs and sets them
        }

        static Node* create_new_node(Node* _parent, int direction){
            if (_parent->game_state == nullptr || _parent->current == -1 || _parent->g_cost == -1) {
                throw std::runtime_error("You shouldn't initialize new Node by mal-constructed parent node!");
            }
            if (!Solver::is_valid_move(_parent->current, _parent->current + direction)) {
                throw std::invalid_argument("invalid move!"); //asserted noexcept calling do_move(..)
            }

            char* _game_state = new char[grid_size*grid_size];
            std::memcpy(_game_state, _parent->game_state, grid_size * grid_size);
            int _current = Solver::do_move(_game_state, direction, _parent->current); //asserted noexcept

            return new Node(_game_state, _parent, _current);
        }

        static char* generate_target() {
            char *target = static_cast<char *>(malloc(sizeof(char) * Node::grid_size * Node::grid_size));
            for (char i = 0; i < Node::grid_size * Node::grid_size; i++)
                target[i] = static_cast<char> (i + 1);

            target[ (Node::grid_size * Node::grid_size) - 1 ] = 0;

            return target;
        }

        static char* generate_random_target() {
            char *target = generate_target();
            std::random_device rd;
            std::mt19937 mt(rd());

            std::shuffle(target, target + Node::grid_size * Node::grid_size, mt);

            return target;
        }


        void shuffle(char*& perm, int num_of_permutations){

            int current = 0;
            for (int i = 0; i < Node::grid_size * Node::grid_size; ++i ){
                if (perm[i] == 0){
                    current = i;
                    break;
                }
            }

            std::random_device rd;
            std::mt19937 mt(rd());
            std::uniform_int_distribution<int> uni(0, Node::grid_size - 1);

            int previous_direction = Node::grid_size + 1; //neither up,down,left nor right
            for (int i = 0; i < num_of_permutations; ++i){
                int random_direction;
                int destination;

                do {
                    random_direction = all_directions[ uni(mt) ];
                    destination = current + random_direction;
                } while(!is_valid_move(current, destination) && random_direction != -1 * previous_direction);


                previous_direction = random_direction;

                perm[current] = perm[destination];
                perm[destination] = 0;
                current = destination;

//        std::cout<<"(within shuffle)"<<std::endl;
//        print_game_state(perm);
            }
        }



    };

    class Compare {
    public:
        bool operator()(Solver::Node* a, Solver::Node* b); //how to compare two nodes
    };
    class game_state_hasher {
    public:
        size_t operator()(const Node* node) const; // how to distinguish nodes from those in the visited-nodes set
    };
    std::vector<Node*> solve();
    explicit Solver(char*) noexcept;
    ~Solver() noexcept;
    bool is_solvable(Node*);
    static short heuristic_function_inversion_distance(const char* game_state);

    static int do_move(char* game_state, int direction);
    static int do_move(char* game_state, int direction, int current);
    static short heuristic_function_walking_distance(char* game_state, int current);
    static short heuristic_function_manhattan(char* game_state);
    static short heuristic_function_manhattan_with_linear_conflict(char* game_state);

    static bool is_valid_move(int origin, int destination);

    static int find_current_blank_space_index(const char* game_state);
private:

    char* init_state;
    std::vector<Node*> solution;
    std::priority_queue<Node*, std::vector<Node*>, Compare> open;
    std::unordered_set<Node*, game_state_hasher> visited;
    std::vector<Node*> find_feasible_solution();
};


#endif //SOLVER_H
