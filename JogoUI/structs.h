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
    _T("abacate"), _T("aberto"), _T("abismo"), _T("acordo"), _T("alegre"), _T("alto"),
    // B
    _T("bala"), _T("barco"), _T("beijo"), _T("bola"), _T("bravo"), _T("burro"),
    // C
    _T("cabra"), _T("cafe"), _T("cama"), _T("carro"), _T("casa"), _T("chuva"),
    // D
    _T("dado"), _T("dança"), _T("dedo"), _T("dente"), _T("desenho"), _T("dormir"),
    // E
    _T("eco"), _T("efeito"), _T("elefante"), _T("elogio"), _T("encontro"), _T("estrela"),
    // F
    _T("faca"), _T("fim"), _T("fato"), _T("feliz"), _T("ferro"), _T("fogo"),
    // G
    _T("gado"), _T("galo"), _T("gato"), _T("gelo"), _T("girar"), _T("grosso"),
    // H
    _T("hamburguer"), _T("harmonia"), _T("heroi"), _T("hoje"), _T("hotel"), _T("humano"),
    // I
    _T("idade"), _T("igreja"), _T("ilha"), _T("imagem"), _T("importante"), _T("interior"),
    // J
    _T("jardim"), _T("jato"), _T("jogo"), _T("joia"), _T("juiz"), _T("justo"),
    // K
    _T("kilo"), _T("karate"), _T("kit"), _T("kiwi"), _T("karaoke"),
    // L
    _T("lago"), _T("lata"), _T("leao"), _T("leite"), _T("livro"), _T("luz"),
    // M
    _T("maca"), _T("mala"), _T("mamute"), _T("mapa"), _T("mar"), _T("mesa"),
    // N
    _T("navio"), _T("neve"), _T("noite"), _T("norte"), _T("nuvem"), _T("numero"),
    // O
    _T("oceano"), _T("olho"), _T("onda"), _T("onibus"), _T("opcao"), _T("ouro"),
    // P
    _T("paco"), _T("pato"), _T("pedra"), _T("peixe"), _T("perto"), _T("pintar"),
    // Q
    _T("quadro"), _T("quente"), _T("quilo"), _T("quinto"), _T("quieto"),
    // R
    _T("raio"), _T("rato"), _T("reino"), _T("rio"), _T("roda"), _T("rosa"),
    // S
    _T("sabor"), _T("sal"), _T("sapato"), _T("selva"), _T("sim"), _T("sol"),
    // T
    _T("taco"), _T("tarde"), _T("tela"), _T("tempo"), _T("terra"), _T("tigre"),
    // U
    _T("urso"), _T("uva"), _T("unico"), _T("urso"), _T("urgente"),
    // V
    _T("vaca"), _T("valor"), _T("vento"), _T("vida"), _T("vivo"), _T("voz"),
    // W
    _T("watt"), _T("web"), _T("wifi"), _T("wok"), _T("whisky"),
    // X
    _T("xadrez"), _T("xarope"), _T("xerox"), _T("xilofone"), _T("xis"),
    // Y
    _T("yeti"), _T("yoga"), _T("yoyo"), _T("yakisoba"),
    // Z
    _T("zebra"), _T("zero"), _T("zigzag"), _T("zoologico"), _T("zangado")
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