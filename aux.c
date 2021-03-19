#include "aux.h"

int open_connection()
{
	int ret;

	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	DIE(sockfd < 0, "Error opening socket");

	struct sockaddr_in serv_addr;
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);
	ret = inet_aton(HOST_IP, &serv_addr.sin_addr);
	DIE(ret < 1, "Address conversion failed");

	ret = connect(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr));
	DIE(ret < 0, "Error connecting");

	return sockfd;
}

char *get_response(char *message)
{
	int bytes, sent = 0;
	int total = strlen(message);
	int i = 0;
	char buffer[BUFLEN];
	char *response = calloc(BUFLEN, sizeof(char));
	unsigned short resp_len = 0;

	int sockfd = open_connection();

	do
	{
		bytes = write(sockfd, message + sent, total - sent);
		DIE(bytes < 0, "Error sending mesage to server");

		if (bytes == 0)
		{
			if(i != 0)
			{
				break;
			}
			else // serverul a intrerupt conexiunea inainte sa se primeasca un mesaj
			{
				close(sockfd);
				sockfd = open_connection();
				continue;
			}
		}

		i++;
		sent += bytes;
	} while (sent < total);

	i = 0;

	while(1)
	{
		memset(buffer, 0, BUFLEN);
		bytes = read(sockfd, buffer, BUFLEN);
		DIE(bytes < 0, "Error receiving mesage from server");

		if(bytes == 0)
		{
			if(i != 0)
			{
				break;
			}
			else // serverul a intrerupt conexiunea inainte sa se primeasca un mesaj
			{
				close(sockfd);
				sockfd = open_connection();
				continue;
			}
		}

		i++;

		buffer[BUFLEN] = '\0';
		if(resp_len == 0)
		{
			strcpy(response, buffer);
			resp_len = BUFLEN;
		}
		else if(strlen(response) + strlen(buffer) >= resp_len)
		{
			response = realloc(response,
				(strlen(response) + strlen(buffer)) * sizeof(char));
			strcat(response, buffer);
			resp_len = strlen(response);
		}
		else
		{
			strcat(response, buffer);
		}
	}

	close(sockfd);

	return response;
}

void add(char *message, const char *line)
{
	strcat(message, line);
	strcat(message, LINE_TERM);
}

char *get(char *url, char *param, char *cookie, char *token_JWT)
{
	char *message = calloc(BUFLEN, sizeof(char));
	char *line = calloc(LINELEN, sizeof(char));

	if (param != NULL)
	{
		sprintf(line, "GET %s/%s HTTP/1.1", url, param);
	}
	else
	{
		sprintf(line, "GET %s HTTP/1.1", url);
	}
	add(message, line);

	add(message, HOST);

	// cookie-ul de sesiune
	if(cookie != NULL)
	{
		sprintf(line, "Cookie: %s", cookie);
		add(message, line);
	}

	// token-ul JWT
	if(token_JWT != NULL)
	{
		sprintf(line, "Authorization: Bearer %s", token_JWT);
		add(message, line);
	}

	strcat(message, LINE_TERM);
	return message;
}

char *post(char *url, char *payload, char *token_JWT)
{
	char *message = calloc(BUFLEN, sizeof(char));
	char *line = calloc(LINELEN, sizeof(char));

	sprintf(line, "POST %s HTTP/1.1", url);
	add(message, line);
	
	add(message, HOST);
	add(message, CONTENT_TYPE);
	add(message, ACCEPT);
	
	// token-ul JWT
	if(token_JWT != NULL)
	{
		sprintf(line, "Authorization: Bearer %s", token_JWT);
		add(message, line);
	}

	sprintf(line, "Content-Length: %ld", strlen(payload));
	add(message, line);

	strcat(message, LINE_TERM);

	add(message, payload);

	return message;
}

char *del(char *url, char *param, char *cookie, char *token_JWT)
{
	char *message = calloc(BUFLEN, sizeof(char));
	char *line = calloc(LINELEN, sizeof(char));

	sprintf(line, "DELETE %s/%s HTTP/1.1", url, param);
	add(message, line);

	add(message, HOST);

	// cookie-ul de sesiune
	if(cookie != NULL)
	{
		sprintf(line, "Cookie: %s", cookie);
		add(message, line);
	}

	// token-ul JWT
	if(token_JWT != NULL)
	{
		sprintf(line, "Authorization: Bearer %s", token_JWT);
		add(message, line);
	}

	strcat(message, LINE_TERM);
	return message;
}

char *check_code(char *response, int *code)
{
	char *line, *p, *copy;
	char *body = calloc(LINELEN, sizeof(char));

	copy = calloc(strlen(response), sizeof(char));
	strcpy(copy, response);

	p = strstr(response, END_OF_HEADER);
	strcpy(body, p + strlen(END_OF_HEADER));

	line = strtok(response, LINE_TERM); // prima linie din raspuns
	printf("\n%s\n", line + strlen(REPONSE_HEADER)); // mesajul din prima linie

	JSON_Value *value = json_value_init_object();
	JSON_Object *entrie;
	JSON_Array *entries;

	p = strtok(line, " ");
	p = strtok(NULL, " ");
	*code = atoi(p);

	if(*code >= 300)
	{
		if(*code == 429) // nu trimite json
		{
			line = strtok(copy, LINE_TERM);
			while(strstr(line, "Retry-After: ") == 0)
			{
				line = strtok(NULL, LINE_TERM);
			}
			printf("Please try again after %d seconds\n",
				atoi(line + strlen("Retry-After: ")));
		}
		else if(strlen(body) != 0)
		{
			printf("Errors:\n");
			if(strstr(body, "[{\"") != 0) // vector de json
			{
				value = json_parse_string(body);
				entries = json_array(value);
				for(unsigned char i = 0; i < json_array_get_count(entries); i++)
				{
					entrie = json_array_get_object(entries, i);
					printf("%s\n", json_object_get_string(entrie, "error"));
				}
			}
			else
			{
				if(strstr(body, "{\"") != 0) // mesaj de eroare in json
				{
					value = json_parse_string(body);
					entrie = json_object(value);
					printf("%s\n", json_object_get_string(entrie, "error"));
				}
				else // mesaj de eroare, plain text
				{
					printf("%s\n", body);
				}
			}
		}
	}
	else if(strlen(body) != 0)
	{
		return body;
	}

	printf("\n");

	free(copy);
	json_value_free(value);

	return NULL;
}

void timeout(int code, char *username, char *cookie, char *token_JWT)
{
	if((code == 401)
		|| (code == 403)
		|| (code == 419)
		|| (code == 440)
		|| (code == 498))
	{
		printf("Credentials timed out!\nLogin again!\n");
		strcpy(username, NULL_STR);
		cookie = NULL;
		token_JWT = NULL;
	}
}