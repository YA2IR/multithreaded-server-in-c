#include <pthread.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>

#include "queue.h"


#define PORT 12345
#define BUFF_SIZE 4096
#define EMPTY -1
#define THREAD_POOL_SIZE 20 
#define BACKLOG_NUM 128
#define FILE_PATH "files/file%d.txt"
#define HTTP_RESPONSE \
        "HTTP/1.1 200 OK\r\n" \
        "Content-Type: text/plain\r\n" \
        "\r\n" 


queue_t* queue; 
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_var = PTHREAD_COND_INITIALIZER;


void check_error(int i, char* msg,int sd){
	if(i == -1) {
		perror(msg);
		close(sd);
		exit(1);
	}
}

int init_server(){

	int sd = socket(AF_INET,SOCK_STREAM,0);

	struct sockaddr_in serv_addr;
	serv_addr.sin_family = AF_INET;
	// added
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); 
	serv_addr.sin_port = htons(PORT);

	check_error(sd, "\nerror creating socket",sd);
	check_error(bind(sd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)), "\nerror binding",sd);
	check_error(listen(sd, BACKLOG_NUM), "\nerror listening",sd);

	return sd;
}

void handle_request(int sd) {

	char buff[BUFF_SIZE];

	ssize_t bytes_read = recv(sd, buff, BUFF_SIZE,0);
	check_error(bytes_read, "receviving error", sd);
			
	int rand_idx = rand() % 50 + 1;
	char fileName[20];
	snprintf(fileName, sizeof(fileName), FILE_PATH, rand_idx);

	FILE *file = fopen(fileName, "r");
	char file_buffer[BUFF_SIZE];
	size_t file_bytes_read;


    ssize_t sent_bytes = send(sd, HTTP_RESPONSE, strlen(HTTP_RESPONSE), 0); \
    check_error(sent_bytes, "Error Sending HTTP Headers: ", sd); 
	while ((file_bytes_read = fread(file_buffer, 1, BUFF_SIZE, file))) 
		check_error(send(sd, file_buffer, file_bytes_read, 0), "Error Sending: ", sd);

	fclose(file);	
	close(sd);
}


void* init_thread() {

	int cd;

	while(true) {
		pthread_mutex_lock(&mutex);
		while((cd = dequeue(queue)) == EMPTY) {
			pthread_cond_wait(&cond_var, &mutex);
		}
		pthread_mutex_unlock(&mutex);
		handle_request(cd);
	}	
	return NULL;
}

int main(){	

	int sd = init_server();
	int client_sd;

	queue = init_queue();

	for(int i = 0; i < THREAD_POOL_SIZE; i++) {
		pthread_t tid;// = malloc(sizeof(pthread_t));
		pthread_create(&tid, NULL, &init_thread, NULL);
	}

	printf("\nserver now accepting connections ...\n");

	while (true) {
		client_sd = accept(sd, NULL, NULL);
		check_error(client_sd, "\naccepting error...",sd);	

		pthread_mutex_lock(&mutex);

		enqueue(queue,client_sd);
		pthread_cond_signal(&cond_var);

		pthread_mutex_unlock(&mutex);
	}	

	pthread_mutex_destroy(&mutex);
	pthread_cond_destroy(&cond_var);
	printf("\nclosing socket...");
	close(sd);
	return 0;
}
