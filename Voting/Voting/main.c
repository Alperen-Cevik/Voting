#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define MAX_STR_LEN 32 // maximum credential length
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0])) // macro to get static array size

typedef struct {
	char username[MAX_STR_LEN];
	char password[MAX_STR_LEN];
	bool voted;
} user_t;

int main() {			//Alperen						Belgin						  Seyfullah
	user_t users[3] = { { "20211701050", "pwd1", 0 }, { "20211701070", "pwd2", 0 }, { "20171704025", "pwd3", 0 } };
	size_t size = ARRAY_SIZE(users);
	int yes = 0, no = 0;
	while (yes < size / 2 + 1 && no < size / 2 + 1) { // checking whether the vote has ended or the result is obvious
		user_t buffer = { "", "", 0 };
		size_t logged_in = -1;
		printf("Enter your username: "); //requesting log in
		scanf_s("%s", &buffer.username, MAX_STR_LEN);
		printf("Enter your password: ");
		scanf_s("%s", &buffer.password, MAX_STR_LEN);
		for (size_t i = 0; i < ARRAY_SIZE(users); i++) { //checking login data
			if (!strcmp(buffer.username, users[i].username) && !strcmp(buffer.password, users[i].password)) {
				logged_in = i; // login successful
				printf("Logged in to %s\n", buffer.username);
			}
		}
		if (logged_in != -1) {
			if (!users[logged_in].voted) {
				while (1) {
					char res[5];
					printf("Vote for ending the class (YES/NO): "); // requesting vote
					scanf_s("%s", res, (unsigned int)sizeof(res));
					users[logged_in].voted = 1;
					if (!strcmp(res, "YES")) { 
						yes++;
						break;
					}
					else if (!strcmp(res, "NO")) {
						no++;
						break;
					}
					else {
						puts("Please enter either 'YES' or 'NO'");
						users[logged_in].voted = 0;
					}
				}
				if (users[logged_in].voted)
					puts("Your vote has been saved.");
			}
			else
				puts("You have already voted for this issue.");
		}
		else
			puts("Wrong username or password.");
	}
	if (yes + no != size) // printing results
		puts("Vote result is obvious, terminating...");
	printf("\nResult of the vote:\nYes: %d\nNo: %d\nTotal user count: %d\n", yes, no, (int)ARRAY_SIZE(users));
	return 0;
}