#pragma once
#ifndef STRUCT
#define STRUCT

#define TAM 50
#define MAX_JOGADORES 20
#define MAXLETRAS_PADRAO 6
#define RITMO_PADRAO 3
#define MAXLETRAS_ABS 12
#define NAME_SHARED_MEMORY _T("memory")
#define NAME_PIPE _T("\\\\.\\pipe\\teste")
#define NAME_MUTEX _T("mutex")
#define NAME_EVENT _T("event") //IF NEEDED

typedef struct {
    TCHAR nome[TAM];
    float pontuacao;
    HANDLE hPipe;
    BOOL entrou, continua;
} Jogador;

typedef struct {
    TCHAR letras[MAXLETRAS_ABS];
    TCHAR usadas[MAXLETRAS_ABS];
    DWORD tamanho;
} Letras_Visiveis;


typedef struct {
    Jogador jogadores[MAX_JOGADORES];
    DWORD num_jogadores;
    Letras_Visiveis* letras;
    DWORD ritmo;
    DWORD max_letras;
    DWORD jogo_ativo;
    DWORD terminar;
    HANDLE hEvent, hMutex, hMapping;
    BOOL continua;
} EstadoJogo;

#endif