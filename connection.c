#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "connection.h"

int r_count = 0;

void err_n_die(const char *fmt, ...)
{
    int errno_save;
    va_list ap;

    // any system or library call can errno, so we need to save it now
    errno_save = errno;

    // print out the fmt+args to standard out
    va_start(ap, fmt);
    vfprintf(stdout, fmt, ap);
    fprintf(stdout, "\n");
    fflush(stdout);

    // print our error message is errno was set.
    if (errno_save != 0)
    {
        fprintf(stdout, "(errno = %d) : %s\n", errno_save, strerror(errno_save));
        fprintf(stdout, "\n");
        fflush(stdout);
    }
    va_end(ap);

    // this is the ..and_die part, Terminate with an error.
    exit(1);
}

ssize_t readline(int fd, void *buf, size_t maxlen)
{
    char c;
    char *bufp = buf;
    int n;
    for (n = 0; n < maxlen - 1; n++)
    { // leave room at end for '\0'
        int rc;
        if ((rc = read(fd, &c, 1)) == 1)
        {
            *bufp++ = c;
            if (c == '\n')
                break;
        }
        else if (rc == 0)
        {
            if (n == 1)
                return 0; /* EOF, no data read */
            else
                break; /* EOF, some data was read */
        }
        else
            return -1; /* error */
    }
    *bufp = '\0';
    return n;
}

int open_server_connection()
{
    // Create a socket descriptor
    int listen_fd;
    if ((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        return -1;
    }

    // Eliminates "Address already in use" error from bind
    int opt_val = 1;
    if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, (const void *)&opt_val, sizeof(int)) < 0)
    {
        return -2;
    }

    // Endpoint for all requests to port on any IP address for this host
    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(SERVER_PORT); /* server port */

    if ((bind(listen_fd, (SA *)&server_addr, sizeof(server_addr))) < 0)
    {
        return -3;
    }

    if ((listen(listen_fd, 1024)) < 0)
    {
        // accepts connection requests
        return -4;
    }

    return listen_fd;
}

int open_client_connection(char *hostname, int port)
{
    int client_fd;
    struct hostent *hp;
    struct sockaddr_in server_addr;

    if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        return -1;

    if ((hp = gethostbyname(hostname)) == NULL)
        return -2;
    bzero((char *)&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    bcopy((char *)hp->h_addr, (char *)&server_addr.sin_addr.s_addr, hp->h_length);
    server_addr.sin_port = htons(port);

    // Establish a connection with the server
    if (connect(client_fd, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
        return -1;
    return client_fd;
}

// request error
void request_error(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg)
{
    char buf[MAXLINE], body[MAXLINE];

    // Create the body of error message first (have to know its length for header)
    sprintf(body,
            ""
            "<!doctype html>\r\n"
            "<head>\r\n"
            "  <title>IIITH WebServer Error</title>\r\n"
            "</head>\r\n"
            "<body>\r\n"
            "  <h2>%s: %s</h2>\r\n"
            "  <p>%s: %s</p>\r\n"
            "</body>\r\n"
            "</html>\r\n",
            errnum, shortmsg, longmsg, cause);

    // Write out the header information for this response
    sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
    if (write(fd, buf, strlen(buf)) < 0)
    {
        err_n_die("write error.");
    }

    sprintf(buf, "Content-Type: text/html\r\n");
    if (write(fd, buf, strlen(buf)) < 0)
    {
        err_n_die("write error.");
    }

    sprintf(buf, "Content-Length: %lu\r\n\r\n", strlen(body));
    if (write(fd, buf, strlen(buf)) < 0)
    {
        err_n_die("write error.");
    }

    // Write out the body last
    if (write(fd, body, strlen(body)) < 0)
    {
        err_n_die("write error.");
    }
}

//
// Reads and discards everything up to an empty text line
//
void request_read_headers(int fd)
{
    char buf[MAXLINE];

    if (readline(fd, buf, MAXLINE) < 0)
    {
        err_n_die("readline error.");
    }
    while (strcmp(buf, "\r\n"))
    {
        if (readline(fd, buf, MAXLINE) < 0)
        {
            err_n_die("readline error.");
        }
    }
    return;
}

//
// returns 1 if reader, 2 if writer else -1.
//
int request_parse_uri(char *uri, int *line_num, char *content)
{
    char *ptr;
    int index = 0;
    char line[MAXLINE];

    if (strlen(uri) == 1 && uri[0] == '/')
    {
        // root
        strcat(content, "./index.html");
        return 0;
    }

    if (strstr(uri, "/reader") != NULL) {
        // reader
        if ((ptr = strstr(uri, "line_num")) == NULL)
        {
            return -1;
        }
        ptr += strlen("line_num") + 1;
        while (*ptr != '\0' && '0' <= *ptr && *ptr <= '9')
        {
            line[index++] = (*ptr);
            ptr++;
        }
        line[index] = '\0';
        *line_num = atoi(line);

        return 1;
    }

    if ((strstr(uri, "/writer") != NULL) || (strstr(uri, "/docs_writer") != NULL)) {
        // writer
        if ((ptr = strstr(uri, "line_num")) == NULL)
        {
            return -1;
        }
        ptr += strlen("line_num") + 1;
        while (*ptr != '\0' && '0' <= *ptr && *ptr <= '9')
        {
            line[index++] = (*ptr);
            ptr++;
        }
        line[index] = '\0';
        *line_num = atoi(line);

        if ((ptr = strstr(uri, "content")) == NULL)
        {
            return -1;
        }
        ptr += strlen("content") + 1;
        index = 0;
        while (*ptr != '\0')
        {
            if (*ptr != '+')
            {
                content[index++] = *ptr;
            }
            else
            {
                content[index++] = ' ';
            }
            ptr++;
        }
        content[index] = '\0';

        return 2;
    }

    if (strstr(uri, "docs_reader") != NULL) {
        // call read_all
        return 0;
    }
    
    if (strstr(uri, "?") == NULL) {
        // That represents that we are not receiving any query.
        content[0] = '\0';
        strcat(content, ".");
        strcat(content, uri);
        
        return 0;
    }

    return -1;
}

//
// Find the file type
//
void request_get_filetype(char *filename, char *filetype)
{
    if (strstr(filename, ".html"))
        strcpy(filetype, "text/html");
    else
        strcpy(filetype, "text/plain");
}

void request_serve_static(int fd, char *filename, int filesize) {
    int srcfd;
    char *srcp, filetype[MAXLINE], buf[MAXLINE];

    request_get_filetype(filename, filetype);
    if ((srcfd = open(filename, O_RDONLY, 0)) < 0)
    {
        err_n_die("open error.");
    }

    // Rather than call read() to read the file into memory,
    // which would require that we allocate a buffer, we memory-map the file
    if ((srcp = mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0)) < 0)
    {
        err_n_die("mmap error.");
    }

    if (close(srcfd) < 0)
    {
        err_n_die("close error.");
    }

    // put together response
    sprintf(buf,
            ""
            "HTTP/1.0 200 OK\r\n"
            "Server: IIITH WebServer\r\n"
            "Content-Length: %d\r\n"
            "Content-Type: %s\r\n\r\n",
            filesize, filetype);

    if (write(fd, buf, strlen(buf)) < 0)
    {
        err_n_die("write error.");
    }
    //  Writes out to the client socket the memory-mapped file
    if (write(fd, srcp, filesize) < 0)
    {
        err_n_die("write error.");
    }
    if (munmap(srcp, filesize) < 0)
    {
        err_n_die("munmap error.");
    }
}
/* Function to read the data present at the given line number
 * and send the data as response */
void read_line_file(int fd, int line_number)
{
    // semaphore to be added here.
    FILE *file = fopen("data.txt", "r");
    if (file == NULL)
    {
        err_n_die("Unable to open the file");
    }
    char buf[MAXLINE], header[MAXLINE];

    int line = 1, found = 0;

    while (fgets(buf, MAXLINE, file))
    {
        if (line == line_number)
        {
            found = 1;
            break;
        }
        line++;
    }
    if (found)
    {
        sprintf(header,
                ""
                "HTTP/1.0 200 OK\r\n"
                "Server: IIITH WebServer\r\n"
                "Content-Length: %ld\r\n"
                "Content-Type: text/plain\r\n\r\n",
                strlen(buf));
        if (write(fd, header, strlen(header)) < 0)
        {
            err_n_die("write error");
        }
        if (write(fd, buf, strlen(buf)) < 0)
        {
            err_n_die("write error");
        }
    }
    else
    {
        err_n_die("Unable to read this line");
    }
}

void edit_files(int fd, int lno, char* newln, char* uri)
{
    const char *filename = "data.txt";
    char temp[] = "temp.txt";
    char str[MAXLINE];
    char header[MAXLINE];
    newln = strcat(newln, "\n");
    int found = 0;
    FILE *fptr1, *fptr2;
    int linectr = 0;
    fptr1 = fopen(filename, "r");
    fptr2 = fopen(temp, "w");
    while (!feof(fptr1))
    {
        strcpy(str, "\0");
        fgets(str, MAXLINE, fptr1);
        if (!feof(fptr1))
        {
            linectr++;
            if (linectr != lno)
                fprintf(fptr2, "%s", str);
            else
            {
                found = 1;
                fprintf(fptr2,"%s",newln);
            }
        }
    }
    if(found){
        if (strstr(uri, "docs_writer") == NULL) {
            sprintf(header,
                ""
                "HTTP/1.0 200 OK\r\n"
                "Server: IIITH WebServer\r\n"
                "Content-Length: %ld\r\n"
                "Content-Type: \r\n\r\n",
                strlen(newln));
            if (write(fd, header , strlen(header))< 0){
                err_n_die("write error"); 
            }
            if (write(fd, newln , strlen(newln))< 0){
                err_n_die("write error") ; 
            }
        }
    }
    else
    {
        err_n_die("Unable to read this line");
    }
    fclose(fptr1);
    fclose(fptr2);
    remove(filename);
    rename(temp,filename);
    if (strstr(uri, "docs_writer") != NULL) {
        read_all(fd);
    }
}

void read_all(int fd)
{
    int srcfd;
    struct stat st;
    char *srcp, buf[MAXLINE];
    stat("data.txt", &st);
    if ((srcfd = open("data.txt", O_RDONLY, 0)) < 0)
    {
        err_n_die("open error.");
    }
    if ((srcp = mmap(0, st.st_size, PROT_READ, MAP_PRIVATE, srcfd, 0)) < 0)
    {
        err_n_die("mmap error.");
    }
    if (close(srcfd) < 0)
    {
        err_n_die("close error.");
    }
    sprintf(buf,
            ""
            "HTTP/1.0 200 OK\r\n"
            "Server: IIITH WebServer\r\n"
            "Content-Length: %ld\r\n"
            "Content-Type: text/plain\r\n\r\n",
            st.st_size );
    if (write(fd, buf, strlen(buf)) < 0)
        {
            err_n_die("write error");
        }
    
    if (write(fd, srcp, st.st_size) < 0)
    {
        err_n_die("write error.");
    }
    if (munmap(srcp, st.st_size) < 0)
    {
        err_n_die("munmap error.");
    }
}

// handle a request
void handle_request(int fd)
{
    char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];

    if (readline(fd, buf, MAXLINE) < 0)
    {
        err_n_die("readline error.");
    }

    sscanf(buf, "%s %s %s", method, uri, version);
    printf("method:%s uri:%s version:%s\n", method, uri, version);
    fflush(stdout);
    if (strcasecmp(method, "GET"))
    {
        request_error(fd, method, "501", "Not Implemented", "server does not implement this method");
        return;
    }

    // Start by reading headers.
    request_read_headers(fd);

    int line_num = -1;
    char content[MAXLINE];
    int is_reader = request_parse_uri(uri, &line_num, content);
    if (is_reader == -1)
    {
        request_error(fd, "path", "404", "Not Found", "server could not find the requested path");
        return;
    }
    if (is_reader == 0) {
        // serve static files.

        if (strstr(uri, "docs_reader") != NULL) {
            read_all(fd);
            return;
        }
        char* filename = content;

        struct stat st;
        if (stat(filename, &st) < 0) {
            // Just do a safe return from here.
            return;
        }
        request_serve_static(fd, filename, st.st_size);

    } else if (is_reader == 1) {
        // request_serve_reader(fd, line_num);
        sem_wait(&mutex1);
        r_count++;
        if (r_count == 1)
            sem_wait(&wrt);
        sem_post(&mutex1);
        read_line_file(fd, line_num);
        // sleep(1); // this is for testing 
        sem_wait(&mutex1);
        r_count--;
        if (r_count == 0)
        {
            sem_post(&wrt);
        }
        sem_post(&mutex1);

    } else {
        // writer
        sem_wait(&wrt);
        edit_files(fd, line_num, content, uri);
        // sleep(1) ; // this is for testing 
        sem_post(&wrt);
    }
    
}
