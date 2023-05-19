#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int Id, Id2, Id3; /* identifikacijski broj segmenta */
int *ZajednickaVarijabla;
int *flags;
int *turn;
 

void brisi(int sig)
{
   /* oslobađanje zajedničke memorije */
   (void) shmdt((char *) ZajednickaVarijabla);
   (void) shmctl(Id, IPC_RMID, NULL);
   (void) shmdt((char *) flags);
   (void) shmctl(Id2, IPC_RMID, NULL);
   (void) shmdt((char *) turn);
   (void) shmctl(Id3, IPC_RMID, NULL);
   exit(0);
}
int main(void)
{
   /* zauzimanje zajedničke memorije */
    Id = shmget(IPC_PRIVATE, sizeof(int), 0600);
    
    if (Id == -1)
        exit(1);  /* greška - nema zajedničke memorije */
    
    ZajednickaVarijabla = (int *) shmat(Id, NULL, 0);
    *ZajednickaVarijabla = 0;

    Id2 = shmget(IPC_PRIVATE, sizeof(int)*2, 0600);
    if(Id2 == -1)
            exit(1);
    flags = (int *) shmat(Id2, NULL, 0);
    flags[0] = 0;
    flags[1] = 0;

    Id3 = shmget(IPC_PRIVATE, sizeof(int), 0600);
    if(Id3 == -1)
        exit(1);
    turn = (int *) shmat(Id3, NULL, 0);
    *turn = 0;

    struct sigaction act;
    
	act.sa_handler = brisi;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	sigaction(SIGINT, &act, NULL);//u slučaju prekida briši memoriju
    
    int m;
    printf("upisi broj M: ");
    scanf("%d", &m);

    int pid = fork();
    if(pid == -1)
        exit(1);
    for(int i = 0; i < m; i++){
        //sleep(1);
        if(pid == 0){
            flags[1] = 1;
            while(flags[0] == 1){
                if((*turn) != 1){
                    flags[1] = 0;
                    while((*turn) != 1){
                        
                    }
                    flags[1] = 1;
                }
            }
            (*ZajednickaVarijabla)++;
            printf("Dijete je povecalo s %d na %d\n", *ZajednickaVarijabla-1, *ZajednickaVarijabla);
            //sleep(1);
            *turn = 0;
            flags[1] = 0;
        }else{
            flags[0] = 1;
            while(flags[1] == 1){
                if((*turn) != 0){
                    flags[0] = 0;
                    while((*turn) != 0){
                        
                    }
                    flags[0] = 1;
                }
            }
            (*ZajednickaVarijabla)++;
            printf("Roditelj je povecao s %d na %d\n", *ZajednickaVarijabla-1, *ZajednickaVarijabla);
            //sleep(1);
            *turn = 1;
            flags[0] = 0;
        }
    }
    
    wait(NULL);
    brisi(0);
    
    return 0;
}