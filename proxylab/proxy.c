#include <stdio.h>
#include "csapp.h"
#include "cache.h"

#define URL_HEADER_LEN 7

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";
static const char *connection_hdr = "Connection: close\r\n";
static const char *proxy_connection_hdr = "Proxy-Connection: close\r\n";

void *handle_request(void *fdp);
void read_requesthdrs(rio_t *rp);
int parse_url(char *uri, char *hostname, char *port, char *remaining);
void clienterror(int fd, char *cause, char *errnum, 
                 char *shortmsg, char *longmsg);
void send_request(int fd, char *hostname, char *remaining);

int main(int argc, char *argv[])
{
    int listenfd, *connfdp;
    char hostname[MAXLINE], port[MAXLINE];
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    pthread_t tid;

    /* Check command line args */
    if (argc != 2) {
	fprintf(stderr, "usage: %s <port>\n", argv[0]);
	exit(1);
    }

    listenfd = Open_listenfd(argv[1]);
    while (1) {
	clientlen = sizeof(clientaddr);
        connfdp = Malloc(sizeof(int));
	*connfdp = Accept(listenfd, (SA *)&clientaddr, &clientlen);
        Getnameinfo((SA *) &clientaddr, clientlen, hostname, MAXLINE, 
                    port, MAXLINE, 0);
        printf("Accepted connection from (%s, %s)\n", hostname, port);      
        Pthread_create(&tid, NULL, handle_request, connfdp);                                           
    }
    return 0;
}

/*
 * handle_request - handle one HTTP request/response transaction
 */
void *handle_request(void *fdp) 
{
    char buf[MAXLINE], method[MAXLINE], url[MAXLINE], version[MAXLINE];
    char hostname[MAXLINE], port[MAXLINE], remaining[MAXLINE];
    rio_t rio, hostrio;
    int fd, hostfd;
    ssize_t read_amount;

    fd = *((int *)fdp);
    Pthread_detach(pthread_self());
    Free(fdp);

    /* Read request line and headers */
    Rio_readinitb(&rio, fd);
    if (!Rio_readlineb(&rio, buf, MAXLINE))
        return NULL;
    printf("%s", buf);
    sscanf(buf, "%s %s %s", method, url, version);       
    if (strcasecmp(method, "GET")) {                     
        clienterror(fd, method, "501", "Not Implemented",
                    "Proxy server does not implement this method");
        return NULL;
    }                                                    
    read_requesthdrs(&rio);                              

    /* Parse URL from GET request */
    if (parse_url(url, hostname, port, remaining)) {
        hostfd = Open_clientfd(hostname + URL_HEADER_LEN, port);
        send_request(hostfd, hostname, remaining);
    }

    Rio_readinitb(&hostrio, hostfd);
    while ((read_amount = Rio_readlineb(&hostrio, buf, MAXLINE)))
        Rio_writen(fd, buf, read_amount); 
    
    Close(fd);
    Close(hostfd);

    return NULL;
}

/*
 * read_requesthdrs - read HTTP request headers
 */
void read_requesthdrs(rio_t *rp) 
{
    char buf[MAXLINE];

    Rio_readlineb(rp, buf, MAXLINE);
    printf("%s", buf);
    while(strcmp(buf, "\r\n")) {
	Rio_readlineb(rp, buf, MAXLINE);
	printf("%s", buf);
    }
    return;
}
/* $end read_requesthdrs */

/*
 * parse_url - parse url into hostname and remaining.
 * returns 1 if successful 0 otherwise.
 */
int parse_url(char *url, char *hostname, char *port, char *remaining)
{
    char *port_start, *remaining_start;
    int copy_size;

    if (!strstr(url, "http://")) // is invalid url form
        return 0;   

    port_start = strstr(url + URL_HEADER_LEN, ":");
    remaining_start = strstr(url + URL_HEADER_LEN, "/");
    if (!remaining_start)
        return 0;

    /* Copy port number to port buf
     * If the port number is not included, the set port number as 80. */
    if (!port_start) {
        strcpy(port, "80");
        port_start = remaining_start;   
    }
    else {
        copy_size = remaining_start - port_start - 1;

        strncpy(port, port_start + 1, copy_size);
        port[copy_size] = '\0';
    }

    /* Copy hostname */
    copy_size = port_start - url;

    strncpy(hostname, url, copy_size);
    hostname[copy_size] = '\0';
    
    /* Copy the remaining part of URL */
    strcpy(remaining, remaining_start);         
    
    return 1;
}


/*
 * clienterror - returns an error message to the client
 */
void clienterror(int fd, char *cause, char *errnum, 
		 char *shortmsg, char *longmsg) 
{
    char buf[MAXLINE], body[MAXBUF];

    /* Build the HTTP response body */
    sprintf(body, "<html><title>Proxy Error</title>");
    sprintf(body, "%s<body bgcolor=""ffffff"">\r\n", body);
    sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg);
    sprintf(body, "%s<p>%s: %s\r\n", body, longmsg, cause);
    sprintf(body, "%s<hr><em>The Proxy Server</em>\r\n", body);

    /* Print the HTTP response */
    sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-type: text/html\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-length: %d\r\n\r\n", (int)strlen(body));
    Rio_writen(fd, buf, strlen(buf));
    Rio_writen(fd, body, strlen(body));
}

void send_request(int fd, char *hostname, char *remaining)
{
    char buf[MAXLINE];

    /* Print the HTTP request */
    sprintf(buf, "GET %s HTTP/1.0\r\n", remaining);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Host: %s\r\n", hostname + URL_HEADER_LEN);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "%s%s%s\r\n", user_agent_hdr, connection_hdr, proxy_connection_hdr);
    Rio_writen(fd, buf, strlen(buf));
}
