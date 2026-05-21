#include <iostream>
#include <cstdint>
#include <cstdlib>
#include <ctime>

#define HISTORY_BITS 4 // 4 ou 10
#define PHT_SIZE (1 << HISTORY_BITS) // 1024

// Contador saturante de 2 bits
// 00: Forte N
// 01: Fraco N
// 10: Fraco T
// 11: Forte T
using Counter = uint8_t;

void update_counter(Counter* c, int taken) {

    if(taken) {
        if(*c < 3)
            (*c)++;
    }
    else {
        if(*c > 0)
            (*c)--;
    }
}

int predict_taken(Counter c) {
    return c >= 2;
}

// Estrutura do preditor

// Branch History Register (BHR)
uint32_t BHR = 0;

// Pattern History Table (PHT)
Counter PHT[PHT_SIZE];

void init_predictor() {

    for(int i = 0; i < PHT_SIZE; i++) {
        PHT[i] = 2; // Fraco T
    }

    BHR = 0;
}

int two_level_predict() {

    return predict_taken(PHT[BHR]);
}

void train_predictor(int outcome) {

    // Atualiza contador
    update_counter(&PHT[BHR], outcome);

    // Atualiza histórico global
    BHR = ((BHR << 1) | outcome) & (PHT_SIZE - 1);
}


// TESTE COM BUBBLE SORT

int main() {

    init_predictor();

    const int n = 50;

    int v[n];

    srand(time(NULL));

    // Vetor aleatório
    for(int i = 0; i < n; i++) {
        v[i] = rand() % 1000;
    }

    int acertos = 0;
    int total = 0;

    // Bubble Sort
    for(int i = 0; i < n - 1; i++) {

        for(int j = 0; j < n - i - 1; j++) {

            // Branch
            int outcome = (v[j] > v[j + 1]);

            // Predição
            int pred = two_level_predict();

            // Estatísticas
            if(pred == outcome)
                acertos++;

            total++;

            // Treina predictor
            train_predictor(outcome);

            // Bubble Sort normal
            if(outcome) {

                int temp = v[j];
                v[j] = v[j + 1];
                v[j + 1] = temp;
            }

            // Debug
            std::cout
                << "BHR: "
                << BHR
                << " | Real: "
                << outcome
                << " | Pred: "
                << pred
                << std::endl;
        }
    }

    std::cout << "\n========================\n";

    std::cout
        << "Total de Branches: "
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