#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<time.h>

#define PROCESS_AMOUNT 20
#define PROCESS_PAGE_TABLE_SIZE 50
#define VIRTUAL_MEMORY_SIZE PROCESS_AMOUNT*PROCESS_PAGE_TABLE_SIZE
#define REAL_MEMORY_SIZE 64
#define TIME_DELTA_PAGE_CREATION 3
#define TIME_DELTA_PAGE_ALOCATION 3

int tempoGlobal = 0;

typedef struct
{
    int idProcessoDono;
    int tempoUltimaReferencia;
    int enderecoReal;
    int enderecoVirtual;
} Pagina;

typedef struct
{
    int pid;
    int tempoCriacao;
    int tempoChamadaPagina;
    Pagina* tabelaPaginas[PROCESS_PAGE_TABLE_SIZE];
} Processo;

typedef struct
{
    int espacosLivres;
    int indexProximaAlocacao;
    int tempoAlocacaoMaisAintiga;
    Pagina* espacoDedicado[REAL_MEMORY_SIZE];
} MemoriaReal;

void exibirTabelaDePaginas(Processo* processo){
    printf("===============================================================\n");
    printf("tabela de paginas do processo %d\n", processo->pid);

    printf("enredeço real | endereço virtual\n");
    for(int i=0; i<PROCESS_PAGE_TABLE_SIZE; i++){
        Pagina* paginaAtual = processo->tabelaPaginas[i];
        
        if(paginaAtual->enderecoReal < 0)
            printf("*             | %03d\n", paginaAtual->enderecoVirtual);
        
        else
            printf("%02d            | %03d\n", paginaAtual->enderecoReal, paginaAtual->enderecoVirtual);
    }
    printf("===============================================================\n");
}

void exibirMemoriaReal(MemoriaReal* memoria){
    printf("===============================================================\n");
    printf("Alocação de memória\n");

    printf("enredeço real | endereço virtual | instante ultima referência\n");
    for(int i=0; i<REAL_MEMORY_SIZE; i++){
        
        if(REAL_MEMORY_SIZE-i <= memoria->espacosLivres)
            printf("%02d            | *                | *\n", i);
        
        else
            printf("%02d            | %03d              | %03d\n", i, memoria->espacoDedicado[i]->enderecoVirtual, memoria->espacoDedicado[i]->tempoUltimaReferencia);
    }
    printf("===============================================================\n");
}

void inicializarMemoriaVirtual(Pagina memoria[]){
    for(int i=0; i<VIRTUAL_MEMORY_SIZE; i++){
        Pagina novaPagina;
        novaPagina.enderecoVirtual = i;
        novaPagina.idProcessoDono = -1;
        novaPagina.enderecoReal = -1;
        novaPagina.tempoUltimaReferencia = -1;
        memoria[i] = novaPagina;
    }
}

void inicializarMemoriaReal(MemoriaReal* memoria){
    memoria->espacosLivres = REAL_MEMORY_SIZE;
    memoria->indexProximaAlocacao = 0;
    memoria->tempoAlocacaoMaisAintiga = -1;
}

void inicializarListaProcessos(Processo listaProcessos[], Pagina memoriaVirtual[]){
    for(int i=0; i<PROCESS_AMOUNT; i++){
        Processo novoProcesso;
        novoProcesso.pid = i;
        novoProcesso.tempoCriacao = 3*i;
        novoProcesso.tempoChamadaPagina = 3*i;

        for(int j=0; j<PROCESS_PAGE_TABLE_SIZE; j++){
            novoProcesso.tabelaPaginas[j] = &memoriaVirtual[i*PROCESS_PAGE_TABLE_SIZE+j];
            memoriaVirtual[i*PROCESS_PAGE_TABLE_SIZE+j].idProcessoDono = novoProcesso.pid;
        }

        listaProcessos[i] = novoProcesso;
    }

}

void alocarPaginaAleatoria(Pagina* tabelaPaginas[], MemoriaReal* memoriaReal)
{

    //selecionar um numero aleatorio entre 0 e PROCESS_PAGE_TABLE_SIZE-1
    //pegar o processo no index aleatorio gerado
    int randomPagePosition = rand()%PROCESS_PAGE_TABLE_SIZE;
    Pagina* paginaAlocada = tabelaPaginas[randomPagePosition];
    printf("O Processo %02d está solicitando alocação da página de endereço virtual %03d no instante %03d\n",
        paginaAlocada->idProcessoDono, paginaAlocada->enderecoVirtual, tempoGlobal);


    //verifica se a pagina ja esta na memoria
    //caso esteja, atualiza seu tempo de ultima referencia
    if(paginaAlocada->enderecoReal > 0)
    {
        paginaAlocada->tempoUltimaReferencia = tempoGlobal;
        printf("A página de endereço virtual %03d já estava na memória no instante %03d\n",
        paginaAlocada->enderecoVirtual, tempoGlobal);
    }
    //caso nao esteja, percorre toda a memoria principal e encontra a pagina com tempo de referencia mais antigo
    else
    {
        if(memoriaReal->espacosLivres > 0)
        {
            memoriaReal->espacoDedicado[memoriaReal->indexProximaAlocacao] = paginaAlocada;
            paginaAlocada->enderecoReal = memoriaReal->indexProximaAlocacao;
            paginaAlocada->tempoUltimaReferencia = tempoGlobal;

            memoriaReal->espacosLivres--;
            if(memoriaReal->espacosLivres > 0) memoriaReal->indexProximaAlocacao++;
            printf("A página de endereço virtual %03d foi alocada no endereço real %02d no instante %03d\n",
                paginaAlocada->enderecoVirtual, paginaAlocada->enderecoReal, tempoGlobal);
        }
        else
        {
            int tempoMaisAntigo = tempoGlobal;
            int indexNaMemoriaTempoMaisAntigo = 0;
            for (int i = 0; i < REAL_MEMORY_SIZE; i++)
            {
                if(memoriaReal->espacoDedicado[i]->tempoUltimaReferencia <= tempoMaisAntigo)
                {
                    tempoMaisAntigo = memoriaReal->espacoDedicado[i]->tempoUltimaReferencia;
                    indexNaMemoriaTempoMaisAntigo = i;
                }
            }
            Pagina* paginaDesalocada = memoriaReal->espacoDedicado[indexNaMemoriaTempoMaisAntigo];

            memoriaReal->espacoDedicado[indexNaMemoriaTempoMaisAntigo] = paginaAlocada;
            paginaAlocada->enderecoReal = indexNaMemoriaTempoMaisAntigo;
            paginaAlocada->tempoUltimaReferencia = tempoGlobal;

            printf("A página de endereço virtual %03d foi alocada no endereço real %02d no instante %03d substituindo a pagina de endereço virtual %03d que havia sido alocada no instante %03d\n",
                paginaAlocada->enderecoVirtual, paginaAlocada->enderecoReal, tempoGlobal, paginaDesalocada->enderecoVirtual, paginaDesalocada->tempoUltimaReferencia);
            
            paginaDesalocada->enderecoReal = -1;
            paginaDesalocada->tempoUltimaReferencia = -1;
        }
        
    }

    //substituir a pagina antiga pela pagina nova
    //logar que a nova pagina substituiu a antiga
}

int main(){
    Pagina memoriaVirtual[VIRTUAL_MEMORY_SIZE];
    inicializarMemoriaVirtual(memoriaVirtual);

    Processo listaProcessos[PROCESS_AMOUNT];
    inicializarListaProcessos(listaProcessos, memoriaVirtual);

    MemoriaReal memoriaReal;
    inicializarMemoriaReal(&memoriaReal);

    srand(time(NULL)); //seed de aleatoriedade

    while (tempoGlobal<=300)
    {
        for(int i=0; i<PROCESS_AMOUNT; i++){
            Processo* processoVisitado = &listaProcessos[i];

            if(tempoGlobal == processoVisitado->tempoChamadaPagina){
                alocarPaginaAleatoria(processoVisitado->tabelaPaginas, &memoriaReal);
                exibirTabelaDePaginas(processoVisitado);
                exibirMemoriaReal(&memoriaReal);
                processoVisitado->tempoChamadaPagina += TIME_DELTA_PAGE_ALOCATION;
            }
        }
        sleep(1);
        tempoGlobal++;
        
    }
    

    return 0;
}