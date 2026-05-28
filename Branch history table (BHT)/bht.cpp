#include <iostream>
#include <vector>
#include <queue>
#include <cstdlib>
#include <ctime>
#include <cstdint>
#include <cstring>

#define TABLE_SIZE 1024

using Counter = uint8_t;


// BHT Predictor (Bimodal)

// 00 -> Forte N
// 01 -> Fraco N
// 10 -> Fraco T
// 11 -> Forte T

Counter BHT[TABLE_SIZE];

int printed = 0;

int acertos = 0;
int total = 0;

// Atualiza contador saturante

void update_counter(Counter* c,
                    int taken) {

    if(taken) {

        if(*c < 3)
            (*c)++;

    } else {

        if(*c > 0)
            (*c)--;
    }
}

// Predição

int predict_taken(Counter c) {

    return c >= 2;
}

// Inicializa predictor

void init_predictor() {

    memset(BHT, 2,
           sizeof(BHT));
}

// Predição BHT

int bht_predict(uint32_t pc) {

    uint32_t index =
        pc % TABLE_SIZE;

    return predict_taken(
        BHT[index]
    );
}

// Treina predictor

void train_predictor(uint32_t pc,
                     int outcome) {

    uint32_t index =
        pc % TABLE_SIZE;

    update_counter(
        &BHT[index],
        outcome
    );
}

// Processa branch

void process_branch(uint32_t pc,
                    int outcome) {

    int pred =
        bht_predict(pc);

    if(pred == outcome)
        acertos++;

    total++;

    train_predictor(pc, outcome);

    if(printed < 600) {

        std::cout
            << "Real: "
            << outcome
            << " | Pred: "
            << pred
            << " | Counter: "
            << (int)BHT[
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

    // Grafo aleatório
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
        << "\n=====================\n";

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