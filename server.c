// Server side C/C++ program to demonstrate Socket programming
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>

#define PORT 8080

int main(int argc, char const *argv[])
{
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};
    char *res = "HTTP/1.1 200 OK\r\nContent-Type:application/pdf\r\nConnection:Closed\r\n\r\n";

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port 8080
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
    if (listen(server_fd, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    printf("Waiting for a connection\n");
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address,
                             (socklen_t *)&addrlen)) < 0)
    {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    valread = read(new_socket, buffer, 1024);
    printf("Connected\n");
    printf("%s\n", buffer);

    printf("sending response\n");

    send(new_socket, res, strlen(res), 0);

    // Open the file
    FILE *pdf_fd;
    if ((pdf_fd = fopen("Syllabus.pdf", "rb")) == NULL)
    {
        printf("Error opening the file\n");
        exit(EXIT_FAILURE);
    }

    //Get file length
    int fileLen;
    fseek(pdf_fd, 0, SEEK_END);
    fileLen = ftell(pdf_fd);
    fseek(pdf_fd, 0, SEEK_SET);

    // //Allocate memory
    // char *pdfBuffer = (char *)malloc(fileLen + 1);
    // if (!pdfBuffer)
    // {
    //     fprintf(stderr, "Memory error!");
    //     fclose(pdf_fd);
    //     exit(EXIT_FAILURE);
    // }
    int totalSent = 0;
    int sent;
    do
    {
        fread(buffer, 1, 1024, pdf_fd);
        sent = send(new_socket, buffer, 1024, 0);
        if (sent < 0)
            exit(EXIT_FAILURE);
        if (sent == 0)
            break;
        totalSent += sent;
    } while (totalSent < fileLen);

    fclose(pdf_fd);
    printf("Response sent\n");

    // //Read file contents into buffer
    // fread(pdfBuffer, fileLen, 1, pdf_fd);
    // printf("Buffer \n%s\n\n", pdfBuffer);
    // //free(buffer);

    // // Send the file as response
    // send(server_fd, pdfBuffer, fileLen, 0);

    return 0;
}