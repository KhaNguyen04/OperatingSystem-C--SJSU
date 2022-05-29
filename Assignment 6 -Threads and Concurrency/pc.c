#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

struct list_s
{
    char *value;
    struct list_s *next;
    int count;
};
struct queue_s //got some help from the book for this part
{
    struct list_s *head;
    struct list_s *tail;
    pthread_mutex_t headLock;
    pthread_mutex_t tailLock;
    pthread_mutex_t emptyLock;
    pthread_cond_t cond;
    int ready;
};
struct hashtable_s
{
    struct list_s *heads[256];
    struct queue_s *queue;
    int maxCount;
};
struct queueList_s
{
    struct queue_s *queue00;
    struct queue_s *queue01;
    struct queue_s *queue10;
    struct queue_s *queue11;
};
struct fileInput_s
{
    char *fileName;
    struct queueList_s *queueList;

};

int hash(char *str) //djb2 method by Dan Bernstein, found on the internet
{
    unsigned long hash = 5381;

    for (int i = 0; str[i] != '\0'; ++i)
        hash = ((hash << 5) + hash) + (unsigned char)str[i];
    return hash % 256;
}
int checkInList(struct list_s *head, char *str)
{
    while (head != NULL)
    {
        if (strcmp(head->value, str) == 0)
        {
            head->count++;
            free(str);
            return head->count;
        }
        head = head->next;

    }
    return 0;
}
void addToList(struct list_s **head, char *str)
{
    struct list_s *newNode = malloc(sizeof(*newNode));
    newNode->count = 1;
    newNode->value = str;
    newNode->next = *head;
    *head = newNode;
}
void addToHashTable(struct hashtable_s *hashtable, char *str)
{
    int h = hash(str);
    int occurrences = checkInList(hashtable->heads[h], str);
    if (occurrences == 0)
    {
        addToList(&hashtable->heads[h], str);
        occurrences++; //local, used just for the next if statement
    }
    if (occurrences > hashtable->maxCount)
        hashtable->maxCount = occurrences;
}
void queueInit(struct queue_s *queue)
{
    struct list_s *tmp = malloc(sizeof(*tmp));
    tmp->next = NULL;
    queue->head = tmp;
    queue->head->value = NULL;
    queue->tail = tmp;
    pthread_mutex_init(&queue->headLock, NULL);
    pthread_mutex_init(&queue->tailLock, NULL);
    pthread_mutex_init(&queue->emptyLock, NULL);
    pthread_cond_init(&queue->cond, NULL);
    queue->ready = 0;
}
void queueListInit(struct queueList_s *queueList)
{
    struct queue_s *queue00 = malloc(sizeof(*queue00));
    queueInit(queue00);
    struct queue_s *queue01 = malloc(sizeof(*queue01));
    queueInit(queue01);
    struct queue_s *queue10 = malloc(sizeof(*queue10));
    queueInit(queue10);
    struct queue_s *queue11 = malloc(sizeof(*queue11));
    queueInit(queue11);
    queueList->queue00 = queue00;
    queueList->queue01 = queue01;
    queueList->queue10 = queue10;
    queueList->queue11 = queue11;
}
void addToQueue(struct queue_s *queue, char *str)
{
    pthread_mutex_lock(&queue->headLock);

    if (queue->head->value == NULL)
    {
        queue->head->value = str;
        pthread_mutex_lock(&queue->emptyLock); //taken from slides
        queue->ready = 1;
        pthread_cond_signal(&queue->cond);
        pthread_mutex_unlock(&queue->emptyLock);
        pthread_mutex_unlock(&queue->headLock);
        return;
    }
    pthread_mutex_unlock(&queue->headLock);
    struct list_s *tmp = malloc(sizeof(struct list_s)); //got some help from the book for this part
    tmp->value = str;
    tmp->next = NULL;
    pthread_mutex_lock(&queue->tailLock);
    queue->tail->next = tmp;
    queue->tail = tmp;
    pthread_mutex_unlock(&queue->tailLock);


}
char *takeFromQueue(struct queue_s *queue)
{
    pthread_mutex_lock(&queue->headLock);
    if (queue->head->value == NULL)
    {
        pthread_mutex_lock(&queue->emptyLock); //taken from slides
        queue->ready = 0;
        pthread_mutex_unlock(&queue->headLock);
        while (!queue->ready)
        {
            pthread_cond_wait(&queue->cond, &queue->emptyLock);
        }
        pthread_mutex_unlock(&queue->emptyLock);
        pthread_mutex_lock(&queue->headLock);
    }

    char *returnMe = queue->head->value;
    struct list_s *oldNew = queue->head;
    if (queue->head->next != NULL)
    {
        queue->head = queue->head->next;
        free(oldNew);
    }
    else
    {
        queue->head->value = NULL;
    }
    pthread_mutex_unlock(&queue->headLock);
    return returnMe;
}

void putWordsInQueue(char *fileName, struct queueList_s *queueList)
{
    FILE *thisFile = fopen((char *) fileName, "r");
    if (thisFile) //taken partially from https://www.delftstack.com/howto/c/c-check-if-file-exists/
    {
        while (!feof(thisFile)) {
            char *ptr;
            if (fscanf(thisFile, "%ms", &ptr) != EOF)
            {
                switch (ptr[0] % 4)
                {
                    case 0:
                        addToQueue(queueList->queue00, ptr);
                        break;
                    case 1:
                        addToQueue(queueList->queue01, ptr);
                        break;
                    case 2:
                        addToQueue(queueList->queue10, ptr);
                        break;
                    case 3:
                        addToQueue(queueList->queue11, ptr);
                        break;
                }
            }
            else
            {
                fclose(thisFile);
                return;
            }

        }
    }
    else
    {
        perror(fileName);
    }

}
void *insertThreadHandler(void *v)
{
    struct fileInput_s *fileInput = (struct fileInput_s *)v;
    putWordsInQueue(fileInput->fileName, fileInput->queueList);
    free(fileInput);
    return v;
}

void makeHashFromQueue(struct hashtable_s *hashtable)
{
    while(1)
    {
        char *str = takeFromQueue(hashtable->queue);
        if (strcmp(str, "SENTINEL STOP") == 0)
        {
            free(str);
            break;
        }
        addToHashTable(hashtable, str);
    }
}
void *hashThreadHandler(void *v)
{
    struct hashtable_s *hashtable = (struct hashtable_s *)v;
    hashtable->maxCount = 0;
    for (int i = 0; i < 256; ++i)
    {
        hashtable->heads[i] = NULL;
    }
    makeHashFromQueue(hashtable);
    return v;
}
void printMaxInListAndFree(struct list_s *head, int max)
{
    while(head != NULL)
    {
        if (head->count == max)
        {
            printf("%s %d\n", head->value, max);
        }
        struct list_s *oldhead = head;
        head = head->next;
        free(oldhead->value);
        free(oldhead);
    }
}
void printMaxInHashAndFree(struct hashtable_s *hashtable, int max)
{
    for (int i = 0; i < 256; ++i)
    {
        printMaxInListAndFree(hashtable->heads[i], max);
    }

}
int findMax(int a, int b, int c, int d)
{
    int highab;
    int highbc;
    int highall;
    if (a > b)
        highab = a;
    else
        highab = b;
    if (c > d)
        highbc = c;
    else
        highbc = d;
    if (highab > highbc)
        highall = highab;
    else
        highall = highbc;
    return highall;
}
int main(int argc, char *argv[]) {\
    if (argc == 1)
    {
        return 0;
    }
    struct queueList_s *queueList = malloc(sizeof(struct queueList_s));
    queueListInit(queueList);
    pthread_t writetid[argc - 1];


    for (int i = 1; i < argc; ++i) { //make writer threads
        struct fileInput_s *fileInput = malloc(sizeof(struct fileInput_s));
        fileInput->fileName = argv[i];
        fileInput->queueList = queueList;
        pthread_create(&writetid[i - 1], NULL, insertThreadHandler, (void *) fileInput);
    }
    pthread_t hashtid[4]; //make reader threads
    struct hashtable_s *hash00 = malloc(sizeof(struct hashtable_s));
    hash00->queue = queueList->queue00;
    pthread_create(&hashtid[0], NULL, hashThreadHandler, (void *) hash00);
    struct hashtable_s *hash01 = malloc(sizeof(struct hashtable_s));
    hash01->queue = queueList->queue01;
    pthread_create(&hashtid[1], NULL, hashThreadHandler, (void *) hash01);
    struct hashtable_s *hash10 = malloc(sizeof(struct hashtable_s));
    hash10->queue = queueList->queue10;
    pthread_create(&hashtid[2], NULL, hashThreadHandler, (void *) hash10);
    struct hashtable_s *hash11 = malloc(sizeof(struct hashtable_s));
    hash11->queue = queueList->queue11;
    pthread_create(&hashtid[3], NULL, hashThreadHandler, (void *) hash11);

    for (int i = 1; i < argc; ++i) { //wait for writer threads to finish
        pthread_join(writetid[i - 1], NULL);
    }
    addToQueue(queueList->queue00, strdup("SENTINEL STOP"));
    addToQueue(queueList->queue01, strdup("SENTINEL STOP"));
    addToQueue(queueList->queue10, strdup("SENTINEL STOP"));
    addToQueue(queueList->queue11, strdup("SENTINEL STOP"));

    for (int i = 0; i < 4; ++i) {
        pthread_join(hashtid[i], NULL);
    }




    int maxCount = findMax(hash00->maxCount, hash01->maxCount, hash10->maxCount, hash11->maxCount);
    printMaxInHashAndFree(hash00, maxCount);
    printMaxInHashAndFree(hash01, maxCount);
    printMaxInHashAndFree(hash10, maxCount);
    printMaxInHashAndFree(hash11, maxCount);

    free(queueList->queue00->head);
    free(queueList->queue01->head);
    free(queueList->queue10->head);
    free(queueList->queue11->head);

    free(hash00);
    free(hash01);
    free(hash10);
    free(hash11);

    pthread_mutex_destroy(&queueList->queue00->headLock);
    pthread_mutex_destroy(&queueList->queue00->tailLock);
    pthread_mutex_destroy(&queueList->queue00->emptyLock);
    pthread_cond_destroy(&queueList->queue00->cond);
    free(queueList->queue00);

    pthread_mutex_destroy(&queueList->queue01->headLock);
    pthread_mutex_destroy(&queueList->queue01->tailLock);
    pthread_mutex_destroy(&queueList->queue01->emptyLock);
    pthread_cond_destroy(&queueList->queue01->cond);
    free(queueList->queue01);

    pthread_mutex_destroy(&queueList->queue10->headLock);
    pthread_mutex_destroy(&queueList->queue10->tailLock);
    pthread_mutex_destroy(&queueList->queue10->emptyLock);
    pthread_cond_destroy(&queueList->queue10->cond);
    free(queueList->queue10);

    pthread_mutex_destroy(&queueList->queue11->headLock);
    pthread_mutex_destroy(&queueList->queue11->tailLock);
    pthread_mutex_destroy(&queueList->queue11->emptyLock);
    pthread_cond_destroy(&queueList->queue11->cond);
    free(queueList->queue11);
    free(queueList);


}