// Dichiarazione delle funzioni da chiamare in main.c
// Definizione dei tipi necessari alla libreria


void imposta_gioco(void);
void gioca(void);
void termina_gioco(void);
void crediti(void);


enum Tipo_zona {bosco,scuola,laboratorio,caverna,strada,giardino,supermercato,centrale_elettrica,deposito_abbandonato,stazione_polizia };
enum Tipo_nemico {nessun_nemico,billi,democane,demotorzone};
enum Tipo_oggetto {nessun_oggetto,bicicletta,maglietta_fuocoinferno,bussola,schitarrata_metallica};




typedef struct Giocatore
{
    char nome[30];
    int mondo;  //0 mondoreale 1 soprasotto
    unsigned int attacco_pischico; 
    unsigned int difesa_pischica;
    unsigned int fortuna;
    struct Zona_mondoreale* pos_mondoreale;
    struct Zona_soprasotto* pos_soprasotto;
    enum Tipo_oggetto zaino[3];
    int n_oggetti;
    
}Giocatore;



typedef struct Zona_mondoreale
{
    enum Tipo_nemico nemico;
    enum Tipo_zona zona;
    enum Tipo_oggetto oggetto;
    struct Zona_mondoreale* avanti;
    struct Zona_mondoreale* indietro;
    struct Zona_soprasotto* link_soprasotto;
}Zona_mondoreale;



typedef struct Zona_soprasotto
{
    enum Tipo_nemico nemico; 
    enum Tipo_zona zona;
    struct Zona_soprasotto* avanti;
    struct Zona_soprasotto* indietro;
    struct Zona_mondoreale* link_mondoreale;
}Zona_soprasotto;  
