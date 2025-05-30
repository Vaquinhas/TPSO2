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

TCHAR* dicionario[] = {
    // A
    _T("A"), _T("ABACATE"), _T("ABERTO"), _T("ABISMO"), _T("ACORDO"), _T("ALEGRE"), _T("ALTO"),
    // B
    _T("B"), _T("BALA"), _T("BARCO"), _T("BEIJO"), _T("BOLA"), _T("BRAVO"), _T("BURRO"),
    // C
    _T("C"), _T("COBRA"), _T("CAFE"), _T("CAMA"), _T("CARRO"), _T("CASA"), _T("CHUVA"),
    // D
    _T("D"), _T("DADO"), _T("DANCA"), _T("DEDO"), _T("DENTE"), _T("DESENHO"), _T("DORMIR"),
    // E
    _T("E"), _T("ECO"), _T("EFEITO"), _T("ELEFANTE"), _T("ELOGIO"), _T("ENCONTRO"), _T("ESTRELA"),
    // F
    _T("F"), _T("FACA"), _T("FIM"), _T("FATO"), _T("FELIZ"), _T("FERRO"), _T("FOGO"),
    // G
    _T("G"), _T("GADO"), _T("GALO"), _T("GATO"), _T("GELO"), _T("GIRAR"), _T("GROSSO"),
    // H
     _T("H"), _T("HAMBURGUER"), _T("HARMONIA"), _T("HEROI"), _T("HOJE"), _T("HOTEL"), _T("HUMANO"),
     // I
     _T("I"), _T("IDADE"), _T("IGREJA"), _T("ILHA"), _T("IMAGEM"), _T("IMPORTANTE"), _T("INTERIOR"),
     // J
     _T("J"), _T("JARDIM"), _T("JATO"), _T("JOGO"), _T("JOIA"), _T("JUIZ"), _T("JUSTO"),
     // K
     _T("K"), _T("KILO"), _T("KARATE"), _T("KIT"), _T("KIWI"), _T("KARAOKE"),
     // L
     _T("L"), _T("LAGO"), _T("LATA"), _T("LEAO"), _T("LEITE"), _T("LIVRO"), _T("LUZ"),
     // M
     _T("M"), _T("MACA"), _T("MALA"), _T("MAMUTE"), _T("MAPA"), _T("MAR"), _T("MESA"),
     // N
     _T("N"), _T("NAVIO"), _T("NEVE"), _T("NOITE"), _T("NORTE"), _T("NUVEM"), _T("NUMERO"),
     // O
     _T("O"), _T("OCEANO"), _T("ORAR"), _T("ONDA"), _T("OLHO"), _T("OPCAO"), _T("OURO"),
     // P
     _T("P"), _T("PACO"), _T("PATO"), _T("PEDRA"), _T("PEIXE"), _T("PERTO"), _T("PINTAR"),
     // Q
      _T("Q"), _T("QUADRO"), _T("QUENTE"), _T("QUILO"), _T("QUINTO"), _T("QUIETO"),
      // R
      _T("R"), _T("RAIO"), _T("RATO"), _T("REINO"), _T("RIO"), _T("RODA"), _T("RODA"),
      // S
      _T("S"), _T("SABOR"), _T("SAL"), _T("SAPATO"), _T("SELVA"), _T("SIM"), _T("SOL"),
      // T
      _T("T"), _T("TACO"), _T("TARDE"), _T("TELA"), _T("TEMPO"), _T("TERRA"), _T("TIGRE"),
      // U
      _T("U"), _T("URSO"), _T("UVA"), _T("UNICO"), _T("URSO"), _T("URGENTE"),
      // V
      _T("V"), _T("VACA"), _T("VALOR"), _T("VENTO"), _T("VIDA"), _T("VIVO"), _T("VOZ"),
      // W
      _T("W"), _T("WATT"), _T("WEB"), _T("WIFI"), _T("WOK"), _T("WHISKY"),
      // X
      _T("X"), _T("XADREZ"), _T("XAROPE"), _T("XEROX"), _T("XILOFONE"), _T("XIS"),
      // Y
       _T("Y"), _T("YETI"), _T("YOGA"), _T("YOYO"), _T("YAKISOBA"),
       // Z
       _T("Z"), _T("ZEBRA"), _T("ZERO"), _T("ZIGZAG"), _T("ZOOLOGICO"), _T("ZANGADO")
};

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
    DWORD i;
    DWORD terminar;
    HANDLE hEvent, hMutex, hMapping;
    BOOL continua;
} EstadoJogo;

#endif