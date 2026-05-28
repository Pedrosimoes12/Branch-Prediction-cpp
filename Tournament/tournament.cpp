#include <iostream>
#include <vector>
#include <queue>
#include <cstdlib>
#include <ctime>
#include <cstdint>
#include <cstring>

#define TABLE_SIZE 1024
#define HISTORY_BITS 10

int printed = 0;

using Counter = uint8_t;

// Tournament Predictor

Counter local_table[TABLE_SIZE];
Counter global_table[TABLE_SIZE];
Counter chooser_table[TABLE_SIZE];

uint16_t global_history = 0;

int acertos = 0;
int total = 0;

void update_counter(Counter* c, int taken) {

    if(taken) {

        if(*c < 3)
            (*c)++;

    } else {

        if(*c > 0)
            (*c)--;
    }
}

int is_taken(Counter c) {
    return c >= 2;
}

void init_predictor() {

    memset(local_table, 2,
           sizeof(local_table));

    memset(global_table, 2,
           sizeof(global_table));

    memset(chooser_table, 0,
           sizeof(chooser_table));
}

int tournament_predict(uint32_t pc) {

    uint32_t index = pc % TABLE_SIZE;

    if(is_taken(chooser_table[index])) {

        return is_taken(
            global_table[
                global_history % TABLE_SIZE
            ]
        );

    } else {

        return is_taken(local_table[index]);
    }
}

void train_predictor(uint32_t pc,
                     int taken) {

    uint32_t index = pc % TABLE_SIZE;

    int local_pred =
        is_taken(local_table[index]);

    int global_pred =
        is_taken(
            global_table[
                global_history % TABLE_SIZE
            ]
        );

    update_counter(
        &local_table[index],
        taken
    );

    update_counter(
        &global_table[
            global_history % TABLE_SIZE
        ],
        taken
    );

    if(local_pred != global_pred) {

        if(local_pred != taken &&
           global_pred == taken) {

            update_counter(
                &chooser_table[index],
                1
            );

        } else if(global_pred != taken &&
                  local_pred == taken) {

            update_counter(
                &chooser_table[index],
                0
            );
        }
    }

    global_history =
        ((global_history << 1)
        | taken)
        & ((1 << HISTORY_BITS) - 1);
}

void process_branch(uint32_t pc,
                    int outcome) {

    int pred =
        tournament_predict(pc);

    int local_pred =
        is_taken(
            local_table[
                pc % TABLE_SIZE
            ]
        );

    int global_pred =
        is_taken(
            global_table[
                global_history %
                TABLE_SIZE
            ]
        );

    if(pred == outcome)
        acertos++;

    total++;

    train_predictor(pc, outcome);

    if(printed < 600) {

        std::cout
            << "GHR: "
            << global_history
            << " | Real: "
            << outcome
            << " | Pred: "
            << pred
            << " | Local: "
            << local_pred
            << " | Global: "
            << global_pred
            << " | Chooser: "
            << (int)chooser_table[
                pc % TABLE_SIZE
            ]
            << std::endl;

        printed++;
    }
}

// Mini SPEC-like workload

struct Node {

    int value;

    std::vector<int> edges;
};

std::vector<Node> graph;

// BFS estilo SPEC
void bfs(int start) {

    std::queue<int> q;

    std::vector<int> visited(
        graph.size(), 0
    );

    q.push(start);

    visited[start] = 1;

    uint32_t pc = 0x400;

    while(!q.empty()) {

        int u = q.front();

        q.pop();

        for(int v : graph[u].edges) {

            // Branch imprevisível
            int outcome =
                (!visited[v]);

            process_branch(pc, outcome);

            if(outcome) {

                visited[v] = 1;

                q.push(v);
            }
        }
    }
}

int main() {

    srand(time(NULL));

    init_predictor();

    const int N = 500;

    graph.resize(N);

    // Gera grafo aleatório
    for(int i = 0; i < N; i++) {

        graph[i].value =
            rand() % 1000;

        int edges =
            rand() % 10;

        for(int j = 0;
            j < edges;
            j++) {

            int to =
                rand() % N;

            graph[i]
                .edges
                .push_back(to);
        }
    }

    // Executa várias BFS
    for(int i = 0;
        i < 100;
        i++) {

        bfs(rand() % N);
    }

    std::cout
        << "Branches: "
        << total
        << std::endl;

    std::cout
        << "Acertos: "
        << acertos
        << std::endl;

    std::cout
        << "Precisao: "
        << (100.0 * acertos / total)
        << "%"
        << std::endl;

    return 0;
}