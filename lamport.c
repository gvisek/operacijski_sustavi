#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdatomic.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#define MEMBAR __sync_synchronize()

//void lock(int i);
//void unlock(int i);
void *work(void *x);
int max();

int ZajednickaVarijabla;
atomic_int *entering;
atomic_int *number;
atomic_int *memory;

int m, n;

void brisi(int sig)
{
   free(number);
   free(entering);
   exit(0);
}

int main(void)
{
   int i;

   printf("upisi n: ");
   scanf("%d", &n);
   printf("upisi m: ");
   scanf("%d", &m);
   
   struct sigaction act;
   act.sa_handler = brisi;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	sigaction(SIGINT, &act, NULL);//u slučaju prekida briši memoriju
   
   pthread_t thr_id[n];
   entering = calloc(n, sizeof(atomic_int));
   number = calloc(n, sizeof(atomic_int));
   ZajednickaVarijabla = 0;

   
   int threadNumber[n];
   for(i = 0; i < n; i++){
      threadNumber[i] = i;
      pthread_create(&thr_id[i], NULL, work, &threadNumber[i]);
   }
   for(i = 0; i < n; i++){
      pthread_join(thr_id[i], NULL);
   }
   printf("a: %d\n", ZajednickaVarijabla);
   fflush(stdout);
   

   free(number);
   free(entering);

   return 0;
}

// void lock(int i){
//    entering[i] = 1;
//    number[i] = max(5) + 1;
//    entering[i] = 0;

//    for(int j = 0; j < n; j++) {
//       while(entering[j] != 0);
//       while(number[j] != 0 && (number[j] < number[i] || (number[j] == number[i] && j < i)));
//         }
// }

// void unlock(int i){
//    number[i] = 0;
// }

void *work(void *x){
  int i = *((int*) x);
    
    printf("Stvorio sam %d dretvu\n", i);
    for (int z = 0; z < m; z++) {
        entering[i] = 1;
        number[i] = max() + 1;
        entering[i] = 0;

        for(int j = 0; j < n; j++) {
            while(entering[j] != 0);
            while(number[j] != 0 && (number[j] < number[i] || (number[j] == number[i] && j < i)));
        }

        ZajednickaVarijabla = ZajednickaVarijabla + 1;
        //printf("dretva %d a= %d\n", i, ZajednickaVarijabla);
        number[i] = 0;
    }

    return NULL;
}

int max(){
   int max = number[0];
   for(int i = 1; i < n; i++){
      if(max < number[i])
         max = number[i];
   }
   return max;
}