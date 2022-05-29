#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>
#include <pthread.h>
#include <string.h>

pthread_mutex_t readLock=PTHREAD_MUTEX_INITIALIZER;
char* SENTINEL_VALUE="PLEASE WORK!!";
typedef struct node{
    char* value;
    struct node *next;
} node;
typedef struct queue{
    node *head;
    node *tail;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
}queue;
void queue_init(queue *q){
    q->head=q->tail=NULL;

    pthread_mutex_init(&q->mutex,NULL);
    pthread_cond_init(&q->cond,NULL);
}
//Conditional variables from https://www.youtube.com/watch?v=0sVGnxg6Z3k&t=604s
//Queue inspired from https://www.youtube.com/watch?v=FcIubL92gaI&t=390s
//Jamie also helps me to recognize problem with the queues
void queue_enqueue(queue *q,char* str){
    node *tmp=malloc(sizeof(node));
    if (tmp!=NULL) {
        tmp->value = str;
        tmp->next = NULL;
        pthread_mutex_lock(&q->mutex);

        if (q->tail != NULL) {
            q->tail->next = tmp;
        }
        q->tail = tmp;
        if (q->head == NULL) {
            q->head = tmp;
        }
        pthread_cond_signal(&q->cond);
        pthread_mutex_unlock(&q->mutex);
    }

}
void* queue_dequeue(queue *q){
    pthread_mutex_lock(&q->mutex);
    while (q->head==NULL)
        pthread_cond_wait(&q->cond,&q->mutex);
    node *tmp=q->head;
    char *words=tmp->value;
    q->head=q->head->next;
    if(q->head==NULL){
        q->tail=NULL;
    }
    free(tmp);
    pthread_mutex_unlock(&q->mutex);
    return words;
}
int bottomTwoBits(char c){
    return c&3;
}

#define BUCKET_COUNT 256
struct hashtable_s{
    struct list_s *heads[BUCKET_COUNT];
    pthread_mutex_t lock;
};
struct list_s{
    struct list_s *next;
    char *str;
    int count;
};

void add_to_list(struct list_s **head, char* str){
    struct list_s *new_node;
    new_node=*head;
    while (new_node!=NULL){
//    for (new_node=*head;new_node!=NULL;new_node=new_node->next){
        if (strcmp(new_node->str,str)==0){
            new_node->count+=1;
            break;
        }
        new_node=new_node->next;
    }
    if (new_node==NULL){
        struct list_s *new_node1=malloc(sizeof *new_node1);
        new_node1->str=strdup(str);
        new_node1->count=1;
        new_node1->next=*head;
        *head=new_node1;

    }
//    new_node->str=strdup(str); //malloc(strlen(str)+1; strcpy(addr,str);
//    new_node->next=*head;
//    *head=new_node;
}
int hash(char *str){
    int total=0;
    for (int i=0;str[i]!='\0';i++){
        total=(total+(unsigned char) str[i])%BUCKET_COUNT;
    }
    return total;
}
void add_to_hashtable(struct hashtable_s *table,char *str){
    int h=hash(str);
    pthread_mutex_lock(&table->lock);
    add_to_list(&table->heads[h],str);
    pthread_mutex_unlock(&table->lock);
}
struct readFileParams{
    char *fileName;
    queue *q1;
    queue *q2;
    queue *q3;
    queue *q4;
};
void *readFile(void* arg){
    struct readFileParams *params=arg;
    char *fileName= strdup(params->fileName);
    queue *q1=params->q1;
    queue *q2=params->q2;
    queue *q3=params->q3;
    queue *q4=params->q4;
    FILE *file;
    file= fopen(fileName,"r");
    if (file==NULL){
        perror(fileName);
    }
    else {
        char *word_buffer;
        while (fscanf(file, "%ms", &word_buffer) != EOF) {
            if (bottomTwoBits(word_buffer[0]) == 0) {
                queue_enqueue(q1, word_buffer);
            } else if (bottomTwoBits(word_buffer[0]) == 1) {
                queue_enqueue(q2, word_buffer);
            } else if (bottomTwoBits(word_buffer[0]) == 2) {
                queue_enqueue(q3, word_buffer);
            } else {
                queue_enqueue(q4, word_buffer);
            }
        }
        free(word_buffer);
        fclose(file);
    }
    free(params);
}


struct queueValueParams{
    queue *q;
    struct hashtable_s* table;
};
//Threads return value fromhttps://www.youtube.com/watch?v=ln3el6PR__Q&list=PLfqABt5AS4FmuQf70psXrsMLEDQXNkLq2&index=6
void *queueValue(void *arg){
    int maxCount=0;
    struct queueValueParams *params=arg;
    queue *q=params->q;
    struct hashtable_s* table=params->table;
    node *tmp=q->head;
    while (strcmp(tmp->value,SENTINEL_VALUE)!=0){
        pthread_mutex_lock(&readLock);
        char* word= queue_dequeue(q);
//        add_to_hashtable(table,tmp->value);
        add_to_hashtable(table,word);
//        tmp=tmp->next;
        tmp=q->head;
        pthread_mutex_unlock(&readLock);
    }
    for (int i=0;i<BUCKET_COUNT;i++){
        for (struct list_s *ptr=table->heads[i];ptr;ptr=ptr->next){
            if (maxCount<ptr->count){
                maxCount=ptr->count;
            }
        }
    }
    int* returnMax=malloc(sizeof(int));
    *returnMax=maxCount;
    free(params);
    return (void*) returnMax;
}

int main(int argc, char **argv) {
    pthread_t p1,p2,p3,p4;
    pthread_mutex_init(&readLock,NULL);
    queue q1,q2,q3,q4;
    int *maxCount = 0;
//    queue *q[4];
//    for (int i=0;i<4;i++){
//        queue_init(q[i]);
//    }
    queue_init(&q1);
    queue_init(&q2);
    queue_init(&q3);
    queue_init(&q4);
    int* returnCount1;
    int* returnCount2;
    int* returnCount3;
    int* returnCount4;
    int fileNum=argc-1;
    pthread_t th_files[fileNum];
    struct hashtable_s table1,table2,table3,table4;
    memset(table1.heads,0,sizeof(table1.heads));
    memset(table2.heads,0,sizeof(table2.heads));
    memset(table3.heads,0,sizeof(table3.heads));
    memset(table4.heads,0,sizeof(table4.heads));
    pthread_mutex_init(&table1.lock,NULL);
    pthread_mutex_init(&table2.lock,NULL);
    pthread_mutex_init(&table3.lock,NULL);
    pthread_mutex_init(&table4.lock,NULL);
    for (int i=1;i<fileNum+1;i++){
        struct readFileParams *params=malloc(sizeof *params);
        params->fileName=strdup(argv[i]);
        params->q1=&q1;
        params->q2=&q2;
        params->q3=&q3;
        params->q4=&q4;
        pthread_create(&th_files[i-1],NULL,&readFile,(void*)params);
    }
    for (int i=0;i<fileNum;i++){
        pthread_join(th_files[i],NULL);
    }
    queue_enqueue(&q1, SENTINEL_VALUE);
    queue_enqueue(&q2, SENTINEL_VALUE);
    queue_enqueue(&q3, SENTINEL_VALUE);
    queue_enqueue(&q4, SENTINEL_VALUE);
    struct queueValueParams *params1=malloc(sizeof *params1);
    params1->table=&table1;
    params1->q=&q1;
    struct queueValueParams *params2=malloc(sizeof *params2);
    params2->table=&table2;
    params2->q=&q2;
    struct queueValueParams *params3=malloc(sizeof *params3);
    params3->table=&table3;
    params3->q=&q3;
    struct queueValueParams *params4=malloc(sizeof *params4);
    params4->table=&table4;
    params4->q=&q4;

    pthread_create(&p1,NULL,&queueValue,(void*)params1);
    pthread_create(&p2,NULL,&queueValue,(void*)params2);
    pthread_create(&p3,NULL,&queueValue,(void*)params3);
    pthread_create(&p4,NULL,&queueValue,(void*)params4);
    pthread_join(p1, (void **) &returnCount1);
    pthread_join(p2, (void **) &returnCount2);
    pthread_join(p3, (void **) &returnCount3);
    pthread_join(p4, (void **) &returnCount4);
    if (maxCount < returnCount1) {
        maxCount = returnCount1;
    }
    if (maxCount < returnCount2)  {
        maxCount = returnCount2;
    }
    if (maxCount < returnCount3) {
        maxCount = returnCount3;
    }
    if (maxCount < returnCount4){
        maxCount = returnCount4;
    }
    for (int i = 0; i < BUCKET_COUNT; i++) {
        for (struct list_s *ptr = table1.heads[i]; ptr; ptr = ptr->next) {
            if (*maxCount == ptr->count) {
                printf("%s %d\n", ptr->str, *maxCount);
            }
        }
    }
    for (int i = 0; i < BUCKET_COUNT; i++) {
        for (struct list_s *ptr = table2.heads[i]; ptr; ptr = ptr->next) {
            if (*maxCount == ptr->count) {
                printf("%s %d\n", ptr->str, *maxCount);
            }
        }
    }
    for (int i = 0; i < BUCKET_COUNT; i++) {
        for (struct list_s *ptr = table3.heads[i]; ptr; ptr = ptr->next) {
            if (*maxCount == ptr->count) {
                printf("%s %d\n", ptr->str, *maxCount);
            }
        }
    }
    for (int i = 0; i < BUCKET_COUNT; i++) {
        for (struct list_s *ptr = table4.heads[i]; ptr; ptr = ptr->next) {
            if (*maxCount == ptr->count) {
                printf("%s %d\n", ptr->str, *maxCount);
            }
        }
    }
    pthread_mutex_destroy(&readLock);
    pthread_mutex_destroy(&table1.lock);
    pthread_mutex_destroy(&table2.lock);
    pthread_mutex_destroy(&table3.lock);
    pthread_mutex_destroy(&table4.lock);
    free(returnCount1);
    free(returnCount2);
    free(returnCount3);
    free(returnCount4);

//    struct list_s *head=NULL;
//    for (struct list_s *ptr=head;ptr;ptr=ptr->next){}
    return 0;
}
