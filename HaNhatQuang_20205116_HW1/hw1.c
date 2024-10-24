#include <stdio.h>
#include <stdlib.h>
#include <string.h>


typedef struct hw1
{
    /* data */
    char username[100];
    char password[100];
    int status;  // 1: Active, 0: Blocked
    int signin;  // 1: Signed in, 0: Not signed in
    struct hw1 *next;
} userInfo;

userInfo *root, *cur, *new1;

//menu
void displayMenu(){
	printf("USER MANAGEMENT PROGRAM \n--------------------------\n  1. Register \n  2. Sign in \n  3. Search \n  4. Sign out \nYour choice(1-4, other to quit) : ");
    return ;
}

userInfo* checkUserExist(char username[100]){ // search user with name and return userInfor if founded
    cur = root;
    while (cur!= NULL){
    	if (!strcmp(cur->username,username)) {
    	  return cur;
    	}
    	cur = cur->next;
    }
    return NULL;
}

// Đọc thông tin người dùng từ file
void getUserInfo(FILE *f) {
    char line[300];
    root = (userInfo*) malloc(sizeof(userInfo));
    root->next = NULL;
    cur = root;

    while (fgets(line, sizeof(line), f)) {
        new1 = (userInfo*) malloc(sizeof(userInfo));
        sscanf(line, "%[^:]:%[^:]:%d", new1->username, new1->password, &new1->status);
        new1->signin = 0; // Chưa đăng nhập
        new1->next = NULL;
        cur->next = new1;
        cur = new1;
    }
}

// Lưu thông tin người dùng vào file
void saveToFile(FILE *f) {
    cur = root->next; // Bỏ qua phần root
    f = freopen("user.txt", "w", f); // Mở lại file để ghi mới
    while (cur != NULL) {
        fprintf(f, "%s:%s:%d\n", cur->username, cur->password, cur->status);
        cur = cur->next;
    }
}

// Chức năng đăng ký tài khoản mới
void Register(FILE *f) {
    printf("--> Register\n");
    char name[100], pass[100];
    printf("Username: ");
    scanf("%s", name);

    if (checkUserExist(name) != NULL) {
        printf("Account exists.\n");
    } else {
        printf("Password: ");
        scanf("%s", pass);
        new1 = (userInfo*) malloc(sizeof(userInfo));
        strcpy(new1->username, name);
        strcpy(new1->password, pass);
        new1->status = 1;  // Active
        new1->signin = 0;
        new1->next = NULL;

        // Thêm vào cuối danh sách liên kết
        cur = root;
        while (cur->next != NULL) cur = cur->next;
        cur->next = new1;

        printf("Successful registration.\n");

        // Ghi vào file
        saveToFile(f);
    }
}

//singin
void signin(FILE *f) {
    printf("--> Sign in\n");
    char name[100], pass[100];
    int attempts = 0;

    printf("Username: ");
    scanf("%s", name);
    cur = checkUserExist(name);

    if (cur == NULL) {
        printf("Cannot find account.\n");
        return;
    }

    if (cur->status == 0) {
        printf("Account is blocked.\n");
        return;
    }

    while (attempts < 2) {
        printf("Password: ");
        scanf("%s", pass);

        if (strcmp(cur->password, pass) == 0) {
            printf("Hello %s.\n", cur->username);
            cur->signin = 1;
            return;
        } else {
            printf("Incorrect password.\n");
            attempts++;
        }
    }

    if (attempts == 3) {
        cur->status = 0;
        printf("Account is blocked.\n");
        saveToFile(f);
    }
}

void search(){
    printf("--> Search\n");
    char name[100];
    printf("Username : ");
    scanf("%s",name);
    cur = checkUserExist(name);
    if (cur == NULL) {
        printf("Cannot find account\n");
        return ;
    }
    if (cur->status == 1) {
        printf("Account is active\n");
    } else printf("Account is blocked\n");
}


// Chức năng đăng xuất
void signout(FILE *f) {
    printf("--> Sign out\n");
    char name[100];

    printf("Username: ");
    scanf("%s", name);
    cur = checkUserExist(name);

    if (cur == NULL) {
        printf("Account not found.\n");
        return;
    }

    if (cur->signin == 0) {
        printf("Account is not signed in.\n");
    } else {
        printf("Goodbye %s.\n", cur->username);
        cur->signin = 0;
        saveToFile(f);
    }
}



int main(){
    // char *s;
    FILE *f = fopen("user.txt", "r+");
    if (!f) {
        printf("Cannot find the data file . Are you want create (y)?");
        char confirm ;
        scanf("%c",&confirm);
        if (confirm == 'y') {
            f = fopen("user.txt", "a+");
        } else
            return 1;
    }
    getUserInfo(f);
    int choice = 1;
    while (choice >=1 && choice <= 4){
        displayMenu();
        scanf ("%d",&choice);
        switch (choice){
            case 1 :
                Register(f);
            break;
            case 2 :
                signin(f);
            break;
            case 3 :
                search();
            break;
            case 4 :
                signout(f);
            break;
            default :
                printf("Exiting program...\n");
            break;
        }
    }
    fclose(f);
    return 0;
}

