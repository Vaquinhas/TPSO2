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

    resultado = RegCreateKeyEx(HKEY_CURRENT_USER, _T("Software\\TrabSO2"), 0, NULL,
        REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, NULL);
    if (resultado != ERROR_SUCCESS) {
        _tprintf(_T("[ERRO] A abrir/criar chave no registry.\n"));
        return;
    }

    // Ler MAXLETRAS (não está a ser usado ainda diretamente)
    resultado = RegQueryValueEx(hKey, _T("MAXLETRAS"), NULL, &tipo, (LPBYTE)&valor, &tamanho);
    if (resultado != ERROR_SUCCESS || valor > MAXLETRAS_ABS || valor <= 0) {
        valor = MAXLETRAS_PADRAO;
        RegSetValueEx(hKey, _T("MAXLETRAS"), 0, REG_DWORD, (BYTE*)&valor, sizeof(DWORD));
    }
    estado->max_letras = valor;
    _tprintf(_T("[ÁRBITRO] MAXLETRAS (do registry): %d\n"), valor);

    // Ler RITMO
    resultado = RegQueryValueEx(hKey, _T("RITMO"), NULL, &tipo, (LPBYTE)&valor, &tamanho);
    if (resultado == ERROR_SUCCESS && valor > 0) {
        estado->ritmo = valor;
    }
    else {
        valor = RITMO_PADRAO;
        estado->ritmo = valor;
        RegSetValueEx(hKey, _T("RITMO"), 0, REG_DWORD, (BYTE*)&valor, sizeof(DWORD));
    }
    _tprintf(_T("[ÁRBITRO] RITMO (do registry): %d\n"), estado->ritmo);

    RegCloseKey(hKey);
}

TCHAR gerar_letra() {
    return _T('A') + rand() % 26;
}

BOOL palavraValidaNasLetras(TCHAR* palavra, TCHAR* letrasDisponiveis) {
    TCHAR letrasTemp[26];
    _tcscpy_s(letrasTemp, 26, letrasDisponiveis); // Copiar para manipular localmente

    for (int i = 0; palavra[i] != _T('\0'); i++) {
        TCHAR* ptr = _tcschr(letrasTemp, palavra[i]);
        if (ptr == NULL) {
            return FALSE; // Letra não existe nas disponíveis
        }
        *ptr = _T(' '); // Marcar como usada
    }

    return TRUE;
}

DWORD WINAPI atendeJogador(LPVOID param) {
    Jogador* jogador = (Jogador*)param;

    HANDLE hMapFile = OpenFileMapping(FILE_MAP_READ, FALSE, NAME_SHARED_MEMORY);
    if (hMapFile == NULL) {
        _tprintf(_T("[JOGOUI] Erro ao abrir memória partilhada: (%d)\n"), GetLastError());

        exit(-1);
    }
    Letras_Visiveis* letras_partilhadas = (Letras_Visiveis*)MapViewOfFile(hMapFile, FILE_MAP_READ, 0, 0, sizeof(Letras_Visiveis));
    if (letras_partilhadas == NULL) {
        _tprintf(_T("[JOGOUI] Erro ao mapear view of file: (%d)\n"), GetLastError());

        CloseHandle(hMapFile);
        exit(-1);
    }
    Letras_Visiveis letras_local;
    CopyMemory(&letras_local, letras_partilhadas, sizeof(Letras_Visiveis));



    TCHAR buf[256];
    BOOL ret;
    DWORD n;

    OVERLAPPED ov;
    HANDLE hEv = CreateEvent(NULL, TRUE, FALSE, NULL);

    do {
        ZeroMemory(&ov, sizeof(OVERLAPPED));
        ov.hEvent = hEv;

        ret = ReadFile(jogador->hPipe, buf, sizeof(buf), &n, &ov);
        if (!ret && GetLastError() == ERROR_IO_PENDING) {
            WaitForSingleObject(hEv, INFINITE);
            GetOverlappedResult(jogador->hPipe, &ov, &n, FALSE);
        }
        else if (!ret) {
            _tprintf(_T("[ÁRBITRO] Erro de leitura: %lu\n"), GetLastError());
            break;
        }

        buf[n / sizeof(TCHAR)] = _T('\0');

        /*logica*/
        if (buf[0] == _T(':')) {
            if (_tcscmp(buf, _T(":ponts")) == 0) {

                _stprintf_s(buf, 256, _T("[ÁRBITRO] Tens %.2f pontos!!"), jogador->pontuacao);
                ZeroMemory(&ov, sizeof(OVERLAPPED));
                ov.hEvent = hEv;

                ret = WriteFile(jogador->hPipe, buf, (DWORD)_tcslen(buf) * sizeof(TCHAR), &n, &ov);
                if (!ret && GetLastError() != ERROR_IO_PENDING) { //End Of File
                    break;
                }

                if (GetLastError() == ERROR_IO_PENDING) {
                    WaitForSingleObject(hEv, INFINITE); //Esperar pelo fim da op
                    GetOverlappedResult(jogador->hPipe, &ov, &n, FALSE); //Obter res
                }

            }

            if (_tcscmp(buf, _T(":sair")) == 0) {
                ZeroMemory(&ov, sizeof(OVERLAPPED));
                ov.hEvent = hEv;

                _stprintf_s(buf, 256, _T("[ÁRBITRO] Saíste do jogo!"));

                ret = WriteFile(jogador->hPipe, buf, (DWORD)_tcslen(buf) * sizeof(TCHAR), &n, &ov);
                if (!ret && GetLastError() != ERROR_IO_PENDING) { //End Of File
                    break;
                }

                if (GetLastError() == ERROR_IO_PENDING) {
                    WaitForSingleObject(hEv, INFINITE); //Esperar pelo fim da op
                    GetOverlappedResult(jogador->hPipe, &ov, &n, FALSE); //Obter res
                }

                ExitThread(-1);
            }
            continue;
        }

        BOOL sim = FALSE, aux = FALSE;

        int tamDic = sizeof(dicionario) / sizeof(dicionario[0]);
        BOOL palavra_encontrada = FALSE;

        for (int i = 0; i < tamDic; i++) {
            if (strcmp(buf, dicionario[i]) == 0) {
                palavra_encontrada = TRUE;
                BOOL letras_validas = TRUE;

                for (int j = 0; j < _tcslen(buf); j++) {
                    BOOL letra_encontrada = FALSE;
                    for (int k = 0; k < MAXLETRAS_PADRAO; k++) {
                        if (buf[j] == letras_local.letras[k]) {
                            letra_encontrada = TRUE;
                            break;
                        }
                    }
                    if (!letra_encontrada) {
                        letras_validas = FALSE;
                        break;
                    }
                }

                if (letras_validas) {
                    _stprintf_s(buf, 256, _T("[ÁRBITRO] Palavra certa boa! "));
                    jogador->pontuacao += 1;
                }
                else {
                    _stprintf_s(buf, 256, _T("[ÁRBITRO] Palavra errada (letras inválidas)"));
                    jogador->pontuacao -= 0.5;
                }
                break; // já encontraste a palavra, não precisas continuar a procurar
            }
        }

        // Se a palavra não estiver no dicionário
        if (!palavra_encontrada) {
            _stprintf_s(buf, 256, _T("[ÁRBITRO] Palavra errada (não está no dicionário)"));
            jogador->pontuacao -= 0.5;
        }



        ZeroMemory(&ov, sizeof(OVERLAPPED));
        ov.hEvent = hEv;

        ret = WriteFile(jogador->hPipe, buf, (DWORD)_tcslen(buf) * sizeof(TCHAR), &n, &ov);
        if (!ret && GetLastError() != ERROR_IO_PENDING) { //End Of File
            break;
        }

        if (GetLastError() == ERROR_IO_PENDING) {
            WaitForSingleObject(hEv, INFINITE); //Esperar pelo fim da op
            GetOverlappedResult(jogador->hPipe, &ov, &n, FALSE); //Obter res
        }

    } while (1);

    CloseHandle(hEv);
    ExitThread(0);
}

DWORD WINAPI thread_letras(LPVOID param) {
    EstadoJogo* estado = (EstadoJogo*)param;
    srand((unsigned)time(NULL));

    while (!estado->terminar) {
        if (estado->letras->tamanho < estado->max_letras) {
            WaitForSingleObject(estado->hMutex, INFINITE);

            // Cópia local da estrutura partilhada
            Letras_Visiveis letras = *estado->letras;

            if (letras.tamanho < MAXLETRAS_ABS) {
                for (int i = 0; i < MAXLETRAS_ABS; i++) {
                    if (!letras.usadas[i]) {
                        letras.letras[i] = gerar_letra();
                        letras.usadas[i] = 1;
                        letras.tamanho++;
                        break;
                    }
                }
            }
            else {
                for (int i = 0; i < MAXLETRAS_ABS; i++) {
                    if (letras.usadas[i]) {
                        letras.letras[i] = gerar_letra();
                        break;
                    }
                }
            }

            CopyMemory(estado->letras, &letras, sizeof(Letras_Visiveis));

            ReleaseMutex(estado->hMutex);

            SetEvent(estado->hEvent);
        }
        Sleep(estado->ritmo * 1000);
    }

    ExitThread(1);
}

DWORD WINAPI thread_teclado(LPVOID param) {
    EstadoJogo* jogo = (EstadoJogo*)param;
    TCHAR buf[256], aux[50];

    while (1) {
        _fgetts(buf, 256, stdin);
        buf[_tcslen(buf) - 1] = _T('\0');

        if (_tcscmp(buf, _T("acelerar")) == 0) {
            _tprintf(_T("Acelerando o jogo"));
            if (jogo->ritmo > 1)
                jogo->ritmo--;
        }
        else if (_tcscmp(buf, _T("travar")) == 0) {
            _tprintf(_T("Travar o jogo"));
            jogo->ritmo++;
        }
        else if (_tcscmp(buf, _T("listar")) == 0) {
            _tprintf(_T("Jogadores: \n"));
            for (int i = 0; i < MAX_JOGADORES; i++) {
                _tprintf(_T("%s"), jogo->jogadores[i].nome);
            }
        }
        else if (_tcsncmp(buf, _T("excluir "), 8) == 0) {
            TCHAR* nomeExcluir = buf + 8;
            BOOL encontrado = FALSE;

            WaitForSingleObject(jogo->hMutex, INFINITE);
            for (int i = 0; i < MAX_JOGADORES; i++) {
                if (jogo->jogadores[i].hPipe != NULL && _tcscmp(jogo->jogadores[i].nome, nomeExcluir) == 0) {
                    _tprintf(_T("[ÁRBITRO] Jogador %s será excluído.\n"), jogo->jogadores[i].nome);
                    TCHAR msg[] = _T("Foste excluído\n");
                    DWORD escritos;
                    WriteFile(jogo->jogadores[i].hPipe, msg, (DWORD)(_tcslen(msg) * sizeof(TCHAR)), &escritos, NULL);

                    // Fechar o pipe
                    CloseHandle(jogo->jogadores[i].hPipe);
                    jogo->jogadores[i].hPipe = NULL;

                    // Limpar nome e pontuação
                    ZeroMemory(jogo->jogadores[i].nome, sizeof(jogo->jogadores[i].nome));
                    jogo->jogadores[i].pontuacao = 0;

                    encontrado = TRUE;
                    break;
                }
            }
            ReleaseMutex(jogo->hMutex);

            if (!encontrado) {
                _tprintf(_T("[ÁRBITRO] Jogador '%s' não encontrado.\n"), nomeExcluir);
            }
        }
        else if (_tcscmp(buf, _T("encerrar")) == 0) {
            _tprintf(_T("[ÁRBITRO] Encerrando o jogo e todos os jogadores...\n"));

            WaitForSingleObject(jogo->hMutex, INFINITE);
            for (int i = 0; i < MAX_JOGADORES; i++) {
                if (jogo->jogadores[i].hPipe != NULL) {
                    // Opcional: enviar mensagem de encerramento ao jogador
                    DWORD escritos;
                    TCHAR msg[] = _T("O jogo foi encerrado pelo árbitro.\n");
                    WriteFile(jogo->jogadores[i].hPipe, msg, (DWORD)(_tcslen(msg) * sizeof(TCHAR)), &escritos, NULL);

                    // Fecha o pipe
                    CloseHandle(jogo->jogadores[i].hPipe);
                    jogo->jogadores[i].hPipe = NULL;
                }

                // Limpa os dados do jogador
                ZeroMemory(jogo->jogadores[i].nome, sizeof(jogo->jogadores[i].nome));
                jogo->jogadores[i].pontuacao = 0;
            }

            // Sinaliza que o jogo terminou (assumindo que tens esta variável)
            jogo->continua = FALSE;

            ReleaseMutex(jogo->hMutex);

            _tprintf(_T("[ÁRBITRO] Jogo encerrado com sucesso.\n"));
        }
    }


    ExitThread(1);
}

int _tmain(int argc, LPTSTR argv[]) {
    HANDLE hThreadLetras, hThreadPipe, hPipe, hThreadJogador, hThreadTeclado;
    EstadoJogo jogo;
    Jogador jogador;
    TCHAR buf[256];
    DWORD i;
    BOOL ret, temp;
    DWORD n;

    OVERLAPPED ov;
    HANDLE hEv = CreateEvent(NULL, TRUE, FALSE, NULL);


#ifdef UNICODE 
    (void) _setmode(_fileno(stdin), _O_WTEXT);
    (void)_setmode(_fileno(stdout), _O_WTEXT);
    (void)_setmode(_fileno(stderr), _O_WTEXT);
#endif

    ZeroMemory(jogo.jogadores, sizeof(jogo.jogadores));

    jogo.hMapping = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, TAM * sizeof(TCHAR), NAME_SHARED_MEMORY);
    if (jogo.hMapping == INVALID_HANDLE_VALUE) {
        _tprintf(_T("[ÁRBITRO] Erro ao mapear o ficheiro: (%d)\n"), GetLastError());
        ExitProcess(1);
    }


    jogo.letras = (Letras_Visiveis*)MapViewOfFile(jogo.hMapping, FILE_MAP_ALL_ACCESS, 0, 0, TAM * sizeof(TCHAR));
    if (jogo.letras == NULL) {
        _tprintf(_T("[ÁRBITRO] Erro ao mapear view of file: (%d)\n"), GetLastError());
        CloseHandle(jogo.hMapping);
        ExitProcess(1);
    }

    jogo.hEvent = CreateEvent(NULL, FALSE, FALSE, NAME_EVENT);
    if (jogo.hEvent == INVALID_HANDLE_VALUE) {
        _tprintf(_T("[ÁRBITRO] Erro ao criar evento.\n"));
        CloseHandle(jogo.hMapping);
        exit(-1);
    }

    jogo.hMutex = CreateMutex(NULL, FALSE, NAME_MUTEX);
    if (jogo.hMutex == INVALID_HANDLE_VALUE) {
        _tprintf(_T("[ÁRBITRO] Erro ao criar mutex.\n"));
        CloseHandle(jogo.hMapping);
        CloseHandle(jogo.hEvent);
        exit(-1);
    }

    jogo.letras->tamanho = 0;
    jogo.terminar = 0;
    ler_config_do_registry(&jogo);
    jogo.jogo_ativo = 1;


    hThreadLetras = CreateThread(NULL, 0, thread_letras, &jogo, 0, NULL);
    if (hThreadLetras == NULL) {
        _tprintf(_T("[ÁRBITRO] Erro ao criar thread.\n"));
        CloseHandle(jogo.hMapping);
        CloseHandle(jogo.hEvent);
        CloseHandle(jogo.hMutex);
        exit(-1);
    }

    hThreadTeclado = CreateThread(NULL, 0, thread_teclado, &jogo, 0, NULL);

    do {
        temp = TRUE;
        hPipe = CreateNamedPipe(NAME_PIPE, PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED, PIPE_WAIT | PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE, MAX_JOGADORES, sizeof(buf), sizeof(buf), 3000, NULL);
        if (hPipe == INVALID_HANDLE_VALUE) {
            _tprintf(_T("[ÁRBITRO] Erro ao criar Named Pipe\n"));
            CloseHandle(jogo.hMapping);
            CloseHandle(jogo.hEvent);
            CloseHandle(jogo.hMutex);
            CloseHandle(hThreadLetras);
            exit(-1);
        }

        if (!ConnectNamedPipe(hPipe, NULL)) {
            _tprintf_s(_T("[ÁRBITRO] Erro na ligação ao jogador! (ConnectNamedPipe\n"));
            exit(-1);
        }


        WaitForSingleObject(jogo.hMutex, INFINITE);
        for (i = 0; i < MAX_JOGADORES && jogo.jogadores[i].hPipe != NULL; i++);
        ReleaseMutex(jogo.hMutex);

        if (i < MAX_JOGADORES) {
            jogo.jogadores[i].hPipe = hPipe;
            ZeroMemory(&ov, sizeof(OVERLAPPED));
            ov.hEvent = hEv;

            ret = ReadFile(jogo.jogadores[i].hPipe, buf, sizeof(buf), &n, &ov);
            if (!ret && GetLastError() == ERROR_IO_PENDING) {
                WaitForSingleObject(hEv, INFINITE);
                GetOverlappedResult(jogo.jogadores[i].hPipe, &ov, &n, FALSE);
            }
            else if (!ret) {
                _tprintf(_T("[ÁRBITRO] Erro de leitura: %lu\n"), GetLastError());
                break;
            }

            buf[n / sizeof(TCHAR)] = _T('\0');

            if (buf[0] == _T(';')) {
                memmove(buf, buf + 1, (_tcslen(buf) + 1) * sizeof(TCHAR));

                BOOL temp = TRUE;
                for (int j = 0; j < MAX_JOGADORES; j++) {
                    if (_tcscmp(buf, jogo.jogadores[j].nome) == 0) {
                        temp = FALSE;
                        ZeroMemory(&ov, sizeof(OVERLAPPED));
                        ov.hEvent = hEv;
                        _stprintf_s(buf, 256, _T("bruh"));

                        ret = WriteFile(jogo.jogadores[i].hPipe, buf, (DWORD)_tcslen(buf) * sizeof(TCHAR), &n, &ov);
                        if (!ret && GetLastError() != ERROR_IO_PENDING) break;
                        if (GetLastError() == ERROR_IO_PENDING) {
                            WaitForSingleObject(hEv, INFINITE);
                            GetOverlappedResult(jogo.jogadores[i].hPipe, &ov, &n, FALSE);
                        }
                    }
                }

                if (temp == TRUE) {
                    ZeroMemory(&ov, sizeof(OVERLAPPED));
                    ov.hEvent = hEv;
                    _tcscpy_s(jogo.jogadores[i].nome, sizeof(jogo.jogadores[i].nome) / sizeof(TCHAR), buf);
                    _stprintf_s(buf, 256, _T("ok"));

                    ret = WriteFile(jogo.jogadores[i].hPipe, buf, (DWORD)_tcslen(buf) * sizeof(TCHAR), &n, &ov);
                    if (!ret && GetLastError() != ERROR_IO_PENDING) break;
                    if (GetLastError() == ERROR_IO_PENDING) {
                        WaitForSingleObject(hEv, INFINITE);
                        GetOverlappedResult(jogo.jogadores[i].hPipe, &ov, &n, FALSE);
                    }
                    jogo.jogadores[i].pontuacao = 0;

                    _tprintf(_T("[ÁRBITRO] Jogador %s juntou-se ao jogo\n"), jogo.jogadores[i].nome);
                    hThreadJogador = CreateThread(NULL, 0, atendeJogador, &jogo.jogadores[i], 0, NULL);
                }
            }
        }
        else {
            _tprintf(_T("[ÁRBITRO] Limite de jogadores atingido! Rejeitar conexão.\n"));
            CloseHandle(hPipe);
        }


    } while (jogo.continua);


    jogo.jogo_ativo = 0;
    jogo.terminar = 1;
    WaitForSingleObject(hThreadLetras, INFINITE);

    UnmapViewOfFile(jogo.letras);
    CloseHandle(jogo.hMapping);
    CloseHandle(jogo.hEvent);
    CloseHandle(jogo.hMutex);
    CloseHandle(hThreadLetras);
    return 0;
}