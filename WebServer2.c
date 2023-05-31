#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#define BUFFER_SIZE 1024
#define ROOT_DIR "/mnt/d/escuela/tercero/SO/ProyectoWeb"
#define PORT 1025
#define IP "127.0.0.1"

int main()
{
    int sockfd, client_socket, status, nbytes;
    char buffer[2048];
    struct sockaddr_in serverAddress, clientAddress;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        perror("Error creating socket");
        exit(1);
    }
    bzero(&serverAddress, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = inet_addr(IP);
    serverAddress.sin_port = htons(PORT);
    status = bind(sockfd, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
    if (status == -1)
    {
        perror("Error binding socket");
        exit(1);
    }
    listen(sockfd, 10);
    printf("Listening on port %d...\n", PORT);
    while (1)
    {
        int clientSize = sizeof(clientAddress);
        client_socket = accept(sockfd, (struct sockaddr *)&clientAddress, &clientSize);
        if (client_socket == -1)
        {
            perror("Error accepting new connection");
            continue; // Try to accept a new connection
        }
        printf("Accepted new connection from %s:%d\n", inet_ntoa(clientAddress.sin_addr), ntohs(clientAddress.sin_port));

        int pid = fork(); // Spawn a new process for the new connection
        if (pid < 0)
        {
            perror("Error forking new process");
            exit(1);
        }
        else if (pid == 0)
        {
            // Child process: handle the new connection
            close(sockfd); // Close the original socket in the child process
            bzero(buffer, sizeof(buffer));

            ssize_t bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
            if (bytes_received < 0)
            {
                printf("Failed to receive data from client.\n");
                close(client_socket);
                continue;
                ;
            }
            buffer[bytes_received] = '\0';
            printf("Received message from client:\n%s\n", buffer);

            while (1)
            {

                // nbytes = read(client_socket, buffer, sizeof(buffer));
                send_page(client_socket);
                close(client_socket);
                /*DIR *dir;
                struct dirent *entry;
                dir = opendir(ROOT_DIR);
                char html_buffer[BUFFER_SIZE];
                strcpy(html_buffer, "<html><head>Directorio</head><body><table><tr><th>Name</th><th>Size</th><th>Date</th></tr>");
                while ((entry = readdir(dir)) != NULL)
                {
                    strcat(html_buffer, "<tr>");
                    strcat(html_buffer, "<td>");
                    strcat(html_buffer, entry->d_name);
                    strcat(html_buffer, "</td>");
                    strcat(html_buffer, "<td>");
                    strcat(html_buffer, "0");
                    strcat(html_buffer, "</td>");
                    strcat(html_buffer, "</tr>");
                }
                strcat(html_buffer, "</table></body></html>\r\n");
                // FILE* file = fopen("/mnt/c/so/ws/directory.html", O_RDONLY);

                //= "<html><body><h1>Hello, World!</h1></body></html>"
                // char *html_content = "<html><head>Directorio x/y/z</head><body><table><tr><th>Name</th><th>Size</th><th>Date</th></tr><tr><th><a>directorio1</a></th><th>0</th><th>2019-02-23 12:12:12</th></tr><tr><th><a>directorio2</a></th><th>3k</th><th>2019-02-23 12:12:12</th></tr></table></body></html>";
                // fgets(html_content, 1024, file);

                // Construir la respuesta HTTP (código de estado 200 OK)
                char response[1024];
                sprintf(response, "HTTP/1.1 200 OK\r\n"
                                  "Content-Type: text/html\r\n"
                                  "Content-Length: %ld\r\n\r\n"
                                  "%s",
                        strlen(html_buffer), html_buffer);

                // Enviar la respuesta al cliente
                ssize_t bytes_sent = send(client_socket, response, strlen(response), 0);

                // Verificar si hubo un error en el envío
                if (bytes_sent < 0)
                {
                    perror("ERROR2");
                }
            }
            printf("Disconnected from client %s:%d\n", inet_ntoa(clientAddress.sin_addr), ntohs(clientAddress.sin_port));
            close(client_socket); // Close the client socket in the child process
            exit(0);
        }
        else
        {
            close(client_socket); // Close the client socket in the parent process
        }*/
            }
            close(sockfd);
            return 0;
        }
    }
}
void create_html()
{
    DIR *dir;
    struct dirent *entry;
    dir = opendir(ROOT_DIR);
    char html_buffer[BUFFER_SIZE];
    strcpy(html_buffer, "<html><head>Directorio</head><body><table><tr><th>Name</th><th>Size</th><th>Date</th></tr>");
    while ((entry = readdir(dir)) != NULL)
    {
        strcat(html_buffer, "<tr>");
        strcat(html_buffer, "<td>");
        strcat(html_buffer, entry->d_name);
        strcat(html_buffer, "</td>");
        strcat(html_buffer, "<td>");
        strcat(html_buffer, "0");
        strcat(html_buffer, "</td>");
        strcat(html_buffer, "</tr>");
    }
    strcat(html_buffer, "</table></body></html>\r\n");
    FILE *html_file = fopen("content.html", "w");
    if (html_file != NULL)
    {
        fprintf(html_file, "%s", html_buffer);
        fclose(html_file);
    }
}
void send_page(int client_socket)
{
    FILE *fp;
    create_html();
    char *buffer;
    long size;
    fp = fopen("content.html", "rb");
    if (fp == NULL)
    {
        return;
    }
    fseek(fp, 0, SEEK_END);
    size = ftell(fp);
    rewind(fp);
    buffer = (char *)malloc(sizeof(char) * size);
    if (buffer == NULL)
    {
        fclose(fp);
        return;
    }
    fread(buffer, 1, size, fp);
    fclose(fp);
    char http_response[2048];
    sprintf(http_response, "HTTP/1.1 200 OK\r\nContent-Length: %ld\r\n\r\n%s", size, buffer);
    send(client_socket, http_response, strlen(http_response), 0);
    send(client_socket, buffer, size, 0);
    free(buffer);
}
