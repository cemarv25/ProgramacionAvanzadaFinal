// Server side C/C++ program to demonstrate Socket programming
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>

#define PORT 8080

char buffer[1024] = {0};
char route;

int create_socket(int opt, struct sockaddr_in address)
{
    int server_fd;
    // Create socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Configuring socket options
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                   &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Forcefully attaching socket to the port 8080
    if (bind(server_fd, (struct sockaddr *)&address,
             sizeof(address)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    return server_fd;
}

void server_listen(int server_fd)
{
    if (listen(server_fd, 100) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
}

int await_connection(int server_fd, struct sockaddr_in address, int addrlen)
{
    int new_socket;
    printf("Waiting for a connection...\n");
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address,
                             (socklen_t *)&addrlen)) < 0)
    {
        perror("accept");
        exit(EXIT_FAILURE);
    }
    read(new_socket, buffer, 1024);

    char req_no_space[100] = {0};
    strncpy(&route, req_no_space, strlen(&route));

    // Substring from the 5th character (/)
    strncpy(req_no_space, buffer + 5, 50);

    char *ptr;
    int index;
    ptr = strchr(req_no_space, ' ');
    index = (int)(ptr - req_no_space);

    strncpy(&route, req_no_space, index);
    printf("route inside await: %s\n", &route);

    return new_socket;
}

FILE *open_file(char *fileName)
{
    FILE *pdf_fd;

    // Open file in read binary mode
    if ((pdf_fd = fopen(fileName, "rb")) == NULL)
    {
        printf("Error opening the file\n");
        exit(EXIT_FAILURE);
    }

    return pdf_fd;
}

int get_file_len(FILE *pdf_fd)
{
    int file_len;
    fseek(pdf_fd, 0, SEEK_END);
    file_len = ftell(pdf_fd);
    fseek(pdf_fd, 0, SEEK_SET);

    return file_len;
}

void send_file(FILE *pdf_fd, int socket, int file_len)
{
    int totalSent = 0;
    int sent;

    // Read and send file in chunks of 1024 bytes until sent bytes = file_len
    do
    {
        fread(buffer, 1, 1024, pdf_fd);
        sent = send(socket, buffer, 1024, 0);
        if (sent < 0)
            exit(EXIT_FAILURE);
        if (sent == 0)
            break;
        totalSent += sent;
    } while (totalSent < file_len);
}

int start_server()
{
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    // Response headers
    char *res = "HTTP/1.1 200 OK\r\nContent-Type:application/pdf\r\nConnection:Closed\r\n\r\n";
    char *res404 = "HTTP/1.1 200 OK\r\nContent-Type:text/html\r\nConnection:Closed\r\n\r\n";

    server_fd = create_socket(opt, address);

    server_listen(server_fd);

    while (1)
    {
        new_socket = await_connection(server_fd, address, addrlen);

        if (!strcmp(&route, "syllabus"))
        {
            // Send response headers
            send(new_socket, res, strlen(res), 0);

            printf("Sending file 'Syllabus.pdf'...\n");
            // Open the file
            FILE *pdf_fd = open_file("Syllabus.pdf");

            // Get file length
            int file_len = get_file_len(pdf_fd);

            // Send file
            send_file(pdf_fd, new_socket, file_len);

            fclose(pdf_fd);
            printf("File sent\n");

            printf("Closing socket...\n\n");
            close(new_socket);
        }
        else if (!strcmp(&route, ""))
        {
            send(new_socket, res404, strlen(res404), 0);
            send(new_socket, "<html><body>Searching for /syllabus ?</body></html>", 41, 0);
            close(new_socket);
        }
        else if (!strcmp(&route, "favicon.ico"))
        {
            continue;
        }
        else
        {
            // Send response headers
            send(new_socket, res404, strlen(res404), 0);
            send(new_socket, "<html><body>404 Not found.</body></html>", 41, 0);
            close(new_socket);
        }

        // printf("Connected\n\n");
        // printf("Request received: \n");
        // printf("%s\nEnd of request\n", buffer);
    }

    return 0;
}

int main(int argc, char const *argv[])
{
    start_server();
}