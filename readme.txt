IDENTIFICAÇÃO
Aluno: André Milhomem Araújo de Godoi
Matrícula: 202033427
GitHub do projeto: https://github.com/andmilhomem/assembler-sb-unb

COMPILAÇÃO
Para compilar o código-fonte para Windows usando GCC, utilize o comando:
gcc montador.c -o montador.exe

EXECUÇÃO
Para executar o programa no Windows, utilize o comando:
.\montador.exe exemplo_arquivo.asm

INFORMAÇÕES
Os testes foram realizados utilizando as seguintes versões:
- Compilador: gcc 12.4.0 (Cygwin)
- Arquitetura: x86_64 (Windows 64 bits)

PROTÓTIPOS DAS FUNÇÕES

Para auxiliar na análise do código, apresento os protótipos das funções utilizadas:

// ANÁLISE: Formatação do código-fonte
long obtem_texto_arquivo_fonte(int argc, char *argv[], char **p_p_texto_fonte);
long preformata_texto(char **p_p_texto_fonte, long tamanho_arquivo);

// ANÁLISE: Estruturação do código-fonte
cabecalho_texto *estrutura_texto(char **p_p_texto_fonte);
struct linha *cria_linha(char *texto_linha, linha *p_linha_anterior, bool *e_corpo_macro);
struct token *cria_token(char *texto_token);
bool e_numero(const char *str);

// ANÁLISE: Expansão de macros
void identifica_e_expande_macro(cabecalho_texto *p_cabecalho_texto, linha *p_linha_inicio_macro);
linha *expande_chamada_macro(linha *p_linha_anterior_chamada, linha *p_linha_posterior_chamada, linha *p_linha_inicio_corpo_macro, char *texto_parametro_1, char *texto_parametro_2, char *texto_argumento_1, char *texto_argumento_2, int num_parametros);
void gera_arquivo_pre(cabecalho_texto *p_cabecalho_texto, char* nome_arquivo_sem_extensao);

// ANÁLISE: Identificação de erros no código-fonte
bool codigo_fonte_contem_erro(cabecalho_texto *p_cabecalho_texto);
void gera_arquivos_com_erros(cabecalho_texto *p_cabecalho_texto, char *nome_arquivo_sem_extensao);

// SÍNTESE: Resolução de diretivas, valores de instruções e referências a rótulos definidos previamente
long consulta_rotulo_lista_simbolos(simbolo **p_p_primeiro_simbolo, char *rotulo, posicao_memoria *p_referencia);
void insere_rotulo_lista_simbolos(simbolo **p_p_primeiro_simbolo, char *rotulo, long endereco);
void insere_posicao_memoria(posicao_memoria **p_p_ultima_posicao, posicao_memoria **p_p_primeira_posicao, long endereco, long valor, long deslocamento);
void gera_codigo_com_pendencias(cabecalho_texto *p_cabecalho_texto, posicao_memoria **p_p_primeira_posicao, simbolo **p_p_primeiro_simbolo);

// SÍNTESE: Resolução da lista de pendências (rótulos definidos após referência)
void resolve_pendencias(simbolo *p_primeiro_simbolo);
void gera_arquivo_o1(posicao_memoria *p_primeira_posicao, simbolo *p_primeiro_simbolo, char* nome_arquivo_sem_extensao);
void gera_arquivo_o2(posicao_memoria *p_primeira_posicao, char* nome_arquivo_sem_extensao);