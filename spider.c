#include "crawler.h"
/*--------------spider function------------------*/

//thread function
void *spider(void *no_argument){
    
    node_t *head = NULL;//linked list head
    char*url;

    CURL *curl;//handle
    TidyDoc tdoc;
    TidyBuffer docbuf = {0};
    TidyBuffer tidy_errbuf = {0};
    int err;

    curl = curl_easy_init();//initialize handle
    
    while(1)
    {
        // Check if we should save checkpoint
        if (should_checkpoint) {
            pthread_mutex_lock(&mutex);
            pthread_mutex_lock(&mutex2);
            file_writer_default();
            should_checkpoint = 0;
            printf("\nCheckpoint saved. Exiting...\n");
            pthread_mutex_unlock(&mutex2);
            pthread_mutex_unlock(&mutex);
            exit(0);
        }

        pthread_mutex_lock(&mutex);//lock
        url=dequeue(&q);
        pthread_mutex_unlock(&mutex);//unlock

        if(strcmp(url,QUEUE_EMPTY) == 0)//break condition
          break;
      
        printf("Crawling......%s\n", url);
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
        
        // Set User-Agent
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "WebCrawler/1.0 (Educational Purpose)");
        
        // Follow redirects (up to 5)
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 5L);
        
        // Set timeouts
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);  // 30 seconds total timeout
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);  // 10 seconds connection timeout

        tdoc = tidyCreate();
        tidySetErrorBuffer(tdoc, &tidy_errbuf);
        tidyBufInit(&docbuf);
 
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &docbuf);
        err = curl_easy_perform(curl);

        if(!err) {
            err = tidyParseBuffer(tdoc, &docbuf); /* parse the input */  
            if(err >= 0) {
                dumpNode(tdoc, tidyGetRoot(tdoc), &head); /* walk the tree */
            }
        }
        
        // Always clean up tidy resources
        tidyBufFree(&docbuf);//clear buffer
        tidyBufFree(&tidy_errbuf);//clear buffer
        tidyRelease(tdoc);  // Release the tidy document
      
        pthread_mutex_lock(&mutex2);//lock
        crawl_frontier(head);
        pthread_mutex_unlock(&mutex2);//unlock

        head = NULL;
    }
    
    // Clean up curl handle
    curl_easy_cleanup(curl);
}//spider ends

//crawl first url
void *first_spider(char *argv){

    node_t *head = NULL;//linked list head
    char*url;

    extract_root(root, argv);//extract domain name
    //write the domain name into Domain.txt file
    FILE* fp;
    fp=fopen(root_list,"w");
    fprintf(fp,"%s",root);
    fclose(fp);//file close

    insert_hash(create_new_node(argv), &q);//insert first url to hash/queue

       url=dequeue(&q);//dequeue url
   if(strcmp(url,QUEUE_EMPTY) == 0)//break condition
        return NULL;
   printf("Crawling......%s\n", url);
    head = crawl(url,head);//crawl first url
    crawl_frontier(head);//insert retrieved urls to hash/ queue
}


