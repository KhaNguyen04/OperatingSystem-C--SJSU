#include <stdio.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <netinet/in.h>
#include "bigbag.h"
#include <sys/stat.h>
#include <string.h>


struct bigbag_entry_s *entry_addr(void *hdr, uint32_t offset) {
    if (offset == 0) return NULL;
    return (struct bigbag_entry_s *)((char*)hdr + offset);
}

uint32_t entry_offset(void *hdr, void *entry) {
    return (uint32_t)((uint64_t)entry - (uint64_t)hdr);
}

int main(int argc, char **argv) {
    if (argc==2){
        int fd = open(argv[1], O_RDWR);
        if (fd==-1){
            fd=open(argv[1],O_RDWR|O_CREAT,S_IWUSR | S_IRUSR);
            if (fd==-1){
                printf("USAGE: ./bigbag [-t] filename");
                exit(1);
            }
            ftruncate(fd, BIGBAG_SIZE);
        }
        void *file_base = mmap(0, BIGBAG_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
        if (file_base == MAP_FAILED) {
            perror("mmap");
            exit(3);
        }
        struct bigbag_hdr_s *hdr = file_base;
        if (hdr->first_free==0){
            hdr->magic= htobe32(BIGBAG_MAGIC);
            struct bigbag_entry_s * free_space;
            free_space= entry_addr(hdr,sizeof(*hdr));
            free_space->entry_magic=BIGBAG_FREE_ENTRY_MAGIC;
            free_space->next=0;
            free_space->entry_len=BIGBAG_SIZE-sizeof(*hdr)-sizeof(*free_space);
            hdr->first_free= entry_offset(hdr,free_space);
        }

        char *line = NULL;
        size_t len = 0;
        while (getline(&line,&len,stdin)!=-1){
            line[strlen(line) - 1] = '\0';
            char ch=line[0];
            if (ch=='l'){
                struct bigbag_entry_s * element;
                element=entry_addr(hdr,hdr->first_element);
                if (element == NULL) {
                    printf("empty bag\n");
                }
                else {
                    while (element->entry_magic == BIGBAG_USED_ENTRY_MAGIC) {
                        printf("%s\n", element->str);
                        if (element->next == 0) {
                            break;
                        }
                        element = entry_addr(hdr, element->next);
                    }
                }
            }
            else if (ch=='c'){
                struct bigbag_entry_s * element;
                element=entry_addr(hdr,hdr->first_element);
                if (element == NULL) {
                    printf("bad entry at offset %d    \n", hdr->first_element);
                    break;
                }
                char keys[]=" ";
                //https://stackoverflow.com/questions/12845923/how-to-find-the-first-occurrence-of-one-of-several-characters-other-than-using
                char *token= strpbrk(line,keys);
                token=token+1;
                while (element->entry_magic==BIGBAG_USED_ENTRY_MAGIC){
                    if (strcmp(token,element->str)==0){
                        printf("found\n");
                        break;
                    }
                    else{
                        if (element->next==0){
                            printf("not found\n");
                            break;
                        }
                        element=entry_addr(hdr,element->next);
                    }
                }

            }
            else if(ch=='a'){
                //Silver01 and Luyen Ho on discord help me to regconize the memory left in the bigbag
                //Silver01 help me parsing string
                char *token=&line[2];
                if (strlen(token)+1>BIGBAG_SIZE-hdr->first_free){
                    printf("out of space\n");
                }
                else {
                    struct bigbag_entry_s *element;
                    struct bigbag_entry_s *free;
                    element = entry_addr(hdr, hdr->first_element);
                    free = entry_addr(hdr, hdr->first_free);
                    if (free == NULL) {
                        printf("bad entry at offset %d    \n", hdr->first_free);
                        break;
                    }
                    struct bigbag_entry_s *new_free;
                    int new_free_offset = sizeof(*new_free) + strlen(token) + entry_offset(hdr, free) + 1;
//                    int memRemain = BIGBAG_SIZE - sizeof(*new_free) - new_free_offset;
//                    if ((strlen(token) + 1 <= memRemain)) {
                    new_free = entry_addr(hdr, new_free_offset);
                    if (new_free == NULL) {
                        printf("bad entry at offset %d    \n", entry_offset(hdr, new_free));
                        break;
                    }
                    hdr->first_free = entry_offset(hdr, new_free);
                    new_free->entry_magic = BIGBAG_FREE_ENTRY_MAGIC;
                    new_free->next = free->next;
                    new_free->entry_len = BIGBAG_SIZE-hdr->first_free;
                    free->entry_magic = BIGBAG_USED_ENTRY_MAGIC;
                    free->entry_len = strlen(token) + 1;
                    strcpy(free->str, token);
                    if (element == NULL) {
                        hdr->first_element = entry_offset(hdr, free);
                        free->next = 0;
                    } else if (strcmp(free->str, element->str) <= 0) {
                        free->next = entry_offset(hdr, element);
                        hdr->first_element = entry_offset(hdr, free);
                    } else {
                        while ((strcmp(token, element->str) > 0) & (element->next != 0)) {
                            element = entry_addr(hdr, element->next);
                        }
                        free->next = element->next;
                        element->next = entry_offset(hdr, free);
                    }
                    printf("added %s\n", token);
//                    } else {
//                        printf("out of space\n");
//                        break;
//                    }
                }
            }
            else{
                printf("%c not used correctly\npossible commands:\na string_to_add\nd string_to_delete\nc string_to_check\nl\n",ch);
            }
        }
        free(line);
    }
    else if (argc==3) {
        if (strcmp(argv[1],"-t")==0) {
            int fd = open(argv[2], O_RDWR);
            if (fd == -1) {
                fd = open(argv[2], O_CREAT | O_RDWR, S_IWUSR | S_IRUSR);
                if (fd == -1) {
                    printf("USAGE: ./bigbag [-t] filename");
                    exit(1);
                }
                ftruncate(fd, BIGBAG_SIZE);
            }
            void *file_base = mmap(0, BIGBAG_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
            if (file_base == MAP_FAILED) {
                perror("mmap");
                exit(3);
            }
            struct bigbag_hdr_s *hdr = file_base;
            if (hdr->first_free==0){
                hdr->magic= htobe32(BIGBAG_MAGIC);
                struct bigbag_entry_s * free_space;
                free_space= entry_addr(hdr,sizeof(*hdr));
                free_space->entry_magic=BIGBAG_FREE_ENTRY_MAGIC;
                free_space->next=0;
                free_space->entry_len=BIGBAG_SIZE-sizeof(*hdr)-sizeof(*free_space);
                hdr->first_free= entry_offset(hdr,free_space);
            }
            char *line = NULL;
            size_t len = 0;
            while (getline(&line, &len, stdin) != -1) {
                line[strlen(line) - 1] = '\0';
                char ch = line[0];
                if (ch == 'l') {
                    struct bigbag_entry_s *element;
                    element = entry_addr(hdr, hdr->first_element);
                    if (element == NULL) {
                        printf("empty bag\n");
                    }
                    else {
                        while (element->entry_magic == BIGBAG_USED_ENTRY_MAGIC) {
                            printf("%s\n", element->str);
                            if (element->next == 0) {
                                break;
                            }
                            element = entry_addr(hdr, element->next);
                        }
                    }
                } else if (ch == 'c') {
                    struct bigbag_entry_s *element;
                    element = entry_addr(hdr, hdr->first_element);
                    if (element == NULL) {
                        printf("bad entry at offset %d    \n", hdr->first_element);
                        break;
                    }
                    char keys[] = " ";
                    //https://stackoverflow.com/questions/12845923/how-to-find-the-first-occurrence-of-one-of-several-characters-other-than-using

                    char *token = strpbrk(line, keys);
                    token = token + 1;
                    while (element->entry_magic == BIGBAG_USED_ENTRY_MAGIC) {
                        if (strcmp(token, element->str) == 0) {
                            printf("found\n");
                            break;
                        } else {
                            if (element->next == 0) {
                                printf("not found\n");
                                break;
                            }
                            element = entry_addr(hdr, element->next);
                        }
                    }

                } else if (ch == 'a') {
//                    int memoryLeft=BIGBAG_SIZE-hdr->first_free-sizeof(*hdr)-sizeof(*entry_addr(hdr,hdr->first_free));
                    char *token=&line[2];
                    if (strlen(token)+1>BIGBAG_SIZE-hdr->first_free){
                        printf("out of space\n");
//                        exit(0);
                    }
                    else {
                        struct bigbag_entry_s *element;
                        struct bigbag_entry_s *free;
                        element = entry_addr(hdr, hdr->first_element);
                        free = entry_addr(hdr, hdr->first_free);
                        if (free == NULL) {
                            printf("bad entry at offset %d    \n", hdr->first_free);
                            break;
                        }
                        struct bigbag_entry_s *new_free;
                        int new_free_offset = sizeof(*new_free) + strlen(token) + entry_offset(hdr, free) + 1;
//                    int memRemain = BIGBAG_SIZE - sizeof(*new_free) - new_free_offset;
//                    if ((strlen(token) + 1 <= memRemain)) {
                        new_free = entry_addr(hdr, new_free_offset);
                        if (new_free == NULL) {
                            printf("bad entry at offset %d    \n", entry_offset(hdr, new_free));
                            break;
                        }
                        hdr->first_free = new_free_offset;
                        new_free->entry_magic = BIGBAG_FREE_ENTRY_MAGIC;
                        new_free->entry_len = BIGBAG_SIZE-hdr->first_free;
//                        new_free->entry_len = free->entry_len - sizeof(*new_free) - strlen(token) - 1;
                        free->entry_magic = BIGBAG_USED_ENTRY_MAGIC;
                        free->entry_len = strlen(token) + 1;
                        strcpy(free->str, token);
                        if (element == NULL) {
                            hdr->first_element = entry_offset(hdr, free);
                            free->next = 0;
                        } else if (strcmp(free->str, element->str) <= 0) {
                            free->next = entry_offset(hdr, element);
                            hdr->first_element = entry_offset(hdr, free);
                        } else {
                            while ((strcmp(token, element->str) > 0) & (element->next != 0)) {
                                element = entry_addr(hdr, element->next);
                            }
                            free->next = element->next;
                            element->next = entry_offset(hdr, free);
                        }
                        printf("added %s\n", token);
//                    } else {
//                        printf("out of space\n");
//                        break;
//                    }
                    }
                }
                else{
                    printf("%c not used correctly\npossible commands:\na string_to_add\nd string_to_delete\nc string_to_check\nl\n",ch);
                }
            }
            free(line);
        }
    }
    else{
        printf("USAGE: ./bigbag [-t] filename\n");
        exit(1);
    }

    exit(0);
}