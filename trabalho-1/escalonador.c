#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<time.h>

#define QUEUE_LENGTH 100
#define PROCESS_AMOUNT 5
#define TIME_SLICE 2

typedef struct
{
    int buffer[QUEUE_LENGTH];
    int posicaoInserir;
    int posicaoConsumir;
} Fila;

typedef struct
{
    int duracaoIO;
    char tipoIO;
    Fila* filaRetorno;
} IO;

typedef struct
{
    int pid;
    int tempoChegada;
    int tempoServico;
    int tempoSaida;
    int tempoExecutado;
    int tempoRetornoIO;
    int nIO;
    int nIOExecutados;
    int temposIO[3];
    IO* tiposIO[3];
} Processo;


void adicionarNaFila(Fila* fila, int itemID){
    fila->buffer[fila->posicaoInserir] = itemID;
    
    if(fila->posicaoInserir < QUEUE_LENGTH-1) fila->posicaoInserir++;
    else fila->posicaoInserir = 0; 

    if(fila->posicaoConsumir == fila->posicaoInserir) fila->posicaoConsumir++;
}

int consumirDaFila(Fila* fila){
    //Fila vazia
    if(fila->posicaoConsumir == fila->posicaoInserir) return -1;

    if(fila->posicaoConsumir < QUEUE_LENGTH-1){
        fila->posicaoConsumir++;
        return fila->buffer[fila->posicaoConsumir-1];
    }

    //Fim do buffer
    fila->posicaoConsumir = 0;
    return fila->buffer[QUEUE_LENGTH-1];
}

void exibirTabela(Processo tabela[]){
    printf("PID        |tChegada  |tServiço  |temposIO  |tiposIO\n");
    for(int i = 0; i<PROCESS_AMOUNT; i++){
        printf("%02d         |%02d        |%02d        |", tabela[i].pid, tabela[i].tempoChegada, tabela[i].tempoServico);
        
        if(tabela[i].nIO == 0) printf("          |          \n");
        else {
            for(int j=0;j<tabela[i].nIO;j++){
                printf("%02d, ", tabela[i].temposIO[j]);
            }
                printf("|");
            for(int j=0;j<tabela[i].nIO;j++){
                printf("%c, ", tabela[i].tiposIO[j]->tipoIO);
            }
                printf("\n");
        }

    }
}

void criarTabela(Processo tabela[], IO* listaIO[]){
    srand(time(NULL));

    for(int i = 0; i<PROCESS_AMOUNT; i++){
        Processo novoProcesso;
        //PID
        novoProcesso.pid = i;
        //Tempo de chegada
        if(i == 0) novoProcesso.tempoChegada = 0;
        else novoProcesso.tempoChegada = tabela[i-1].tempoChegada + (1 + rand()%5);
        //Tempo de serviço
        novoProcesso.tempoServico = 2+rand()%10;
        //Tempos de IO
        int nIO = rand()%4;
        for(int j=0;j<nIO;j++){
            if(j==0) novoProcesso.temposIO[j] = 1+rand()%(novoProcesso.tempoServico-1); //colocando modulo do tempoServico corre o risco de pedir IO no momento da saida
            else novoProcesso.temposIO[j] = novoProcesso.temposIO[j-1]+rand()%(novoProcesso.tempoServico-novoProcesso.temposIO[j-1]);
            
            //pela operacao anterior, tenho garantido que o tempoIO eh menor que o tempo de servico, mas pode ser que ele seja igual
            //ao tempo de entrada ou ao tempo de IO anterior, ai somo 1
            if(novoProcesso.temposIO[j] == novoProcesso.temposIO[j-1]) novoProcesso.temposIO[j]++;
            if(novoProcesso.temposIO[j] == novoProcesso.tempoServico-1) { novoProcesso.nIO = j; break; }

            int indexTipoIO = rand()%3;
            if(indexTipoIO == 0) novoProcesso.tiposIO[j] = listaIO[j];
            if(indexTipoIO == 1) novoProcesso.tiposIO[j] = listaIO[j];
            if(indexTipoIO == 2) novoProcesso.tiposIO[j] = listaIO[j];
            novoProcesso.nIO = j+1;
        }
        //Tempo executado
        novoProcesso.tempoExecutado = 0;
        novoProcesso.nIOExecutados = 0;
        //Adicionando a tabela
        tabela[i] = novoProcesso;
    }
}

void inicializarIO(IO* io, int duracao, char tipo, Fila* fila){
    io->duracaoIO = duracao;
    io->tipoIO = tipo;
    io->filaRetorno = fila;
}

void inicializarFila(Fila* fila){
    fila->posicaoConsumir=0;
    fila->posicaoInserir=0;
}

int tempoGlobal = 0;

int main(){
    Fila prioridadeAlta, prioridadeBaixa, filaIO;
    inicializarFila(&prioridadeAlta);
    inicializarFila(&prioridadeBaixa);
    inicializarFila(&filaIO);

    IO memoria, fita, impressora;
    inicializarIO(&memoria, 3, 'M', &prioridadeBaixa);
    inicializarIO(&fita, 5, 'F', &prioridadeAlta);
    inicializarIO(&impressora, 7, 'I', &prioridadeAlta);
    IO* listaIO[3] = {&memoria, &fita, &impressora};

    Processo tabelaProcessos[PROCESS_AMOUNT];
    criarTabela(tabelaProcessos, listaIO);
    exibirTabela(tabelaProcessos);

    int processoQueVaiChegar = 0;
    int idProcessoNaCPU = -1;
    int tempoNaCPU = 0;
    int processosFaltamTerminar = PROCESS_AMOUNT;

    while(1)
    {
        //Passo 1 - Entrada de processos nas filas de prioridade (exceto caso de preempção)
        //Chegada de processos novos
        if(tabelaProcessos[processoQueVaiChegar].tempoChegada == tempoGlobal){
            printf("O processo %02d chegou no instante %02d\n", tabelaProcessos[processoQueVaiChegar].pid, tempoGlobal);
            adicionarNaFila(&prioridadeAlta, processoQueVaiChegar);
            if(processoQueVaiChegar<PROCESS_AMOUNT-1){
                processoQueVaiChegar++;
            }
        }

        //Retorno de processos do IO
        for(int i=0;i<PROCESS_AMOUNT;i++){
            Processo* processoVisitado = &tabelaProcessos[i];
            if(processoVisitado->tempoRetornoIO == tempoGlobal){
                IO* IOExecutado = processoVisitado->tiposIO[processoVisitado->nIOExecutados];
                printf("O processo %02d retornou do IO %c no instante %02d e está voltando para a fila\n", i, IOExecutado->tipoIO, tempoGlobal);
                adicionarNaFila(IOExecutado->filaRetorno, i);
                processoVisitado->nIOExecutados++;
            }
        }

        //Passo 2 - Verifica se o Processo deve sair da CPU
        if(idProcessoNaCPU!=-1){
            Processo* processoNaCPU = &tabelaProcessos[idProcessoNaCPU];

            //Verifica se o processo terminou sua execução
            if(processoNaCPU->tempoExecutado == processoNaCPU->tempoServico){
                printf("O processo %02d terminou a execução no instante %02d\n", processoNaCPU->pid, tempoGlobal);
                idProcessoNaCPU = -1;
                tempoNaCPU = 0;
                processoNaCPU->tempoSaida = tempoGlobal;
                processosFaltamTerminar--;
            }

            //confere se o processo precisa chamar IO
            for(int i=0;i<processoNaCPU->nIO;i++){
                if(processoNaCPU->temposIO[i] == processoNaCPU->tempoExecutado){
                    printf("O processo %02d entrou na fila de IO no instante %02d\n", processoNaCPU->pid, tempoGlobal);
                    adicionarNaFila(&filaIO, idProcessoNaCPU);
                    idProcessoNaCPU = -1;
                    tempoNaCPU = 0;
                }
            }
            
            //verifica se o processo já executou durante sua fatia de tempo
            if(idProcessoNaCPU != -1){
                if(tempoNaCPU==TIME_SLICE){
                    printf("O processo %02d cedeu a CPU no instante %02d\n", processoNaCPU->pid, tempoGlobal);
                    adicionarNaFila(&prioridadeBaixa, idProcessoNaCPU);
                    idProcessoNaCPU = -1;
                    tempoNaCPU = 0;
                }


                else{
                    tempoNaCPU++;
                    processoNaCPU->tempoExecutado++;
                }
            }
        }

        //Passo 3 - Verifica se algum processo está esperando na fila de IO
        int idProcessoChamouIO = consumirDaFila(&filaIO);
        while (idProcessoChamouIO != -1)
        {
            Processo* processoChamouIO = &tabelaProcessos[idProcessoChamouIO];
            IO* IOSolicitado = processoChamouIO->tiposIO[processoChamouIO->nIOExecutados]; //o numero de IO executados tambem corresponde ao index na lista do IO que deve ser executado
            processoChamouIO->tempoRetornoIO = tempoGlobal + IOSolicitado->duracaoIO;
            printf("O processo %02d pediu IO no instante %02d e vai terminar no instante %02d\n", processoChamouIO->pid, tempoGlobal, processoChamouIO->tempoRetornoIO);
            idProcessoChamouIO = consumirDaFila(&filaIO);
        }
         

        //Passo 4 - Verifica se a CPU está pronta para receber algum processo
        if(idProcessoNaCPU==-1){
            int idProcessoChegando;
            //verifica se a fila de prioridade alta tem algum processo aguardando
            idProcessoChegando = consumirDaFila(&prioridadeAlta);
            if(idProcessoChegando!=-1){
                printf("O processo %02d assumiu controle da CPU no instante %02d\n", idProcessoChegando, tempoGlobal);
                idProcessoNaCPU = idProcessoChegando;
                tempoNaCPU++;
                tabelaProcessos[idProcessoChegando].tempoExecutado++;
            }
            //verifica se a fila de prioridade vaixa tem algum processo aguardando
            else{
                idProcessoChegando = consumirDaFila(&prioridadeBaixa);
                if(idProcessoChegando!=-1){
                    printf("O processo %02d assumiu controle da CPU no instante %02d\n", idProcessoChegando, tempoGlobal);
                    idProcessoNaCPU = idProcessoChegando;
                    tempoNaCPU++;
                    tabelaProcessos[idProcessoChegando].tempoExecutado++;
                }
            }
        }

        
        tempoGlobal++;
        if(processosFaltamTerminar==0) break;
    }
    
}


/*
PREMISSAS
1) os PIDs são sequenciais e os tempos de chegada são aleatórios, mas acompanham os PIDs,
   ou seja os processos de menor PID chegam primeiro que os de maior PID

2) caso o buffer da fila seja completamente preenchido, o processo no inicio da fila perde sua vez,
   essa abordagem foi escolhida, por ser mais facil de se implementar e porque, em um ambiente de simulação,
   esse problema é facilmente contornavel aumentando o tamanho do buffer ou reduzindo a quantidade de processos

3) a variavel idProcessoNaCPU informa o PID do processo que está usando a CPU no momento, quando a variavel vale -1
   isso quer dizer que não tem ninguem na CPU

4) nenhum processo pode pedir IO assim que inicia sua execução

5) se um processo pede IO no instante em que termina sua fatia de tempo, ele consegue pedir IO antes de ser retirado da CPU

6) o ambiente simulado possui recursos ilimitados, ou seja, assim que um processo entra na fila de IO ele vai ser atendido,
   independente se houver algum outro processo utilizando o mesmo IO
*/