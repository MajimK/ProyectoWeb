#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <dirent.h>
#include <sys/stat.h>
#include <ctype.h>
#include <time.h>
#define BUFFER_SIZE 1024
// #define ROOT_DIR "/mnt/d/escuela/tercero/SO/ProyectoWeb"
// #define PORT 1025
#define IP "127.0.0.1"
#define RED_COLOR "\033[0;31m"
#define DEFAULT_COLOR "\033[0m"
#define BLUE_COLOR "\033[0;34m"
#define YELLOW_COLOR "\033[0;33m"
#define GREEN_COLOR "\033[0;32m"

char *create_html(char **nombres, int cant, char **date, char **size);
void url(char *str);
void obtener_nombres(const char *root, char **names, int *numNames, char **date, char **size);
void send_page(int client_socket, char *Root, char *original_Root);
int main(int argc, char *argv[])
{

    int PORT = atoi(argv[1]);
    if (PORT == 0)
    {
        fprintf(stderr, RED_COLOR "Error: invalid Port\n" DEFAULT_COLOR);
        exit(1);
    }
    char *ROOT_DIR = argv[2];
    DIR *dir = opendir(argv[2]);
    if (dir == NULL)
    {
        fprintf(stderr, RED_COLOR "Error: invalid Directory Root\n" DEFAULT_COLOR);
        exit(1);
    }
    closedir(dir);
    printf(GREEN_COLOR "Root: %s\n" DEFAULT_COLOR, ROOT_DIR);

    int server_socket, client_socket, status, nbytes;
    char buffer[BUFFER_SIZE];
    struct sockaddr_in serverAddress, clientAddress;
    socklen_t addr_len = sizeof(clientAddress);

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1)
    {
        perror("Error creating socket");
        exit(1);
    }

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(PORT);

    status = bind(server_socket, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
    if (status == -1)
    {
        perror("Error binding socket");
        exit(1);
    }

    if (listen(server_socket, 10) < 0)
    {
        fprintf(stderr, "Error: listen error\n");
        exit(1);
    }

    char host[25] = "http://localhost:";
    strcat(host, argv[1]);
    printf("WebServer is running on %s ...\n", host);

    while (1)
    {
        client_socket = accept(server_socket, (struct sockaddr *)&clientAddress, &addr_len);
        if (client_socket == -1)
        {
            perror("Error accepting new connection");
            exit(1); // Try to accept a new connection
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
            close(server_socket); // Close the original socket in the child process
            char ROOTcopia[128] = "";
            strcat(ROOTcopia, ROOT_DIR);
            send_page(client_socket, ROOTcopia, ROOT_DIR);
            exit(0);
        }
        close(client_socket);
    }
    close(server_socket);
    return 0;
}
void send_page(int client_socket, char *Root, char *original_Root)
{
    char buffer[BUFFER_SIZE];
    ssize_t bytes_received;

    while ((bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0)) > 0)
    {
        printf("Received message from client:\n%s\n", buffer);
        char method[10];
        sscanf(buffer, "%s", method);

        char path[BUFFER_SIZE];
        sscanf(buffer, "%*s %s", path);
        url(path);

        if (Root[strlen(Root) - 1] == '/')
            Root[strlen(Root) - 1] = '\0';

        char *contenido_html = malloc(4096);

        if (strcmp(method, "GET") == 0 && strcmp(path, "/favicon.ico") != 0)
        {
            char tempRoot[BUFFER_SIZE] = "";
            strcat(tempRoot, original_Root);
            strcat(tempRoot, path);

            int type = 0;
            DIR *dir = opendir(tempRoot);
            if (dir)
            {
                type = 1;
                close(dir);
            }
            else
            {
                type = 2;
            }
            tempRoot[0] = '\0';
            if (type == 1)
            {
                Root[0] = '\0';
                strcpy(Root, original_Root);
                strcat(Root, path);
                DIR *dir = opendir(Root);
                if (dir == NULL)
                {
                    printf("Cannot open directory: %s\n", Root);
                    exit(1);
                }
                int cant = 0;
                struct dirent *entry;
                while ((entry = readdir(dir)) != NULL)
                {
                    if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
                    {
                        cant++;
                    }
                }
                char **nombres = (char **)malloc(cant * sizeof(char *));
                char **date = (char **)malloc(cant * sizeof(char *));
                char **size = (char **)malloc(cant * sizeof(char *));
                if (nombres == NULL)
                {
                    printf("Error: cannot assign memory\n");
                    exit(1);
                }
                obtener_nombres(Root, nombres, &cant, date, size);
                contenido_html = create_html(nombres, cant, date, size);
            }
            else if (type == 2)
            {
                char file_path[128] = "";
                strcpy(file_path, original_Root);
                strcat(file_path, path);
                FILE *file = fopen(file_path, "rb");
                if (file == NULL)
                {
                    char error_response[128];
                    sprintf(error_response, "HTTP/1.1 404 Not Found\r\n"
                                            "Content-Length: %d\r\n\r\n"
                                            "File not found or cannot be opened",
                            32);
                    send(client_socket, error_response, strlen(error_response), 0);
                }
                else
                {
                    // tamño del archivo
                    fseek(file, 0, SEEK_END);
                    long file_size = ftell(file);
                    fseek(file, 0, SEEK_SET);
                    // guardar el contenido del archivo
                    char *file_buffer = malloc(file_size);
                    fread(file_buffer, file_size, 1, file);
                    // Construir la respuesta HTTP con el contenido del archivo
                    char response[4096];
                    sprintf(response, "HTTP/1.1 200 OK\r\n"
                                      "Content-Type: application/octet-stream; charset=UTF-8\r\n"
                                      "Content-Disposition: attachment; filename=\"%s\"\r\n"
                                      "Content-Length: %ld\r\n\r\n",
                            path, file_size);
                    send(client_socket, response, strlen(response), 0);
                    // Enviar el contenido del archivo al cliente
                    send(client_socket, file_buffer, file_size, 0);
                    // Liberar memoria y cerrar el archivo
                    free(file_buffer);
                    fclose(file);
                    continue;
                }
            }
        }
        char response[4098];
        sprintf(response, "HTTP/1.1 200 OK\r\n"
                          "Content-Type: text/html; charset=UTF-8\r\n"
                          "Content-Length: %ld\r\n\r\n"
                          "%s",
                strlen(contenido_html), contenido_html);

        send(client_socket, response, strlen(response), 0);
    }
    close(client_socket);
}
void url(char *str)
{
    char *p = str;
    char hex[3] = {0};
    while (*str)
    {
        if (*str == '%' && isxdigit((unsigned char)*(str + 1)) && isxdigit((unsigned char)*(str + 2)))
        {
            hex[0] = *(str + 1);
            hex[1] = *(str + 2);
            *p = strtol(hex, NULL, 16);
            str += 2;
        }
        else if (*str == '+')
            *p = ' ';
        else
            *p = *str;

        str++;
        p++;
    }
    *p = '\0';
}
void obtener_nombres(const char *root, char **names, int *numNames, char **date, char **size)
{
    char size_str[20];
    char date_str[20];
    DIR *dir;
    struct dirent *ent;
    struct stat st;

    // Abrir el directorio
    dir = opendir(root);
    if (dir == NULL)
    {
        printf("No se pudo abrir el directorio %s\n", root);
        exit(EXIT_FAILURE);
    }

    int i = 0;
    while ((ent = readdir(dir)) != NULL)
    {
        // Ignorar los nombres "." y ".."
        if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
        {
            continue;
        }
        char filepath[100];
        snprintf(filepath, sizeof(filepath), "%s/%s", root, ent->d_name);
        if (stat(filepath, &st) == 0)
        {
            sprintf(size_str, "%ld bytes", st.st_size);
            strftime(date_str, 20, "%y-%m-%d", localtime(&st.st_mtime));
        }
        // Copiar el nombre del archivo o carpeta al array
        char *temp_date = strdup(date_str);
        char *temp_size = strdup(size_str);
        date[i] = temp_date;
        size[i] = temp_size;
        names[i] = strdup(ent->d_name);
        i++;
    }

    *numNames = i;

    // Cerrar el directorio
    closedir(dir);
}

char *create_html(char **nombres, int cant, char **date, char **size)
{
    char *html_buffer = malloc(4096);
    free(html_buffer);
    int i = 0;
    strcat(html_buffer, "<html>"
                        "<head>"
                        "<style>"
                        "body {"
                        "background-color: darkcyan;"
                        "}"
                        "h1 {"
                        "color: limegreen;"
                        "font-size: 50px;"
                        "font-family: 'Lucida Sans', 'Lucida Sans Regular', 'Lucida Grande', 'Lucida Sans Unicode', Geneva, Verdana, sans-serif;"
                        "text-align: center;"
                        "text-transform: uppercase;"
                        "letter-spacing: 1px;"
                        "text-shadow: 2px 2px 4px rgba(0, 0, 0, 0.6);"
                        "}"

                        "table {"
                        "border-collapse: collapse;"
                        "width: 95%;"
                        "font-family: 'Lucida Sans', 'Lucida Sans Regular', 'Lucida Grande', 'Lucida Sans Unicode', Geneva, Verdana, sans-serif;"
                        "border-radius: 8px;"
                        "overflow: hidden;"
                        "}"

                        "th,"
                        "td {"
                        "padding: 8px;"
                        "text-align: left;"
                        "border-bottom: 1px solid #ddd;"
                        "}"

                        "th {"
                        "background-color: burlywood;"
                        "}"

                        "tr:nth-child(even) {"
                        "background-color: darkgray;"
                        "}"
                        "tr:nth-child(odd) {"
                        "background-color: grey;"
                        "}"

                        "tr:hover {"
                        "background-color: aliceblue;"
                        "cursor: pointer;"
                        "}"

                        "td:first-child {"
                        "border-left: 1px solid #ddd;"
                        "}"

                        "td:last-child {"
                        "border-right: 1px solid #ddd;"
                        "}"
                        "</style>"
                        "</head>"

                        "<body>"
                        "<h1>FILES</h1>"
                        "<table id=\"myTable\">"
                        "<tr>"
                        "<th onclick=\"sortTable(0,'str')\">Name</th>"
                        "<th onclick=\"sortTable(1,'int')\">Size</th>"
                        "<th onclick=\"sortTable(2,'str')\">Date</th>"
                        "</tr>");
    while (i < cant)
    {
        strcat(html_buffer, "<tr onclick=\"SendRequest('");
        strcat(html_buffer, nombres[i]);
        strcat(html_buffer, "')\"><td>");
        strcat(html_buffer, nombres[i]);
        strcat(html_buffer, "<td>");
        strcat(html_buffer, size[i]);
        strcat(html_buffer, "</td>");
        strcat(html_buffer, "<td>");
        strcat(html_buffer, date[i]);
        strcat(html_buffer, "</td>");
        strcat(html_buffer, "</tr>");
        i++;
    }
    strcat(html_buffer, "</table>"
                        "<script>"
                        "function SendRequest(name) {"
                        "var current = window.location.href;"
                        "console.log(current);"
                        "if (current[current.length - 1] == '/') { window.location.href = current + name; }"
                        "else { window.location.href = current + '/' + name }"
                        "}"
                        "function sortTable(n, type) {"
                        "var table, rows, switching, i, x, y, shouldSwitch, dir, switchcount = 0;"
                        "table = document.getElementsByTagName(\"table\")[0];"
                        "switching = true;"

                        "dir = \"asc\";"

                        "while (switching) {"

                        "switching = false;"
                        "rows = table.rows;"

                        "for (i = 1; i < (rows.length - 1); i++) {"

                        "shouldSwitch = false;"

                        "x = rows[i].getElementsByTagName(\"td\")[n];"
                        "y = rows[i + 1].getElementsByTagName(\"td\")[n];"

                        "if (dir == \"asc\") {"
                        "if ((type == 'str' && x.innerHTML.toLowerCase() > y.innerHTML.toLowerCase()) || (type == 'int' && parseInt(x.innerHTML) > parseInt(y.innerHTML))) {"

                        "shouldSwitch = true;"
                        "break;"
                        "}"
                        "} else if (dir == \"desc\") {"
                        "if ((type == 'str' && x.innerHTML.toLowerCase() < y.innerHTML.toLowerCase()) || (type == 'int' && parseInt(x.innerHTML) < parseInt(y.innerHTML))) {"

                        "shouldSwitch = true;"
                        "break;"
                        "}"
                        "}"
                        "}"
                        "if (shouldSwitch) {"

                        "rows[i].parentNode.insertBefore(rows[i + 1], rows[i]);"
                        "switching = true;"

                        "switchcount++;"
                        "} else {"

                        "if (switchcount == 0 && dir == \"asc\") {"
                        "dir = \"desc\";"
                        "switching = true;"
                        "}"
                        "}"
                        "}"
                        "}"
                        "</script>"
                        "</body>"

                        "</html>");
    return html_buffer;
}