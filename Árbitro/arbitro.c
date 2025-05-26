#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "structs.h"

void ler_config_do_registry(EstadoJogo* estado) {
    HKEY hKey;
    DWORD tipo = REG_DWORD;
    DWORD valor;
    DWORD tamanho = sizeof(DWORD);
    LONG resultado;

    resultado = RegCreateKeyEx(HKEY_CURRENT_USER, "Software\\TrabSO2", 0, NULL,
        REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, NULL);
    if (resultado != ERROR_SUCCESS) {
        _tprintf(_T("[ERRO] A abrir/criar chave no registry.\n"));
        return;
    }

    // Ler MAXLETRAS (não está a ser usado ainda diretamente)
    resultado = RegQueryValueEx(hKey, "MAXLETRAS", NULL, &tipo, (LPBYTE)&valor, &tamanho);
    if (resultado != ERROR_SUCCESS || valor > MAXLETRAS_ABS || valor <= 0) {
        valor = MAXLETRAS_PADRAO;
        RegSetValueEx(hKey, "MAXLETRAS", 0, REG_DWORD, (BYTE*)&valor, sizeof(DWORD));
    }
    estado->max_letras = valor;
    _tprintf(_T("MAXLETRAS (do registry): %d\n"), valor);

    // Ler RITMO
    resultado = RegQueryValueEx(hKey, "RITMO", NULL, &tipo, (LPBYTE)&valor, &tamanho);
    if (resultado == ERROR_SUCCESS && valor > 0) {
        estado->ritmo = valor;
    }
    else {
        valor = RITMO_PADRAO;
        estado->ritmo = valor;
        RegSetValueEx(hKey, "RITMO", 0, REG_DWORD, (BYTE*)&valor, sizeof(DWORD));
    }
    _tprintf(_T("RITMO (do registry): %d\n"), estado->ritmo);

    RegCloseKey(hKey);
}


TCHAR gerar_letra() {
    return _T('A') + rand() % 26;
}

void inicializar_jogo(EstadoJogo* estado) {
    memset(estado, 0, sizeof(EstadoJogo));
    //estado->ritmo = RITMO_PADRAO;
    estado->letras.tamanho = 0;
    ler_config_do_registry(estado);
    // Inicialização mais robusta do gerador aleatório
}

void adicionar_letra(Letras_Visiveis* lv) {
    if (lv->tamanho < MAXLETRAS_ABS) {
        for (int i = 0; i < MAXLETRAS_ABS; i++) {
            if (!lv->usadas[i]) {
                lv->letras[i] = gerar_letra();
                lv->usadas[i] = 1;
                lv->tamanho++;
                break;
            }
        }
    }
    else {
        for (int i = 0; i < MAXLETRAS_ABS; i++) {
            if (lv->letras[i]) {
                lv->letras[i] = gerar_letra();
                break;
            }
        }
    }
}

void imprimir_letras(Letras_Visiveis* lv) {
    _tprintf(_T("Letras Visiveis: "));
    for (int i = 0; i < MAXLETRAS_ABS; i++) {
        if (lv->usadas[i]) {
            _tprintf(_T("%c "), lv->letras[i]);
        }
        else {
            _tprintf(_T("_ "));
        }
    }
    _tprintf(_T("\n"));
}

DWORD WINAPI thread_letras(LPVOID param) {
    EstadoJogo* estado = (EstadoJogo*)param;
    srand(time(NULL));

    while (!estado->terminar) {
        if (estado->jogo_ativo && estado->letras.tamanho < estado->max_letras) {
            adicionar_letra(&estado->letras);
            imprimir_letras(&estado->letras);
        }
        Sleep(estado->ritmo * 1000);
    }
    return 0;
}

int _tmain(int argc, LPTSTR argv[]) {
    HANDLE hThread;
    EstadoJogo jogo;


#ifdef UNICODE
    _setmode(_fileno(stdin), _O_WTEXT);
    _setmode(_fileno(stdout), _O_WTEXT);
    _setmode(_fileno(stderr), _O_WTEXT);
#endif

    inicializar_jogo(&jogo);
    jogo.jogo_ativo = 1;

    hThread = CreateThread(NULL, 0, thread_letras, &jogo, 0, NULL);
    if (hThread == NULL) {
        _tprintf(stderr, _T("Erro ao criar thread.\n"));
        return 1;
    }

    _tprintf(_T("[Arbitro] Jogo Iniciado. Pressione ENTER para sair...\n"));
    getchar();

    jogo.jogo_ativo = 0;
    jogo.terminar = 1;
    WaitForSingleObject(hThread, INFINITE);
    CloseHandle(hThread);
    return 0;
}