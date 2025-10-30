#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>


// Definição de estruturas de dados utilizadas ao longo do programa

typedef enum {BEGIN_L, END_L, MACRO_L, ENDMACRO_L, CORPO_MACRO_L, CHAMADA_MACRO_L, INDEFINIDA_L} tipo_linha;
typedef enum {ROTULO, MAIS, VIRGULA, BEGIN, SPACE, CONST, END, MACRO, ENDMACRO, PARAMETRO_MACRO, CHAMADA_MACRO, ADD, SUB, MULT, DIV, JMP, JMPN, JMPP, JMPZ, COPY, LOAD, STORE, INPUT, OUTPUT, STOP, NUMERO, NOME} tipo_token;

// ROTULO é o que resulta de XXXXXX: -> XXXXXX . A única análise é dos dois pontos finais. Não se considera a validade do rótulo.

typedef struct token {
    char *texto_token;
    tipo_token tipo_token;
    struct token *p_token_posterior;
} token;

typedef struct linha {
    long num_linha;
    char *texto_linha;
    tipo_linha tipo_linha;
    token *p_primeiro_token;
    struct linha *p_linha_anterior;
    struct linha *p_linha_posterior;
    char *rotulo_macro;
} linha;

typedef struct cabecalho_texto {
    linha *p_primeira_linha;
    linha *p_ultima_linha;
    long total_linha;
} cabecalho_texto;

typedef struct posicao_memoria {
    long endereco; // Funciona como identificador
    long valor;
    long deslocamento;
    struct posicao_memoria *proxima_pendencia;
    struct posicao_memoria *proxima_posicao;
} posicao_memoria;

typedef struct simbolo {
    char *rotulo;
    long endereco;
    bool esta_definido;
    posicao_memoria *p_primeira_pendencia;
    struct simbolo *p_proximo_simbolo;
} simbolo;



long consulta_rotulo_lista_simbolos(simbolo **p_p_primeiro_simbolo, char *rotulo, posicao_memoria *p_referencia) {
    simbolo *p_simbolo_atual = *p_p_primeiro_simbolo;
    simbolo *p_ultimo_simbolo = *p_p_primeiro_simbolo;
    posicao_memoria *p_pendencia_atual = NULL;
    posicao_memoria *p_ultima_pendencia = NULL; 
    bool simbolo_esta_na_lista = false;

    // Checa se rótulo já está na lista de símbolos
    while (p_simbolo_atual != NULL) {
        if (strcmp(rotulo, p_simbolo_atual->rotulo) == 0) {  // Símbolo está na lista
            if (p_simbolo_atual->esta_definido == true) {    // Símbolo já teve endereço definido
                return p_simbolo_atual->endereco;
            }
            else { // Símbolo ainda não teve endereço definido
                p_pendencia_atual =  p_simbolo_atual->p_primeira_pendencia;
                if (p_pendencia_atual == NULL) p_simbolo_atual->p_primeira_pendencia = p_referencia; // Não há pendências
                else { // Há pendências
                    while (p_pendencia_atual != NULL) { // Encontra última pendência
                        p_ultima_pendencia = p_pendencia_atual;
                        p_pendencia_atual = p_pendencia_atual->proxima_pendencia;
                    }
                    p_ultima_pendencia->proxima_pendencia = p_referencia; // Insere no final da lista de pendências
                }
                return -1;
            }
        }
        p_ultimo_simbolo = p_simbolo_atual; // Identifica último símbolo da lista
        p_simbolo_atual = p_simbolo_atual->p_proximo_simbolo;
    }
    
    // Insere novo símbolo na lista (heap)
    simbolo *p_novo_simbolo = malloc(sizeof(simbolo));
    if (p_novo_simbolo == NULL) {
        perror("Erro na alocação de memória para o símbolo");
    exit(1);
    }

    p_novo_simbolo->endereco = -1;
    p_novo_simbolo->esta_definido = false;
    p_novo_simbolo->p_primeira_pendencia = p_referencia;
    p_novo_simbolo->rotulo = strdup(rotulo);
    p_novo_simbolo->p_proximo_simbolo = NULL;
    if (*p_p_primeiro_simbolo != NULL) p_ultimo_simbolo->p_proximo_simbolo = p_novo_simbolo; // Se não é primeiro símbolo, faz conexão com ele
    else *p_p_primeiro_simbolo = p_novo_simbolo; // Se é primeiro símbolo, insere referência no cabeçalho
    
    return -1;
}

void insere_rotulo_lista_simbolos(simbolo **p_p_primeiro_simbolo, char *rotulo, long endereco) {
    simbolo *p_simbolo_atual = *p_p_primeiro_simbolo;
    simbolo *p_ultimo_simbolo = *p_p_primeiro_simbolo;

    // Checa se rótulo já está na lista de símbolos
    while (p_simbolo_atual != NULL) {
        if (strcmp(rotulo, p_simbolo_atual->rotulo) == 0) {  // Símbolo está na lista
            if (p_simbolo_atual->esta_definido == true) {    // Símbolo já teve endereço definido
                printf("Erro de redefinição de símbolo!");
                exit(1);
            }
            else { // Símbolo ainda não teve endereço definido
                p_simbolo_atual->esta_definido = true;       
                p_simbolo_atual->endereco = endereco;
                return;
            }
        }
        p_ultimo_simbolo = p_simbolo_atual; // Identifica último símbolo da lista
        p_simbolo_atual = p_simbolo_atual->p_proximo_simbolo;
    }

    // Insere novo símbolo na lista (heap)
    simbolo *p_novo_simbolo = malloc(sizeof(simbolo));
    if (p_novo_simbolo == NULL) {
        perror("Erro na alocação de memória para o símbolo");
    exit(1);
    }

    p_novo_simbolo->endereco = endereco;
    p_novo_simbolo->esta_definido = true;
    p_novo_simbolo->p_primeira_pendencia = NULL;
    p_novo_simbolo->rotulo = strdup(rotulo);
    p_novo_simbolo->p_proximo_simbolo = NULL;
    if (*p_p_primeiro_simbolo != NULL) p_ultimo_simbolo->p_proximo_simbolo = p_novo_simbolo; // Se não é primeiro símbolo, faz conexão com o último
    else *p_p_primeiro_simbolo = p_novo_simbolo; // Se é primeiro símbolo, insere referência no cabeçalho

    return;
}

void insere_posicao_memoria(posicao_memoria **p_p_ultima_posicao, posicao_memoria **p_p_primeira_posicao, long endereco, long valor, long deslocamento) {
    posicao_memoria *p_nova_posicao = malloc(sizeof(posicao_memoria));
    if (p_nova_posicao == NULL) {
        perror("Erro na alocação de memória para a posição de memória");
    exit(1);
    }
    
    p_nova_posicao->endereco = endereco;
    p_nova_posicao->valor = valor;
    p_nova_posicao->deslocamento = deslocamento; // Offset em que caso de soma a nome
    p_nova_posicao->proxima_pendencia = NULL;
    p_nova_posicao->proxima_posicao = NULL;

    if (*p_p_primeira_posicao != NULL) (*p_p_ultima_posicao)->proxima_posicao = p_nova_posicao; // Se não é primeira posição, faz conexão com a última
    else *p_p_primeira_posicao = p_nova_posicao; // Se é primeira posicao, insere referência no cabeçalho
    *p_p_ultima_posicao = p_nova_posicao;
}

void gera_codigo_com_pendencias(cabecalho_texto *p_cabecalho_texto, posicao_memoria **p_p_primeira_posicao, simbolo **p_p_primeiro_simbolo) {
    token *p_token_atual = NULL;
    linha *p_linha_atual = p_cabecalho_texto->p_primeira_linha;
    posicao_memoria *p_ultima_posicao = *p_p_primeira_posicao;
    long endereco_atual = 0;

    while (p_linha_atual != NULL) {
        p_token_atual = p_linha_atual->p_primeiro_token;

        while (p_token_atual != NULL) {
            switch (p_token_atual->tipo_token) {
                case ROTULO: 
                    insere_rotulo_lista_simbolos(p_p_primeiro_simbolo, p_token_atual->texto_token, endereco_atual); 
                    break;
                case ADD:
                    insere_posicao_memoria(&p_ultima_posicao, p_p_primeira_posicao, endereco_atual, 1, 0);
                    endereco_atual++;
                    break;
                case SUB:
                    insere_posicao_memoria(&p_ultima_posicao, p_p_primeira_posicao, endereco_atual, 2, 0);
                    endereco_atual++;
                    break; 
                case MULT:
                    insere_posicao_memoria(&p_ultima_posicao, p_p_primeira_posicao, endereco_atual, 3, 0);
                    endereco_atual++;
                    break; 
                case DIV:
                    insere_posicao_memoria(&p_ultima_posicao, p_p_primeira_posicao, endereco_atual, 4, 0);
                    endereco_atual++;
                    break; 
                case JMP: 
                    insere_posicao_memoria(&p_ultima_posicao, p_p_primeira_posicao, endereco_atual, 5, 0);
                    endereco_atual++;
                    break; 
                case JMPN:
                    insere_posicao_memoria(&p_ultima_posicao, p_p_primeira_posicao, endereco_atual, 6, 0);
                    endereco_atual++;
                    break; 
                case JMPP:
                    insere_posicao_memoria(&p_ultima_posicao, p_p_primeira_posicao, endereco_atual, 7, 0);
                    endereco_atual++;
                    break; 
                case JMPZ:
                    insere_posicao_memoria(&p_ultima_posicao, p_p_primeira_posicao, endereco_atual, 8, 0);
                    endereco_atual++;
                    break; 
                case LOAD:
                    insere_posicao_memoria(&p_ultima_posicao, p_p_primeira_posicao, endereco_atual, 10, 0);
                    endereco_atual++;
                    break; 
                case STORE:
                    insere_posicao_memoria(&p_ultima_posicao, p_p_primeira_posicao, endereco_atual, 11, 0);
                    endereco_atual++;
                    break; 
                case INPUT:
                    insere_posicao_memoria(&p_ultima_posicao, p_p_primeira_posicao, endereco_atual, 12, 0);
                    endereco_atual++;
                    break; 
                case OUTPUT:
                    insere_posicao_memoria(&p_ultima_posicao, p_p_primeira_posicao, endereco_atual, 13, 0);
                    endereco_atual++;
                    break;
                case STOP:
                    insere_posicao_memoria(&p_ultima_posicao, p_p_primeira_posicao, endereco_atual, 14, 0);
                    endereco_atual++;
                    break;
                case COPY:
                    insere_posicao_memoria(&p_ultima_posicao, p_p_primeira_posicao, endereco_atual, 9, 0);
                    endereco_atual++;
                    break;
                case CONST:
                    if (p_token_atual->p_token_posterior && p_token_atual->p_token_posterior->tipo_token == NUMERO) {
                        insere_posicao_memoria(&p_ultima_posicao, p_p_primeira_posicao, endereco_atual, atoi(p_token_atual->p_token_posterior->texto_token), 0);
                        endereco_atual++;
                    }
                    break;
                case SPACE:
                    insere_posicao_memoria(&p_ultima_posicao, p_p_primeira_posicao, endereco_atual, 0, 0);
                    endereco_atual++;
                    if (p_token_atual->p_token_posterior && p_token_atual->p_token_posterior->tipo_token == NUMERO) {
                        int num = atoi(p_token_atual->p_token_posterior->texto_token);
                        for ( int i = 1; i < num; i++) {
                            insere_posicao_memoria(&p_ultima_posicao, p_p_primeira_posicao, endereco_atual, 0, 0);
                            endereco_atual++;
                        }
                    }    
                    break;
                case NOME:
                    long deslocamento = 0;
                    if (p_token_atual->p_token_posterior && p_token_atual->p_token_posterior->p_token_posterior && p_token_atual->p_token_posterior->tipo_token == MAIS) {
                        deslocamento = atoi(p_token_atual->p_token_posterior->p_token_posterior->texto_token);
                    }
                    insere_posicao_memoria(&p_ultima_posicao, p_p_primeira_posicao, endereco_atual, -1, deslocamento);
                    long endereco_rotulo = consulta_rotulo_lista_simbolos(p_p_primeiro_simbolo, p_token_atual->texto_token, p_ultima_posicao);
                    p_ultima_posicao->valor = endereco_rotulo;
                    endereco_atual++;
                    break;

            }
            p_token_atual = p_token_atual->p_token_posterior;
        }
        p_linha_atual = p_linha_atual->p_linha_posterior;
    }
}

void resolve_pendencias(simbolo *p_primeiro_simbolo) {
    simbolo *p_simbolo_atual = p_primeiro_simbolo;
    posicao_memoria *p_pendencia_atual = NULL;

    while (p_simbolo_atual != NULL) {
        long endereco_rotulo = p_simbolo_atual->endereco;
        p_pendencia_atual = p_simbolo_atual->p_primeira_pendencia;
        while (p_pendencia_atual != NULL) {
            p_pendencia_atual->valor = endereco_rotulo + p_pendencia_atual->deslocamento;
            p_pendencia_atual = p_pendencia_atual->proxima_pendencia;
        }
        p_simbolo_atual = p_simbolo_atual->p_proximo_simbolo;
    }
}

void gera_arquivo_pre(cabecalho_texto *p_cabecalho_texto, char* nome_arquivo_sem_extensao) {
    char nome_arquivo_com_extensao[strlen(nome_arquivo_sem_extensao) + 5];
    sprintf(nome_arquivo_com_extensao, "%s.pre", nome_arquivo_sem_extensao);

    FILE *arquivo = fopen(nome_arquivo_com_extensao, "w"); 
    if (arquivo == NULL) {
        perror("Erro ao criar o arquivo .pre");
        exit(1);
    }

    linha *p_linha_atual = p_cabecalho_texto->p_primeira_linha;

    while (p_linha_atual != NULL) {
        fprintf(arquivo, "%s\n", p_linha_atual->texto_linha);
        p_linha_atual = p_linha_atual->p_linha_posterior;
    }
    fclose(arquivo);
}

void gera_arquivo_o1(posicao_memoria *p_primeira_posicao, simbolo *p_primeiro_simbolo, char* nome_arquivo_sem_extensao) {
    char nome_arquivo_com_extensao[strlen(nome_arquivo_sem_extensao) + 4];
    sprintf(nome_arquivo_com_extensao, "%s.o1", nome_arquivo_sem_extensao);

    FILE *arquivo = fopen(nome_arquivo_com_extensao, "w"); 
    if (arquivo == NULL) {
        perror("Erro ao criar o arquivo .o1");
        exit(1);
    }

    posicao_memoria *p_posicao_atual = p_primeira_posicao;

    while (p_posicao_atual != NULL) {
        fprintf(arquivo, "%ld ", p_posicao_atual->valor);
        p_posicao_atual = p_posicao_atual->proxima_posicao; 
    }
                                          
    fprintf(arquivo, "\n\n# Pendências #");

    simbolo *p_simbolo_atual = p_primeiro_simbolo;

    while (p_simbolo_atual != NULL) {
        if (p_simbolo_atual->p_primeira_pendencia == NULL) {
            fprintf(arquivo, "\n%s -> não há", p_simbolo_atual->rotulo);
        }
        else {
            posicao_memoria *p_pendencia_atual = p_simbolo_atual->p_primeira_pendencia;
            fprintf(arquivo, "\n%s ->", p_simbolo_atual->rotulo);
            while (p_pendencia_atual != NULL) {
                fprintf(arquivo, " %ld", p_pendencia_atual->endereco);
                p_pendencia_atual = p_pendencia_atual->proxima_pendencia;
            }
        }
        p_simbolo_atual = p_simbolo_atual->p_proximo_simbolo;
    }
    fclose(arquivo);
}

void gera_arquivo_o2(posicao_memoria *p_primeira_posicao, char* nome_arquivo_sem_extensao) {
    char nome_arquivo_com_extensao[strlen(nome_arquivo_sem_extensao) + 4];
    sprintf(nome_arquivo_com_extensao, "%s.o2", nome_arquivo_sem_extensao);

    FILE *arquivo = fopen(nome_arquivo_com_extensao, "w"); 
    if (arquivo == NULL) {
        perror("Erro ao criar o arquivo .o2");
        exit(1);
    }

    posicao_memoria *p_posicao_atual = p_primeira_posicao;

    while (p_posicao_atual != NULL) {
        fprintf(arquivo, "%ld ", p_posicao_atual->valor);
        p_posicao_atual = p_posicao_atual->proxima_posicao; 
    }
    fclose(arquivo);
}



// CABEÇALHO DO TEXTO

// Protótipos de funções utilizadas ao longo do programa

long obtem_texto_arquivo_fonte(int argc, char *argv[], char **p_p_texto_fonte);
long preformata_texto(char **p_p_texto_fonte, long tamanho_arquivo);
cabecalho_texto *estrutura_texto(char **p_p_texto_fonte);
struct linha *cria_linha(char *texto_linha, linha *p_linha_anterior, bool *e_corpo_macro);
struct token *cria_token(char *texto_token);
bool e_numero(const char *str);
void identifica_e_expande_macro(cabecalho_texto *p_cabecalho_texto, linha *p_linha_inicio_macro);
// void exclui_linha_da_lista(cabecalho_texto *p_cabecalho_texto, linha *p_linha_atual);
linha *expande_chamada_macro(linha *p_linha_anterior_chamada, linha *p_linha_posterior_chamada, linha *p_linha_inicio_corpo_macro, char *texto_parametro_1, char *texto_parametro_2, char *texto_argumento_1, char *texto_argumento_2, int num_parametros);

int main(int argc, char *argv[]) {

    char *texto_fonte = NULL; // declara ponteiro para região da heap a ser criada região para armazenar texto que será obtido do arquivo-fonte
    long tamanho_arquivo = obtem_texto_arquivo_fonte(argc, argv, &texto_fonte);

    if (tamanho_arquivo == -1)
        return -1;

    tamanho_arquivo = preformata_texto(&texto_fonte, tamanho_arquivo);

    if (tamanho_arquivo == -1)
        return -1;
    
    //#####################RETIRAR



    printf("%s\n", texto_fonte);
    //#####################RETIRAR

    // Estrutura texto
    cabecalho_texto *p_cabecalho_texto = estrutura_texto(&texto_fonte);

    // Expande macros
    bool expandiu_macro = false;

    linha *p_linha_atual = p_cabecalho_texto->p_primeira_linha;

    while (p_linha_atual != NULL) {
        if (p_linha_atual->tipo_linha == MACRO_L) {
            identifica_e_expande_macro(p_cabecalho_texto, p_linha_atual);
            expandiu_macro = true;
        }
        if (expandiu_macro) {
            p_linha_atual = p_cabecalho_texto->p_primeira_linha;
            expandiu_macro = false;
        }
        else p_linha_atual = p_linha_atual->p_linha_posterior;
    }

    int tamanho_nome_arquivo = strlen(argv[1]);
    char *nome_arquivo_sem_extensao = strdup(argv[1]);
    nome_arquivo_sem_extensao[tamanho_nome_arquivo - 4] = '\0';

    gera_arquivo_pre(p_cabecalho_texto, nome_arquivo_sem_extensao);

    posicao_memoria *p_primeira_posicao = NULL;
    simbolo *p_primeiro_simbolo = NULL;

    gera_codigo_com_pendencias(p_cabecalho_texto, &p_primeira_posicao, &p_primeiro_simbolo);

    gera_arquivo_o1(p_primeira_posicao, p_primeiro_simbolo, nome_arquivo_sem_extensao);

    resolve_pendencias(p_primeiro_simbolo);

    gera_arquivo_o2(p_primeira_posicao, nome_arquivo_sem_extensao);

    // LIBERAR LINHAS, CABEÇALHOS E TOKENS AO FINAL, percorrendo estruturas de dados


    free(texto_fonte);

    return 0;
}


long obtem_texto_arquivo_fonte(int argc, char *argv[], char **p_p_texto_fonte) {
 /*   // RESTAURAR ##### 
    
    // Trata erro de carga do programa
    if ((argc > 2) || argc == 1) {
        printf("# Erro na carga do programa!\nPara carregar o programa, use o terminal.\nIndique como único argumento o nome (com extensão e sem caminho) do arquivo ASM que deseja montar.\nO arquivo deve estar localizado no mesmo diretório do montador.");
        return -1;
    }

    
    // Abre arquivo ASM
    FILE *arquivo_fonte = fopen(argv[1], "r");
    
 */   // RESTAURAR #####

// RETIRAR #####
FILE *arquivo_fonte = fopen(".\\casos_teste\\entradas\\correto_desformatado.asm", "r");
// RETIRAR #########

    // Trata erro de abertura do arquivo ASM
    if (!arquivo_fonte) {
        perror("# Erro na abertura do arquivo!\nVerifique se o arquivo ASM informado realmente existe e se seu nome está correto!");
        return -1;
    }

    // Identifica tamanho do arquivo ASM e trata erros correspondentes
    if (fseek(arquivo_fonte, 0, SEEK_END)) {
        perror("# Erro na leitura do arquivo!");
        return -1;
    }
    long tamanho_arquivo = ftell(arquivo_fonte);
    if (tamanho_arquivo == -1L) {
        perror("# Erro na leitura do arquivo!");
        return -1;
    }
    if (fseek(arquivo_fonte, 0, SEEK_SET)) {
        perror("# Erro na leitura do arquivo!");
        return -1;
    }

    // Aloca memória para o texto-fonte e guarda referência no ponteiro texto_fonte
    *p_p_texto_fonte = malloc(tamanho_arquivo + 1); // Adiciona 1 para inserir '\0' após leitura
    
    if (!*p_p_texto_fonte) {
        printf("# Erro na alocação do texto em memória!");
        return -1;
    }

    // Copia conteúdo do arquivo para memório alocada e acrescenta '\0' ao final
    size_t elementos_lidos = fread(*p_p_texto_fonte, 1, tamanho_arquivo, arquivo_fonte);
    if (elementos_lidos != tamanho_arquivo) {
        printf(" Erro na leitura do arquivo!");
        return -1;
    }
    (*p_p_texto_fonte)[tamanho_arquivo] = '\0';
    fclose(arquivo_fonte);

    return tamanho_arquivo;
}

long preformata_texto(char **p_p_texto_fonte, long tamanho_arquivo) {

    // Aloca memória para buffer que receberá texto preformatado e declarada ponteiro correspondente
    char *p_texto_preformatado = NULL;
    p_texto_preformatado = malloc(3*(tamanho_arquivo + 1)); // Aloca o triplo por causa da formatação do "+", que pode, em tese, triplicar o tamanho do texto

    // Preformata o texto
    char *p_texto_fonte = *p_p_texto_fonte;

    long nova_posicao = 0;
    char caractere_anterior = '\0';
    bool e_comentario = false;

    for(long i = 0; p_texto_fonte[i] != '\0'; i++) {
        char caractere_atual = p_texto_fonte[i];
        char caractere_futuro = p_texto_fonte[i+1];
        if (caractere_atual == '\t') caractere_atual = ' '; // Substitui tab por espaço
        if (caractere_atual == ';') { // Checa início de comentário para começar a ignorar
            e_comentario = true; 
            continue;
        }
        if (e_comentario) { // Checa fim de comentário para parar de ignorar
            if (caractere_atual != '\n') continue; 
            else e_comentario = false;
        }
        if ((nova_posicao == 0) && (isspace(caractere_atual))) continue;
        if (caractere_atual == '\r') continue; // Ignora carriage return
        if ((caractere_atual == '\n') && (caractere_anterior == '\n')) continue; // Ignora sequência de novas linhas
        if ((caractere_atual == ' ') && (caractere_anterior == ' ')) continue; // Ignora sequência de espaços
        if ((caractere_atual == ' ') && (caractere_anterior == '\n')) continue; // Ignora espaços de início de linha
        if ((caractere_atual == '\n') && (nova_posicao > 1) && (p_texto_preformatado[nova_posicao - 1] == ' ') && (p_texto_preformatado[nova_posicao - 2] == ':')) continue; // Trata caso ": \n"
        if ((caractere_atual == ' ') && ((caractere_futuro == '\n') || (caractere_futuro == '\r'))) continue; // Ignora espaços de final de linha
        if ((caractere_atual == '\n') && (caractere_anterior == ':')) caractere_atual = ' '; // Substitui nova linha após rótulo por espaço

        // Formata espaços antes e após símbolos "+" e ","
        if ((caractere_atual == '+') && (caractere_futuro != ' ') && (caractere_anterior != ' ')) {   // Caso x+y -> x + y
            p_texto_preformatado[nova_posicao] = ' ';
            p_texto_preformatado[nova_posicao + 1] = '+';
            p_texto_preformatado[nova_posicao + 2] = ' ';
            nova_posicao += 3;
            caractere_anterior = ' ';
        }
        else if ((caractere_atual == '+') && (caractere_futuro != ' ') && (caractere_anterior == ' ')) {   // Caso x +y -> x + y
            p_texto_preformatado[nova_posicao] = '+';
            p_texto_preformatado[nova_posicao + 1] = ' ';
            nova_posicao += 2;
            caractere_anterior = ' ';
        }
        else if ((caractere_atual == '+') && (caractere_futuro == ' ') && (caractere_anterior != ' ')) {   // Caso x+ y -> x + y
            p_texto_preformatado[nova_posicao] = ' ';
            p_texto_preformatado[nova_posicao + 1] = '+';
            nova_posicao += 2;
            caractere_anterior = '+';
        }
        else if ((caractere_atual == ',') && (caractere_futuro != ' ')) { // Formata espaços após o símbolo "   "
                p_texto_preformatado[nova_posicao] = ',';
                p_texto_preformatado[nova_posicao + 1] = ' ';
                nova_posicao += 2;
                caractere_anterior = ' ';
        }
        // Situação padrão
        else {
            p_texto_preformatado[nova_posicao] = toupper(caractere_atual); // Armazena caractere no buffer, transformando para letra maiúscula, se necessário
            nova_posicao++;
            caractere_anterior = caractere_atual;
        }
    }
    
    p_texto_preformatado[nova_posicao] = '\0'; // Adiciona caractere de terminação de string

    // Realoca memória para o texto-fonte (agora preformatado) 
    long tamanho_texto_preformatado = (long)(strlen(p_texto_preformatado));
    void *p_temporario = realloc(*p_p_texto_fonte, tamanho_texto_preformatado + 1);
    if (!p_temporario) {
        perror("# Erro na realocação de memória do texto preformatado!");
        free(p_texto_preformatado);
        return -1;
    }
    *p_p_texto_fonte = p_temporario;

    // Copia texto do buffer para nova região de memória
    strcpy(p_texto_fonte, p_texto_preformatado);

    free(p_texto_preformatado);
    return tamanho_texto_preformatado;
}

bool e_numero(const char *str) { // Verifica se string é composta apenas por caracteres numéricos
    if (*str == '\0') {
        return false; // string vazia não é numérica
    }
    while (*str) {
        if (!isdigit((unsigned char)*str)) {
            return false;
        }
        str++;
    }
    return true;
}

struct token *cria_token(char *texto_token) {

    token *p_token_atual = malloc(sizeof(token)); // Aloca memória para token na heap
    
    if (p_token_atual == NULL) {
        perror("Erro na alocação de memória para o token");
        exit(1);
    }

    int pos_ultimo_caractere = strlen(texto_token) - 1;

    p_token_atual->p_token_posterior = NULL;

    if (texto_token[pos_ultimo_caractere] == ':') { // Identifica rótulos XXXX:
        texto_token[pos_ultimo_caractere] = '\0';
        p_token_atual->tipo_token = ROTULO;

    }
    else if (texto_token[0] == '&') { // Identifica parâmetros de macros &XXXX
        strcpy(texto_token, texto_token + 1);
        p_token_atual->tipo_token = PARAMETRO_MACRO;
    }
    // Identifica demais tipos de token
    else if (strcmp(texto_token, "+") == 0) p_token_atual->tipo_token = MAIS;
    else if (strcmp(texto_token, ",") == 0) p_token_atual->tipo_token = VIRGULA;
    else if (strcmp(texto_token, "BEGIN") == 0) p_token_atual->tipo_token = BEGIN;
    else if (strcmp(texto_token, "END") == 0) p_token_atual->tipo_token = END;
    else if (strcmp(texto_token, "SPACE") == 0) p_token_atual->tipo_token = SPACE;
    else if (strcmp(texto_token, "CONST") == 0) p_token_atual->tipo_token = CONST;
    else if (strcmp(texto_token, "MACRO") == 0) p_token_atual->tipo_token = MACRO;
    else if (strcmp(texto_token, "ENDMACRO") == 0) p_token_atual->tipo_token = ENDMACRO;
    else if (strcmp(texto_token, "ADD") == 0) p_token_atual->tipo_token = ADD;
    else if (strcmp(texto_token, "SUB") == 0) p_token_atual->tipo_token = SUB;
    else if (strcmp(texto_token, "MULT") == 0) p_token_atual->tipo_token = MULT;
    else if (strcmp(texto_token, "DIV") == 0) p_token_atual->tipo_token = DIV;
    else if (strcmp(texto_token, "JMP") == 0) p_token_atual->tipo_token = JMP;
    else if (strcmp(texto_token, "JMPN") == 0) p_token_atual->tipo_token = JMPN;
    else if (strcmp(texto_token, "JMPP") == 0) p_token_atual->tipo_token = JMPP;
    else if (strcmp(texto_token, "JMPZ") == 0) p_token_atual->tipo_token = JMPZ;
    else if (strcmp(texto_token, "COPY") == 0) p_token_atual->tipo_token = COPY;
    else if (strcmp(texto_token, "LOAD") == 0) p_token_atual->tipo_token = LOAD;
    else if (strcmp(texto_token, "STORE") == 0) p_token_atual->tipo_token = STORE;
    else if (strcmp(texto_token, "INPUT") == 0) p_token_atual->tipo_token = INPUT;
    else if (strcmp(texto_token, "OUTPUT") == 0) p_token_atual->tipo_token = OUTPUT;
    else if (strcmp(texto_token, "STOP") == 0) p_token_atual->tipo_token = STOP;
    else if (e_numero(texto_token)) p_token_atual->tipo_token = NUMERO;
    else p_token_atual->tipo_token = NOME;

    p_token_atual->texto_token = texto_token;

    // RETIRAR ###########################################################
   // printf("%s : %i\n",p_token_atual->texto_token , p_token_atual->tipo_token);
    // RETIRAR ########################################################### 
   
    return p_token_atual;
}

struct linha *cria_linha(char *texto_linha, linha *p_linha_anterior, bool *e_corpo_macro) {
    
    bool e_inicio_macro = false;
    bool e_fim_macro = false;
    bool e_inicio_modulo = false;
    bool e_fim_modulo = false;

    linha *p_linha_atual = malloc(sizeof(linha)); // Aloca memória para linha na heap

    if (p_linha_atual == NULL) {
        perror("Erro na alocação de memória para a linha");
        exit(1);
    }

    char copia_linha[2 * (strlen(texto_linha) + 1)]; // Declara buffer na pilha, com memória dobrada para tratamento posterior de   s
    
    int tamanho_linha = strlen(texto_linha) + 1;

    int pos_origem = 0;
    int pos_destino = 0;

    while (pos_origem < tamanho_linha) { // copia texto da linha para buffer dando espaço antes de  s
        if (texto_linha[pos_origem] == ',') {
            copia_linha[pos_destino] = ' ';
            pos_destino++;
        }
        copia_linha[pos_destino] = texto_linha[pos_origem];
        pos_destino++;
        pos_origem++;
    }

    char *texto_token_buffer = strtok(copia_linha, " "); // Declara buffer para texto do token

    token *p_token_atual = NULL; // Declara ponteiro para receber endereço do token após criação
    token *p_token_anterior = NULL; // Declara ponteiro para token anterior na lista
    p_linha_atual->p_primeiro_token = NULL;
    p_linha_atual->p_linha_anterior = NULL;

    while (texto_token_buffer != NULL) {
        char *texto_token_heap = malloc(strlen(texto_token_buffer) + 1); // Aloca memória para o texto do token na heap
    
        if (texto_token_heap == NULL) {
            perror("Erro na alocação de memória para o texto do token");
            exit(1);
        }

        strcpy(texto_token_heap, texto_token_buffer);

        p_token_atual = cria_token(texto_token_heap);

        if (p_linha_atual->p_primeiro_token == NULL) p_linha_atual->p_primeiro_token = p_token_atual; // Indica início de lista
        else { // Atualiza referência em elemento antecedente da lista
            p_token_anterior->p_token_posterior = p_token_atual;
        }

        if (p_token_atual->tipo_token == MACRO) e_inicio_macro  = true;
        if (p_token_atual->tipo_token == ENDMACRO) e_fim_macro = true;
        if (p_token_atual->tipo_token == BEGIN) e_inicio_modulo = true;
        if (p_token_atual->tipo_token == END) e_fim_modulo = true;

        p_token_anterior = p_token_atual;

        texto_token_buffer = strtok(NULL, " ");
    }

    p_linha_atual->num_linha = 0; // só enumerar após expansão de macros
    p_linha_atual->texto_linha = texto_linha; // texto sem espaço antes da  
    p_linha_atual->p_linha_anterior = p_linha_anterior; // recebido como argumento da função
    p_linha_atual->p_linha_posterior = NULL;
    
    if (e_inicio_macro) p_linha_atual->tipo_linha = MACRO_L;
    else if (e_fim_macro) p_linha_atual->tipo_linha = ENDMACRO_L;
    else if (e_inicio_modulo) p_linha_atual->tipo_linha = BEGIN_L;
    else if (e_fim_modulo) p_linha_atual->tipo_linha = END_L;
    else if (*e_corpo_macro) p_linha_atual->tipo_linha = CORPO_MACRO_L;
    else p_linha_atual->tipo_linha = INDEFINIDA_L;

    // Armazena rótulo da macro nas linhas
    if (p_linha_atual->tipo_linha == MACRO_L) { // Primeira linha da macro pega rótulo do primeiro token
        char *texto_primeiro_token = malloc(strlen((p_linha_atual->p_primeiro_token)->texto_token) + 1);
        strcpy(texto_primeiro_token, (p_linha_atual->p_primeiro_token)->texto_token);
        p_linha_atual->rotulo_macro = texto_primeiro_token;
    }
    else if ((p_linha_atual->tipo_linha == CORPO_MACRO_L) || (p_linha_atual->tipo_linha == ENDMACRO_L)) { // Demais linhas da macro copiam rótulo das linhas anteriores
        p_linha_atual->rotulo_macro = p_linha_anterior->rotulo_macro;
    }
    else p_linha_atual->rotulo_macro = NULL;
 
    return p_linha_atual;

}

cabecalho_texto *estrutura_texto(char **p_p_texto_fonte) {

    cabecalho_texto *p_cabecalho_texto = malloc(sizeof(cabecalho_texto)); // Aloca memória para cabeçalho na heap

    if (p_cabecalho_texto == NULL) {
        perror("Erro na alocação de memória para o cabeçalho");
        exit(1);
    }

    char *p_reentrancia; // Salva ponteiro de linha para reentrância e, assim, evitar que strtok de tokens quebre strtok de linhas
    char *texto_linha_buffer = strtok_r(*p_p_texto_fonte, "\n", &p_reentrancia); // Declara buffer para texto do linha

    if (texto_linha_buffer == NULL) return NULL;  // Trata caso de texto sem caracteres diferentes de nova linha
    
    bool e_corpo_macro = false;

    linha *p_primeira_linha = NULL; // Declara ponteiro para a primeira linha
    linha *p_linha_atual = NULL; // Declara ponteiro para receber endereço da linha após criação
    linha *p_linha_anterior = NULL; // Declara ponteiro para a linha anterior

    while (texto_linha_buffer != NULL) {
        char *texto_linha_heap = malloc(strlen(texto_linha_buffer) + 1); // Aloca memória para o texto da linha na heap
    
        if (texto_linha_heap == NULL) {
            perror("Erro na alocação de memória para o texto da linha");
            exit(1);
        }

        strcpy(texto_linha_heap, texto_linha_buffer);

        p_linha_atual = cria_linha(texto_linha_heap, p_linha_anterior, &e_corpo_macro);

//RETIRAR ##########################
 //       printf("%saaaaaaaaaaaaaaaaaa\n",p_linha_atual->texto_linha);
//##################################3

        if (p_primeira_linha == NULL) p_primeira_linha = p_linha_atual; // Indica linha de início de texto
        else { // Atualiza referências sobre linha antecedente do texto
            p_linha_anterior->p_linha_posterior = p_linha_atual;
            p_linha_atual->p_linha_anterior = p_linha_anterior;
        }

        if (p_linha_atual->tipo_linha == MACRO_L) e_corpo_macro = true;
        if (p_linha_atual->tipo_linha == ENDMACRO_L) e_corpo_macro = false;

        p_linha_anterior = p_linha_atual;

        texto_linha_buffer = strtok_r(NULL, "\n", &p_reentrancia);
    }

    p_cabecalho_texto->p_primeira_linha = p_primeira_linha;
    p_cabecalho_texto->p_ultima_linha = p_linha_atual;
    p_cabecalho_texto->total_linha = 0; // não conta número de linhas ainda

    return p_cabecalho_texto;
}

/*
void exclui_linha_da_lista(cabecalho_texto *p_cabecalho_texto, linha *p_linha_atual) {
    if (p_linha_atual->p_linha_anterior == NULL) { // Caso de primeira linha do texto
        if (p_linha_atual->p_linha_posterior == NULL) p_cabecalho_texto->p_primeira_linha = NULL;
        else p_cabecalho_texto->p_primeira_linha = p_linha_atual->p_linha_posterior;
    }
    else { 
        if (p_linha_atual->p_linha_posterior == NULL) { // Caso de linha de fim de texto
            p_linha_atual->p_linha_anterior->p_linha_posterior = NULL;
        } 
        else { // Caso de linha de meio de texto
            p_linha_atual->p_linha_anterior->p_linha_posterior = p_linha_atual->p_linha_posterior;
            p_linha_atual->p_linha_posterior->p_linha_anterior = p_linha_atual->p_linha_anterior;
        }
    }
}
    */

linha *expande_chamada_macro(linha *p_linha_anterior_chamada, linha *p_linha_posterior_chamada, linha *p_linha_inicio_corpo_macro, char *texto_parametro_1, char *texto_parametro_2, char *texto_argumento_1, char *texto_argumento_2, int num_parametros) {
    linha *p_linha_atual_corpo = p_linha_inicio_corpo_macro;
    linha *p_linha_atual_expandida = NULL;
    linha *p_linha_anterior = p_linha_anterior_chamada;
    linha *p_primeira_linha_expandida = NULL;

    // Calcula variação máxima de tamanho da lista expandida, considerando que haverá, no máximo, dois argumentos por linha
    int tamanho_menor_parametro = 0;
    int tamanho_maior_argumento = 0;
    int variacao_tamanho_linha = 0;

    if (num_parametros == 1) {
        if (strlen(texto_argumento_1) > strlen(texto_parametro_1) ) // Argumento maior que parâmetro
            variacao_tamanho_linha = 2 * (strlen(texto_argumento_1) - strlen(texto_parametro_1));
    }
    else if (num_parametros == 2) {
        if (strlen(texto_parametro_1) < strlen(texto_parametro_2)) tamanho_menor_parametro = strlen(texto_parametro_1);
        else tamanho_menor_parametro = strlen(texto_parametro_2);
        if (strlen(texto_argumento_1) > strlen(texto_argumento_2)) tamanho_maior_argumento = strlen(texto_argumento_1);
        else tamanho_maior_argumento = strlen(texto_argumento_2);

        if (tamanho_maior_argumento > tamanho_menor_parametro) variacao_tamanho_linha = 2 * (tamanho_maior_argumento - tamanho_menor_parametro);
    }

    // Expande chamada de macro
    int tamanho_linha_corpo;
    bool e_macro = false; // Apenas para chamar a criação de linha
    bool e_primeira_linha_expandida = true;

    while (p_linha_atual_corpo != NULL) { // Percorre linhas do corpo da macro
        char *posicao_busca = NULL;
        int posicao_origem = 0;
        int posicao_destino = 0;
        char *texto_linha_expandida = NULL;
        char *texto_linha_corpo = p_linha_atual_corpo->texto_linha;

        texto_linha_expandida = malloc(strlen((p_linha_atual_corpo->texto_linha)) + variacao_tamanho_linha + 1);
        if (texto_linha_expandida == NULL) {
        perror("Erro na alocação de memória para o texto da linha expandida");
        exit(1);
        }
        if (num_parametros > 0) { // Macro tem parâmetros
            while (texto_linha_corpo[posicao_origem] != '\0') { // Percorre o texto caractere a caractere
                if (texto_linha_corpo[posicao_origem] != '&') {
                    texto_linha_expandida[posicao_destino] = texto_linha_corpo[posicao_origem];
                    posicao_origem++;
                    posicao_destino++;
                }
                else {
                    posicao_busca = texto_linha_corpo + posicao_origem + 1;
                    char *parametro_no_corpo = strdup(posicao_busca);
                    parametro_no_corpo = strtok(parametro_no_corpo, ",");
                    if (strcmp(parametro_no_corpo, texto_parametro_1) == 0) {   // Verifica se é parâmetro 1
                        memcpy(texto_linha_expandida + posicao_destino, texto_argumento_1, strlen(texto_argumento_1));
                        posicao_destino += strlen(texto_argumento_1);
                        posicao_origem += 1 + strlen(texto_parametro_1);
                    }
                    else if ((texto_parametro_2 != NULL) && (strcmp(parametro_no_corpo, texto_parametro_2) == 0)) { // Verifica se é parâmtero 2
                        memcpy(texto_linha_expandida + posicao_destino, texto_argumento_2, strlen(texto_argumento_2));
                        posicao_destino += strlen(texto_argumento_2);
                        posicao_origem += 1 + strlen(texto_parametro_2);
                    }
                    else { // Se não encontrar correspondência a parâmetro, apenas repete caractere
                        posicao_origem++;
                        posicao_destino++;
                    }
                }
            }
            texto_linha_expandida[posicao_destino] = '\0';
            
            p_linha_atual_expandida = cria_linha(texto_linha_expandida, p_linha_anterior, &e_macro);
        }
        else { // Macro não tem parâmetros
            strcpy(texto_linha_expandida, p_linha_atual_corpo->texto_linha);
            p_linha_atual_expandida = cria_linha(texto_linha_expandida, p_linha_anterior, &e_macro);
        }
        if (e_primeira_linha_expandida) {
            p_primeira_linha_expandida = p_linha_atual_expandida;
            e_primeira_linha_expandida = false;
        }
        else { // Se não é a primeira linha expandida, altera a referência na linha na anterior. Para a primeira isso é feito fora, porque envolve cabeçalho.
            p_linha_anterior->p_linha_posterior = p_linha_atual_expandida;
        }    
    
        p_linha_anterior = p_linha_atual_expandida;
        p_linha_atual_corpo = p_linha_atual_corpo->p_linha_posterior;
    }

    // Conecta última linha expandida à primeira linha após chamada da macro (caso existente)
    p_linha_anterior->p_linha_posterior = p_linha_posterior_chamada;
    if ((p_linha_posterior_chamada) != NULL) p_linha_posterior_chamada->p_linha_anterior = p_linha_anterior;

    // Retorna a primeira linha expandida
    return p_primeira_linha_expandida;
}

/*

Ir na linha antes da chamada de macro e mudar a referência à posterior dela (que será a primeira expandida)
Ir na última expandida: conecta-la a proxima e a proxima a ela
assim, a chamada estará automaticamente desconectada.

*/

void identifica_e_expande_macro(cabecalho_texto *p_cabecalho_texto, linha *p_linha_inicio_macro) {

    // Identifica rótulo da macro
    char *rotulo_macro = p_linha_inicio_macro->rotulo_macro;

    // Identifica parâmetros da macro
    token *parametros_macro[] = {NULL, NULL};

    token *p_token_atual = p_linha_inicio_macro->p_primeiro_token;

    int num_parametros = 0;

    while ((p_token_atual != NULL) && (num_parametros < 2)) {
        if (p_token_atual->tipo_token == PARAMETRO_MACRO) {
            parametros_macro[num_parametros] = p_token_atual;
            num_parametros++;
        }
        p_token_atual = p_token_atual->p_token_posterior;
    }

    char *texto_parametro_1 = NULL;
    char *texto_parametro_2 = NULL;

    if (num_parametros > 0) {
        texto_parametro_1 = malloc(strlen(parametros_macro[0]->texto_token) + 1);

        if (texto_parametro_1 == NULL) {
            perror("Erro na alocação de memória para o parâmetro de uma macro");
            exit(1);
        }

        strcpy(texto_parametro_1, parametros_macro[0]->texto_token);
    }

    if (num_parametros > 1) {
        texto_parametro_2 = malloc(strlen(parametros_macro[1]->texto_token) + 1);

        if (texto_parametro_2 == NULL) {
            perror("Erro na alocação de memória para o parâmetro de uma macro");
            exit(1);
        }

        strcpy(texto_parametro_2, parametros_macro[1]->texto_token);
    }

    // Desconecta da lista as linhas de início, meio e fim de macro, mantendo a ligação interna entre linhas de meio
    linha *p_primeira_linha_corpo_macro = p_linha_inicio_macro->p_linha_posterior;
    linha *p_linha_atual = p_primeira_linha_corpo_macro;
    linha *p_linha_atual_temp = NULL;

    while (p_linha_atual) {
    //while (p_linha_atual && p_linha_atual->rotulo_macro && (strcmp(p_linha_atual->rotulo_macro, rotulo_macro) == 0)) {

        if ((p_linha_atual->rotulo_macro) && (strcmp(p_linha_atual->rotulo_macro, rotulo_macro) == 0) && (p_linha_atual->tipo_linha == ENDMACRO_L)) {

            // Conecta linhas antes e depois da definição da macro, caso existentes
            if (p_linha_inicio_macro->p_linha_anterior == NULL) { // Caso de primeira linha do texto
                if (p_linha_atual->p_linha_posterior == NULL) p_cabecalho_texto->p_primeira_linha = NULL;
                else p_cabecalho_texto->p_primeira_linha = p_linha_atual->p_linha_posterior;
            }
            else { 
                if (p_linha_atual->p_linha_posterior == NULL) { // Caso de linha de fim de texto
                    p_linha_inicio_macro->p_linha_anterior->p_linha_posterior = NULL;
                } 
                else { // Caso de linha de meio de texto
                    p_linha_inicio_macro->p_linha_anterior->p_linha_posterior = p_linha_atual->p_linha_posterior;
                    p_linha_atual->p_linha_posterior->p_linha_anterior = p_linha_inicio_macro->p_linha_anterior;
                }
            }
    
            // Desconecta linha de início da definição da macro
            p_linha_inicio_macro->p_linha_anterior = NULL;
            p_linha_inicio_macro->p_linha_posterior = NULL;

            // Desconecta linha de fim da definição da macro
            p_linha_atual_temp = p_linha_atual->p_linha_posterior;
            p_linha_atual->p_linha_anterior->p_linha_posterior = NULL;
            p_linha_atual->p_linha_posterior = NULL;
            p_linha_atual->p_linha_anterior = NULL;
            
            p_linha_atual = p_linha_atual_temp;
            break;
        }
        else p_linha_atual = p_linha_atual->p_linha_posterior;    
    }

    // Identifica e expande linhas que contêm chamadas da macro

    char *texto_argumento_1 = NULL;
    char *texto_argumento_2 = NULL;
    char *inicio_argumentos = NULL;
    char *inicio_rotulo = NULL;
    int tamanho_argumento = 0;
    char *posicao_virgula = NULL;
    int tamanho_texto_linha_expandida = 0;
    int tamanho_texto_linha_original = 0;
    int diferenca_tamanho_parametro_1 = 0;
    int diferenca_tamanho_parametro_2 = 0;

    while (p_linha_atual != NULL) { // Percorre linhas após linhas de definição de macro

        p_token_atual = p_linha_atual->p_primeiro_token;

        while (p_token_atual != NULL) { // Percorre tokens de cada linha
         
            if (strcmp(p_token_atual->texto_token, rotulo_macro) == 0) { // A linha contém chamada de macro    

                linha *p_primeira_linha_expandida = NULL;
                
                if (num_parametros > 0) { // Caso em que é necessário substituir parâmetros por argumentos
                
                    inicio_rotulo = strstr(p_linha_atual->texto_linha, rotulo_macro);
                    if (inicio_rotulo != NULL) inicio_rotulo = strdup(inicio_rotulo);
                    inicio_argumentos = inicio_rotulo + strlen(rotulo_macro) + 1;
                    tamanho_argumento = strlen(inicio_argumentos);

                    texto_argumento_1 = malloc(tamanho_argumento + 1);

                    if (texto_argumento_1 == NULL) {
                        perror("Erro na alocação de memória para o argumento de uma macro");
                        exit(1);
                    }

                    strcpy(texto_argumento_1, inicio_argumentos);
                    if (texto_argumento_1[strlen(texto_argumento_1) - 1] == ' ') texto_argumento_1[strlen(texto_argumento_1) - 1] = '\0'; // Conserta eventual espaço sobressalente   
                }
                
                if (num_parametros == 2) {

                    posicao_virgula = strstr(inicio_argumentos, ",");

                    posicao_virgula[0] = '\0'; // Corta texto do primeiro argumento na  
                    texto_argumento_2 = posicao_virgula + 2; // Inicia texto do segundo argumento após espaço    
                if (texto_argumento_2[strlen(texto_argumento_2) - 1] == ' ') texto_argumento_2[strlen(texto_argumento_2) - 1] = '\0'; // Conserta eventual espaço sobressalente   
                }

                strtok(texto_argumento_1, ",");

                p_primeira_linha_expandida = expande_chamada_macro(p_linha_atual->p_linha_anterior, p_linha_atual->p_linha_posterior, p_primeira_linha_corpo_macro, texto_parametro_1, texto_parametro_2, texto_argumento_1, texto_argumento_2, num_parametros);
                
                // Conecta linha anterior à chamada à primeira linha expandida (o caminho contrário foi feita na função de expansão)
                if ((p_linha_atual->p_linha_anterior) == NULL) { // Caso de chamada de macro no início do texto
                    p_cabecalho_texto->p_primeira_linha = p_primeira_linha_expandida;
                }
                else {
                    p_linha_atual->p_linha_anterior->p_linha_posterior = p_primeira_linha_expandida;
                }
                //p_linha_atual = p_linha_atual->p_linha_posterior->p_linha_anterior; // Como linha atual, troca a linha de chamada pela última linha expandida
/*              
                // Conecta última linha expandida à linha posterior à chamada (e vice-versa)
                if ((p_linha_atual->p_linha_posterior != NULL)) {
                    linha *p_ultima_linha_expandida = p_primeira_linha_expandida;
                    while ((p_ultima_linha_expandida->p_linha_posterior) != NULL) { // Encontra a última linha expandida
                        p_ultima_linha_expandida = p_ultima_linha_expandida->p_linha_posterior;
                    }
                    p_ultima_linha_expandida->p_linha_posterior = p_linha_atual->p_linha_posterior;
                    p_linha_atual->p_linha_posterior->p_linha_anterior = p_ultima_linha_expandida;
                    
                }
 */      
            }

            p_token_atual = p_token_atual->p_token_posterior;

        }

        p_linha_atual = p_linha_atual->p_linha_posterior;

    }
/*
    free(texto_parametro_1);
    free(texto_parametro_2);
    free(texto_argumento_1);
    free(texto_argumento_2);
*/
}


    // Agora, separei coropo da macro e desconectei inicio e fim da macro.
    // Tenho que encontrar chamadas da macro (usando p_linha_posterior e rotulo)
    // Depois que achar chamadas, calcular tamanhos para expansão e expandir.






    // Identifica chamadas da macro e faz extensão






//MALLOCAR NOVAS LINHAS

    // PESQUISAR LINHA MACRO_L
// VER TEXTO DO RÓTULO
// CALCULAR NÚMERO DE ARGUMENTOS E NOME DOS ARGUMENTOS VETOR DE DUAS POSIÇÕES
// EXCLUIR ESSA LINHA (ALTERAR AS REFERÊNCIAS NA ANTERIOR E NA POSTERIOR)
// DESALOCAR MEMÓRIA?

// CONTAR NÚMERO DE LINHAS QUE CONTÊM O RÓTULO E SÃO CORPO_MACRO_L
// CRIAR ARRAY COM ESSE NÚMERO DE POSIÇÕES
// ARMAZENAR AS REFERÊNCIAS ÀS LINHAS
// ARMAZENAR PARÂMETROS
// PESQUISAR LINHAS CORPO_MACRO_L QUE TENHAM O MESMO RÓTULO
// AINDA NÃO DESALOCAR MEMÓRIA
// MAS JÁ EXCLUIR AS LINHAS DA LISTA (CUIDADO COM OS INDICES DOS LOOPS) TALVEZ SEJA REFERENCIA DO PROXIMO

// EXCLUIR ENDMACRO, REFAZENDO REFERÊNCIAS

// PESQUISAR AS LINHAS SUBEQUENTES QUE FAZEM REFERÊNCIA À MACRO (NOME = ROTULO)
// ARMAZENAR ARGUMENTOS (USAR '\0') PARA SEM ARGUMENTOS
// SUBSTITUIR PARÂMETROS E USAR FUNÇÃO CRIA_LINHA PARA CRIAR LINHAS
// QUANDO TERMINAR, LIGAR A ÚLTIMA LINHA DE POIS DA ENDMACRO?

// TALVEZ NÃO PRECISE MAIS DO RÓTULO DE CHAMADA DE MACRO


/* 

- nome da string ou do array é um endereço (o primeiro)
- array ~ &array[0]
- aritmética de ponteiros (p++)
- struct pode ficar na stack (instanciação normal)
- struct pode ficar na heap (instanciação como ponteiro com alocação dinâmica)
        * é o caso de ponteiro para heap
        * struct Pessoa *ptr = malloc(sizeof(struct Pessoa));
        * e inicializar valores (ver folha)
- desdiferenciação: (*x).y ~ x->y
- caracteres são inteiros; convém fazer casting para unsigned char
- sempre usar p ou ptr para indicar ponteiro


- sempre declarar protótipos das funções
    * tipo_retorno nome_funcao(tipo_param1 nome1, tipo_param2 nome2, ...);
- receber nome do arquivo na main
- guardá-lo para gerar os arquivos futuros
- lembrar de fechar o arquivo após uso fclose()
- rewind() para retornar ao início
- ftell() fseek() fgetpos() fsetpos()
- calcular tamanho do arquivo e alocar dobro da memória para o produto do preprocessamento
    * malloc: 
       - não inicializa; 
       - recebe nº de bytes; 
       - usar sizeof() para especificar; 
       - retorna endereço inicial; 
       - checar se endereço não é NULL (!nome) para erro;
       - reserva área na heap
       - no final do código, desalocar com free(endereço)
    * calloc: 
        - passo tamanho e número de elementos
        - inicializa com zeros
    * realloc:
        - retorna novo endereço com adição de memória
        - copia região antiga para a nova
        - não sobrescrever endereço original antes de checar erro

- posso ler caractere a caractere (fgetc) ou string (fgets)
    * ele armazena em buffer em qualquer caso (não lê diretão do disco)

- funções relevantes na primeira passada
    * strtok(), vai tokenizando substituindo delimitador por '\0' e retornando endereços
    * strchr(), strstr() fazem buscas
    * há cópia, concatenação e comparação
    * strlen(): nº de caracteres (não conta '\0')
    * isalnum(), ispunct(), isspace() ['\n', ' ', '\t'], islower(), isupper(), isalpha(), isdigit()

- as funções de str tem variante mais segura em que limitamos o tamanho

- sempre guardar tamanho das strings (1 + nº de caracteres)

- inicilizar structs:

token t = {NULL, -1};
linha l = {-1, NULL, NULL, NULL, NULL};


*/


// vou ter structs com dados das linhas alocar dinamicamente os campos
// de strings.
// o que cada struct conterá? já pensar nos tokens? 
// tokens e tipos
// estarão envoltas em nós (com ant, prox, struct)
// alocar structs estática ou dinamicamente
// tipo de ponteiro e casting

