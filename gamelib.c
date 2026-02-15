  #include "gamelib.h"
  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>
  #include <unistd.h> 
  #include <time.h>

  /*---NOTE---
  1)Si suppongono valori di attacco,difesa e fortuna superiori ai 20 punti se vengono usati i corrispettivi buff
  2)Se il giocatore digita 0 alla scelta del numero giocatori si interpreta come un Early Return al menù principale
  3)Un giocatore non può aggiungere o cancellare zone se prima non ha generato la mappa di base 
  4)Se un giocatore si trova nell'ultima zona del soprasotto o del mondoreale si suppone che abbia già incontrato il boss e che dunque è morto o l'ha sconfitto,pertanto può tornare indietro e basta.

  */

  // Definizioni delle funzioni in gamelib.c.
  // Piu altre funzioni di supporto.
  // Le funzioni richiamabili in main.c non devono essere static.
  // Le altre devono essere static (non visibili all'esterno).

  //Variabili di controllo e globali
  static Giocatore* giocatori[4]= {NULL,NULL,NULL,NULL};
  static int num_giocatori;
  static int isOn; // 0 gioco mai impostato, 1 gioco già impostato (coincide con quando si finisce di creare la mappa)
  static struct Zona_mondoreale* prima_zona_mondoreale= NULL; 
  static struct Zona_soprasotto* prima_zona_soprasotto= NULL;
  static const char* const zone[]= {"bosco","scuola","laboratorio","caverna","strada","giardino","supermercato","centrale_elettrica","deposito_abbandonato","stazione_polizia"};
  static const char* const nemici[]= {"nessun_nemico","billi","democane","demotorzone"};
  static const char* const oggetti[]= {"nessun_oggetto","bicicletta","maglietta_fuocoinferno","bussola","schitarrata_metallica"};





  //Funzioni di imposta gioco
  static void svuota_mappa(); //svuota la mappa    
  static void genera_mappa();
  static void inserisci_zona();
  static void cancella_zona();
  static void stampa_zona();
  static void stampa_mappa();
  static void chiudi_mappa();
  static void loading_animation(); //funzione aggiuntiva puramente estetica
 
  //Funzioni aggiuntive a gioca
  static void avanza(int i);
  static void indietreggia(int i);
  static void cambia_mondo(int i);
  static int passa();
  static void stampa_giocatore(int i);
  static void stampa_zona_giocatore(int i);
  static void raccogli_oggetto(int i);
  static void utilizza_oggetto(int i);
  static int combatti(int i);
  





  void imposta_gioco(void) { 
      int UndiciVirgolaCinque=0;
      int buff;
      int map;
      unsigned short playerchoice= 0; //variabile per controllare le scelte del giocatore
      unsigned short mapchoice= 0; //controllo menu game master
      srand(time(NULL));
    
      //Controllo se il gioco è gia stato impostato 
    for(int i =0; i<4;i++) {
      if(giocatori[i] != NULL) {
          free(giocatori[i]);
          giocatori[i] = NULL;  //evito dangling pointer
      }
      }

      do {
        puts("[Quanti giocatori vogliono giocare? 1-4]");
        //anche qui viene controllato un ipotetico input che non sia un numero
        if(scanf("%d",&num_giocatori) != 1 || num_giocatori < 1 || num_giocatori > 4) {
          puts("ATTENZIONE! Inserire un numero da 1 a 4\n");
          while(getchar() != '\n');
          getchar();
          system("clear");
        } else break;

        if(num_giocatori == 0) {
          return;
        }
      } while(1); //rieseguo se il giocatore sbaglia 

      
      //pulizia del buffer post scanf e pre fgets
      while (getchar() != '\n');
    
      for(int i= 0; i< num_giocatori; i++) {
        giocatori[i]= (Giocatore*) malloc(sizeof(Giocatore));
        printf("Benvenuto Giocatore %d \n", i+1);
        puts("Inserisci il tuo nickname");
        fgets(giocatori[i]->nome,30,stdin);
        giocatori[i]->nome[strcspn(giocatori[i]->nome, "\n")] = '\0'; //elimino il newline
        
        //generazione statistiche del giocatore
        giocatori[i]-> attacco_pischico = (rand() %20)+1;
        giocatori[i]-> difesa_pischica = (rand() %20)+1;
        giocatori[i]-> fortuna = (rand() %20)+1;
        giocatori[i]-> n_oggetti = 0;
        giocatori[i] -> mondo = 0; //si inizia nel mondoreale
        for(int j=0; j<3; j++) giocatori[i]->zaino[j] = nessun_oggetto;

        sleep(1);

        do {
        //scelta buff attacco e difesa a discapito dell'altro
          puts("Puoi ora scegliere se vuoi aumentare attacco di 3 ma diminuire difesa di 3 o viceversa");
          puts("1. Aumenta l'attacco pischico");
          puts("2. Aumenta la difesa pischica");
          //non riappare nei case dopo che viene scelto da un giocatore
          if(!UndiciVirgolaCinque) puts("3. Puoi anche diventare UndiciVirgolaCinque se vuoi"); 
          puts("4. Nessuna Modifica");
          puts("This action will have consequences...");
          

          if(scanf("%d",&buff) != 1 || buff < 1 || buff>4) {
            puts("Devi inserire un numero da 1 a 4 per personalizzare il tuo personaggio\n");
            while (getchar() != '\n');
            continue;
          }
          while (getchar() != '\n');
      
          
          switch (buff)
          {
            case 1:
            puts("Hai aumentato l'attacco di 3 a discapito della difesa che diminuisce di 3\n");
            giocatori[i]-> attacco_pischico += 3; 
            giocatori[i]-> difesa_pischica = (giocatori[i]-> difesa_pischica >= 3) ? giocatori[i]-> difesa_pischica -3 : 0;
            playerchoice = 1;
            break;   

            case 2:
            puts("Hai aumentato la difesa di 3 a discapito dell'attacco che diminuisce di 3\n");
            giocatori[i]-> difesa_pischica +=3;
            giocatori[i]-> attacco_pischico= (giocatori[i]-> attacco_pischico >= 3) ? giocatori[i]-> attacco_pischico -3 : 0;
            playerchoice = 1;
            break;

            case 3:
            if(!UndiciVirgolaCinque) {
              giocatori[i]-> attacco_pischico += 4;
              giocatori[i]-> difesa_pischica += 4;
              giocatori[i]-> fortuna = (giocatori[i]-> fortuna >= 7) ? giocatori[i]-> fortuna -7 : 0;
              strcpy(giocatori[i]-> nome, "UndiciVirgolaCinque");
              UndiciVirgolaCinque= 1;
              playerchoice = 1;
            break;
            } else {
              puts("Esiste già un giocatore che è UndiciVirgolaCinque");
            }
          
          case 4:
          puts("Non hai modificato nulla");
          playerchoice = 1;
            break;
          }
        } while(!playerchoice);
      } 
      
        system("clear");
        //stampa statistiche
        for(int j= 0; j <num_giocatori; j++) {
          printf("Statistiche di %s: \n",giocatori[j]-> nome);
          printf("|Attacco pischico:| %u\n", giocatori[j]-> attacco_pischico);
          printf("|Difesa pischica:| %u\n", giocatori[j]-> difesa_pischica);
          printf("|Fortuna:| %u\n", giocatori[j]-> fortuna);
          printf("\n");
          }

        
        //menu con 6 scelte per quanto riguarda le mappe
        do {
          puts("\nArrivati a questo punto puoi scegliere cosa fare con la mappa di gioco\n");
          puts("1.Puoi scegliere se generare le mappe di gioco, ovviamente non puoi giocare se non lo fai");
          puts("2.Puoi inserire una zona a tuo piacimento, nemico e oggetto annessi");
          puts("3.Puoi essere spietato e cancellare una zona a tuo piacimento");
          puts("4.Osservare tutte le zone di gioco,come se fossi Re Kaioh");
          puts("5.Osserva una zona in particolare");
          puts("6.Puoi anche chiudere la mappa ed iniziare a giocare");
          //medesimo controllo scanf in caso di caratteri o valori outOfBounds rispetto al menù
          if(scanf("%d",&map) != 1 || map < 1 || map>6) {
            puts("\nDevi inserire un numero da 1 a 6 per scegliere cosa fare con le mappe");
            while (getchar() != '\n');
            continue;
          }
          while (getchar() != '\n');

          switch (map)
          {
            case 1:
            genera_mappa();
            mapchoice=0;

            break;

            case 2:
            inserisci_zona();
            mapchoice=0;

            break;

            case 3:
            cancella_zona();
            mapchoice=0;

            break;

            case 4:
            stampa_mappa();
            mapchoice=0;
            
            break;

            case 5:
            stampa_zona();
            mapchoice=0;

            break;

            case 6:
            chiudi_mappa();
            if(isOn==1)mapchoice=1; 

            break;

          }
      } while(!mapchoice);
 }

  void gioca(void) {
    unsigned int num_giocatori= 0;
    unsigned int game= 0; //0 gioco terminato 1 gioco terminato
    int scelta;

    system("clear");
    if(isOn == 0) {
      puts("\n\n\n");
      puts("[ATTENZIONE! Non puoi giocare senza aver prima impostato il gioco]");
      puts("\n\n\n");
      return;
    }
    puts("Ora è il momento di giocare");
    //contiamo quanti giocatori giocano e ne teniamo conto in una variabile,tutti iniziano nel mondoreale
    for(int i=0; i<4; i++){
      if (giocatori[i]!=NULL){
        num_giocatori++;
        giocatori[i]->pos_mondoreale= prima_zona_mondoreale;
        giocatori[i]-> mondo= 0; 
     }
    }
   //continua a iterare finchè non finiamo la partita

    while(!game) {
   //controllo se i giocatori sono ancora vivi
   int giocatori_vivi = 0;
        for (int i = 0; i < 4; i++) {
            if (giocatori[i] != NULL) {
                giocatori_vivi++;
            }
        }
        if (giocatori_vivi == 0) {
            puts("                     GAME OVER                      ");
            game = 1; 
            break; 
        }
    
    int shuffle[4] = {0,1,2,3};

    //shuffle fisher yates
    for(int i=3; i>0;i--){
     int j= rand()% (i+1);
     int temp= shuffle[i];
     shuffle[i]= shuffle[j];
     shuffle[j]= temp;
    }

    for (int i=0; i<4; i++){
    int k= shuffle[i];

  
    unsigned int avanzato=0; //avanza non può essere chiamata più di una volta
    unsigned int turno= 0;
    
   //se il giocatore è morto lo saltiamo, ma anche se non è mai stato allocato, visto che shuffle è sempre da 4.
    if(giocatori[k]== NULL){
    continue;
    }

    while(!turno && giocatori[k]!= NULL && !game) {
    printf("\nBenvenuto %s [%d], è ora di giocare",giocatori[k]->nome,k+1);    
    puts("\nPotrai vincere solo se sconfiggerai il demotorzone");
    puts("1.Puoi avanzare nella mappa se vuoi");
    puts("2.Puoi indietreggiare");
    puts("3.Puoi cambiare mondo");
    puts("4.Combatti,se nella tua zona c'è un nemico");
    puts("5.Stampa le tue statistiche");
    puts("6.Stampa le statistiche della zona in cui ti trovi");
    puts("7.Raccogli un oggetto, zaino permettendo");
    puts("8.Utilizza un oggetto");
    puts("9.Cedi il turno al prossimo giocatore");
    
    if (scanf("%d", &scelta) != 1 || scelta < 1 || scelta > 9){
    system("clear");
    puts("\nNon hai inserito una scelta valida");
    while (getchar() != '\n');
    continue; 
    }
    switch(scelta){
    //AVANZA
    case 1:
    if(avanzato==0){
     avanza(k);
    }else {
      puts("Non puoi avanzare nuovamente");
    }
    break;
    //INDIETREGGIA
    case 2:
    if(avanzato==0){
     indietreggia(k); 
     } else {
      puts("Non puoi indietreggiare nuovamente");
    }
    break;
    //CAMBIA MONDO   
    case 3:
    if(avanzato==0){
    cambia_mondo(k); 
    avanzato=1; // consideriamo il fallimento del lancio del dado come un tentativo di avanzamento
     } else {
      puts("Non puoi cambiare mondo di nuovo");
    }
    break;
    //COMBATTI
    case 4:
    system("clear");
    if(avanzato==0){
     int esito= combatti(k);
     if(esito==2) {
      puts("\nCongratulazioni! Hai sconfitto il demotorzonee vinto il gioco!\n");
      game=1; 

     //salvo il nome del vincitore su un file di testo 
      FILE *file_vittorie = fopen("vittorie.txt", "a"); // "a" sta per append (aggiungi in coda)
      if (file_vittorie != NULL) {
      fprintf(file_vittorie, "%s\n", giocatori[k]->nome);
      fclose(file_vittorie);
     }

    } else if (esito==0) {
      if (giocatori[k]->difesa_pischica <= 0) {
        puts("\nSei morto,il tuo personaggio è stato rimosso dal gioco.\n");
        free(giocatori[k]);
        giocatori[k] = NULL;
        turno=1;
        break; 
      }
    }
  
    //STAMPA STATISTICHE GIOCATORE
    case 5:
    system("clear");
    stampa_giocatore(k);
    break;
    //STAMPA STATISTICHE ZONA
    case 6:
    system("clear");
    stampa_zona_giocatore(k);
    break;
    //RACCOGLI OGGETTO
    case 7:
    system("clear");
    raccogli_oggetto(k);
    break;
    //UTILIZZA OGGETTO
    case 8:
    system("clear");
    utilizza_oggetto(k);
    break;
    //PASSA TURNO
    case 9:
    system("clear");
    turno = passa();
    //turno= 1;  //il turno finisce e si passa al prossimo giocatore
    break;
     }  
    }
   } 
  }
 }
 svuota_mappa();
 isOn=0;
}

static void avanza(int i){
  unsigned int nemico=0;
  
   if(giocatori[i]== NULL ||(giocatori[i]->pos_mondoreale==NULL && giocatori[i]->pos_soprasotto==NULL)) {
    return;
   }

   //controllo se dove si trova al momento il player c'è un nemico da combattere
   if(giocatori[i]->mondo==0){
    if(giocatori[i]->pos_mondoreale->nemico!=nessun_nemico){
      nemico= 1;  
    }
   } else if (giocatori[i]->mondo==1){
    if(giocatori[i]->pos_soprasotto->nemico!=nessun_nemico){
      nemico= 1;
    }

   }
   if(nemico) {
    system("clear");
    puts("\nNella tua zona c'è un nemico, lo dovrai combattere");
    puts("\nPremi 4 nel menù di gioco");
    return;
   }


  //se il giocatore si trova nel mondo reale avanza di uno
  if(giocatori[i]->mondo== 0){
    //se esiste la zona in cui al momento il giocatore si trova e quella dopo non è null si avanza
   if(giocatori[i]->pos_mondoreale!=NULL && giocatori[i]->pos_mondoreale->avanti !=NULL){ 

    giocatori[i]->pos_mondoreale= giocatori[i]->pos_mondoreale->avanti;
    //aggiorno puntatore del soprasotto
    giocatori[i]->pos_soprasotto= giocatori[i]->pos_mondoreale->link_soprasotto;
   } //Altrimenti significa che siamo nell'ultima zona
   else{
    printf("\n%s, sei nell'ultima zona del Mondo Reale\n",giocatori[i]->nome);
    return;
   }
   //soprasotto se mondo==1;
  } else {
    //se esiste la zona in cui al momento il giocatore si trova e quella dopo non è null si avanza
    if (giocatori[i]->pos_soprasotto!=NULL && giocatori[i]->pos_soprasotto->avanti !=NULL){
      //avanzo
      giocatori[i]->pos_soprasotto =  giocatori[i]->pos_soprasotto->avanti;
      //aggiorno puntatore soprasotto
      giocatori[i]->pos_mondoreale= giocatori[i]->pos_soprasotto->link_mondoreale;
    }else {
      printf("\n%s,sei nell'ultima zona del soprasotto\n",giocatori[i]->nome);
      return;
    }
  }
 int c= 1; 
 struct Zona_mondoreale* temp= prima_zona_mondoreale; 
 //scorro finche non trovo la zona verso dove si è spostato, lo faccio solo per mondoreale tanto ho il corrispettivo del soprasotto
   while(temp!=NULL && temp!= giocatori[i]->pos_mondoreale) {
   temp= temp->avanti;
   c++;
   }
   //prendo la zona in cui il giocatore si è spostato
  int tipoZona= giocatori[i]->pos_mondoreale->zona;
  system("clear");  
  printf("%s avanza nella zona %d di tipo %s nel %s",
  giocatori[i]->nome,c,zone[tipoZona], (giocatori[i]->mondo==0) ? "Mondo Reale": "Soprasotto");
}


static void indietreggia(int i ){
  unsigned int nemico=0;
  
   if(giocatori[i]== NULL ||(giocatori[i]->pos_mondoreale==NULL && giocatori[i]->pos_soprasotto==NULL)) {
    return;
   }

   //controllo se dove si trova al momento il player c'è un nemico da combattere
   if(giocatori[i]->mondo==0){
    if(giocatori[i]->pos_mondoreale->nemico!=nessun_nemico){
      nemico= 1;  
    }
   } else if (giocatori[i]->mondo==1){
    if(giocatori[i]->pos_soprasotto->nemico!=nessun_nemico){
      nemico= 1;
    }
    if(nemico) {
    system("clear");
    puts("\nNella tua zona c'è un nemico, lo dovrai combattere");
    puts("\nPremi 4 nel menù di gioco");
    return;
   }
    
    // mondoreale
    if(giocatori[i]->mondo == 0) {
      // Controllo se esiste una zona "dietro" (indietro != NULL)
      if(giocatori[i]->pos_mondoreale->indietro != NULL) {
            
      // sposta indietro
      giocatori[i]->pos_mondoreale = giocatori[i]->pos_mondoreale->indietro;
            
      // sincronizziamo anche il sottosopra
      giocatori[i]->pos_soprasotto = giocatori[i]->pos_mondoreale->link_soprasotto;
     } else {
      printf("\n%s, sei all'inizio del mondo reale! non puoi andare più indietro.\n", giocatori[i]->nome);
      return;
     }
    }
     //soprasotto
    else {
      if(giocatori[i]->pos_soprasotto->indietro != NULL) {
            
        // sposta indietro
      giocatori[i]->pos_soprasotto = giocatori[i]->pos_soprasotto->indietro;
            
        // sincronizzazione soprasotto
      giocatori[i]->pos_mondoreale = giocatori[i]->pos_soprasotto->link_mondoreale;
      
    } else {
        printf("\n%s, sei all'inizio del Sottosopra! Non puoi tornare indietro.\n", giocatori[i]->nome);
        return;
      }
    }
}
//Stampa la zona in cui ci si è spostati indietreggiando
int c= 1; //contatore per contare la zona dove si sposta il player
 struct Zona_mondoreale* temp= prima_zona_mondoreale; 
 //scorro finche non trovo la zona verso dove si è spostato, lo faccio solo per mondoreale tanto ho il corrispettivo del soprasotto
   while(temp!=NULL && temp!= giocatori[i]->pos_mondoreale) {
   temp= temp->avanti;
   c++;
   }
   //prendo la zona in cui il giocatore si è spostato
  int tipoZona= giocatori[i]->pos_mondoreale->zona;
  system("clear");  
  printf("---\n%s indietreggia nella zona %d di tipo %s nel %s\n---",
  giocatori[i]->nome,c,zone[tipoZona], (giocatori[i]->mondo==0) ? "Mondo Reale": "Soprasotto");
}

static void cambia_mondo(int i) {
    // controllo se ci sono giocatori null
    if(giocatori[i] == NULL) return;

    // dal mondo reale al soprasotto
    if (giocatori[i]->mondo == 0) {
        //non puoi cambiare mondo se sei nel mondoreale e c'è un nemico nella zona corrente
        if (giocatori[i]->pos_mondoreale->nemico != nessun_nemico) {
            system("clear");
            printf("\n[BLOCCATO] %s, c'è un nemico qui!\n", giocatori[i]->nome);
            puts("Non puoi concentrarti per aprire il portale mentre sei sotto attacco.\n");
            puts("Devi prima sconfiggerlo (premi 4 nel menù di gioco).\n");
            return;
        }

        if (giocatori[i]->pos_soprasotto == NULL) {
            puts("Cambio mondo Fallito.\n");
            return;
        }

        // se non ci sono nemici cambi mondo
        printf("\n%s riesce nell'impossibile...", giocatori[i]->nome);
        sleep(1); 
        giocatori[i]->mondo = 1; // 1 equivale al soprasotto
        
        int tipo = giocatori[i]->pos_soprasotto->zona;
        printf("\nSei stato catapultato nel soprasotto!\nZona attuale: %s\n", zone[tipo]);
    } 

    // dal soprasotto al mondoreale 
    else {
        // devi essere fortunato per scappare dal soprasotto....
        printf("\n%s tenta di scappare dal soprasotto...\n", giocatori[i]->nome);
        puts("\nlancio del dado..\n");
        sleep(1);
        int dado = (rand() % 20) + 1;
        printf("risultato del dado: %d La tua Fortuna: %d)\n", dado, giocatori[i]->fortuna); 

        if (dado < giocatori[i]->fortuna) {
            printf("Il portale si apre.\n");
            giocatori[i]->mondo = 0; // 0 mondoreale 
            int tipo = giocatori[i]->pos_mondoreale->zona;
            printf("\nSei tornato al sicuro nel MONDO REALE.\nZona attuale: %s\n", zone[tipo]);
        } else {
            puts("\nIl portale rimane chiuso.\n");
            puts("\nsei rimasto intrappolato nel soprasotto.\n");
        }
    }
}



static int passa() {
  return 1;
}

static void stampa_giocatore(int i) {

    // 1. Controllo di sicurezza: se il giocatore non esiste, esci.
    if (giocatori[i] == NULL) {
        return;
    }
 

    printf("Ecco le statistiche di: %s\n", giocatori[i]->nome);
    printf("Attacco pischico: %d\n", giocatori[i]->attacco_pischico);
    printf("Difesa pischica: %d\n", giocatori[i]->difesa_pischica);
    printf("Fortuna: %d\n", giocatori[i]->fortuna);
    
    // dove si trova il giocatore
    if (giocatori[i]->mondo == 0) {
        printf("%s si trova nel Mondo Reale\n", giocatori[i]->nome);
        if (giocatori[i]->pos_mondoreale != NULL) {
            int n_zona = giocatori[i]->pos_mondoreale->zona;
            printf("Zona del mondoreale: %s\n", zone[n_zona]);
        }
    } else {
        printf("%s si trova nel Soprasotto\n", giocatori[i]->nome);
        if (giocatori[i]->pos_soprasotto != NULL) {
            int n_zona = giocatori[i]->pos_soprasotto->zona;
            printf("Zona del soprasotto: %s\n", zone[n_zona]);
        }
    }

    // oggetti nello zaino
    puts("\n[Zaino]");
    
    int zaino_vuoto = 0;
    for (int j = 0; j < 4; j++) {
        int id_oggetto = giocatori[i]->zaino[j];
        
        if (id_oggetto > 0 && id_oggetto < 5) {
            printf("Oggetto %d: %s\n", j + 1, oggetti[id_oggetto]);
            zaino_vuoto = 1;
        }
    }
    
    if (zaino_vuoto == 0) {
        printf("%s il tuo zaino è vuoto\n", giocatori[i]->nome);
    }
}



static void stampa_zona_giocatore(int i) {
    if (giocatori[i] == NULL) return;

    int id_zona = 0;
    int id_oggetto = 0;
    int id_nemico = 0;
    int avanti = 0;
    int indietro = 0;

    // A seconda del mondo, peschiamo i dati dalla struct corretta
    if (giocatori[i]->mondo == 0) {
        struct Zona_mondoreale* temp = giocatori[i]->pos_mondoreale;
        
        if (temp == NULL) {
            puts("\nErrore: ti trovi nel vuoto!");
            return;
        }
        id_zona = temp->zona;
        id_oggetto = temp->oggetto;
        id_nemico = temp->nemico;
        avanti = (temp->avanti != NULL);
        indietro = (temp->indietro != NULL);
    } else {
        struct Zona_soprasotto* temp = giocatori[i]->pos_soprasotto;
        
        if (temp == NULL) {
            puts("\nErrore: ti trovi nel vuoto!");
            return;
        }
        
        id_zona = temp->zona;
        id_nemico = temp->nemico;
        avanti = (temp->avanti != NULL);
        indietro = (temp->indietro != NULL);
    }
    system("clear");
    puts("[Statistiche Zona]\n");
    
    printf("Luogo:%s\n", zone[id_zona]);

    if (id_nemico > 0 && id_nemico < 4) {
        printf("Nemico: %s, attenzione %s, lo dovrai combattere!\n", nemici[id_nemico], giocatori[i]->nome );
    } else {
        printf("Nemico: Nessuno\n");
    }

    if (id_oggetto > 0 && id_oggetto < 5) {
        printf("Oggetto: %s\n", oggetti[id_oggetto]);
    } else {
        puts("Nessun oggetto\n");
    }

    puts("Dove puoi andare?");
    if (indietro) puts("[<- Indietro]");
    if (avanti) puts("[Avanti ->]");
    if (!indietro && !avanti) puts("Nessuna via d'uscita!");
    puts("\n");
} 

static void raccogli_oggetto(int k) {
    if (giocatori[k] == NULL) return;

    enum Tipo_nemico* nemico_ptr = NULL;
    enum Tipo_oggetto* oggetto_ptr = NULL;

    if (giocatori[k]->mondo == 0) {
        if (giocatori[k]->pos_mondoreale == NULL) return;
        nemico_ptr = &(giocatori[k]->pos_mondoreale->nemico);
        oggetto_ptr = &(giocatori[k]->pos_mondoreale->oggetto);
    } else {
        if (giocatori[k]->pos_soprasotto == NULL) return;
        nemico_ptr = &(giocatori[k]->pos_soprasotto->nemico);
        }

    if (*nemico_ptr != 0) { 
        printf("\nAttenzione %s, c'è un nemico in zona (%s)!\n", giocatori[k]->nome, nemici[*nemico_ptr]);
        printf("Non puoi raccogliere nulla finché non lo elimini.\n");
        return;
    }

    if (*oggetto_ptr == 0) { 
    puts("Ti guardi intorno ma non vedi nulla di utile...");
      return;
    }

    int slot_libero = -1;
    for (int j = 0; j < 4; j++) { 
        if (giocatori[k]->zaino[j] == 0) { 
            slot_libero = j;
            break; 
        }
    }

    if (slot_libero == -1) {
        printf("\n[ZAINO PIENO] Non hai più spazio per raccogliere: %s\n", oggetti[*oggetto_ptr]);
        return;
    }

    giocatori[k]->zaino[slot_libero] = *oggetto_ptr; 
    
    printf("\n %s ha raccolto: %s (Inserito nello Slot %d)\n", 
           giocatori[k]->nome, oggetti[*oggetto_ptr], slot_libero + 1);

    *oggetto_ptr = 0;
    
}


static void utilizza_oggetto(int i) {
    int scelta;
    if (giocatori[i] == NULL) return;

    //Controlliamo se ha almeno un oggetto
    int oggetti_presenti = 0;
    for (int j = 0; j < 4; j++) { 
        if (giocatori[i]->zaino[j] != 0) oggetti_presenti++;
    }

    if (oggetti_presenti == 0) {
        printf("\n[ZAINO VUOTO] %s, non hai nessun oggetto da utilizzare!\n", giocatori[i]->nome);
        return;
    }

    // 2. Mostriamo l'inventario per la scelta
    printf("\nZaino di %s ---\n", giocatori[i]->nome);
    for (int j = 0; j < 4; j++) {
        int id_ogg = giocatori[i]->zaino[j];
        if (id_ogg != 0) {
            printf(" %d. Usa %s\n", j + 1, oggetti[id_ogg]);
        } else {
            printf(" %d. [Slot Vuoto]\n", j + 1);
        }
    }
    puts(" 5. Annulla e chiudi lo zaino\n");
    puts("\nQuale slot vuoi utilizzare? (1-4, o 5 per annullare): ");
    printf("Scelta: ");
    // Pulizia buffer integrata per evitare loop infiniti se inserisce lettere
    if (scanf("%d", &scelta) != 1 || scelta < 1 || scelta > 5) {
        puts("\nScelta non valida.");
        while(getchar() != '\n'); 
        return;
    }

    if (scelta == 5) {
        puts("Hai chiuso lo zaino.");
        return;
    }

    int index = scelta - 1;
    int id_oggetto_scelto = giocatori[i]->zaino[index];

    if (id_oggetto_scelto == 0) {
        puts("\nHai selezionato uno slot vuoto");
        return;
    }

    // 3. Applichiamo gli effetti in base all'oggetto
    printf("%s utilizza: %s!\n", giocatori[i]->nome, oggetti[id_oggetto_scelto]);

    switch (id_oggetto_scelto) {
        case 1: // bicicletta
            giocatori[i]->fortuna += 2;
            printf("È il tuo giorno fortunato %s! La tua Fortuna aumenta di 2 (Ora: %d)\n", giocatori[i]->nome, giocatori[i]->fortuna);
            break;
            
        case 2: // maglietta_fuocoinferno
            giocatori[i]->difesa_pischica+= 3;
            printf("Inspiegabilmente ti senti più resistente e . +20 Punti Vita (Ora: %d)\n", giocatori[i]->difesa_pischica);
            break;
            
        case 3: // bussola
            puts("Trovi una particolare bussola.\n");
            puts("Un varco tra i mondi si rivela, prova a guardare..");
            puts("Guardi la bussola per orientarti...\n");
            // Sbircia la prossima zona in sicurezza
            if (giocatori[i]->mondo == 0 && giocatori[i]->pos_mondoreale->avanti != NULL) {
                printf(" La bussola indica che la PROSSIMA ZONA è: %s\n", zone[giocatori[i]->pos_mondoreale->avanti->zona]);
            } else if (giocatori[i]->mondo == 1 && giocatori[i]->pos_soprasotto->avanti != NULL) {
                printf(" La bussola indica che la PROSSIMA ZONA è: %s\n", zone[giocatori[i]->pos_soprasotto->avanti->zona]);
            } else {
                puts("La bussola smette di funzionare senza motivo\n");
            }
            break;
            
        case 4: // schitarrata_metallica
            giocatori[i]->attacco_pischico += 5;
            giocatori[i]->difesa_pischica += -5;
            puts(" EFFETTO: Ozzy ti dona i suoi poteri metal! \n");
            break;

    }

    giocatori[i]->zaino[index] = 0; // 0 = nessun_oggetto
}

 static int combatti(int k) {
  int scelta;
  
    if (giocatori[k] == NULL || giocatori[k]->difesa_pischica <= 0) return 0;

    enum Tipo_nemico* p_nemico = NULL;
    
    if (giocatori[k]->mondo == 0) {
        if (giocatori[k]->pos_mondoreale == NULL) return 0;
        p_nemico = &(giocatori[k]->pos_mondoreale->nemico);
    } else {
        if (giocatori[k]->pos_soprasotto == NULL) return 0;
        p_nemico = &(giocatori[k]->pos_soprasotto->nemico);
    }

    if (*p_nemico == 0) {
        printf("\n %s, non c'è nessun nemico in questa zona.]\n", giocatori[k]->nome);
        return 0;
    }

    int id_nemico = *p_nemico;
    
    // si implementa una logica di combattimento semplice basata su statistiche predefinite per ogni nemico
    int vita_nemico, attacco_nemico;
    switch(id_nemico) {
        case 1: // billi
            vita_nemico = 15; attacco_nemico = 4; break;
        case 2: // democane
            vita_nemico = 20; attacco_nemico = 6; break;
        case 3: // demotorzone
            vita_nemico = 45; attacco_nemico = 12; break;
    }

    printf("%s deve combattere contro %s!\n", giocatori[k]->nome, nemici[id_nemico]);
    puts("(La tua difesa pischica rappresenta la tua ''resistenza''. Se scende a 0, muori!)\n");

    //Logica di combattimento
    // si combatte finché il nemico ha vita e il giocatore ha ancora difesa
    while (giocatori[k]->difesa_pischica > 0 && vita_nemico > 0) {
        printf("[%s] DIFESA (Vita): %d | Att: %d | Fort: %d\n", giocatori[k]->nome, giocatori[k]->difesa_pischica, giocatori[k]->attacco_pischico, giocatori[k]->fortuna);
        printf("[%s] VITA: %d | Att: %d\n", nemici[id_nemico], vita_nemico, attacco_nemico);
        printf("1.Attacca\n");
        printf("2.Utilizza un oggetto dallo zaino\n");
        printf("3.Tenta la fuga (Richiede Fortuna)\n");
        printf("Scelta: ");
        if (scanf("%d", &scelta) != 1) {
            while(getchar() != '\n'); 
            continue;
        }

        if (scelta == 2) {
            utilizza_oggetto(k);
        } 
        else if (scelta == 3) {
            int dado = (rand() % 20) + 1;
            if (dado < giocatori[k]->fortuna) {
                printf("\n[Fuga Riuscita] %s riesce a fuggire\n", giocatori[k]->nome);
                return 0; 
            } else {
                printf("\n[Fuga Fallita] Il %s ti sbarra la strada!\n", nemici[id_nemico]);
            }
        }
        //Attacco del giocatore
        //Se l'attacco del giocatore supera una prova di "Fortuna" del nemico, il colpo va a segno e il nemico perde vita.
        else if (scelta == 1) {
            int potenza_colpo = giocatori[k]->attacco_pischico + (rand() % giocatori[k]->fortuna);
            vita_nemico -= potenza_colpo;
            printf("\n[->] Colpisci il %s con una potenza di %d!\n", nemici[id_nemico], potenza_colpo);  
        } else {
            puts("\nScelta confusa. Perdi l'iniziativa!\n");
        }

        if (vita_nemico <= 0) break;

        // Contrattacco del nemico
        // Se l'attacco del nemico supera una prova di "Fortuna" del giocatore, il giocatore perde Difesa.
        int schivata = (rand() % 10) + giocatori[k]->fortuna;
        
        if (attacco_nemico > schivata / 2) {
            int danni_subiti = 2; 
            if (id_nemico == 3) danni_subiti = 4;
            giocatori[k]->difesa_pischica -= danni_subiti;
            printf("[<-] %s fa breccia nelle tue difese! Perdi %d Difesa!\n", nemici[id_nemico], danni_subiti);
        } else {
            printf("[<-] %s ti attacca, ma riesci a schivare\n", nemici[id_nemico]);
        }
        
        sleep(1); 
    }

    // Esito del combattimento
    if (giocatori[k]->difesa_pischica <= 0) {
        puts("Non sei riuscito a resistere.\n");
        printf(" %s è stato dilaniato dal %s...\n", giocatori[k]->nome, nemici[id_nemico]);
        return 0; 
    } 
    else {
      puts("\nSei riuscito a sconfiggere il nemico!\n");
        printf("%s giace morto ai tuoi piedi.\n", nemici[id_nemico]);

        if (id_nemico == 3) {
            return 2; // demotorzone sconfitto
        }

        if (rand() % 2 == 0) {
            puts("Il corpo del mostro si dissolve in cenere... La zona è libera.\n");
            *p_nemico = 0; 
        } else {
            puts("Uccidi il mostro, ma in lontananza senti i versi di un altro in avvicinamento...\n");
        }
        
        return 0;
    }
}




  void termina_gioco(void) {
    puts("\n\n");
    puts("========================================\n");
    puts("            FINE DELLA PARTITA          \n");
    puts("========================================\n");
    puts("Grazie per aver affrontato il Demotorzone!\n");
    puts("I sopravvissuti (e i caduti) vi salutano\n\n");

    //pulizia giocatori
     for (int i = 0; i < 4; i++) {
        if (giocatori[i] != NULL) {
            printf(" - %s abbandona la partita.\n", giocatori[i]->nome);
            free(giocatori[i]);
            giocatori[i] = NULL;
        }
    }

    //pulizia mondoreale e soprasotto
    struct Zona_mondoreale* scorri_reale = prima_zona_mondoreale;
    
    while (scorri_reale != NULL) {
        struct Zona_mondoreale* temp_reale = scorri_reale;
       // Se esiste la controparte nel Sottosopra, libero prima quella
        if (temp_reale->link_soprasotto != NULL) {
            free(temp_reale->link_soprasotto);
        }
        scorri_reale = scorri_reale->avanti;        
        free(temp_reale);
    }

    //reset dei puntatori globali della mappa
    prima_zona_mondoreale = NULL;
    prima_zona_soprasotto = NULL;

    puts("\n Memoria liberata con successo. Chiusura in corso...\n");
    
  
    return; 
}

void crediti(void) {
    puts("\n");
    puts("========================================\n");
    puts("               CREDITI                  \n");
    puts("========================================\n");
    
    
    puts("Sviluppatore: [Christian Cercizi]\n");
    puts("Matricola:395553");
    puts("Gioco:Cose Strane\n");
    puts("VINCITORI DELLE ULTIME 3 PARTITE:\n");

  
    FILE *file_vittorie = fopen("vittorie.txt", "r");
    
    if (file_vittorie == NULL) {
        puts("Nessuno ha ancora sconfitto il Demotorzone...\n");
    } else {
        char ultime_tre[3][100]; 
        char buffer[100];
        int contatore_righe = 0;

        // leggo il file riga per riga
        while (fgets(buffer, sizeof(buffer), file_vittorie) != NULL) {
            // pulisco il newline
            buffer[strcspn(buffer, "\n")] = 0; 
            strcpy(ultime_tre[contatore_righe % 3], buffer);
            contatore_righe++;
        }
        fclose(file_vittorie);

        if (contatore_righe == 0) {
            puts("(Il registro delle vittorie e' vuoto)\n");
        } else {
            int inizio = (contatore_righe < 3) ? 0 : contatore_righe - 3;            
            for (int i = inizio; i < contatore_righe; i++) {
                printf("  - %s\n", ultime_tre[i % 3]);
            }
        }
    }
}
  

  static void loading_animation(void) {
   system("clear");
    const char* testo= "Loading";
    printf("%s", testo);
    fflush(stdout); 
    for (int i=0; i<3; i++) {
      sleep(1);
      printf(".");
      fflush(stdout);
    }
  }  

  static void svuota_mappa(){
    struct Zona_mondoreale* first= prima_zona_mondoreale;

    while(first != NULL) {
      struct Zona_mondoreale* deleteM= first;
      struct Zona_soprasotto* deleteS = first -> link_soprasotto; //prendo il corrispettivo del soprasotto
      first= first->avanti;

      free(deleteM);
      //free soprasotto solo se esiste il link
      if (deleteS != NULL) {
        free(deleteS);
      }
    }
    prima_zona_mondoreale= NULL;
    prima_zona_soprasotto= NULL;
  }

  static void genera_mappa(void) {
     unsigned int tipo_zona,nemico_reale,nemico_soprasotto,obj,boss;

    if (prima_zona_mondoreale != NULL){
      svuota_mappa();
      system("clear");
      puts("\nLe mappe erano già settate e ora sono state rimosse");
      }
      puts("Ora è il momento di generare le mappe di gioco");
      loading_animation();
      
      //temp locali alla func, tengo traccia dell'ultima zona per mondo, per evitare di scorrere ogni volta tutto
      struct Zona_mondoreale* temp1= NULL;
      struct Zona_soprasotto* temp2= NULL;  


    for(int i=0; i<15; i++) {
       struct Zona_mondoreale* mondoreale= (Zona_mondoreale*) malloc(sizeof(struct Zona_mondoreale));
       struct Zona_soprasotto* soprasotto= (Zona_soprasotto*) malloc(sizeof(struct Zona_soprasotto)); 
       tipo_zona = rand()%10;
       mondoreale -> zona= tipo_zona;
       soprasotto -> zona= tipo_zona;

       //nemici mondoreale
       nemico_reale= rand()%100;
       if(nemico_reale > 20 && nemico_reale < 40 ){
        mondoreale-> nemico= nessun_nemico;
       } else if (nemico_reale > 40 && nemico_reale < 75){
        mondoreale-> nemico = democane;
       } else {
        mondoreale-> nemico = billi;
       }

       //nemici soprasotto
      nemico_soprasotto= rand()%100;
       if(nemico_soprasotto< 50){
        soprasotto-> nemico= nessun_nemico;
       } else soprasotto-> nemico = democane;

       //oggetti mondo reale
       obj= rand()%100;
       if(obj <= 45) {
       mondoreale-> oggetto= (rand()%4) +1; 
       } else {
        mondoreale-> oggetto = nessun_oggetto; 
       }

       mondoreale-> link_soprasotto= soprasotto;
       soprasotto-> link_mondoreale= mondoreale;

       mondoreale-> avanti= NULL;
       soprasotto-> avanti= NULL;

       if(prima_zona_mondoreale== NULL) {
        prima_zona_mondoreale= mondoreale;
        prima_zona_soprasotto= soprasotto;
        mondoreale->indietro= NULL;
        soprasotto->indietro= NULL;
       } else {
        temp1->avanti= mondoreale;
        mondoreale->indietro=temp1;
        temp2->avanti=soprasotto;
        soprasotto->indietro=temp2;
       }
       temp1= mondoreale;
       temp2= soprasotto;
      }

      boss= rand()%15;
       struct Zona_soprasotto* locazione_boss=prima_zona_soprasotto;
       for(int j=0; j < boss;j++) {
        locazione_boss= locazione_boss-> avanti;
       }
       locazione_boss->nemico= demotorzone;
       puts("\nLe mappe sono state create");
       puts("\nÈ in oltre presente un demotorzone");
      }


    static void inserisci_zona(void) {
      int i; //posizione di inserimento desiderata
      int counter = 0; //conta n zone attuali
      unsigned int scelta;
      int check= 0;

      struct Zona_mondoreale* scorri= prima_zona_mondoreale;
      
      //se si invoca inserisci_zona prima di aver generato le mappe si ritorna al menù del game master
      if (scorri== NULL){
        system("clear");
        puts("[Devi prima generare la mappa di base per poter aggiungere altre zone!]");
        return;
      }
      
      //calcolo lunghezza struct per poter inserire la posizione
      while (scorri != NULL){
        counter++;
        scorri= scorri->avanti;
      }
      
      system("clear");
      printf("\nAttualmente ci sono %d zone, in quale posizione vuoi inserirla?\n",counter);
      puts("\nDovrai reinserire un nemico e un oggetto per la nuova zona");
      puts("\nBeware what you choose");
      sleep(1);
      puts("\nInserisci la posizione in cui la vuoi inserire");
      printf("Scelta: ");
      if (scanf("%d", &i) != 1 || i < 1 ){
        puts("\nNon hai inserito una posizione valida per l'inserimento");
        return;
      }
      struct Zona_mondoreale* mondoreale= (struct Zona_mondoreale*) malloc(sizeof(struct Zona_mondoreale));
      struct Zona_soprasotto* soprasotto= (struct Zona_soprasotto*) malloc(sizeof(struct Zona_soprasotto));

     //Oggetto Mondo Reale
      do{
        puts("Inserisci l'oggetto nella nuova zona del mondo reale");
        puts("0.Nessun Oggetto, 1.Bici, 2.Maglietta Fuoco Inferno, 3.Bussola, 4. Schitarrata Metallica");
        scanf("%d", &scelta);
      }while (scelta < 0 || scelta > 4);
      mondoreale-> oggetto= scelta;

      //Nemico Mondo Reale
      do {
        puts("Inserisci il nemico della nuova zona del mondo reale");
        puts("0.Nessun Nemico, 1.Democane, 2.Billi");
        scanf("%d", &scelta);
      }while(scelta < 0 || scelta > 2);
      mondoreale-> nemico= scelta;

      //Nemico soprasotto (No demotorzone)
      struct Zona_soprasotto* cercaBoss = prima_zona_soprasotto;
      while (cercaBoss!=NULL){
        if (cercaBoss -> nemico == demotorzone){
           check= 1; 
           break;
        }
        cercaBoss= cercaBoss->avanti;
      }

      do{
      puts("Inserisci il nemico della nuova zona del soprasotto");
      puts("0.Nessun Nemico, 1.Democane, 2. Demotorzone");
      scanf("%d", &scelta);
      if (scelta== 2 && check==1){puts("Esiste già un demotorzone!");scelta=-11;}
      }while(scelta < 0 || scelta > 2);
      soprasotto-> nemico= scelta;

     //la zona creata nel mondo reale deve avere il suo corrispettivo nello stesso punto
     mondoreale->link_soprasotto= soprasotto;
     soprasotto->link_mondoreale= mondoreale;
     mondoreale->avanti= NULL;
     mondoreale->indietro= NULL;
     soprasotto->avanti= NULL;
     soprasotto->indietro= NULL; 

     //head insertion
     if(i ==1) {
      mondoreale->avanti = prima_zona_mondoreale;
      soprasotto->avanti = prima_zona_soprasotto;
      if(prima_zona_mondoreale!= NULL) {
        prima_zona_mondoreale -> indietro = mondoreale;
      }
      if(prima_zona_soprasotto!= NULL) {
        prima_zona_soprasotto -> indietro = soprasotto;
      }
      //aggiorno puntatori globali 
      prima_zona_mondoreale = mondoreale;
      prima_zona_soprasotto = soprasotto;
    } 
    //tail or whatever i position insertion 
     else {
     struct Zona_mondoreale* prevM= prima_zona_mondoreale;
     for(int j= 1; j< i-1;j++) {
      prevM= prevM -> avanti;
     }
     struct Zona_soprasotto* prevS= prevM-> link_soprasotto;
     mondoreale->avanti= prevM->avanti;
     soprasotto->avanti= prevS->avanti;

     mondoreale->indietro= prevM;
     soprasotto->indietro= prevS;

     if (prevM->avanti != NULL) {
      prevM->avanti->indietro = mondoreale;
      prevS->avanti->indietro = soprasotto;
     }
     prevM->avanti= mondoreale;
     prevS->avanti= soprasotto;
    }
    puts("");
     printf("Zona %d inserita",i);
     
  }

  static void cancella_zona(void) {
   int i, counter = 0; 
    //impossibile cancellare zone se non si genera la mappa
    if(prima_zona_mondoreale== NULL) {
    system("clear");
    puts("[Non puoi cancellare una mappa vuota!]");
    return;
    }
   struct Zona_mondoreale* scorri= prima_zona_mondoreale;
   while(scorri!= NULL) {
    counter++;
    scorri= scorri->avanti;
   } 
   system("clear");
   printf("\nAttualmente ci sono %d zone\n",counter);
   sleep(1);
   puts("Quale zona vuoi cancellare?");
    if (scanf("%d", &i) != 1 || i < 1 || i > counter){
      puts("\nNon hai inserito una posizione valida da cancellare");
      while (getchar() != '\n');
      return;
    }
    struct Zona_mondoreale* cancM= prima_zona_mondoreale;
    for(int j=1; j<i; j++) {
      cancM= cancM -> avanti;
    }
    struct Zona_soprasotto*  cancS= cancM->link_soprasotto;

    if(cancM -> indietro != NULL) {
      cancM-> indietro->avanti= cancM->avanti;
    } else {
      //head deletion
      prima_zona_mondoreale= cancM-> avanti;
    }
    if (cancM->avanti != NULL){
      cancM->avanti->indietro= cancM->indietro;
    }
    

    if(cancS -> indietro != NULL) {
      cancS-> indietro->avanti= cancS->avanti;
    } else {
      //head deletion
      prima_zona_soprasotto= cancS-> avanti;
    }
    if (cancS->avanti != NULL){
      cancS->avanti->indietro= cancS->indietro;
    }
    free(cancM);
    cancM= NULL;
    free(cancS);
    cancS=NULL;

    printf("Zona %d eliminata",i);
}

 static void stampa_mappa(void) {
  int sceltaMappa;
  int counter= 0;

  if(prima_zona_mondoreale == NULL){
    system("clear");
    puts("[La mappa è vuota non la puoi stampare]");
    return;
  }
  system("clear");
  puts("Puoi scegliere se stampare tutte le zone del mondo reale o tutte le zone del soprasotto");
  sleep(1);
  puts("0. Stampa Mondo Reale, 1. Stampa Soprasotto");
  if (scanf("%d", &sceltaMappa) != 1 || sceltaMappa < 0 || sceltaMappa> 1){
      puts("\nNon hai inserito un valore valido");
      while (getchar() != '\n');
      return;
    }
    //stampa tutte le zone del mondo reale
    if (sceltaMappa==0){
      struct Zona_mondoreale* scorri = prima_zona_mondoreale;
      while (scorri!=NULL){ 
        counter++;
        printf("\nNumero Zona:%d Tipo:%s\n",counter,zone[scorri->zona]);
        printf("Nemico: %s\n", nemici[scorri->nemico]);
        printf("Oggetto: %s\n", oggetti[scorri->oggetto]);
       scorri= scorri->avanti;
      }
      //stampa tutte le zone del soprasotto
    } else {
       struct Zona_soprasotto* scorri = prima_zona_soprasotto;
       while (scorri!=NULL){ 
        counter++;
        printf("\nNumero Zona:%d Tipo:%s\n",counter,zone[scorri->zona]);
        printf("Nemico: %s\n", nemici[scorri->nemico]);
        scorri= scorri->avanti;
    }
  }
}

  static void stampa_zona(void){
   int i,counter=0;
    if(prima_zona_mondoreale == NULL){
    system("clear");
    puts("[La mappa è vuota non la puoi stampare]");
    return;
  }
  struct Zona_mondoreale* scorri = prima_zona_mondoreale;
  while (scorri!=NULL){ 
   counter++;
   scorri= scorri->avanti;
}
  puts("Quale Zona vuoi osservare?");
  printf("\nAttualmente ci sono %d zone\n",counter);
  sleep(1);
  puts("Quale zona vuoi osservare?");
    if (scanf("%d", &i) != 1 || i < 1 || i> counter){
    puts("\nNon hai inserito un valore valido");
    while (getchar() != '\n');
    return;
    }
    //scorro il puntatore finchè non arrivo alla i desiderata
    struct Zona_mondoreale* tempM = prima_zona_mondoreale;
    for (int j = 1; j < i; j++){
      tempM= tempM->avanti;
    }
    struct Zona_soprasotto* tempS = tempM->link_soprasotto;
    puts("\n[Mondo Reale]");
    printf("\nZona %d, Tipo:%s",i, zone[tempM->zona]);
    printf("\nNemico: %s", nemici[tempM->nemico]);
    printf("\nOggetto: %s\n", oggetti[tempM->oggetto]);
    
    puts("\n[Soprasotto:]");
    printf("\nZona %d, Tipo:%s",i, zone[tempS->zona]);
    printf("\nNemico: %s\n", nemici[tempS->nemico]);
}

 static void chiudi_mappa() {
  int counter = 0;
  int boss=0;

  struct Zona_mondoreale* mondoreale = prima_zona_mondoreale;
  while(mondoreale!=NULL){ 
   counter++;
   mondoreale= mondoreale->avanti;
  }
  struct Zona_soprasotto* soprasotto= prima_zona_soprasotto;
  while(soprasotto!=NULL){
    if(soprasotto->nemico==demotorzone) {
      boss++;
    }
    soprasotto= soprasotto->avanti;
  }
  //si considera giocabile una mappa con almeno 15 zone, non meno
  if(counter<15) {
   system("clear");
   printf("[Attualmente ci sono %d zone, ne servono almeno 15]",counter);
   isOn=0;
   return; 
  }
  if(boss!=1) {
   system("clear");
   printf("Ci deve essere solo un demotorzone");
   isOn=0;
   return; 
  }

  isOn= 1;
  system("clear");
  puts("Ora puoi iniziare a giocare, buon divertimento");

}


      

       
  
