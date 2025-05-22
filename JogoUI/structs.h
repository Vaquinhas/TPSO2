#pragma once
#ifndef STRUCT
#define STRUCT

#define MAX_JOGADORES 20
#define MAXLETRAS_PADRAO 6
#define RITMO_PADRAO 3
#define MAXLETRAS_ABS 12

typedef struct {
    char nome[50];
    float pontuacao;
    int ativo;
} Jogador;

typedef struct {
    char letras[MAXLETRAS_ABS];
    char usadas[MAXLETRAS_ABS];
    int tamanho;
} Letras_Visiveis;

typedef struct {
    Jogador jogadores[MAX_JOGADORES];
    int num_jogadores;
    Letras_Visiveis letras;
    int ritmo;
    int max_letras;
    int jogo_ativo;
    int terminar;
} EstadoJogo;

#endif