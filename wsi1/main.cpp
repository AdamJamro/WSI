#include <iostream>
#include <cmath>
#include <random>
#include <queue>
#include <vector>
#include <cstring>
#include <list>
#include <algorithm>


static const int shuffle_depth = 100;
char* generate_target();

void shuffle(char*&, int);
void print_game_state(char*&);
bool is_valid_move(int,int);
int do_move(char*,int);
int do_move(char*,int, int);
int find_current_blank_space_index(const char*);
// used to represent game state graph nodes
struct Node {
    Node* parent;
    char* game_state;
    int current = -1;
    int f_cost = -1;
    int h_cost = -1;
    int g_cost = -1;
    static const int grid_size = 4;

    struct Point{
        int x, y;
    };

    static Point to_coordinate(int index){
        if (index < 0 || index >= grid_size*grid_size)
            throw std::runtime_error("Invalid index. Cannot convert to point.");

        Point result{ index % grid_size, index / grid_size };
        return result;
    }

    int get_heuristic_cost(){
        if (h_cost == -1) h_cost = calculate_heuristic_cost();
        return h_cost;
    }

    int get_distance_cost() {
        if (g_cost == -1) g_cost = calculate_distance_cost();
        return g_cost;
    }

    int get_f_cost() {
        if (f_cost == -1) f_cost = calculate_f_cost();
        return f_cost;
    }


    int calculate_heuristic_cost() const {
        int sum = 0;
        for ( int i = 0 ; i < grid_size*grid_size ; i++ ) {
            if ( game_state[i] == 0 ) {
                continue; // ignonre blank space
            }
            Point current_coordinate = to_coordinate(i);
            Point destination_coordinate = to_coordinate(game_state[i] - 1); // game_state includes value from 1 to grid_size^2 not - to grid_size^2 - 1
            sum +=  abs(current_coordinate.x - destination_coordinate.x)
                    + abs(current_coordinate.y - destination_coordinate.y); // manhattan distance
        }
        return (sum) / 2;
    }

    int calculate_distance_cost() const {
        if (parent == nullptr) {
            return 0;
        }

        return parent->g_cost + 1;
    }

    int calculate_f_cost() {
        return get_distance_cost() + get_heuristic_cost();
    }

    int get_direction_towards_parent() const{
        if (parent == nullptr) {
            return grid_size + 1; //any unfeasible dir
        }
        return parent->current - current;
    }

    Node(char* _game_state, Node* _parent){
        game_state = _game_state;
        current = find_current_blank_space_index(game_state);
        parent = _parent;
        get_f_cost();
    }

    ~Node() {
        delete[] game_state;
    }

    explicit Node(char* _game_state) {
        game_state = _game_state;
        current = find_current_blank_space_index(game_state);
        parent = nullptr;
        get_f_cost(); // calculates h,g,f costs and sets them
    }

    Node() {
        game_state = generate_target();
        shuffle(game_state, shuffle_depth);
        current = find_current_blank_space_index(game_state);
        parent = nullptr;
        get_f_cost(); // calculates h,g,f costs and sets them
    }

    Node(Node* parent, int direction) {
        if (parent->current == -1 || parent->f_cost == -1 || parent->g_cost == -1) {
            throw std::runtime_error("You shouldn't initialize new Node by mal-constructed parent node!");
        }

        game_state = new char[grid_size*grid_size];
        std::memcpy(game_state, parent->game_state, grid_size*grid_size);

        current = do_move(game_state, direction, parent->current); //throws exception
        this->parent = parent;
        get_f_cost(); // calculates h,g,f costs based on parent and sets them
    }

    class Compare {
    public:
        bool operator()(Node* a, Node* b)
        {
            if (a->f_cost > b->f_cost) {
                return true;
            } else if (a->f_cost == b->f_cost && a->h_cost > b->h_cost) {
                return true;
            }
            return false;
        }
    };


    enum direction {
        up = -grid_size,
        down = grid_size,
        left = -1,
        right = 1
    };
};




static std::vector<Node::direction> all_directions({Node::direction::up, Node::direction::down, Node::direction::left, Node::direction::right});


int main() {
    int num_of_iterations = 0;

    Node* base_node = new Node();
    std::cout << "initial permutation:"<< std::endl;
    std::list<Node*> nodes_to_delete;


    print_game_state(base_node->game_state);
//    std::vector<Node*> visited;

    std::priority_queue<Node*, std::vector<Node*>, Node::Compare> open;

    open.push(base_node);

    std::vector<Node*> feasible_solutions;

    int current_min_val = Node::grid_size * Node::grid_size * Node::grid_size; //upper bound on  manhattan sum / 2

    while (!open.empty()) {
        num_of_iterations++;
        auto* current_node = open.top();
//        std::cout << "\n";
//        std::cout << "current_node->h_cost " << current_node->h_cost << "\n";
//        std::cout << "current_node->g_cost " << current_node->g_cost << "\n";
//        std::cout << "current_node->f_cost " << current_node->f_cost << "\n";
//        std::cout << "\n";
        open.pop();
        nodes_to_delete.push_back(current_node);

        if (current_min_val < current_node->f_cost - 1) {
            continue;
        }

        for (int direction : all_directions) {

            if (current_node->get_direction_towards_parent() == direction) {
                continue;
            }

            Node* new_node;
            try {
                new_node = new Node(current_node, direction);
            }
            catch (const std::invalid_argument & e) {
                continue;
            }
            open.push(new_node);
        }

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
                current_min_val = current_node->get_f_cost();
                std::cout<<"i found a solution candidate!\n";
            }
        }
//        if (!feasible_solutions.empty()) {
//            break;
//        }
    }


    //// PRINTING SOLUTION
    for (auto candidate : feasible_solutions) {
        Node* tmp_node = candidate;
        int num_of_steps = 0;
        std::cout<< "\nfeasible solution:\n";
        print_game_state(candidate->game_state);
        while (tmp_node->parent != nullptr) {
            num_of_steps++;
            print_game_state(tmp_node->parent->game_state);
            tmp_node = tmp_node->parent;
        }
        std::cout << "solution needed to perform: " << num_of_steps << " steps\n";
    }
    std::cout << "number of iterations of this algorithm " << num_of_iterations << " steps\n";



//    for ( auto* node : nodes_to_delete ) {
//        delete node;
//    }
    return 0;
}

//// END OF MAIN //// ////
//// //// ///// //// //// //// //// ///// //// //// //// //// ///// //// //// //// //// ///// //// ////
//// //// ///// //// //// //// //// ///// //// //// //// //// ///// //// //// //// //// ///// //// ////
//// //// ///// //// //// //// //// ///// //// //// //// //// ///// //// //// //// //// ///// //// ////
//// //// ///// //// //// //// //// ///// //// //// //// //// ///// //// //// //// //// ///// //// ////
//// //// ///// //// //// //// //// ///// //// //// //// //// ///// //// //// //// //// ///// //// ////



char* generate_target() {
    char *target = static_cast<char *>(malloc(sizeof(char) * Node::grid_size * Node::grid_size));
    for (char i = 0; i < Node::grid_size * Node::grid_size; i++)
        target[i] = static_cast<char> (i + 1);

    target[ (Node::grid_size * Node::grid_size) - 1 ] = 0;

    return target;
}

//void generate_random_target() {
//    char *target = static_cast<char*>(malloc(sizeof(char) * Node::grid_size * Node::grid_size));
//    std::random_device rd;
//    std::mt19937 mt(rd());
//    std::uniform_int_distribution<int> uni(0, Node::grid_size - 1);
//
//}

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

int do_move(char* game_state, int direction) {
    int current = find_current_blank_space_index(game_state);
    return do_move(game_state, direction, current);
}

// returns current index of 0 in the puzzle
int do_move(char* game_state, int direction, int current){

    int destination = current + direction;

    if (!is_valid_move(current, destination)){
        throw std::invalid_argument("invalid move direction");
    }

    game_state[current] = game_state[destination];
    game_state[destination] = 0;
    current = destination;

    return current;
}

void print_game_state(char* &game){
    int i = 0;
    while (i < Node::grid_size * Node::grid_size){
        std::cout<<static_cast<int>(*(game + i))<<"\t";
        if (i % Node::grid_size == Node::grid_size - 1)
            std::cout<<std::endl;
        i++;
    }
    std::cout<<"\n";
}

bool is_valid_move(int origin, int destination){
    return destination >= 0
           && destination < Node::grid_size * Node::grid_size
           && (abs((destination % Node::grid_size) - (origin % Node::grid_size)) == 1 || abs(destination - origin) != 1);
}

int find_current_blank_space_index(const char* game_state) {
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