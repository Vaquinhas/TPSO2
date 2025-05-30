#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "structs.h"

#define THREADNAME _T("Threadname")

DWORD WINAPI gerePipes(LPVOID param) {
    Jogador* jogador = (Jogador*)param;
    BOOL ret;
    TCHAR buf[256];
    DWORD n;

    OVERLAPPED ov;
    HANDLE hEv = CreateEvent(NULL, TRUE, FALSE, NULL);
    do {
        if (jogador->entrou == FALSE) {
            _stprintf_s(buf, 256, _T(";%s"), jogador->nome);

            ZeroMemory(&ov, sizeof(OVERLAPPED));
            ov.hEvent = hEv;

            ret = WriteFile(jogador->hPipe, buf, (DWORD)_tcslen(buf) * sizeof(TCHAR), &n, &ov);
            if (!ret && GetLastError() != ERROR_IO_PENDING) { //End Of File
                _tprintf_s(_T("[LEITOR] %d (%d bytes)... (WriteFile)\n"), ret, n);
                break;
            }

            if (GetLastError() == ERROR_IO_PENDING) {
                WaitForSingleObject(hEv, INFINITE); //Esperar pelo fim da op
                GetOverlappedResult(jogador->hPipe, &ov, &n, FALSE); //Obter res
            }

            ZeroMemory(&ov, sizeof(OVERLAPPED));
            ov.hEvent = hEv;



            ret = ReadFile(jogador->hPipe, buf, sizeof(buf), &n, &ov);
            if (!ret && GetLastError() != ERROR_IO_PENDING) { //End Of File
                _tprintf_s(_T("[LEITOR] %d (%d bytes)... (WriteFile)\n"), ret, n);
                break;
            }

            if (GetLastError() == ERROR_IO_PENDING) {
                WaitForSingleObject(hEv, INFINITE); //Esperar pelo fim da op
                GetOverlappedResult(jogador->hPipe, &ov, &n, FALSE); //Obter res
            }

            buf[n / sizeof(TCHAR)] = _T('\0');

            if (_tcscmp(buf, _T("bruh")) == 0) {
                _tprintf(_T("[JOGOUI] Esse nome já existe!\n"));
                jogador->continua = FALSE;
                ExitThread(0);
            }

            _tprintf(_T("[JOGOUI] Boa Sorte! Já podes começar a digitar\n"));
            jogador->entrou = TRUE;
        }

        _fgetts(buf, 256, stdin);
        buf[_tcslen(buf) - 1] = _T('\0');

        ZeroMemory(&ov, sizeof(OVERLAPPED));
        ov.hEvent = hEv;
        ret = WriteFile(jogador->hPipe, buf, (DWORD)_tcslen(buf) * sizeof(TCHAR), &n, &ov);
        if (!ret && GetLastError() != ERROR_IO_PENDING) { //End Of File
            _tprintf_s(_T("[LEITOR] %d (%d bytes)... (WriteFile)\n"), ret, n);
            break;
        }

        if (GetLastError() == ERROR_IO_PENDING) {
            WaitForSingleObject(hEv, INFINITE); //Esperar pelo fim da op
            GetOverlappedResult(jogador->hPipe, &ov, &n, FALSE); //Obter res
        }

        ZeroMemory(&ov, sizeof(OVERLAPPED));
        ov.hEvent = hEv;

        ret = ReadFile(jogador->hPipe, buf, sizeof(buf), &n, &ov);
        if (!ret && GetLastError() != ERROR_IO_PENDING) { //End Of File
            _tprintf_s(_T("[LEITOR] %d (%d bytes)... (WriteFile)\n"), ret, n);
            break;
        }

        if (GetLastError() == ERROR_IO_PENDING) {
            WaitForSingleObject(hEv, INFINITE); //Esperar pelo fim da op
            GetOverlappedResult(jogador->hPipe, &ov, &n, FALSE); //Obter res
        }

        buf[n / sizeof(TCHAR)] = _T('\0');

        _tprintf(_T("%s\n"), buf);
        if (_tcscmp(buf, _T("[ÁRBITRO] Saíste do jogo!")) == 0)
            ExitThread(1);

        ZeroMemory(buf, sizeof(buf));

    } while (1);
}

int _tmain(int argc, TCHAR* argv[]) {
    Jogador jogador;
    HANDLE hThreadPipe, hMutex, hEvent, hPipe;
    TCHAR buf[256];

#ifdef UNICODE 
    (void) _setmode(_fileno(stdin), _O_WTEXT);
    (void)_setmode(_fileno(stdout), _O_WTEXT);
    (void)_setmode(_fileno(stderr), _O_WTEXT);
#endif

    if (argc > 1) {
        _tcscpy_s(jogador.nome, 256, argv[1]);
    }
    else {
        _tprintf(_T("Erro ao inicializar programa, nome não especificado\n"));
        exit(-1);
    }

    _tprintf(_T("[JOGOUI] Esperar pelo pipe...\n"));
    if (!WaitNamedPipe(NAME_PIPE, NMPWAIT_WAIT_FOREVER)) {
        _tprintf(_T("[JOGOUI] Erro ao ligar ao pipe... (WaitNamedPipe)\n"));
        exit(-1);
    }

    jogador.hPipe = CreateFile(NAME_PIPE, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
    if (jogador.hPipe == INVALID_HANDLE_VALUE) {
        _tprintf_s(_T("[JOGOUI] Erro ao ligar ao pipe... (CreateFile)\n"));
        exit(-1);
    }

    DWORD modo = PIPE_READMODE_MESSAGE;
    SetNamedPipeHandleState(jogador.hPipe, &modo, NULL, NULL);

    _tprintf(_T("[JOGOUI] Ligado com sucesso! :D \n"));

    jogador.entrou = FALSE;
    jogador.continua = TRUE;
    jogador.pontuacao = 0;
    hThreadPipe = CreateThread(NULL, 0, gerePipes, &jogador, 0, NULL);

    HANDLE hMapFile = OpenFileMapping(FILE_MAP_READ, FALSE, NAME_SHARED_MEMORY);
    if (hMapFile == NULL) {
        _tprintf(_T("[JOGOUI] Erro ao abrir memória partilhada: (%d)\n"), GetLastError());
        CloseHandle(jogador.hPipe);
        exit(-1);
    }

    Letras_Visiveis* letras_partilhadas = (Letras_Visiveis*)MapViewOfFile(hMapFile, FILE_MAP_READ, 0, 0, sizeof(Letras_Visiveis));
    if (letras_partilhadas == NULL) {
        _tprintf(_T("[JOGOUI] Erro ao mapear view of file: (%d)\n"), GetLastError());
        CloseHandle(jogador.hPipe);
        CloseHandle(hMapFile);
        exit(-1);
    }

    hMutex = CreateMutex(NULL, FALSE, NAME_MUTEX);
    if (hMutex == INVALID_HANDLE_VALUE) {
        _tprintf(_T("[JOGOUI] Erro ao criar mutex: (%d)"), GetLastError());
        CloseHandle(jogador.hPipe);
        UnmapViewOfFile(letras_partilhadas);
        CloseHandle(hMapFile);
        exit(-1);
    }

    hEvent = CreateEvent(NULL, TRUE, FALSE, NAME_EVENT);
    if (hEvent == INVALID_HANDLE_VALUE) {
        _tprintf(_T("[JOGOUI] Erro ao criar mutex: (%d)"), GetLastError());
        UnmapViewOfFile(letras_partilhadas);
        CloseHandle(hMapFile);
        CloseHandle(jogador.hPipe);
        CloseHandle(hMutex);
        exit(-1);
    }

    while (jogador.continua) {
        WaitForSingleObject(hMutex, INFINITE);

        Letras_Visiveis letras_local;
        CopyMemory(&letras_local, letras_partilhadas, sizeof(Letras_Visiveis));

        ReleaseMutex(hMutex);

        _tprintf(_T("[JOGOUI] Letras visíveis na memória partilhada: "));
        for (int i = 0; i < MAXLETRAS_ABS; i++) {
            if (letras_local.usadas[i]) {
                _tprintf(_T("%c "), letras_local.letras[i]);
            }
            else {
                _tprintf(_T("_ "));
            }
        }
        _tprintf(_T("\n"));
        WaitForSingleObject(hEvent, INFINITE);
    }


    CloseHandle(jogador.hPipe);
    UnmapViewOfFile(letras_partilhadas);
    CloseHandle(hMapFile);
    return 0;
}