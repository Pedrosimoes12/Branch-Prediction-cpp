#include <iostream>
#include <cstdint>
#include <cstring>

#define HISTORY_BITS 4 // 4 bits, para testar com 10 basta comentar esse define
#define TABLE_SIZE 1024

// Contador saturante de 2 bits
// 00: Forte N
// 01: Fraco N
// 10: Fraco T
// 11: Forte T
using Counter = uint8_t;

void update_counter(Counter* c, int taken) {
    if (taken) {
        if (*c < 3)
            (*c)++;
    } else {
        if (*c > 0)
            (*c)--;
    }
}

int is_taken(Counter c) {
    return c >= 2;
}

// Estruturas do preditor
Counter local_table[TABLE_SIZE];     // Tabela local (bimodal)
Counter global_table[TABLE_SIZE];    // Tabela global
Counter chooser_table[TABLE_SIZE];   // 0-1: local, 2-3: global

uint16_t global_history = 0;

void init_predictor() {
    std::memset(local_table, 2, sizeof(local_table));
    std::memset(global_table, 2, sizeof(global_table));
    std::memset(chooser_table, 0, sizeof(chooser_table));
}

int tournament_predict(uint32_t pc) {
    uint32_t index = pc % TABLE_SIZE;

    // Escolhe com base na chooser table
    if (is_taken(chooser_table[index])) {
        return is_taken(global_table[global_history % TABLE_SIZE]);
    } else {
        return is_taken(local_table[index]);
    }
}

void train_predictor(uint32_t pc, int taken) {
    uint32_t index = pc % TABLE_SIZE;

    int local_pred =
        is_taken(local_table[index]);

    int global_pred =
        is_taken(global_table[global_history % TABLE_SIZE]);

    // Atualiza preditores
    update_counter(&local_table[index], taken);

    update_counter(
        &global_table[global_history % TABLE_SIZE],
        taken
    );

    // Atualiza chooser table
    if (local_pred != global_pred) {

        if (local_pred != taken && global_pred == taken) {
            // Global acertou
            update_counter(&chooser_table[index], 1);

        } else if (global_pred != taken && local_pred == taken) {
            // Local acertou
            update_counter(&chooser_table[index], 0);
        }
    }

    // Atualiza histórico global com 10 bits
    /*global_history =
        ((global_history << 1) | taken)
        & (TABLE_SIZE - 1);
    }*/

    // Atualiza Histórico global com 4 bits
    global_history =
        ((global_history << 1) | taken)
        & ((1 << HISTORY_BITS) - 1);
    }
int main() {

    init_predictor();

    uint32_t pc = 0x400;

    const int n = 50;

    int v[n];

    // Preenche vetor com números aleatórios
    srand(time(NULL));

    for(int i = 0; i < n; i++) {
        v[i] = rand() % 100;
    }

    int acertos = 0;
    int total = 0;

    // Bubble Sort
    for(int i = 0; i < n - 1; i++) {

        for(int j = 0; j < n - i - 1; j++) {

            // Branch
            int outcome = (v[j] > v[j + 1]);

            // Predição
            int pred = tournament_predict(pc);

            int local_pred =
                is_taken(local_table[pc % TABLE_SIZE]);

            int global_pred =
                is_taken(
                    global_table[
                        global_history % TABLE_SIZE
                    ]
                );

            // Estatísticas
            if(pred == outcome)
                acertos++;

            total++;

            // Debug
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
                << (int)chooser_table[pc % TABLE_SIZE]
                << std::endl;

            // Treina predictor
            train_predictor(pc, outcome);

            // Bubble Sort normal
            if(outcome) {
                int temp = v[j];
                v[j] = v[j + 1];
                v[j + 1] = temp;
            }
        }
    }

    std::cout << "Total de branches: "
              << total << std::endl;

    std::cout << "Acertos: "
              << acertos << std::endl;

    std::cout << "Precisao: "
              << (100.0 * acertos / total)
              << "%" << std::endl;

    return 0;
}