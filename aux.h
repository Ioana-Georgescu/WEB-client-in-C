#ifndef _AUX_
#define _AUX_

#include <stdio.h>      /* printf, sprintf */
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "parson.h"

#define DIE(assertion, call_description)	\
	do {									\
		if (assertion) {					\
			fprintf(stderr, "(%s, %d): ",	\
					__FILE__, __LINE__);	\
			perror(call_description);		\
			exit(EXIT_FAILURE);				\
		}									\
	} while(0)

#define NULL_STR "\0"

#define BUFLEN 4096
#define LINELEN 1000
#define LINE_TERM "\r\n"
#define END_OF_HEADER "\r\n\r\n"
#define REPONSE_HEADER "HTTP/1.X XXX "

#define HOST_IP "3.8.116.10"
#define PORT 8080

#define REGISTER "/api/v1/tema/auth/register"
#define LOGIN "/api/v1/tema/auth/login"
#define LIB_ACC "/api/v1/tema/library/access"
#define BOOKS "/api/v1/tema/library/books"
#define LOGOUT "/api/v1/tema/auth/logout"

#define HOST "Host: ec2-3-8-116-10.eu-west-2.compute.amazonaws.com"
#define CONTENT_TYPE "Content-Type: application/json"
#define ACCEPT "Accept: application/json"

int open_connection();
char *get_response(char *message);

char *get(char *url, char *param, char *cookie, char *token_JWT);
char *post(char *url, char *payload, char *token_JWT);
char *del(char *url, char *param, char *cookie, char *token_JWT);

char *check_code(char *response, int *code);
void timeout(int code, char *username, char *cookie, char *token_JWT);

void register_client();
char *login_client(char *username);
char *lib_acc(char *username, char *cookie);
void get_books(char *username, char *cookie, char *token_JWT);
void get_book(char *username, char *cookie, char *token_JWT);
void add_book(char *username, char *cookie, char *token_JWT);
void del_book(char *username, char *cookie, char *token_JWT);
void logout(char *cookie);

#endif