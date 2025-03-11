
#include <stdio.h>
#include <stdbool.h>

int main()
{
    bool stay = true;
    printf("WELCOME TO WTP CHAT \n");
    while(stay){
        int chatSelection;
        printf("Select an action \n");
        printf(" 1. Chat globally\n");
        printf(" 2. Chat privately\n");
        printf(" 3. Change status\n");
        printf(" 4. List all users connected\n");
        printf(" 5. Get information about a user\n");
        printf(" 6. Help\n");
        printf(" 7. Exit\n");
        printf(" -> ");
        scanf("%d", &chatSelection);
        printf("%d\n", chatSelection);
        
        switch (chatSelection) {
            case 1:
                printf("GLOBAL CHAT\n");
                break;
            case 2:
                printf("PRIVATE CHAT\n");
                // list all users available to chat
                // select one user somehow
                // send and receive chats
                break;
            case 3:
                printf("CHANGE STATUS\n");
                // can change from active, ocuppied or inactive
                break;
            case 4:
                printf("LIST ALL USERS CONNECTED\n");
                // self explanatory
                break;
            case 5:
                printf("SEE INFO ABOUT A USER\n");
                // list all users
                // select one
                // show username and ip address
                break;
            case 6:
                int stillNeedsHelp = true;
                while(stillNeedsHelp){
                    int helpSelection;
                    printf("HELP SECTION\n");
                    printf("Select the option where you want to get info about\n");
                    printf(" 1. Chat globally\n");
                    printf(" 2. Chat privately\n");
                    printf(" 3. Change status\n");
                    printf(" 4. List all users connected\n");
                    printf(" 5. Get information about a user\n");
                    printf(" 6. Help\n");
                    printf(" 7. Exit\n");
                    printf(" -> ");
                    scanf("%d", &helpSelection);
                    
                    switch(helpSelection){
                        case 1:
                            printf("GLOBAL CHAT -----------------------------------------------\n");
                            printf("-----------------------------------------------------------\n");
                            break;
                        case 2:
                            printf("PRIVATE CHAT ----------------------------------------------\n");
                            printf("-----------------------------------------------------------\n");
                            break;
                        case 3:
                            printf("CHANGE STATUS ---------------------------------------------\n");
                            printf("-----------------------------------------------------------\n");
                            break;
                        case 4:
                            printf("LIST ALL USERS CONNECTED ----------------------------------\n");
                            printf("-----------------------------------------------------------\n");
                            break;
                        case 5:
                            printf("SEE INFO ABOUT A USER -------------------------------------\n");
                            printf("-----------------------------------------------------------\n");
                            break;
                        case 6:
                            printf("HELP SECTION ----------------------------------------------\n");
                            printf("-----------------------------------------------------------\n");
                            break;
                        case 7:
                            printf("EXIT ------------------------------------------------------\n");
                            printf("-----------------------------------------------------------\n");
                            break;
                        default:
                            printf("Not an option :/\n");
                    }
                    
                    int needsHelp;
                    printf("Do you still need help?\n 1. Yes\n 2. No\n -> ");
                    scanf("%d", &needsHelp);
                    if(needsHelp == 2){
                        stillNeedsHelp = false;
                    }
                }
                
                break;
            case 7:
                printf("Bye\n");
                stay = false;
                break;
            default:
                printf("Not an option :/\n");
        }
    }
    
    
    return 0;
}