#pragma once
#ifndef STRUCT
#define STRUCT

#define MAX_JOGADORES 20
#define MAXLETRAS_PADRAO 6
#define RITMO_PADRAO 3
#define MAXLETRAS_ABS 12

typedef struct {
    TCHAR nome[50];
    float pontuacao;
    DWORD ativo;
} Jogador;

typedef struct {
    TCHAR letras[MAXLETRAS_ABS];
    TCHAR usadas[MAXLETRAS_ABS];
    DWORD tamanho;
} Letras_Visiveis;

typedef struct {
    Jogador jogadores[MAX_JOGADORES];
    DWORD num_jogadores;
    Letras_Visiveis letras;
    DWORD ritmo;
    DWORD max_letras;
    DWORD jogo_ativo;
    DWORD terminar;
} EstadoJogo;

#endif