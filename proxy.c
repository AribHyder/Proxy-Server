#include <stdio.h>
#include "csapp.h"
#include <string.h>
#include <stdlib.h>
#include "cache.h"

#define MAX_CACHE_SIZE 4000000
#define MAX_OBJECT_SIZE 1000000

void doit(int fd, CacheList *cache);
int parse_url(const char *url, char *host, char *port, char *path);
void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg);



/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";

//main from tiny with cache added into the doit function declaration

int main(int argc, char **argv)
{
  int listenfd, connfd;
  char hostname[MAXLINE], port[MAXLINE];
  socklen_t clientlen;
  struct sockaddr_storage clientaddr;
  
  CacheList cache_store;
  cache_init(&cache_store);
  
  /* Check command line args */
  if (argc != 2) {
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
    exit(1);
  }

  Signal(SIGPIPE, SIG_IGN);
  listenfd = Open_listenfd(argv[1]);
  while (1) {
    clientlen = sizeof(clientaddr);
    connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen); //line:netp:tiny:accept
    Getnameinfo((SA *) &clientaddr, clientlen, hostname, MAXLINE,
        port, MAXLINE, 0);
    printf("Accepted connection from (%s, %s)\n", hostname, port);
    
    doit(connfd, &cache_store);
   
    Close(connfd);
  }
}

//parse_url function taken directly from HW9

int parse_url(const char *url, char *host, char *port, char *path) {
  //variable declaration for moving through the url
  int a = 7;

  //return 0 if url does not start with http://
  if (strncasecmp(url, "http://", a) != 0)
  {
    return 0;
  }

  //copy remainder of url besides http://
  strcpy(host, url + a);

  //check for : or / to get host value
  int b = 0;
  while (url[a] != '\0')
  {
    if ((url[a] == ':') || (url[a] == '/'))
    {
      break;
    }
    else
    {
      //copy over host value from url
      host[b] = url[a];
    }

    a++;
    b++;
  }

  host[b] = '\0';

  //set the default port 80 for now, will be altered if port is given
  port[0]='8';
  port[1]='0';
  port[2]='\0';

  //check if port is given in url
  if (url[a] == ':') {
    a++;
    int c = 0;

    //copy over given port value from url until null terminator or / is reached
    while (1)
    {
      if ((url[a] == '\0') || (url[a] == '/'))
      {
        break;
      }
      else
      {
        port[c] = url[a];
        a++;
        c++;
      }
    }
    port[c] = '\0';
  }

  //set a default path which will be altered if one is given in the url
  path[0] = '/';
  path[1] = '\0';

  //check if path is given in the url
  if (url[a] == '/')
  {
    int d = 0;
    //loop until the end of the url and copy path since it is given
    while (1)
    {
      if (url[a] == '\0')
      {
        break;
      }
      else
      {
        path[d] = url[a];
        a++;
        d++;
      }
    }
    path[d] = '\0';
  }
  //return 1 because http:// was given
  return 1;
}

void doit(int fd, CacheList *cache)
{
  //variable declarations 

  int serverfd, length;
  char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
  char host[MAXLINE], port[MAXLINE], path[MAXLINE];
  rio_t rio, server_rio;
  char headerFile[MAXLINE] = "";
  CachedItem *item;
  
  /* Read request line and headers */
  rio_readinitb(&rio, fd);
  if (!rio_readlineb(&rio, buf, MAXLINE)){
    return;
  }
  //break up request to extract method, uri, and version from HTTP header
  sscanf(buf, "%s %s %s", method, uri, version);

  //check if it is a GET request
  if (strcasecmp(method, "GET")) {
    clienterror(fd, method, "501", "Not Implemented",
        "Tiny does not implement this method");
  }
  //change version to HTTP/1.0  
  if (!strcasecmp(version, "HTTP/1.1")) {
    strcpy(version, "HTTP/1.0\r\n");
  }
  char get[MAXLINE] = "";
  /* Parse URI from GET request */
  parse_url(uri, host, port, path);
  int count = 0;


  sprintf(get,"GET %s %s\r\n",path,version);
  //check for item in the cache and if its found, return object directly
  item = find((const char*)uri, cache);
  if (item) {

    rio_writen(fd, item->headers, strlen(item->headers));
    rio_writen(fd, item->item_p, item->size); 

    return;
  }
  
  //connect to web server, initialize, and get the request
  serverfd = open_clientfd(host, port);
  rio_readinitb(&server_rio, serverfd);

  rio_writen(serverfd, get, strlen(get));

  //move through request and make certain modifications to some headers
  while(rio_readlineb(&rio, buf, MAXLINE) > 2) {
    if(!strncasecmp(buf, "Host", 4)) {
      count = 1;

    }
    if(!strncasecmp(buf, "If-Modified-Since", 17)) {

      continue;
    }
    else if(!strncasecmp(buf, "If-None-Match", 13)) {
  
      continue;
      
    }
    else if(!strncasecmp(buf, "User-Agent", 10)) {
      
      bzero(buf, strlen(buf));
      strncpy(buf, user_agent_hdr, 88);

    }
    else if(!strncasecmp(buf, "Connection", 10)) {
      bzero(buf, strlen(buf));
      strncpy(buf, "Connection: close\r\n", 19);
  
    }
    else if(!strncasecmp(buf, "Proxy-Connection", 16)) {
      bzero(buf, strlen(buf));
      strncpy(buf, "Proxy-Connection: close\r\n", 25);

    }
    
    rio_writen(serverfd, buf, strlen(buf));
  }
  //make sure that connection and proxy-connection are both set to close
  rio_writen(serverfd, "Connection: close\r\n", 19); 
  rio_writen(serverfd, "Proxy-Connection: close\r\n", 25);


  //if host was not defined, then add it in here
  if(count ==0) {
    bzero(buf, strlen(buf));    
    strncpy(buf, "Host: ", 6);
    strncat(buf, host, strlen(host));
    strncat(buf, "\r\n", 2);
    rio_writen(serverfd, buf, strlen(buf));
  }

  

  //terminate properly
  rio_writen(serverfd, "\r\n", 2);
  
  char content[MAXLINE];
  int a = 0;
  //check response for status 200 OK, and if content length is specified (save headers to headerFile)
  while (rio_readlineb(&server_rio, buf, MAXLINE) > 2) {

    rio_writen(fd, buf, strlen(buf));

    strcat(headerFile, buf);
    
    if(!strncasecmp(buf+9, "200 OK", 6)) {
      a++;
    }
    if(!strncasecmp(buf, "Content-Length: ", 16)) {
      a++;
      strcpy(content, buf+16); 
    }
    
  }
  //take content length and capture it as an integer
  length = atoi(content);
  
  strncat(headerFile, "\r\n", 2);

  rio_writen(fd, "\r\n", 2);
  ssize_t i;
  i = 0;

  char temp[MAXLINE];
  ssize_t counter = 0;
  void *contentFile;
  
  contentFile = calloc(1, MAX_OBJECT_SIZE);
  //save binary data and also use a counter to get its length
  while((i=rio_readnb(&server_rio, temp, MAXLINE))) {

    counter = counter + i;
    rio_writen(fd, temp, i);
    strncat(contentFile, temp, i);

  }
  
  //check if count for binary data equals specified content-length from its header
  //check if this binary data count is also less than the max object size
  if ((counter == length) && (counter < MAX_OBJECT_SIZE)) {

    a++;
  }
  //if all caching test cases were passed, then allow proxy to cache the URL
  if (a == 3) {
    
    cache_URL(uri, headerFile, contentFile, counter, cache);
    /*
    printf("url   :\n%s\n\n", cache->first->url);
    printf("headers   :\n%s\n\n", (char*)cache->first->headers);
    printf("item   :\n%s\n\n", (char*)cache->first->item_p);
    printf("size   :\n%zd\n\n", cache->first->size);
    printf("cachesize   :\n%zd\n\n", cache->size);
    */
  }

}

void clienterror(int fd, char *cause, char *errnum,
    char *shortmsg, char *longmsg)
{
  char buf[MAXLINE], body[MAXBUF];

  /* Build the HTTP response body */
  sprintf(body, "<html><title>Tiny Error</title>");
  sprintf(body, "%s<body bgcolor=""ffffff"">\r\n", body);
  sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg);
  sprintf(body, "%s<p>%s: %s\r\n", body, longmsg, cause);
  sprintf(body, "%s<hr><em>The Tiny Web server</em>\r\n", body);

  /* Print the HTTP response */
  sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
  sprintf(buf, "%sContent-type: text/html\r\n", buf);
  sprintf(buf, "%sContent-length: %d\r\n\r\n", buf, (int)strlen(body));
  rio_writen(fd, buf, strlen(buf));
  rio_writen(fd, body, strlen(body));
}
