#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

long obtem_texto_arquivo_fonte(int argc, char *argv[], char **p_p_texto_fonte);
long preformata_texto(char **p_p_texto_fonte, long tamanho_arquivo);
//int identifica_macros();
//int expande_macros();

int main(int argc, char *argv[]) {

    char *p_texto_fonte = NULL; // declara ponteiro para região que armazenará texto que será obtido do arquivo-fonte
    long tamanho_arquivo = obtem_texto_arquivo_fonte(argc, argv, &p_texto_fonte);

    if (tamanho_arquivo == -1)
        return -1;

    tamanho_arquivo = preformata_texto(&p_texto_fonte, tamanho_arquivo);

    if (tamanho_arquivo == -1)
        return -1;
    
    //#####################RETIRAR
    FILE *arquivo = fopen("saida.txt", "w"); // "w" = write (escrita)
    if (arquivo == NULL) {
        perror("Erro ao abrir o arquivo");
        return -1;
    }
    fprintf(arquivo, p_texto_fonte);

    printf("%s\n",p_texto_fonte);
    //#####################RETIRAR

    free(p_texto_fonte);
    
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
    p_texto_preformatado = malloc(tamanho_arquivo + 1);

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

        p_texto_preformatado[nova_posicao] = toupper(caractere_atual); // Armazena caractere no buffer, transformando para letra maiúscula, se necessário
        nova_posicao++;
        caractere_anterior = caractere_atual;
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
*/


// vou ter structs com dados das linhas alocar dinamicamente os campos
// de strings.
// o que cada struct conterá? já pensar nos tokens? 
// tokens e tipos
// estarão envoltas em nós (com ant, prox, struct)
// alocar structs estática ou dinamicamente
// tipo de ponteiro e casting