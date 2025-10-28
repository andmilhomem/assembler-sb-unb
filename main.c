#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>


// Definição de estruturas de dados utilizadas ao longo do programa

typedef enum {BEGIN_L, END_L, MACRO_L, ENDMACRO_L, CORPO_MACRO, CHAMADA_MACRO_L} tipo_linha;
typedef enum {ROTULO, MAIS, VIRGULA, BEGIN, SPACE, CONST, END, MACRO, ENDMACRO, ARGUMENTO_MACRO, CHAMADA_MACRO, ADD, SUB, MULT, DIV, JMP, JMPN, JMPP, JMPZ, COPY, LOAD, STORE, INPUT, OUTPUT, STOP, NUMERO, NOME} tipo_token;

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
} linha;

typedef struct cabecalho_texto {
    linha *p_primeira_linha;
    linha *p_ultima_linha;
    long total_linha;
} cabecalho_texto;

// CABEÇALHO DO TEXTO

// Protótipos de funções utilizadas ao longo do programa

long obtem_texto_arquivo_fonte(int argc, char *argv[], char **p_p_texto_fonte);
long preformata_texto(char **p_p_texto_fonte, long tamanho_arquivo);
cabecalho_texto *estrutura_texto(char **p_p_texto_fonte);
struct linha *cria_linha(char *texto_linha, linha *p_linha_anterior, bool *e_corpo_macro);
struct token *cria_token(char *texto_token);
bool e_numero(const char *str);

//int identifica_macros();
//int expande_macros();

int main(int argc, char *argv[]) {

    char *texto_fonte = NULL; // declara ponteiro para região da heap a ser criada região para armazenar texto que será obtido do arquivo-fonte
    long tamanho_arquivo = obtem_texto_arquivo_fonte(argc, argv, &texto_fonte);

    if (tamanho_arquivo == -1)
        return -1;

    tamanho_arquivo = preformata_texto(&texto_fonte, tamanho_arquivo);

    if (tamanho_arquivo == -1)
        return -1;
    
    //#####################RETIRAR
    FILE *arquivo = fopen("saida.txt", "w"); // "w" = write (escrita)
    if (arquivo == NULL) {
        perror("Erro ao abrir o arquivo");
        return -1;
    }
    fprintf(arquivo, "%s", texto_fonte);

    printf("%s\n", texto_fonte);
    //#####################RETIRAR

    cabecalho_texto *p_cabecalho_texto = estrutura_texto(&texto_fonte);

//TRATAR AQUI CORPO DE MACRO E MACRO   ( e depois ver quem está chamando estrutura texto, quais são os retornos possíveis)



    // LIBERAR LINHAS, CABEÇALHOS E TOKENS AO FINAL, percorrendo estruturas de dados


    free(texto_fonte);
    
    return 0;
}


long obtem_texto_arquivo_fonte(int argc, char *argv[], char **p_p_texto_fonte) {
    // Trata erro de carga do programa
    if ((argc > 2) || argc == 1) {
        printf("# Erro na carga do programa!\nPara carregar o programa, use o terminal.\nIndique como único argumento o nome (com extensão e sem caminho) do arquivo ASM que deseja montar.\nO arquivo deve estar localizado no mesmo diretório do montador.");
        return -1;
    }

    // Abre arquivo ASM
    FILE *arquivo_fonte = fopen(argv[1], "r");

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
        else if ((caractere_atual == ',') && (caractere_futuro != ' ')) { // Formata espaços após o símbolo "vírgula"
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
    if (texto_token[0] == '&') { // Identifica argumentos de macros &XXXX
        strcpy(texto_token, texto_token + 1);
        p_token_atual->tipo_token = ARGUMENTO_MACRO;
    }
    // Identifica demais tipos de token
    if (strcmp(texto_token, "+") == 0) p_token_atual->tipo_token = MAIS;
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
    printf("%s : %i\n",p_token_atual->texto_token , p_token_atual->tipo_token);
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

    char copia_linha[2 * (strlen(texto_linha) + 1)]; // Declara buffer na pilha, com memória dobrada para tratamento posterior de vírgulas
    
    int tamanho_linha = strlen(texto_linha) + 1;

    int pos_origem = 0;
    int pos_destino = 0;

    while (pos_origem < tamanho_linha) { // copia texto da linha para buffer dando espaço antes de vírgulas
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
    p_linha_atual->texto_linha = texto_linha; // texto sem espaço antes da vírgula
    p_linha_atual->p_linha_anterior = p_linha_anterior; // recebido como argumento da função
    p_linha_atual->p_linha_posterior = NULL;

    if (e_inicio_macro) p_linha_atual->tipo_linha = MACRO_L;
    if (e_fim_macro) p_linha_atual->tipo_linha = ENDMACRO_L;
    if (e_inicio_modulo) p_linha_atual->tipo_linha = BEGIN_L;
    if (e_fim_modulo) p_linha_atual->tipo_linha = END_L;

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

