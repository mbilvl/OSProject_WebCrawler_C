/* ----- Header File ------*/
#include "crawler.h"
void help();
//main starts 
int main(int argc, char **argv)
{   
	//Signal handler call
    signal(SIGINT, safe_signal_handler);//ctrl+C 
    // Don't handle SIGSEGV or SIGABRT - let them crash normally for debugging

    bootup();//bootup function call

    return 0;
}//main ends

// Safe signal handler - only sets a flag
void safe_signal_handler(int sig){
    should_checkpoint = 1;
}
//write to file
void file_writer_default(){

    FILE *fp_queue;
    fp_queue = fopen(waiting_list, "w");
    FILE *fp_hash;    
    fp_hash = fopen(found_list, "w");

    write_hash_to_file(fp_hash);
    print_queue_to_file(q,fp_queue);

    fclose(fp_queue);
    fclose(fp_hash);

}
