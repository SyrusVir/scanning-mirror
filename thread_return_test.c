#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>

void* test_func(void* arg)
{
    pthread_mutex_t lock = (pthread_mutex_t*)arg;
    sleep(5);
    for(int i = 0; i < 10; i++)
    {
        pthread_mutex_lock(&lock);
        printf("Child Locked\n");
        pthread_mutex_unlock(&lock);
        printf("Child unLocked\n");

    }
    
    return NULL;
}

int main() 
{
    pthread_t tid;
    pthread_mutex_t lock;
    pthread_mutex_init(&lock, NULL);
   
    pthread_mutex_t lock_copy = lock;
   
    pthread_mutex_t lock_dummy;
    pthread_mutex_init(&lock_dummy,NULL);
    
    pthread_create(&tid, NULL, &test_func,&lock);

    for(int i = 0; i < 10; i++)
    {
        pthread_mutex_lock(&lock);
        printf("Parent Locked\n");
        sleep(10);
        pthread_mutex_unlock(&lock);
        printf("Parent unLocked\n"); 
        sleep(2);
    }

    pthread_join(tid, NULL);
}