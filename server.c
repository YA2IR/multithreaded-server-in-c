#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "hash_table.h"
#include "queue.h"

#define PORT 4444
#define BUFF_SIZE 1024
#define EMPTY -1
#define STOP -2
#define THREAD_POOL_SIZE 20
#define BACKLOG_NUM 128

/*
    predefined http responses/response headers:
*/
#define HTTP_200_RESPONSE "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n"
#define HTTP_400_RESPONSE \
    "HTTP/1.1 400 Bad Request\r\nContent-Type: text/plain\r\nContent-Length: 16\r\n\r\n400 Bad Request\n"
#define HTTP_404_RESPONSE \
    "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\nContent-Length: 13\r\n\r\n404 Not Found\n"
#define HTTP_405_RESPONSE \
    "HTTP/1.1 405 Method Not Allowed\r\nContent-Type: text/plain\r\nContent-Length: 22\r\nAllow: GET\r\n\r\n405 Method Not Allowed\n"
#define HTTP_500_RESPONSE \
    "HTTP/1.1 500 Internal Server Error\r\nContent-Type: text/plain\r\nContent-Length: 25\r\n\r\n500 Internal Server Error\n"
#define HTTP_302_RESPONSE(REDIRECT_TO) "HTTP/1.1 302 Found\r\nLocation: " REDIRECT_TO "\r\nContent-Length: 0\r\n\r\n"

queue_t* queue;
hash_table_t* table;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_var = PTHREAD_COND_INITIALIZER;

// register routes/files here, they should be in the "files" directory
char* routes[] = {"/landing_page.html", "/another_page.html"};

/*  the route that will be redirected to whenever someone requests "/" ,
    and more generally, when someone requests a route that doesn't exist: */
#define REDIRECT_TO "/landing_page.html"
int num_routes = sizeof(routes) / sizeof(char*);

void check_error(int i, char* msg, int sd) {
    if (i == -1) {
        perror(msg);
        close(sd);
        exit(1);
    }
}

void send_http_response(int sd, char* response) {
    ssize_t sent_bytes = send(sd, response, strlen(response), 0);
    check_error(sent_bytes, "Error Sending HTTP Response: ", sd);
    close(sd);
}

int init_server() {
    int sd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(PORT);

    check_error(sd, "\nerror creating socket", sd);
    check_error(bind(sd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)), "\nerror binding", sd);
    check_error(listen(sd, BACKLOG_NUM), "\nerror listening", sd);

    return sd;
}

void handle_request(int sd) {
    char buff[BUFF_SIZE];

    ssize_t bytes_read = recv(sd, buff, BUFF_SIZE, 0);
    check_error(bytes_read, "receviving error", sd);

    char method[10], route[50];

    // malformed request
    if ((sscanf(buff, "%9s %49s", method, route)) != 2) {
        send_http_response(sd, HTTP_400_RESPONSE);
        return;
    }

    // check if not GET
    if (strcmp(method, "GET") != 0) {
        send_http_response(sd, HTTP_405_RESPONSE);  // method not allowed
        return;
    }

    // redirection logic
    if (!exists(table, route)) {
        send_http_response(sd, HTTP_302_RESPONSE(REDIRECT_TO));
        return;
    }

    // all cool, format the route into a file path
    char formatted_route[60];
    snprintf(formatted_route, sizeof(formatted_route), "files/%s", route);

    FILE* file = fopen(formatted_route, "r");
    if (file == NULL) {
        if (errno == ENOENT) {
            send_http_response(sd, HTTP_404_RESPONSE);
        } else {
            send_http_response(sd, HTTP_500_RESPONSE);
        }
        return;
    }

    size_t file_bytes_read;

    ssize_t sent_bytes = send(sd, HTTP_200_RESPONSE, strlen(HTTP_200_RESPONSE), 0);
    check_error(sent_bytes, "Error Sending HTTP Response: ", sd);  // header

    while ((file_bytes_read = fread(buff, 1, BUFF_SIZE, file))) {  // actual file content
        check_error(send(sd, buff, file_bytes_read, 0), "Error Sending: ", sd);
    }

    fclose(file);
    close(sd);
}

void* init_thread() {
    int cd;
    while (true) {
        pthread_mutex_lock(&mutex);
        while ((cd = dequeue(queue)) == EMPTY) {
            pthread_cond_wait(&cond_var, &mutex);
        }
        pthread_mutex_unlock(&mutex);

        if (cd == STOP) {
            break;
        }

        handle_request(cd);
    }
    return NULL;
}

int main() {
    pthread_t threads[THREAD_POOL_SIZE];
    int sd, client_sd;

    table = init_table(routes, num_routes);
    sd = init_server();
    queue = init_queue();

    for (int i = 0; i < THREAD_POOL_SIZE; i++) {
        check_error(pthread_create(&threads[i], NULL, &init_thread, NULL), "error creating thread\n", sd);
    }

    printf("\nserver now accepting connections ...\n");

    while (true) {
        client_sd = accept(sd, NULL, NULL);
        check_error(client_sd, "\naccepting error...", sd);

        pthread_mutex_lock(&mutex);

        enqueue(queue, client_sd);
        pthread_cond_signal(&cond_var);

        pthread_mutex_unlock(&mutex);
    }

    // signaling all threads to stop :
    pthread_mutex_lock(&mutex);
    for (int i = 0; i < THREAD_POOL_SIZE; i++) {
        enqueue(queue, STOP);
    }

    pthread_cond_broadcast(&cond_var);
    pthread_mutex_unlock(&mutex);

    for (int i = 0; i < THREAD_POOL_SIZE; i++) {
        pthread_join(threads[i], NULL);
    }

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond_var);
    close(sd);
    free(queue);
    free_table(table);
    printf("\nall cleaned up...\n");
    return 0;
}
