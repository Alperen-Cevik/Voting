#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#pragma warning(disable : 4996) // disabling not safe warnings

#define MAX_STR_LEN 32 // maximum credential length
#define MAX_ISSUE_COUNT 10 // maximum count of voting issues
#define MAX_ISSUE_DESC_LEN 128 // maximum issue description length

typedef enum { // enum type for votes
	VOTE_NONE = 0,
	VOTE_YES,
	VOTE_NO,
	VOTE_ABS,
} vote_t; char* vote_res_in_string[4] = { "NONE", "YES", "NO", "ABSTAINED" }; // to print vote in a readable format

typedef struct {
	char issues[MAX_ISSUE_COUNT][MAX_ISSUE_DESC_LEN];
	size_t count;
} issue_list_t; // issue list to store issues and keep track of the count

struct user {
	char username[MAX_STR_LEN];
	char password[MAX_STR_LEN];
	vote_t votes[MAX_ISSUE_COUNT];
	bool is_admin;
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

	if (*new_user = calloc(1, sizeof(user_t))) { // allocating new user
		memcpy(*new_user, buffer, sizeof(user_t)); // copying buffer to new user
		user_list->count++; // incrementing count
	}
}

linked_user_list_t* load_users(const char* fileName) { // function to load users from a file
	linked_user_list_t* user_list = create_user_list();
	FILE* file; // file handle
	errno_t res; // errno for debugging

	bool failure = 0;

	if (res = fopen_s(&file, fileName, "rb")) {
		fprintf(stderr, "Cannot open file. Error no: %d\n", res); // write error to stderr
		free_user_list(user_list); // cleaning up memory in case of failure
		return NULL;
	}

	user_t buffer = { "", "", { VOTE_NONE }, 0, NULL }; // temporary buffer to store user data

	while (fread(&buffer, sizeof(user_t), 1, file) == 1) { // reading user data
		buffer.next = NULL;
		add_new_user(user_list, &buffer);
	}

	fclose(file);

	return user_list;
}

bool save_users(const char* fileName, linked_user_list_t* user_list) { // function to save user data such as usernames, passwords and votes
	FILE* file; // file pointer
	errno_t res; // errno for debugging

	if (res = fopen_s(&file, fileName, "wb")) {
		fprintf(stderr, "Cannot open file. Error no: %d\n", res); // write error to stderr
		return 0;
	}

	for (user_t* ptr = user_list->users; ptr; ptr = ptr->next) {
		if (fwrite(ptr, sizeof(user_t), 1, file) != 1) { // writing formatted user data to the file
			fputs("Failed to write the user data to the file.\n", stderr);
			fclose(file);
			return 0;
		}
	}

	fclose(file);
	
	return 1;
}

bool load_issues(const char* fileName, issue_list_t* issue_list) { // loading issues from the specified text file
	FILE* file; // file handle
	errno_t res; // errno for debugging

	if (res = fopen_s(&file, fileName, "r")) {
		fprintf(stderr, "Cannot open file. Error no: %d\n", res); // write error to stderr
		return 0;
	}

	memset(issue_list, 0, sizeof(issue_list_t));

	do {
		int n;
		if ((n = fscanf(file, "\n%127[^\n]", &issue_list->issues[issue_list->count])) == 1) // reading issue description
			issue_list->count++;
		else if (n != EOF) {
			fputs("Failed to match issue description or id.\n", stderr);
			break;
		}
		else
			break;
	} while (1);

	fclose(file);

	return 1;
}

bool save_issues(const char* fileName, issue_list_t* issue_list) { // function to save user data such as usernames, passwords and votes
	FILE* file; // file pointer
	errno_t res; // errno for debugging

	if (res = fopen_s(&file, fileName, "w")) {
		fprintf(stderr, "Cannot open file. Error no: %d\n", res); // write error to stderr
		return 0;
	}

	for (size_t i = 0; i < issue_list->count; i++) {
		if (fprintf(file, "%s\n", issue_list->issues[i]) < 0) { // writing formatted user data to the file
			fputs("Failed to write the issue.\n", stderr);
			fclose(file);
			return 0;
		}
	}

	fclose(file);
	return 1;
}


bool is_vote_result_obvious_or_complete(linked_user_list_t* user_list, issue_list_t* issue_list, size_t index, char** res) { // function to build vote sums from users
	int yes = 0, no = 0, abs = 0;
	for (user_t* ptr = user_list->users; ptr; ptr = ptr->next) { 
		switch (ptr->votes[index]) {
		case VOTE_NO:
			no++;
			break;
		case VOTE_YES:
			yes++;
			break;
		case VOTE_ABS:
			abs++;
			break;
		}
	}

	//checking if everyone abstained from voting
	if (abs == user_list->count) {
		*res = "Everyone abstained";
		return 1;
	}

	// checking whether the voting has ended or the result is obvious
	if (!(yes < ((user_list->count - abs) / 2 + 1) && no < ((user_list->count - abs) / 2 + 1) && yes + no != user_list->count - abs)) {
		*res = (yes == no ? "DRAW" : (yes > no ? vote_res_in_string[VOTE_YES] : vote_res_in_string[VOTE_NO]));
		return 1;
	}

	return 0;
}

/*
	I could have done something way more complex and dynamic like the user list above
	but i didn't think its necesarry for a small scaled program like this.
*/

void terminate_issue(linked_user_list_t* user_list, issue_list_t* issue_list, size_t index) {
	if (issue_list->count) { // checking if there are issues to terminate
		if (index + 1 < MAX_ISSUE_COUNT) { // checking if valid index
			// deleting the issue by moving the rest of the list by -1 to overwrite it 
			memmove(&issue_list->issues[index], &issue_list->issues[index + 1], (issue_list->count - 1 - index) * MAX_ISSUE_DESC_LEN);
			for (user_t* ptr = user_list->users; ptr; ptr = ptr->next)
				// doing the same for the user votes
				memmove(&ptr->votes[index], &ptr->votes[index + 1], (MAX_ISSUE_COUNT - 1 - index) * sizeof(vote_t));
		}
		issue_list->count--;
	}
}

bool scan_for_completed_issues(linked_user_list_t* user_list, issue_list_t* issue_list) {
	bool save = 0;
	for (size_t i = 0; i < issue_list->count; i++) {
		char* result = "";
		if (is_vote_result_obvious_or_complete(user_list, issue_list, i, &result)) {
			puts("Printing results...\n");
			printf("Issue description: %s\nVoting result: %s\n\n", issue_list->issues[i], result); // printing results
			puts("Terminating issue...\n");
			terminate_issue(user_list, issue_list, i);
			i--;
			save = 1;
		}
	}
	return save;
}


int main() {
	const char* users_fileName = "users", *issues_fileName = "issues.txt"; // name of the file that stores user data

	linked_user_list_t* user_list = load_users(users_fileName); // loading users from file
	
	if (!user_list) {
		fputs("Failed to load users.\n", stderr);
		return EXIT_FAILURE;
	}

	// uncomment the lines below if you don't have any accounts saved. (don't forget to uncomment them after first run)
	//user_t u = { "admin", "password", { VOTE_NONE }, 1, NULL }; // adding the first admin account
	//add_new_user(user_list, &u); 
	//save_users(users_fileName, user_list);

	if (!user_list->count) {
		fputs("No users found\n", stderr);
		return EXIT_FAILURE;
	}
	
	issue_list_t issue_list = { { "" }, 0 };
	load_issues(issues_fileName, &issue_list);

	bool running = 1;
	while (running) {
		if (scan_for_completed_issues(user_list, &issue_list)) {
			save_users(users_fileName, user_list);
			save_issues(issues_fileName, &issue_list);
		}

		user_t* current_user = NULL; // pointer to the logged in user for easier access

		char username[MAX_STR_LEN] = { 0 }; // requesting log in
		printf("Enter your username: ");
		scanf_s("\n%31[^\n]s", username, MAX_STR_LEN);

		char password[MAX_STR_LEN] = { 0 };
		printf("Enter your password: ");
		scanf_s("\n%31[^\n]", password, MAX_STR_LEN);

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
			puts("2- Vote");
			puts("3- Create a new issue");
			puts("4- Log out");
			puts("5- Exit program");
			if (current_user->is_admin)
				puts("6- Create a new user");
			printf("Select one of these actions above: ");

			int action = 0;
			scanf_s("%d", &action);

			switch (action) {
			case 1:
				printf("\nEnter your new password: "); // requesting new password
				scanf_s("\n%31[^\n]", current_user->password, MAX_STR_LEN);
				if (save_users(users_fileName, user_list)) // saving new password with users
					puts("Your password has been successfully changed.\n");
				break;
			case 2:
				if (!issue_list.count) {
					puts("In order to vote, you need to create a voting issue.\n");
					break;
				}

				puts("");
				for (size_t i = 0; i < issue_list.count; i++) // printing all the issues
					printf("%lld- %s (Currently voted: %s)\n", i + 1, issue_list.issues[i], vote_res_in_string[current_user->votes[i]]);

				size_t selection;
				do {
					printf("Select an issue to vote: ");
					scanf_s("%lld", &selection);
					if (!selection || selection > issue_list.count)
						puts("Please select a valid issue.");
				} while (!selection || selection > issue_list.count); // looping until valid input

				vote_t* vote = &current_user->votes[selection - 1]; // subtract 1 to use it as index

				do {
					puts("\n1- Yes");
					puts("2- No");
					puts("3- Abstain");
					printf("Vote: "); // requesting vote
					scanf_s("%d", vote);
					if (*vote > VOTE_ABS || *vote < VOTE_YES) {
						*vote = VOTE_NONE;
						puts("Please select a valid option.");
					}
				} while (*vote == VOTE_NONE); // looping until valid input

				save_users(users_fileName, user_list); // saving votes with users

				puts("Your vote has been saved.\n");
				break;
			case 3:
				if (issue_list.count < MAX_ISSUE_COUNT) {
					printf("\nEnter a description for the new issue: ");
					scanf_s("\n%121[^\n]", issue_list.issues[issue_list.count++], MAX_ISSUE_DESC_LEN);
					if (save_issues(issues_fileName, &issue_list))
						puts("A new issue has been successfully created.\n");
				}
				else
					puts("\nMaximum number of issues reached, please finish the current voting issues to create a new one.\n");
				break;
			case 4:
				current_user = NULL;
				puts("Logged out.\n");
				break;
			case 5:
				current_user = NULL;
				running = 0;
				break;
			case 6: {
				if (current_user->is_admin) {
					user_t buffer = { "", "", { VOTE_NONE }, 0, NULL }; // temporary buffer to store user data
					printf("\nEnter a username: ");
					scanf_s("\n%31[^\n]", buffer.username, MAX_STR_LEN);
					printf("Enter a password: ");
					scanf_s("\n%31[^\n]", buffer.password, MAX_STR_LEN);
					add_new_user(user_list, &buffer);
					if (save_users(users_fileName, user_list))
						puts("A new user has been successfully created.\n");
				}
				else
					puts("You don't have the permission to create new users!\n");
				break;
			}
			default:
				puts("Please select a valid action.\n");
				break;
			}
		}
	}

	free_user_list(user_list); // cleaning up memory
	return 0;
}