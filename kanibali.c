#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>

typedef struct Osoba {
    char vrsta;
    int broj;
} Osoba;

typedef struct Node {
    Osoba data;
    struct Node *prev;
    struct Node *next;
} Node;

typedef struct List {
    Node *head;
    Node *tail;
} List;

// Create a new node with the given data
Node *create_node(Osoba data) {
    Node *new_node = (Node *) malloc(sizeof(Node));
    new_node->data = data;
    new_node->prev = NULL;
    new_node->next = NULL;
    return new_node;
}

// Initialize a new doubly linked list
List *initialize_list() {
    List *new_list = (List *) malloc(sizeof(List));
    new_list->head = NULL;
    new_list->tail = NULL;
    return new_list;
}

// Add a new element to the end of the list
void add_element(List *list, Osoba data) {
    Node *new_node = create_node(data);
    if (list->tail == NULL) {
        // If the list is empty, set the new node as both the head and tail
        list->head = new_node;
        list->tail = new_node;
    } else {
        // Otherwise, add the new node to the end of the list
        list->tail->next = new_node;
        new_node->prev = list->tail;
        list->tail = new_node;
    }
}

// Remove the first node in the list with the given data
void remove_element(List *list, Osoba data) {
    Node *current = list->head;
    while (current != NULL) {
        if (current->data.vrsta == data.vrsta && current->data.broj == data.broj) {
            if (current == list->head) {
                // If the node to be removed is the head, update the head
                list->head = current->next;
                if (list->head != NULL) {
                    list->head->prev = NULL;
                }else{
                    list->tail = NULL;
                }
            } else if (current == list->tail) {
                // If the node to be removed is the tail, update the tail
                list->tail = current->prev;
                if (list->tail != NULL) {
                    list->tail->next = NULL;
                }
            } else {
                // If the node to be removed is in the middle of the list, update the neighboring nodes
                current->prev->next = current->next;
                current->next->prev = current->prev;
            }
            free(current);
            return;
        }
        current = current->next;
    }
}


pthread_mutex_t m;
pthread_cond_t red[6];
int camac[7];
int camac_vrsta[7];
int br_misionara = 0;
int br_kanibala = 0;
int strana_camca = 0;
int camac_misionari = 0;
int camac_kanibali = 0;
int broj_ljudi = 0;
int camac_prevozi = 0;
pthread_attr_t attr;
pthread_t thr_id;
List *lijeva_strana;
List *desna_strana;

void ispis(){
    char *strana = strana_camca ? "L" : "D";
    printf("\tc[%s]:{", strana);
    for(int i = 0; i < camac_kanibali + camac_misionari; i++){
        char *vrsta = camac_vrsta[i] ? "K" : "M";
        printf("%s%d ", vrsta, camac[i]);
    }
    printf("}  ");
    printf("DO:{");
    Node *current = desna_strana->head;
    while(current != NULL){
        printf("%c%d ", current->data.vrsta, current->data.broj);
        current = current->next;
    } 
    printf("}  ");
    printf("LO:{");
    current = lijeva_strana->head;
    while(current != NULL){
        printf("%c%d ", current->data.vrsta, current->data.broj);
        current = current->next;
    } 
    printf("}\n");
    
}
void ispisCamac(){
    for(int i = 0; i < camac_kanibali + camac_misionari; i++){
        char *vrsta = camac_vrsta[i] ? "K" : "M";
        printf("%s%d ", vrsta, camac[i]);
    }
    printf("\n");
}

void *kanibal(void *arg){
    int strana = rand() & 1;
    int br = ++br_kanibala;
    int id;
    pthread_mutex_lock(&m);
    broj_ljudi++;
    char *txt_strana = strana ? "lijevu" : "desnu";
    printf("K%d: došao na %s stranu\n", br, txt_strana);
    Osoba kanibal = {'K', br};
    if(strana == 1){
        add_element(lijeva_strana, kanibal);
    }else{
        add_element(desna_strana, kanibal);
    }
    ispis();
    while(camac_prevozi == 1 || strana_camca != strana || (camac_misionari != 0 && camac_misionari - camac_kanibali == 0) || camac_misionari+camac_kanibali>=7){
        pthread_cond_wait(&red[3-strana], &m);
    }
    camac_kanibali++;
    if(strana == 1){
        remove_element(lijeva_strana, kanibal);
    }else{
        remove_element(desna_strana, kanibal);
    }
    printf("K%d: ušao u čamac\n", br);
    if(camac_misionari+camac_kanibali >= 3){
        pthread_cond_signal(&red[5]);
    }
    camac[camac_kanibali+camac_misionari-1] = br;
    camac_vrsta[camac_kanibali+camac_misionari-1] = 1;
    ispis();
    pthread_cond_wait(&red[4], &m);
    broj_ljudi--;
    pthread_mutex_unlock(&m);
}

void *misionar(void *arg){
    int strana = rand() & 1;
    int br = ++br_misionara;
    
    pthread_mutex_lock(&m);
    broj_ljudi++;
    char *txt_strana = strana ? "lijevu" : "desnu";
    printf("M%d: došao na %s stranu\n", br, txt_strana);
    Osoba misionar = {'M', br};
    if(strana == 1){
        add_element(lijeva_strana, misionar);
    }else{
        add_element(desna_strana, misionar);
    }
    ispis();
    while(camac_prevozi == 1 || strana_camca != strana || (camac_kanibali-camac_misionari >= 2 || camac_kanibali+camac_misionari >= 7)){
        pthread_cond_wait(&red[1-strana], &m);
    }
    camac_misionari++;
    if(strana == 1){
        remove_element(lijeva_strana, misionar);
    }else{
        remove_element(desna_strana, misionar);
    }
    printf("M%d: ušao u čamac\n", br);
    if(camac_misionari+camac_kanibali >= 3){
        pthread_cond_signal(&red[5]);
        pthread_cond_signal(&red[3-strana]);
    }
    camac[camac_kanibali+camac_misionari-1] = br;
    camac_vrsta[camac_kanibali+camac_misionari-1] = 0;
    ispis();
    pthread_cond_wait(&red[4], &m);
    broj_ljudi--;
    pthread_mutex_unlock(&m);
}

void *boat(void *arg){
    printf("C: prazan na desnoj strani.\n");
    while(1){
        pthread_mutex_lock(&m);
        char *txt_strana = strana_camca ? "lijevoj" : "desnoj";
        ispis();
        while(camac_kanibali + camac_misionari < 3){
            pthread_cond_wait(&red[5], &m);
        }
        printf("C: Polazim za 1 sekundu \n");
        ispis();
        pthread_mutex_unlock(&m);
        sleep(1);
        pthread_mutex_lock(&m);
        printf("C: polazim s %s na %s stranu\n", strana_camca?"lijeve":"desne", strana_camca?"desnu":"lijevu");
        camac_prevozi = 1;
        pthread_mutex_unlock(&m);
        sleep(2);
        pthread_mutex_lock(&m);
        strana_camca = 1 - strana_camca;
        printf("C: Došao na %s stranu: ", strana_camca?"lijevu":"desnu");
        ispisCamac();
        printf("C: prazan na %s strani.\n", strana_camca?"lijevoj":"desnoj");
        camac_prevozi = 0;
        pthread_cond_broadcast(&red[4]);
        camac_kanibali = 0;
        camac_misionari = 0;
        pthread_cond_broadcast(&red[1-strana_camca]);
        pthread_cond_broadcast(&red[3-strana_camca]);
        pthread_mutex_unlock(&m);
    }
}

void *stvaraj_kanibale(void *arg){
    while(1){
        pthread_create(&thr_id, &attr, kanibal, NULL);
        sleep(1);
    }
}
void *stvaraj_misionare(void *arg){
    while(1){
        pthread_create(&thr_id, &attr, misionar, NULL);
        sleep(2);
    }
}



int main(){
    lijeva_strana = initialize_list();
    desna_strana = initialize_list();
    

    pthread_mutex_init(&m, NULL);
    pthread_cond_init(&red[0], NULL);
    pthread_cond_init(&red[1], NULL);
    pthread_cond_init(&red[2], NULL);
    pthread_cond_init(&red[3], NULL);
    pthread_cond_init(&red[4], NULL);
    pthread_cond_init(&red[5], NULL);

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    srand ((unsigned int) time(NULL));

    //stvori čamac
    pthread_create(&thr_id, &attr, boat, NULL);

    //stvori dretve koje stvaraju misionare i kanibale
    pthread_create(&thr_id, &attr, stvaraj_kanibale, NULL);
    pthread_create(&thr_id, &attr, stvaraj_misionare, NULL);

    while(broj_ljudi != -1){
        sleep(1);
    }
    pthread_mutex_destroy(&m);
    pthread_cond_destroy(&red[0]);
    pthread_cond_destroy(&red[1]);
    pthread_cond_destroy(&red[2]);
    pthread_cond_destroy(&red[3]);
    pthread_cond_destroy(&red[4]);
    pthread_cond_destroy(&red[5]);

    return 0;
}