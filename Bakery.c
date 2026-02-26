#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

//Define per le tombstone
#define VUOTO 0
#define OCCUPATO 1
#define TOMB 2

//Define grandezze statiche strutture dati
#define MAX_NAME 20 //Massima lunghezza dei nomi di ingredienti o ricette
#define MAX_LINE 5000 //Massima lunghezza del buffer
#define SIZE_RICETTARIO 25000 //Grandezza iniziale Hash Table ricettario
#define SIZE_DISPENSA 4100 //Grandezza iniziale Hash Table dispensa
#define SIZE_SLOT 100 //grandezza di ogni slot della dispensa
#define SIZE_CARICO 2500 //grandezza heap ordini carico


//Struct per le strutture dati

//struct ricette e ricettario
typedef struct Lista_ingredienti{
    int indice;
    unsigned int quantity;
    struct Lista_ingredienti* next;
} Lista_ingredienti;


typedef struct Ricetta{
    char nome[MAX_NAME];
    Lista_ingredienti* lista; //testa alla lista di ingredienti necessari
} Ricetta;

typedef struct Slot_ricettario{
    Ricetta ricetta;
    unsigned int stato;
} Slot_ricettario;

typedef struct Tabella_ricette{
    Slot_ricettario hashtable[SIZE_RICETTARIO];
} Tabella_ricette;

//struct ingredienti e dispensa
typedef struct Ingrediente{
    unsigned quantity;
    unsigned scadenza;
} Ingrediente;

typedef struct Slot_dispensa{
    char nome[MAX_NAME];
    Ingrediente minheap[SIZE_SLOT];
    unsigned int n_elementi;
} Slot_dispensa;

typedef struct Tabella_dispensa{
    Slot_dispensa hashtable[SIZE_DISPENSA];
} Tabella_dispensa;

//struct organizzazione ordini (maxheap carico spedizione, coda d'attesa, coda ordini pronti) 
typedef struct Ordine{
    int indice;
    unsigned int tempo_arrivo;
    unsigned int quanti;
    unsigned int peso;
} Ordine;

typedef struct Ordine_attesa{
    int indice;
    unsigned int tempo_arrivo;
    unsigned int quanti;
    Lista_ingredienti* testa;
} Ordine_attesa;

typedef struct Max_heap{
    Ordine carico[SIZE_CARICO];
    unsigned int n_elementi;
} Max_heap;

typedef struct Nodo_pronti{
    Ordine ordine;
    struct Nodo_pronti* next;
} Nodo_pronti;

typedef struct Nodo_attesa{
    Ordine_attesa ordine;
    struct Nodo_attesa* next;
} Nodo_attesa;

typedef struct Coda_pronti{
    Nodo_pronti* testa;
    Nodo_pronti* retro;
} Coda_pronti;

typedef struct Coda_attesa{
    Nodo_attesa* testa;
    Nodo_attesa* retro;
} Coda_attesa;


//header funzioni:

//funzioni che gestiscono gli ordini
void ordine(const char* buffer);
int check_ingredienti(Lista_ingredienti* testa, const unsigned int num);
unsigned int rimuovi_ingredienti(Lista_ingredienti* testa, const unsigned int num);
int ricerca_ingrediente(const char* nome);
void prepara(const int index, const unsigned int num, const unsigned int peso, const unsigned int tempo);
void in_attesa(const int index, const unsigned int num, Lista_ingredienti* testa);

//funzioni che gestiscono le code degli ordini
void enqueue_pronti(Coda_pronti* coda, Nodo_pronti* nuovo);
void enqueue_attesa(Coda_attesa* coda, Nodo_attesa* nuovo);
Ordine dequeue_pronti(Coda_pronti* coda);
unsigned int cerca_attesa(const int indice); //funzione che cerca se è presente un ordine col nome della ricetta da rimuovere in attesa
unsigned int cerca_pronti(const int indice);
void inserisci_nodo(Coda_pronti* coda, Nodo_pronti* nuovo);
void rimuovi_nodo(Coda_attesa* coda, Nodo_attesa* curr, Nodo_attesa* prev);

//funzioni che gestiscono i rifornimenti, la hashtable della dispensa e i relativi min_heap
void rifornimento(const char* buffer);
unsigned int inserisci_dispensa(const char* nome, const unsigned int quantity, const unsigned int scadenza);
//funzione che fa il check degli ordini in attesa che possono essere preparati dopo il rifornimento
void prepara_attesa(Coda_attesa* coda);
//funzioni di gestione del min_heap
void inserisci_minheap(Slot_dispensa* heap, Ingrediente nuovo);
void min_heapify(Slot_dispensa* heap, unsigned int i);
Ingrediente estrai_min(Slot_dispensa* heap);

//funzioni che gestiscono le ricette
void rimuovi_ricetta(const char* buffer);
void aggiungi_ricetta(const char* buffer);
void inserisci_ricetta(const char* nome, const char* buffer, Slot_ricettario* slot);
void inserisci_ingrediente(const int i, const unsigned int quantity, Lista_ingredienti** testa);
int ricerca_ricettario(const char* nome);

//funzione che gestisce il carico del camioncino
void spedizione(Coda_pronti* coda);
//funzioni che gestiscono il maxheap del camioncino
void inserisci_maxheap(Ordine nuovo);
void max_heapify(unsigned int i);
Ordine estrai_max();

//funzione di hash djb2 usata sia dal ricettario che dalla dispensa
unsigned int hash_function(const char* nome);

//funzioni che deallocano la memoria dinamica
void free_attesa(Nodo_attesa* testa);
void free_pronti(Nodo_pronti* testa);
void free_lista(Lista_ingredienti* testa);

//funzioni ausiliarie
unsigned int numlen(unsigned int num);

//Variabili globali:
unsigned int tempo = 0;
unsigned int period = 0;
unsigned int capienza = 0;
Tabella_ricette ricettario;
Tabella_dispensa dispensa;
Max_heap camioncino;
Coda_attesa attesa;
Coda_pronti pronti;

/*main della funzione con il compito di inizializzare le strutture dati,
riconoscere l'input e chiamare le rispettive funzioni, tenere traccia del tempo
e chiamare la funzione spedizione, deallocare la memoria dinamica a fine input */
int main(){

    //inizializzo ricettario, dispensa, maxheap camioncino, coda attesa e coda pronti
    for (int i = 0; i < SIZE_RICETTARIO; i++){
        ricettario.hashtable[i].stato = VUOTO;
    }
    for (int i = 0; i < SIZE_DISPENSA; i++){
        dispensa.hashtable[i].n_elementi = 0;
    }
    camioncino.n_elementi = 0;
    attesa.testa = NULL;
    attesa.retro = NULL;
    pronti.testa = NULL;
    pronti.retro = NULL;

    char buffer[MAX_LINE]; //buffer che riceve l'input da stdin

    //prima chiamata a fgets riceve i valori di period e capienza
    if (!fgets(buffer, MAX_LINE, stdin)){
        printf("Errore di lettura dell'input");
        exit(EXIT_FAILURE);
    }
    sscanf(buffer, "%u %u", &period, &capienza);

    //loop che legge da stdin finché non arriva input vuoto
    while(1){
        
        //check tempo spedizione
        if (tempo != 0 && tempo % period == 0){
            spedizione(&pronti); //funzione che sposta gli ordini pronti dalla coda al camioncino
        }

        //gestione dell'input
        if (!fgets(buffer, MAX_LINE, stdin)){
            break;
        } 
        buffer[strcspn(buffer,"\n")] = '\0'; //rimpiazza \n con \0

        //chiama la funzione richiesta dall'input
        if (buffer[0] == 'o'){
            ordine(buffer + 7);
        }
        else if (buffer[0] == 'a'){
            aggiungi_ricetta(buffer + 17);
        }
        else if (buffer[0] == 'r'){
            if (buffer[2] == 'f'){
                rifornimento(buffer + 13);
            }
            else{
                rimuovi_ricetta(buffer +16);
            }
        }
        else{
            break;
        }

        //aumento il tempo di 1 ad ogni evento;
        tempo++;
    }

    //funzioni che deallocano la memoria dinamica
    /*free_attesa(attesa.testa);
    free_pronti(pronti.testa);
    for(int i = 0; i < SIZE_RICETTARIO; i++){
        if (ricettario.hashtable[i].stato == OCCUPATO)
            free_lista(ricettario.hashtable[i].ricetta.lista); //dealloca la lista ingredienti
    }*/
    return 0;
}

//funzioni che gestiscono gli ordini e relative strutture dati
void ordine(const char* buffer){
    //salvo nelle variabili nome e molteplicità dell'ordine
    char nome_ricetta[MAX_NAME];
    unsigned int num;
    sscanf(buffer, "%255s %u", nome_ricetta, &num);

    //chiamata alla funzione che cerca la ricetta nell'hashtable
    int i = ricerca_ricettario(nome_ricetta);
    if (i == -1){
        printf("rifiutato\n"); //ricetta non presente
        return;
    }
    else{
        printf("accettato\n"); //ricetta trovata
    }
    Lista_ingredienti* testa = ricettario.hashtable[i].ricetta.lista; //puntatore alla lista di ingredienti della ricetta
    //Se gli ingredienti necessari * num sono presenti in dispensa, rimuove tali ingredienti e mette la ricetta nella coda pronti
    if (check_ingredienti(testa, num)){
        unsigned int peso = rimuovi_ingredienti(testa, num); //rimuovi ingredienti restituisce la somma degli ingredienti rimossi
        prepara(i, num, peso, tempo); //inserisce l'ordine nella coda pronti
    }
    else{
        in_attesa(i, num, testa); //posiziona l'ordine in attesa insieme agli ingredienti necessari
    }

}

//controlla sia che ci siano abbastanza ingredienti sia elimina quelli scaduti
int check_ingredienti(Lista_ingredienti* testa, const unsigned int num){
    Lista_ingredienti* curr = testa;
    while (curr != NULL){
        int i = curr->indice;
        Slot_dispensa* heap = &dispensa.hashtable[i];
        if (heap->n_elementi == 0) return 0;
        while (heap->n_elementi > 0 && heap->minheap[0].scadenza <= tempo){
            estrai_min(heap);
        }
        if (heap->n_elementi == 0){
            return 0;
        }
        unsigned int necessari = curr->quantity * num;
        unsigned int presenti = 0;
        for (unsigned int j = 0; j < heap->n_elementi && presenti < necessari; ++j){
            presenti = presenti + heap->minheap[j].quantity;
        }
        if (presenti < necessari){ 
            return 0;
        }
        curr = curr->next;
    }
    return 1;
}



//funzione che viene chiamata solo dopo check_ingredienti
unsigned int rimuovi_ingredienti(Lista_ingredienti* testa, const unsigned int num) {
    Lista_ingredienti* curr = testa;
    unsigned int peso_totale = 0;
    while (curr != NULL) {
        int i = curr->indice;
        unsigned int necessari = curr->quantity * num;
        Slot_dispensa* heap = &dispensa.hashtable[i];
        unsigned int rimossi = 0;
        unsigned int scadenza_ultimo = 0;
        while (rimossi < necessari && heap->n_elementi > 0) {
            Ingrediente estratto = estrai_min(heap);
            rimossi = rimossi + estratto.quantity;
            scadenza_ultimo = estratto.scadenza;
        }
        if (rimossi > necessari) {
            Ingrediente di_troppo;
            di_troppo.scadenza = scadenza_ultimo;
            di_troppo.quantity = rimossi - necessari;
            inserisci_minheap(heap, di_troppo);
            rimossi = rimossi - di_troppo.quantity;
        }
        peso_totale = peso_totale + rimossi;
        curr = curr->next;
    }
    return peso_totale;
}

int ricerca_ingrediente(const char* nome){
    unsigned int i = hash_function(nome) % SIZE_DISPENSA;
    unsigned int inizio = i;
    int j = 0;
    while (strcmp(dispensa.hashtable[i].nome, nome) != 0){
        j++;
        i = (i + j*j) % SIZE_DISPENSA;
        if (i == inizio){
            return -1; //l'ingrediente non c'è
        }
    }
    if (dispensa.hashtable[i].n_elementi > 0){
        return i; //l'ingrediente c'è ed è presente
    }
    else{
        return -1;//l'ingrediente è esaurito
    }
}

//alloca nuovo nodo per la lista e chiama la funzione enqueue
void prepara(const int i, const unsigned int num, const unsigned int peso, const unsigned int tempo_arrivo){
    //Alloca il nuovo nodo e salva i dati in esso
    Nodo_pronti* nuovo = malloc(sizeof(Nodo_pronti));
    nuovo->ordine.indice = i;
    nuovo->ordine.peso = peso;
    nuovo->ordine.quanti = num;
    nuovo->ordine.tempo_arrivo = tempo_arrivo;
    nuovo->next = NULL;

    //Inserisce il nuovo nodo nella coda degli ordini pronti
    if (tempo_arrivo == tempo){
        enqueue_pronti(&pronti, nuovo);
    }
    else{
        inserisci_nodo(&pronti, nuovo);
    }
}

void in_attesa(const int i, const unsigned int num, Lista_ingredienti* testa){
    //Alloca il nuovo nodo e ci salva dentro i dati
    Nodo_attesa* nuovo = malloc(sizeof(Nodo_attesa));
    nuovo->ordine.indice = i;
    nuovo->ordine.quanti = num;
    nuovo->ordine.tempo_arrivo = tempo;
    nuovo->ordine.testa = testa;
    nuovo->next = NULL;
    //Inserisce il nuovo nodo nella coda d'attesa
    enqueue_attesa(&attesa, nuovo);
}

//funzioni che gestiscono le code
void enqueue_pronti(Coda_pronti* coda, Nodo_pronti* nuovo){
    //Caso coda vuota
    if (coda->retro == NULL){
        coda->testa = coda->retro = nuovo;
        return;
    }
    //Altrimenti
    coda->retro->next = nuovo;
    coda->retro = nuovo;
}

void enqueue_attesa(Coda_attesa* coda, Nodo_attesa* nuovo){
    //Caso coda vuota
    if (coda->retro == NULL){
        coda->testa = coda->retro = nuovo;
        return;
    }
    //Altrimenti
    coda->retro->next = nuovo;
    coda->retro = nuovo;
}

Ordine dequeue_pronti(Coda_pronti* coda){
    Nodo_pronti* temp = coda->testa;
    Ordine ordine = temp->ordine;
    coda->testa = coda->testa->next;

    //se il next è NULL il retro coincide con la testa
    if (coda->testa == NULL){
        coda->retro = NULL;
    }
    free(temp);
    return ordine;
}

unsigned int cerca_attesa(const int i){
    Nodo_attesa* curr = attesa.testa;
    while (curr != NULL){
        if (curr->ordine.indice == i){
            return 1;
            }
        curr = curr->next;
        }
    return 0;
}   
    
unsigned int cerca_pronti(const int i){ 
    Nodo_pronti* curr = pronti.testa;
    while (curr != NULL){
        if (curr->ordine.indice == i){
            return 1;
        }
        curr = curr->next;
    }
    return 0;
}

void inserisci_nodo(Coda_pronti* coda, Nodo_pronti* nuovo){
    Nodo_pronti* curr = coda->testa;
    // Caso 1: Lista vuota, il nuovo nodo diventa sia testa che retro
    if (coda->testa == NULL){
        coda->testa = nuovo;
        coda->retro = nuovo;
        return;
    }
    // Caso 2: Il nuovo nodo deve essere inserito in testa (tempo_arrivo più piccolo)
    if (nuovo->ordine.tempo_arrivo < curr->ordine.tempo_arrivo){
        nuovo->next = curr;
        coda->testa = nuovo;
        return;
    }
    // Caso 3: Inserimento nel mezzo o alla fine
    while (curr->next != NULL && curr->next->ordine.tempo_arrivo < nuovo->ordine.tempo_arrivo){
        curr = curr->next;
    }
    // Inserimento del nuovo nodo nella posizione trovata
    nuovo->next = curr->next;
    curr->next = nuovo;
    // Se il nuovo nodo è stato inserito alla fine, aggiorna il puntatore retro
    if (nuovo->next == NULL){
        coda->retro = nuovo;
    }
}

void rimuovi_nodo(Coda_attesa* coda, Nodo_attesa* curr, Nodo_attesa* prev){
    //Caso in cui è il primo della lista
    if (prev == NULL){
        coda->testa = curr->next;
        free(curr);
        return;
    }
    prev->next = curr->next;
    free(curr);
    //Se curr era l'ultimo nodo, cambio il puntatore alla coda della lista
    if (prev->next == NULL){
        coda->retro = prev;
    }
}

//funzioni che gestiscono i rifornimenti, la hashtable della dispensa e i relativi min_heap
void rifornimento(const char* buffer){
    //Variabili in cui salvare i parametri degli ingredienti
    char nome[MAX_NAME];
    Ingrediente nuovo;
    //Finché non viene letto '\0'
    while (*buffer){
        //salva i parametri dell'ingrediente
        sscanf(buffer, "%255s %u %u", nome, &nuovo.quantity, &nuovo.scadenza);

        //offset per il prossimo ingrediente
        buffer = buffer + strlen(nome) + numlen(nuovo.quantity) + numlen(nuovo.scadenza) + 3;
        
        //se la scandenza è minore o uguale al tempo ignoro l'ingrediente
        if (nuovo.scadenza > tempo){
            inserisci_dispensa(nome, nuovo.quantity, nuovo.scadenza);
        }

    }
    printf("rifornito\n");
    prepara_attesa(&attesa);
}

unsigned int inserisci_dispensa(const char* nome, const unsigned int quantity, const unsigned int scadenza){
    Ingrediente nuovo;
    nuovo.quantity = quantity;
    nuovo.scadenza = scadenza;  
    //Blocco di codice che si occupa del trovare l'indice adatto nella tabella
    unsigned int i = hash_function(nome) % SIZE_DISPENSA;
    int j = 0;
    while (dispensa.hashtable[i].n_elementi > 0 && strcmp(dispensa.hashtable[i].nome, nome) != 0){
        j++;
        i = (i + j*j) % SIZE_DISPENSA;
    }
    //Se il nome è diverso e non ci sono ingredienti nella tabella
    if (strcmp(dispensa.hashtable[i].nome, nome) != 0){
        strcpy (dispensa.hashtable[i].nome, nome);
            dispensa.hashtable[i].minheap[0] = nuovo;
            if (quantity != 0){
                dispensa.hashtable[i].n_elementi++;
            }
        }
    else if (dispensa.hashtable[i].n_elementi == 0){
        dispensa.hashtable[i].minheap[0] = nuovo;
        dispensa.hashtable[i].n_elementi++;
    }
    else{
        inserisci_minheap(&dispensa.hashtable[i], nuovo);
    }
    return i;
}

//Controlla quali ordini possono essere preparati tra quelli in attesa e li mette nella coda pronti togliendoli da attesa
void prepara_attesa(Coda_attesa* coda){
    Nodo_attesa* curr = coda->testa;
    Nodo_attesa* prev = NULL;
    Nodo_attesa* prox = NULL;
    while (curr != NULL){
        prox = curr->next;
        if (check_ingredienti(curr->ordine.testa, curr->ordine.quanti)){
            unsigned int peso = rimuovi_ingredienti(curr->ordine.testa, curr->ordine.quanti);
            prepara(curr->ordine.indice, curr->ordine.quanti, peso, curr->ordine.tempo_arrivo);
            rimuovi_nodo(&attesa, curr, prev);
        }
        else{
            prev = curr;
        }
        curr = prox;
    }
}

//funzioni di gestione del min_heap
void inserisci_minheap(Slot_dispensa* heap, Ingrediente nuovo){
    unsigned int i = heap->n_elementi; //indice della fine dell'array
    //inserisco il nuovo elemento all'indice i
    heap->minheap[i] = nuovo;
    heap->n_elementi++;
    //riordino il minheap risalendo l'albero
    Ingrediente temp;
    while (i > 0 && heap->minheap[(i - 1) / 2].scadenza > heap->minheap[i].scadenza){
        //scambia con il genitore
        temp = heap->minheap[i];
        heap->minheap[i] = heap->minheap[(i - 1)/ 2];
        heap->minheap[(i - 1) / 2] = temp;

        i = (i - 1) / 2;
    }
}

void min_heapify(Slot_dispensa* heap, unsigned int i){
    unsigned int minimo = i;
    while (1) {
        unsigned int left = 2 * i + 1;
        unsigned int right = 2 * i + 2;
        // Compara col figlio sinistro
        if (left < heap->n_elementi && heap->minheap[left].scadenza < heap->minheap[minimo].scadenza){
            minimo = left;
        }
        // Compara col figlio destro
        if (right < heap->n_elementi && heap->minheap[right].scadenza < heap->minheap[minimo].scadenza){
            minimo = right;
        }
        // Se il minimo è cambiato, scambia ed aggiorna l'indice i
        if (minimo != i){
            Ingrediente temp = heap->minheap[i];
            heap->minheap[i] = heap->minheap[minimo];
            heap->minheap[minimo] = temp;
            i = minimo;  // Continua a riordinare dal nodo appena scambiato
        } else{
            break;  // Non serve più riordinare, esci dal ciclo
        }
    }
}

Ingrediente estrai_min(Slot_dispensa* heap){
    Ingrediente min = heap->minheap[0];
    //Caso n_elementi == 1
    if (heap->n_elementi == 1){
        heap->n_elementi--;
        heap->minheap[0].quantity = 0;
        heap->minheap[0].scadenza = 0;
        return min;
    }
    //sostituisce la radice con l'ultimo elemento
    unsigned int i = heap->n_elementi;
    heap->minheap[0] = heap->minheap[i - 1];
    heap->n_elementi--;
    //Riporta l'ultimo elemento fuori dall'heap a 0
    heap->minheap[i - 1].quantity = 0;
    heap->minheap[i - 1].scadenza = 0;
    //Riordina il min-heap
    min_heapify(heap, 0);
    //Restituisce il minimo salvato all'inizio
    return min;
}

//funzioni che si occupano di gestire il ricettario
void rimuovi_ricetta(const char* buffer){
    char nome[MAX_NAME];
    sscanf(buffer, "%255s", nome);
    int i = ricerca_ricettario(nome);
    //se non trove la ricetta nel ricettario
    if (i == -1){
        printf("non presente\n");
        return;
    }
    //non rimuove la ricetta se ci sono ordini in sospeso o pronti
    if (cerca_attesa(i) || cerca_pronti(i)){
        printf("ordini in sospeso\n");
        return;
    }
    free_lista(ricettario.hashtable[i].ricetta.lista);
    ricettario.hashtable[i].ricetta.lista = NULL;
    ricettario.hashtable[i].stato = TOMB;
    printf("rimossa\n");
}

void aggiungi_ricetta(const char* buffer){
    char nome[MAX_NAME];
    sscanf(buffer, "%255s", nome);
    //se la ricerca restituisce qualcosa di diverso da -1, allora la ricetta è già presente
    if (ricerca_ricettario(nome) != -1){
        printf("ignorato\n");
        return;
    }
    else{
        unsigned int i = hash_function(nome) % SIZE_RICETTARIO;
        int j = 0;
        int primo_tomb = -1; // traccia il primo TOMB per riutilizzo ottimale
        
        // Continua finché non trovi uno slot VUOTO
        while (ricettario.hashtable[i].stato != VUOTO){
            if (ricettario.hashtable[i].stato == TOMB && primo_tomb == -1){
                primo_tomb = i; // salva il primo TOMB trovato
            }
            j++;
            i = (i + j*j) % SIZE_RICETTARIO;
        }
        
        // Se hai trovato un TOMB durante il probing, riusalo (migliore per la ricerca)
        if (primo_tomb != -1){
            i = primo_tomb;
        }
        
        inserisci_ricetta(nome, buffer + strlen(nome) + 1, &ricettario.hashtable[i]);
        printf("aggiunta\n");
    }
}

void inserisci_ricetta(const char* nome, const char* buffer, Slot_ricettario* slot){
    strcpy(slot->ricetta.nome, nome);
    slot->stato = OCCUPATO;
     //inserimento lista ingredienti
    char nome_ingrediente[MAX_NAME];
    unsigned int quantity;
    while (*buffer){
        sscanf(buffer,"%255s %u", nome_ingrediente, &quantity);
        buffer = buffer + strlen(nome_ingrediente) + numlen(quantity) + 2;
        int i = ricerca_ingrediente(nome_ingrediente);
        if (i == -1){
            i = inserisci_dispensa(nome_ingrediente, 0, 0);
        }
        inserisci_ingrediente(i , quantity, &slot->ricetta.lista);
    }

}

void inserisci_ingrediente(const int i, const unsigned int quantity, Lista_ingredienti** testa){
    Lista_ingredienti* nuovo = malloc(sizeof(Lista_ingredienti));
    nuovo->indice = i;
    nuovo->quantity = quantity;
    nuovo->next = NULL;
    //se la lista è vuota
    if (*testa == NULL){
        *testa = nuovo;
    }
    else{ //altrimenti
        nuovo->next = *testa;
        *testa = nuovo;
    }
}

int ricerca_ricettario(const char* nome){
    unsigned int i = hash_function(nome) % SIZE_RICETTARIO;
    unsigned int inizio = i;
    int j = 0;
    while (ricettario.hashtable[i].stato == TOMB ||
            (ricettario.hashtable[i].stato == OCCUPATO && strcmp(ricettario.hashtable[i].ricetta.nome, nome) != 0)){
                j++;
                i = (i + j*j) % SIZE_RICETTARIO;
                if (i == inizio){
                    return -1; //l'hashtable è praticamente piena e la ricetta non è presente
                }
        }
    if (ricettario.hashtable[i].stato == OCCUPATO && strcmp(ricettario.hashtable[i].ricetta.nome, nome) == 0){
        return i; //la ricetta si trova all'indice i
    }
    else{
        return -1;
    }
}

//funzione che gestisce la spedizione del camioncino
void spedizione(Coda_pronti* coda){
    Nodo_pronti* curr = coda->testa;
    unsigned int totale = 0;
    //inserisce gli ordini pronti nel camioncino finché il prox ordine non supera la capienza massima
    while (curr != NULL && totale + curr->ordine.peso <= capienza){
        inserisci_maxheap(curr->ordine);
        totale = totale + curr->ordine.peso;
        dequeue_pronti(coda);
        curr = coda->testa;
    }
    if (camioncino.n_elementi == 0){
        printf("camioncino vuoto\n");
        return;
    }
    Ordine da_caricare;
    while (camioncino.n_elementi > 0){
        da_caricare = estrai_max();
        printf("%u %s %u\n", da_caricare.tempo_arrivo, ricettario.hashtable[da_caricare.indice].ricetta.nome, da_caricare.quanti);
    }
}

//funzioni di gestione del max_heap
void inserisci_maxheap(Ordine nuovo){
    unsigned int i = camioncino.n_elementi; //indice della fine dell'array
    //inserisco il nuovo elemento all'indice i
    camioncino.carico[i] = nuovo;
    camioncino.n_elementi++;
    //riordino il maxheap risalendo l'albero
    Ordine temp;
    while (i > 0 && camioncino.carico[(i - 1) / 2].peso < camioncino.carico[i].peso){
        //scambia con il genitore
        temp = camioncino.carico[i];
        camioncino.carico[i] = camioncino.carico[(i - 1)/ 2];
        camioncino.carico[(i - 1) / 2] = temp;

        i = (i - 1) / 2;
    }
}

void max_heapify(unsigned int i){
    unsigned int massimo = i;
    while (1){
        unsigned int left = 2 * i + 1;
        unsigned int right = 2 * i + 2;
        // Compara col figlio sinistro
        if (left < camioncino.n_elementi){
            if (camioncino.carico[left].peso > camioncino.carico[massimo].peso ||
               (camioncino.carico[left].peso == camioncino.carico[massimo].peso &&
                camioncino.carico[left].tempo_arrivo < camioncino.carico[massimo].tempo_arrivo)){
                massimo = left;
            }
        }
        // Compara col figlio destro
        if (right < camioncino.n_elementi){
            if (camioncino.carico[right].peso > camioncino.carico[massimo].peso ||
               (camioncino.carico[right].peso == camioncino.carico[massimo].peso &&
                camioncino.carico[right].tempo_arrivo < camioncino.carico[massimo].tempo_arrivo)){
                massimo = right;
            }
        }
        // Se il massimo è cambiato, scambia ed aggiorna l'indice i
        if (massimo != i){
            Ordine temp = camioncino.carico[i];
            camioncino.carico[i] = camioncino.carico[massimo];
            camioncino.carico[massimo] = temp;
            i = massimo;
        } else{
            break;  // Non serve più riordinare, esci dal ciclo
        }
    }
}


Ordine estrai_max(){
    Ordine max = camioncino.carico[0];
    //Caso n_elementi == 1
    if (camioncino.n_elementi == 1){
        camioncino.n_elementi--;
        camioncino.carico[0].indice = -1;
        camioncino.carico[0].peso = 0;
        camioncino.carico[0].tempo_arrivo = 0;
        camioncino.carico[0].quanti = 0;
        return max;
    }
    //sostituisce la radice con l'ultimo elemento
    unsigned int i = camioncino.n_elementi;
    camioncino.carico[0] = camioncino.carico[i - 1];
    camioncino.n_elementi--;
    //Riporta l'ultimo elemento fuori dall'heap a 0
    camioncino.carico[i].indice = -1;
    camioncino.carico[i].peso = 0;
    camioncino.carico[i].tempo_arrivo = 0;
    camioncino.carico[i].quanti = 0;
    //Riordina il max-heap
    max_heapify(0);
    //Restituisce il massimo salvato all'inizio
    return max;
}

//funzione di hash djb2
unsigned int hash_function(const char* nome){
    unsigned int hash = 5381;
    int c;
    while ((c = *nome++)){
        if (isupper(c)){
            c = c + 32;
        }
        hash = ((hash << 5) + hash) + c;
    }
    return hash;
}

//funzioni che liberano la memoria allocata
void free_attesa(Nodo_attesa* testa){
    Nodo_attesa* curr = testa;
    Nodo_attesa* prox;
    while (curr != NULL){
        prox = curr->next;
        free(curr);
        curr = prox;
    }
}

void free_pronti(Nodo_pronti* testa){
    Nodo_pronti* curr = testa;
    Nodo_pronti* prox;
    while (curr != NULL){
        prox = curr->next;
        free(curr);
        curr = prox;
    }
}

void free_lista(Lista_ingredienti* testa){
    Lista_ingredienti* curr = testa;
    Lista_ingredienti* prox;
    while (curr != NULL){
        prox = curr->next;
        free(curr);
        curr = prox;
    } 
}

//funzione ausiliaria che calcola la lunghezza in cifre di un numero intero positivo
unsigned int numlen(unsigned int num){
    unsigned int count = 0;
    if (num == 0){
        return 1;
    }
    else{
        while (num != 0){
            num = num / 10;
            count++;
        }
        return count;
    }
}