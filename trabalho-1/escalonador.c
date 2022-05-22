#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<time.h>

#define QUEUE_LENGTH 100
#define PROCESS_AMOUNT 2
#define TIME_SLICE 2

typedef struct
{
    int duracaoIO;
    char tipoIO;
    char filaRetorno;
} IO;

typedef struct
{
    int pid;
    int tempoChegada;
    int tempoServico;
    int tempoSaida;
    int tempoExecutado;
    int temposIO[3];
    IO tiposIO[3];
} Processo;

typedef struct
{
    int buffer[QUEUE_LENGTH];
    int posicaoInserir;
    int posicaoConsumir;
} Fila;

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
    printf("PID       |tChegada  |tServiço  |temposIO  |tiposIO\n");
    for(int i = 0; i<PROCESS_AMOUNT; i++){
        printf("%02d         |%02d        |%02d        |          |          \n", tabela[i].pid, tabela[i].tempoChegada, tabela[i].tempoServico);
    }
}

void criarTabela(Processo tabela[]){
    //srand(110);

    for(int i = 0; i<PROCESS_AMOUNT; i++){
        Processo novoProcesso;
        //PID
        novoProcesso.pid = i;
        //Tempo de chegada
        if(i == 0) novoProcesso.tempoChegada = 0;
        else novoProcesso.tempoChegada = tabela[i-1].tempoChegada + (1 + rand()%5);
        //Tempo de serviço
        novoProcesso.tempoServico = 1+rand()%11;
        //Tempo executado
        novoProcesso.tempoExecutado = 0;
        //Adicionando a tabela
        tabela[i] = novoProcesso;
    }
}

void inicializarIO(IO* io, int duracao, char tipo, char filaId){
    io->duracaoIO = duracao;
    io->tipoIO = tipo;
    io->filaRetorno = filaId;
}

void inicializarFila(Fila* fila){
    fila->posicaoConsumir=0;
    fila->posicaoInserir=0;
}

int tempoGlobal = 0;

int main(){
    IO memoria, fita, impressora;
    inicializarIO(&memoria, 3, 'M', 'B');
    inicializarIO(&fita, 5, 'F', 'A');
    inicializarIO(&impressora, 7, 'I', 'A');

    Fila prioridadeAlta, prioridadeBaixa, filaIO;
    inicializarFila(&prioridadeAlta);
    inicializarFila(&prioridadeBaixa);
    inicializarFila(&filaIO);

    Processo tabelaProcessos[PROCESS_AMOUNT];
    criarTabela(tabelaProcessos);
    exibirTabela(tabelaProcessos);

    int processoQueVaiChegar = 0;
    int idProcessoNaCPU = -1;
    int tempoNaCPU = 0;
    int processosFaltamTerminar = PROCESS_AMOUNT;

    while(1)
    {
        //Passo 1 - Chegada de Processos
        //Chegada de processos novos
        if(tabelaProcessos[processoQueVaiChegar].tempoChegada == tempoGlobal){
            printf("O processo %02d chegou no instante %02d\n", tabelaProcessos[processoQueVaiChegar].pid, tempoGlobal);
            adicionarNaFila(&prioridadeAlta, processoQueVaiChegar);
            if(processoQueVaiChegar<PROCESS_AMOUNT-1){
                processoQueVaiChegar++;
            }
        }

        //Retorno de processos do IO

        //Processos que sofreram preempção

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

            //TODO conferir se o processo chamou IO
            
            //verifica se o processo já executou durante sua fatia de tempo
            else if(tempoNaCPU==TIME_SLICE){
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

        //Passo 3 - Verifica se a CPU está pronta para receber algum processo
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
*/