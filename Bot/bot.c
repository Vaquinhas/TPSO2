#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "structs.h"

int palavra_e_possivel(const TCHAR* palavra, const Letras_Visiveis* letras) {
    int usadas[MAXLETRAS_ABS] = { 0 };

    for (int i = 0; palavra[i] != _T('\0'); i++) {
        BOOL encontrada = FALSE;
        for (int j = 0; j < MAXLETRAS_ABS; j++) {
            if (!usadas[j] && letras->usadas[j] && letras->letras[j] == palavra[i]) {
                usadas[j] = 1;  // Marcar como usada
                encontrada = TRUE;
                break;
            }
        }
        if (!encontrada)
            return 0; // letra da palavra não encontrada nas visíveis
    }
    return 1;
}

int _tmain(int argc, TCHAR* argv[]) {
#ifdef UNICODE
    _setmode(_fileno(stdin), _O_WTEXT);
    _setmode(_fileno(stdout), _O_WTEXT);
    _setmode(_fileno(stderr), _O_WTEXT);
#endif

    if (argc != 3) {
        _tprintf(_T("Uso: %s <NOME_BOT> <TEMPO_REACAO_SEGUNDOS>\n"), argv[0]);
        return 1;
    }

    TCHAR nome[TAM];
    _tcscpy_s(nome, TAM, argv[1]);
    int tempo_reacao = _ttoi(argv[2]);
    if (tempo_reacao <= 0) tempo_reacao = 3;

    // Conectar ao pipe
    if (!WaitNamedPipe(NAME_PIPE, NMPWAIT_WAIT_FOREVER)) {
        _tprintf(_T("[BOT] Pipe indisponível.\n"));
        return 1;
    }

    HANDLE hPipe = CreateFile(NAME_PIPE, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
    if (hPipe == INVALID_HANDLE_VALUE) {
        _tprintf(_T("[BOT] Erro ao abrir o pipe.\n"));
        return 1;
    }

    // Registar-se como jogador (envia ;nome)
    TCHAR buf[256];
    _stprintf_s(buf, 256, _T(";%s"), nome);

    DWORD n;
    OVERLAPPED ov = { 0 };
    ov.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    WriteFile(hPipe, buf, (DWORD)_tcslen(buf) * sizeof(TCHAR), &n, &ov);
    WaitForSingleObject(ov.hEvent, INFINITE);
    GetOverlappedResult(hPipe, &ov, &n, FALSE);

    ReadFile(hPipe, buf, sizeof(buf), &n, &ov);
    WaitForSingleObject(ov.hEvent, INFINITE);
    GetOverlappedResult(hPipe, &ov, &n, FALSE);
    buf[n / sizeof(TCHAR)] = _T('\0');

    if (_tcscmp(buf, _T("bruh")) == 0) {
        _tprintf(_T("[BOT] Nome já existe!\n"));
        CloseHandle(hPipe);
        return 1;
    }

    _tprintf(_T("[BOT %s] Iniciado com tempo de reação de %d segundos\n"), nome, tempo_reacao);

    HANDLE hMap = OpenFileMapping(FILE_MAP_READ, FALSE, NAME_SHARED_MEMORY);
    Letras_Visiveis* letras = (Letras_Visiveis*)MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, sizeof(Letras_Visiveis));

    HANDLE hMutex = OpenMutex(SYNCHRONIZE, FALSE, NAME_MUTEX);
    HANDLE hEvent = OpenEvent(SYNCHRONIZE, FALSE, NAME_EVENT);

    srand((unsigned)time(NULL));

    while (1) {
        WaitForSingleObject(hMutex, INFINITE);

        Letras_Visiveis copia;
        CopyMemory(&copia, letras, sizeof(Letras_Visiveis));

        ReleaseMutex(hMutex);

        // Escolher palavra
        const int total = sizeof(dicionario) / sizeof(dicionario[0]);
        const TCHAR* escolhida = NULL;
        int tentativas = 50;
        while (tentativas--) {
            const TCHAR* tentativa = dicionario[rand() % total];
            if (_tcslen(tentativa) > 1 && palavra_e_possivel(tentativa, &copia)) {
                escolhida = tentativa;
                break;
            }
        }

        if (escolhida) {
            _tprintf(_T("[BOT %s] Tenta: %s\n"), nome, escolhida);

            // Enviar palavra
            _tcscpy_s(buf, 256, escolhida);
            ZeroMemory(&ov, sizeof(ov));
            ov.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
            WriteFile(hPipe, buf, (DWORD)_tcslen(buf) * sizeof(TCHAR), &n, &ov);
            WaitForSingleObject(ov.hEvent, INFINITE);
            GetOverlappedResult(hPipe, &ov, &n, FALSE);

            // Ler resposta
            ReadFile(hPipe, buf, sizeof(buf), &n, &ov);
            WaitForSingleObject(ov.hEvent, INFINITE);
            GetOverlappedResult(hPipe, &ov, &n, FALSE);
            buf[n / sizeof(TCHAR)] = _T('\0');
            _tprintf(_T("[BOT %s] Resposta: %s\n"), nome, buf);
        }
        else {
            _tprintf(_T("[BOT %s] Nenhuma palavra possível com letras atuais.\n"), nome);
        }

        Sleep(tempo_reacao * 1000);
    }

    // Cleanup (não chega aqui normalmente)
    UnmapViewOfFile(letras);
    CloseHandle(hMap);
    CloseHandle(hPipe);
    return 0;
}
