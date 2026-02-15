#include "gamelib.h"
#include <stdio.h>
#include <stdlib.h>


// Questo file continene solo la definizione della funzione main
// con il menu principale 1-2-3

int main(void) {
    unsigned short int scelta= 0;
    system("clear");
    puts("Benvenuto in Cose Strane\n");
     

    do {
    puts("\n----Menù di gioco----\n");
    puts("1.Imposta gioco");
    puts("2.Gioca");
    puts("3.Termina gioco");
    puts("4.Mostra Crediti");
    
    //controllo della scanf
    if(scanf("%hu", &scelta)!= 1) {
       puts("");
       puts("[INSERISCI UN NUMERO NON LETTERE]"); 
       while(getchar() != '\n'); //pulizia stdin
       getchar(); //ciò che inserisce l'utente fa tornare ad una condizione normale, pertanto non è proprio un vero input
       system("clear");
       continue; //riprendo il ciclo do while
   };
    //pulizia schermo dopo un errore
    system("clear");  
    
        switch (scelta)
        {
            case 1:
            imposta_gioco();
            break;
        
            case 2:
            gioca();
            break;


            case 3:
            termina_gioco();
            break;
             
            case 4:
            crediti();
            break;
       
    
            default:
            puts("Errore, inserisci un numero da 1 a 4 per giocare: \n\n");          
            break;  

        }
    } while(scelta!= 3);

    


   return 0;
}
