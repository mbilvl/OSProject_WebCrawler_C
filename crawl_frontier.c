#include "crawler.h"

/*  -----------CRAWL FRONTIER-------------  */
/*--------- It validate the urls
      Populate the hash table
      Populate the queue
      Empty the url array-----------*/

void crawl_frontier(node_t *head){

    size_t dLength = strlen(root);//length of doamin name
    char *temp;//for concatenation
    node_t *tmp_node;//for copying head->next and free
    
    //start loop
    while(head != NULL){
      if (strncmp("http://", head->url, 7) == 0){
        if(strncmp(root,(head->url)+7,dLength) == 0){
          insert_hash(create_new_node(head->url), &q);//insert into hash
        }

      }else if (strncmp("https://", head->url, 8) ==0){
        if(strncmp(root,(head->url)+8,dLength) ==0) {
          insert_hash(create_new_node(head->url), &q);//insert into hash
        } 

      }else if(strncmp("//", head->url, 2) ==0){//start from double slash 
        if(strncmp(root,(head->url)+2,dLength) ==0) 
        {//refined from root domain
          temp=strdup(head->url);
          snprintf(head->url, URL_LEN, "https:%s", temp);
          free(temp);  // Free the duplicated string
          insert_hash(create_new_node((head->url)), &q);//insert into hash
        }

      }else{  
          //concatenate root in front of incomplete urls
        if(head->url[0] == '/'){
          temp=strdup(head->url);
          snprintf(head->url, URL_LEN, "https://%s%s", root, temp);
          free(temp);  // Free the duplicated string
          insert_hash(create_new_node(head->url), &q);//insert new node into hash
        }else{
          temp=strdup(head->url);
          snprintf(head->url, URL_LEN, "https://%s/%s", root, temp);
          free(temp);  // Free the duplicated string
          insert_hash(create_new_node(head->url), &q);//insert new node into hash
        }
          
      }
      tmp_node = head;
      head = head->next;
      free(tmp_node);//free prev node
    }
    //end loop
}

