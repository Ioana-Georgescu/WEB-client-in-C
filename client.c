#include "aux.h"

int main()
{
	char command[15]; // cea mai lunga comanda : 13
	char *cookie = NULL, *token_JWT = NULL;
	char username[50];

	strcpy(username, NULL_STR);

	while(1)
	{
		scanf("\n%[^\n]", command);

		if(strcmp(command, "register") == 0)
		{
			register_client();
		}
		else if(strcmp(command, "login") == 0)
		{
			if(strcmp(username, NULL_STR) != 0)
			{
				printf("User %s is already logged in!\nLogout first!\n",
					username);
			}
			else
			{
				cookie = login_client(username);
			}
		}
		else if(strcmp(command, "enter_library") == 0)
		{
			if(cookie == NULL)
			{
				printf("Login first!\n");
			}
			else
			{
				token_JWT = lib_acc(username, cookie);
			}
		}
		else if(strcmp(command, "logout") == 0)
		{
			if(cookie == NULL)
			{
				printf("Login first!\n");
			}
			else
			{
				logout(cookie);
				cookie = NULL;
				token_JWT = NULL;
				strcpy(username, NULL_STR);
			}
		}
		else if(strcmp(command, "get_books") == 0)
		{
			if(token_JWT == NULL)
			{
				printf("Enter library first!\n");
			}
			else
			{
				get_books(username, cookie, token_JWT);
			}
		}
		else if(strcmp(command, "get_book") == 0)
		{
			if(token_JWT == NULL)
			{
				printf("Enter library first!\n");
			}
			else
			{
				get_book(username, cookie, token_JWT);
			}
		}
		else if(strcmp(command, "add_book") == 0)
		{
			if(token_JWT == NULL)
			{
				printf("Enter library first!\n");
			}
			else
			{
				add_book(username, cookie, token_JWT);
			}
		}
		else if(strcmp(command, "delete_book") == 0)
		{
			if(token_JWT == NULL)
			{
				printf("Enter library first!\n");
			}
			else
			{
				del_book(username, cookie, token_JWT);
			}
		}
		else if(strcmp(command, "exit") == 0)
		{
			return 0;
		}
		else
		{
			printf("Invalid command\n");
		}
	}

	return 0;
}
