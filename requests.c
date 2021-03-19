#include "aux.h"

void register_client()
{
	char *payload = NULL;
	char username[50], password[50];
	int code;

	printf("Insert username:\n");
	scanf("\n%[^\n]", username);
	printf("Insert password:\n");
	scanf("\n%[^\n]", password);

	JSON_Value *root_value = json_value_init_object();
	JSON_Object *root_object = json_object(root_value);

	json_object_set_string(root_object, "username", username);
	json_object_set_string(root_object, "password", password);

	payload = json_serialize_to_string_pretty(root_value);

	check_code(get_response(post(REGISTER, payload, NULL)), &code);

	json_free_serialized_string(payload);
	json_value_free(root_value);
}

char *login_client(char *username)
{
	char password[50];
	char *payload = NULL, *response, *line;
	char *cookie = NULL, *copy;
	int code;

	printf("Insert username:\n");
	scanf("\n%[^\n]", username);
	printf("Insert password:\n");
	scanf("\n%[^\n]", password);

	JSON_Value *root_value = json_value_init_object();
	JSON_Object *root_object = json_object(root_value);

	json_object_set_string(root_object, "username", username);
	json_object_set_string(root_object, "password", password);

	payload = json_serialize_to_string_pretty(root_value);

	response = get_response(post(LOGIN, payload, NULL));
	copy = malloc(strlen(response) * sizeof(char));
	strcpy(copy, response);
	check_code(response, &code);

	// get cookie
	if(code < 300)
	{
		line = strtok(copy, LINE_TERM);
		while((line != NULL) && (strstr(line, "Set-Cookie: ") == 0))
		{
			line = strtok(NULL, LINE_TERM);
		}
		if(line != NULL)
		{
			cookie = line + strlen("Set-Cookie: ");
		}
	}
	else
	{
		strcpy(username, NULL_STR);
	}

	json_free_serialized_string(payload);
	json_value_free(root_value);

	return cookie;
}

char *lib_acc(char *username, char *cookie)
{
	char *response, *jsons;
	char *token_JWT = NULL;
	int code;

	JSON_Value *value = json_value_init_object();
	JSON_Object *entrie;

	response = get_response(get(LIB_ACC, NULL, cookie, NULL));
	jsons = check_code(response, &code);

	if(code < 300)
	{
		token_JWT = calloc(strlen(jsons), sizeof(char));
		value = json_parse_string(jsons);
		entrie = json_object(value);
		strcpy(token_JWT, json_object_get_string (entrie, "token"));
	}
	else
	{
		timeout(code, username, cookie, token_JWT);
	}
	json_value_free(value);

	return token_JWT;
}

void get_books(char *username, char *cookie, char *token_JWT)
{
	char *response, *jsons;
	int code;

	JSON_Value *value = json_value_init_object();
	JSON_Array *entries;
	JSON_Object *entrie;

	response = get_response(get(BOOKS, NULL, cookie, token_JWT));
	jsons = check_code(response, &code);

	if(code < 300)
	{
		if(strstr(jsons, "{\"") != 0)
		{
			value = json_parse_string(jsons);
			entries = json_array(value);
			for(int i = 0; i < json_array_get_count(entries); i++)
			{
				entrie = json_array_get_object(entries, i);
				printf("#%d: ", (int)json_object_get_number(entrie, "id"));
				printf("\"%s\"\n", json_object_get_string(entrie, "title"));
			}
		}
		else
		{
			printf("The library is empty\n");
		}
	}
	else
	{
		timeout(code, username, cookie, token_JWT);
	}
	json_value_free(value);
}

void get_book(char *username, char *cookie, char *token_JWT)
{
	char *response, *jsons;
	char id[10];
	int code, i;

	while(1)
	{
		printf("Insert book id:\n");
		scanf("\n%[^\n]", id);
		for(i = 0; i < strlen(id); i++)
		{
			if((id[i] < '0') || (id[i] > '9'))
			{
				break;
			}
		}
		if(i < strlen(id))
		{
			printf("Invalid id! Must be a number!\nTry again:\n");
		}
		else
		{
			break;
		}
	}

	JSON_Value *value = json_value_init_object();
	JSON_Array *entries;
	JSON_Object *entrie;

	response = get_response(get(BOOKS, id, cookie, token_JWT));
	jsons = check_code(response, &code);

	if(code < 300)
	{
		value = json_parse_string(jsons);
		entries = json_array(value);
		for(i = 0; i < json_array_get_count(entries); i++)
		{
			entrie = json_array_get_object(entries, i);
			printf("title: \"%s\"\n", json_object_get_string(entrie, "title"));
			printf("author: %s\n", json_object_get_string(entrie, "author"));
			printf("publisher: %s\n", json_object_get_string(entrie, "publisher"));
			printf("genre: %s\n", json_object_get_string(entrie, "genre"));
			printf("number of pages: %d\n", (int)json_object_get_number(entrie, "page_count"));

			if(i != json_array_get_count(entries) - 1)
			{
				printf("\n");
			}
		}
	}
	else
	{
		timeout(code, username, cookie, token_JWT);
	}
	json_value_free(value);
}

void add_book(char *username, char *cookie, char *token_JWT)
{
	char *payload = NULL, *response;
	char title[LINELEN], author[LINELEN], genre[50];
	char pg_cnt[10], pub[LINELEN];
	int pages, code, i;

	printf("Insert title:\n");
	scanf("\n%[^\n]", title);
	printf("Insert author:\n");
	scanf("\n%[^\n]", author);
	printf("Insert genre:\n");
	scanf("\n%[^\n]", genre);
	while(1)
	{
		printf("Insert number of pages:\n");
		scanf("\n%[^\n]", pg_cnt);
		for(i = 0; i < strlen(pg_cnt); i++)
		{
			if((pg_cnt[i] < '0') || (pg_cnt[i] > '9'))
			{
				break;
			}
		}
		if(i < strlen(pg_cnt))
		{
			printf("Invalid number of pages! Must be a number!\nTry again:\n");
		}
		else
		{
			break;
		}
	}
	pages = atoi(pg_cnt);
	printf("Insert pub:\n");
	scanf("\n%[^\n]", pub);

	JSON_Value *root_value = json_value_init_object();
	JSON_Object *root_object = json_object(root_value);

	json_object_set_string(root_object, "title", title);
	json_object_set_string(root_object, "author", author);
	json_object_set_string(root_object, "genre", genre);
	json_object_set_number(root_object, "page_count", pages);
	json_object_set_string(root_object, "publisher", pub);

	payload = json_serialize_to_string_pretty(root_value);

	response = get_response(post(BOOKS, payload, token_JWT));
	check_code(response, &code);
	if(code >= 300)
	{
		timeout(code, username, cookie, token_JWT);
	}

	json_free_serialized_string(payload);
	json_value_free(root_value);
}

void del_book(char *username, char *cookie, char *token_JWT)
{
	char *response;
	int code, i;
	char id[10];

	while(1)
	{
		printf("Insert book id:\n");
		scanf("\n%[^\n]", id);
		for(i = 0; i < strlen(id); i++)
		{
			if((id[i] < '0') || (id[i] > '9'))
			{
				break;
			}
		}
		if(i < strlen(id))
		{
			printf("Invalid id! Must be a number!\nTry again:\n");
		}
		else
		{
			break;
		}
	}

	response = get_response(del(BOOKS, id, cookie, token_JWT));
	check_code(response, &code);
	if(code >= 300)
	{
		timeout(code, username, cookie, token_JWT);
	}
}

void logout(char *cookie)
{
	char *response;
	int code;

	response = get_response(get(LOGOUT, NULL, cookie, NULL));
	check_code(response, &code);
}