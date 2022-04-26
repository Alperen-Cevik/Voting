#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#pragma warning(disable : 4996) // disabling not safe warnings

#define MAX_STR_LEN 32 // maximum credential length

typedef enum { // enum type for votes
	VOTE_NONE = 0,
	VOTE_YES,
	VOTE_NO,
	VOTE_ABS,
} vote_res_t; char* vote_res_in_string[3] = { "YES", "NO", "ABSTAINED" }; // to print vote in a readable format

struct user {
	char username[MAX_STR_LEN];
	char password[MAX_STR_LEN];
	vote_res_t vote_res;
	struct user* next; // user pointer that points to the next user node 
}; // user struct that stores the username, password, vote result and the next user node

typedef struct user user_t;

typedef struct {
	user_t* users;
	size_t count;
} linked_user_list_t; // a linked user list struct that stores first user node and the count of the users

linked_user_list_t* create_user_list() { // function to allocate a new list object
	return calloc(1, sizeof(linked_user_list_t));
}

void free_user_list(linked_user_list_t* user_list) { // function to free the list object
	user_t* ptr = user_list->users;
	while (ptr) {
		user_t* temp = ptr;
		ptr = ptr->next;
		free(temp);
	}
	free(user_list);
}

void add_new_user(linked_user_list_t* user_list, user_t* buffer) { // function to create and add a new user
	user_t** new_user = &user_list->users;
	while (*new_user) // searching for the last node
		new_user = &(*new_user)->next;

	*new_user = calloc(1, sizeof(user_t)); // allocating new user
	memcpy(*new_user, buffer, sizeof(user_t)); // copying buffer to new user
	user_list->count++; // incrementing count
}

linked_user_list_t* load_users(const char* fileName) { // function to load users from a file
	linked_user_list_t* user_list = create_user_list();
	FILE* file; // file handle
	errno_t res; // errno for debugging

	bool failure = 0;

	if (res = fopen_s(&file, fileName, "r")) {
		fprintf(stderr, "Cannot open file. Error no: %d\n", res); // write error to stderr
		failure = 1;
	}

	do {
		user_t buffer = { "", "", VOTE_NONE, NULL }; // temporary buffer to store user data
		int n;
		if ((n = fscanf(file, "%31s%31s%d", buffer.username, buffer.password, &buffer.vote_res)) == 3)
			add_new_user(user_list, &buffer);
		else if (n != EOF) {
			fputs("Failed to match username, password or vote result.\n", stderr);
			fclose(file);
			failure = 1;
			break;
		}
		else
			break;
	} while (1);

	fclose(file);

	if (failure) {
		free_user_list(user_list); // cleaning up memory in case of failure
		return NULL;
	}

	return user_list;
}

bool save_users(const char* fileName, linked_user_list_t* user_list) { // function to save user data such as usernames, passwords and votes
	FILE* file; // file pointer
	errno_t res; // errno for debugging

	if (res = fopen_s(&file, fileName, "w")) {
		fprintf(stderr, "Cannot open file. Error no: %d\n", res); // write error to stderr
		return EXIT_FAILURE;
	}

	for (user_t* ptr = user_list->users; ptr; ptr = ptr->next) {
		if (fprintf(file, "%s %s %d\n", ptr->username, ptr->password, ptr->vote_res) < 0) { // writing formatted user data to the file
			fputs("Failed to write the user data to the file.\n", stderr);
			fclose(file);
			return 0;
		}
	}
	fclose(file);
	return 1;
}

void build_vote_results(linked_user_list_t* user_list, int* yes, int* no, int* abs) { // function to build vote sums from users
	*yes = 0;
	*no = 0;
	*abs = 0;
	for (user_t* ptr = user_list->users; ptr; ptr = ptr->next) {
		switch (ptr->vote_res) {
		case VOTE_NO:
			(*no)++;
			break;
		case VOTE_YES:
			(*yes)++;
			break;
		case VOTE_ABS:
			(*abs)++;
			break;
		}
	}
}

int main() {
	const char* fileName = "users.txt"; // name of the file that stores user data

	linked_user_list_t* user_list = load_users(fileName); // loading users from file

	if (!user_list) {
		fputs("Failed to load users.\n", stderr);
		return EXIT_FAILURE;
	}

	int yes = 0, no = 0, abs = 0;
	build_vote_results(user_list, &yes, &no, &abs);

	while (yes < ((user_list->count - abs) / 2 + 1) && no < ((user_list->count - abs) / 2 + 1) && yes + no != user_list->count - abs) { // checking whether the voting has ended or the result is obvious
		user_t* current_user = NULL; // pointer to the logged in user for easier access

		char username[MAX_STR_LEN]; // requesting log in
		printf("Enter your username: ");
		scanf_s("%31s", username, MAX_STR_LEN);

		char password[MAX_STR_LEN];
		printf("Enter your password: ");
		scanf_s("%31s", password, MAX_STR_LEN);

		for (user_t* ptr = user_list->users; ptr; ptr = ptr->next) { //checking login data
			if (!strcmp(username, ptr->username) && !strcmp(password, ptr->password)) {
				current_user = ptr; // login successful
				printf("Logged in to %s\n\n", username);
			}
		}

		if (!current_user) // login failed
			puts("Wrong username or password.\n");

		while (current_user) { // looping until user logs out
			puts("1- Change your password");
			if (current_user->vote_res != VOTE_NONE)
				printf("2- Vote (Currently voted: %s)\n", vote_res_in_string[current_user->vote_res - 1]); // printing the latest vote
			else
				puts("2- Vote");
			puts("3- Log out");
			printf("Select one of these actions above: ");

			int action = 0;
			scanf_s("%d", &action);

			switch (action) {
			case 1:
				printf("Enter your new password: "); // requesting new password
				scanf_s("%31s", current_user->password, MAX_STR_LEN);
				if (save_users(fileName, user_list)) // saving new password with users
					puts("Your password has been successfully changed.\n");
				break;
			case 2:
				current_user->vote_res = VOTE_NONE;
				while (current_user->vote_res == VOTE_NONE) { // looping until valid input
					puts("\n1- Yes");
					puts("2- No");
					puts("3- Abstain");
					printf("Vote for ending the class: "); // requesting vote
					scanf_s("%d", &current_user->vote_res);
					if (current_user->vote_res > VOTE_ABS || current_user->vote_res < VOTE_YES) {
						current_user->vote_res = VOTE_NONE;
						puts("Please select a valid option.\n");
					}
					else
						break;
				}
				save_users(fileName, user_list); // saving votes with users
				puts("Your vote has been saved.\n");
				break;
			case 3:
				current_user = NULL;
				puts("Logged out.\n");
				break;
			default:
				puts("Please select a valid action.\n");
				break;
			}
		}

		build_vote_results(user_list, &yes, &no, &abs);
	}

	if (yes + no != user_list->count - abs) // checking if the result is obvious
		puts("Vote result is obvious, terminating...");

	printf("\nResult of the vote: %s\nYes: %d\nNo: %d\nAbsent: %d\nTotal user count: %d\n", (yes == no ? "DRAW" : (yes > no ? vote_res_in_string[0] : vote_res_in_string[1])), yes, no, abs, (int)user_list->count); // printing results

	for (user_t* ptr = user_list->users; ptr; ptr = ptr->next) // resetting votes to be able to vote again on start
		ptr->vote_res = VOTE_NONE;

	save_users(fileName, user_list); // saving reseted votes

	free_user_list(user_list); // cleaning up memory

	return 0;
}