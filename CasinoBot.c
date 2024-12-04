#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <concord/discord.h>
#include <unistd.h>
#include <inttypes.h> //SCNu64

typedef struct Card{
    int number; // 1 is ace, 11 jack, 12 queen, 13 king
    char *suite; //d, s, h, c
} Card;

typedef struct Player{
    u64snowflake userID;
    int bet;
    Card pokerCard1;
    Card pokerCard2;
    Card bjHand[11];
    int ready;
    int bJStay;
    struct Player *next;
} Player;

typedef struct UserBalance{
    u64snowflake userID;
    int balance;
    struct UserBalance *next;
} UserBalance;

FILE *dataFile;
FILE *botKeyFile;
char *botKey;

Card baseDeck[52] = {{1,"hearts"},{2,"hearts"},{3,"hearts"},{4,"hearts"},{5,"hearts"},{6,"hearts"},{7,"hearts"},{8,"hearts"},{9,"hearts"},{10,"hearts"},{11,"hearts"},{12,"hearts"},{13,"hearts"},
    {1,"diamonds"},{2,"diamonds"},{3,"diamonds"},{4,"diamonds"},{5,"diamonds"},{6,"diamonds"},{7,"diamonds"},{8,"diamonds"},{9,"diamonds"},{10,"diamonds"},{11,"diamonds"},{12,"diamonds"},{13,"diamonds"},
    {1,"clubs"},{2,"clubs"},{3,"clubs"},{4,"clubs"},{5,"clubs"},{6,"clubs"},{7,"clubs"},{8,"clubs"},{9,"clubs"},{10,"clubs"},{11,"clubs"},{12,"clubs"},{13,"clubs"},
    {1,"spades"},{2,"spades"},{3,"spades"},{4,"spades"},{5,"spades"},{6,"spades"},{7,"spades"},{8,"spades"},{9,"spades"},{10,"spades"},{11,"spades"},{12,"spades"},{13,"spades"}};
Card Deck[52];
Card pokerRiver[5];
Card dealerHand[11];
int cardsLeft=52;
int currentPot;

int getRand();
Card drawCard();
void resetDeck();

int gameRunning = 0; //Indicates if another game is currently running, and which type. 1: black jack, 2: poker
int takingBets = 0; //Indicates when the game is waiting for players to bet
void runBetCommand(struct discord *client, const struct discord_message *event);
void runPokerCommand(struct discord *client, const struct discord_message *event);
void runBJCommand(struct discord *client, const struct discord_message *event);
void runHitCommand(struct discord *client, const struct discord_message *event);
void runStayCommand(struct discord *client, const struct discord_message *event);
void checkBJWin(struct discord *client, const struct discord_message *event, int);
void runJoinCommand(struct discord *client, const struct discord_message *event);
void displayPlayers(struct discord *client, const struct discord_message *event, int);
void checkPokerWinner();

void addPlayer(u64snowflake, int);
Player *getPlayer(u64snowflake);
void removePlayer(u64snowflake);
Player *PlayerList;
int PlayerCount=0;
UserBalance *balanceList;
UserBalance *getUserBalance(u64snowflake);

int readyCheck();
void readUserData();
void writeUserData();
void getBotKey();

int main(){
    readUserData();
    resetDeck();
    getBotKey();

    ccord_global_init();
    struct discord *client = discord_init(botKey);

    discord_set_on_command(client, "!startblackj", &runBJCommand);
    discord_set_on_command(client, "!startpoker", &runPokerCommand);
    discord_set_on_command(client, "!join", &runJoinCommand);
    discord_set_on_command(client, "!bet", &runBetCommand);
    discord_set_on_command(client, "!hit", &runHitCommand);
    discord_set_on_command(client, "!stay", &runStayCommand);

    discord_run(client);

    discord_cleanup(client);
    ccord_global_cleanup();
}

void runJoinCommand(struct discord *client, const struct discord_message *event){
    discord_delete_message(client,event->channel_id,event->id,NULL,NULL);
    addPlayer(event->author->id,0);
}

void runBJCommand(struct discord *client, const struct discord_message *event){
    discord_delete_message(client,event->channel_id,event->id,NULL,NULL);
    resetDeck();//TODO: Wipe Dealer hand
    gameRunning = 1;
    currentPot=0;
    Player *currentPlayer = PlayerList;
    while (currentPlayer != NULL){
        currentPlayer->bjHand[0] = drawCard();
        currentPlayer->bjHand[1] = drawCard();
        currentPlayer->bJStay = 0;
        currentPlayer = currentPlayer->next;
    }
    displayPlayers(client, event, 1);
    dealerHand[0] = drawCard();
    char text[256];
    snprintf(text,256,"Dealer: :%s:%d",dealerHand[0].suite,dealerHand[0].number);
    struct discord_create_message params = {.content = text};
    discord_create_message(client,event->channel_id,&params,NULL);
}

void removePlayer(u64snowflake deletedUID){
    Player *playerPointer = PlayerList;
    Player *previousPointer = NULL;
    while (playerPointer != NULL){
        if (playerPointer->userID == deletedUID){
            if (previousPointer == NULL){
                PlayerList = PlayerList->next;
                free(playerPointer);
                PlayerCount -= 1;
                return;
            }
            else if(playerPointer->next == NULL){
                previousPointer->next = NULL;
                free(playerPointer);
                PlayerCount -= 1;
                return;
            }
            else {
                previousPointer->next = playerPointer->next;
                free(playerPointer);
                PlayerCount -= 1;
                return;
            }
        }
    }
}

void displayPlayers(struct discord *client, const struct discord_message *event, int mode){
    Player *currentPlayer = PlayerList;
    switch (mode){
        case 0:
        while (currentPlayer != NULL){
            char text[256];
            snprintf(text,256,"%lu | $%d",currentPlayer->userID, currentPlayer->bet);
            struct discord_create_message params = {.content = text};
            discord_create_message(client,event->channel_id,&params,NULL);
            currentPlayer = currentPlayer->next;
        }
        break;
        case 1:
        while (currentPlayer != NULL){
            char text[256];
            snprintf(text,256,"%lu | $%d",currentPlayer->userID, currentPlayer->bet);
            for (int i=0; i<11;i++){
                if (currentPlayer->bjHand[i].number == 0){
                    break;
                }
                int len = strlen(text);
                snprintf(text+len,(sizeof text) - len,", :%s:%d",currentPlayer->bjHand[i].suite, currentPlayer->bjHand[i].number);
            }
            struct discord_create_message params = {.content = text};
            discord_create_message(client,event->channel_id,&params,NULL);
            currentPlayer = currentPlayer->next;
        }
        break;
    }
}

void runStayCommand(struct discord *client, const struct discord_message *event){
    if (gameRunning != 1){
        return;
    }
    Player *stayPlayer = getPlayer(event->author->id);
    stayPlayer->bJStay = 1;
    stayPlayer->ready = 1;
    int allStay = 1;
    Player *currentPlayer = PlayerList;
    while (currentPlayer != NULL){
        if (currentPlayer->bJStay == 0){
            allStay = 0;
        }
        currentPlayer = currentPlayer->next;
    }
    checkBJWin(client,event,allStay);
}

void runHitCommand(struct discord *client, const struct discord_message *event){
    if (gameRunning != 1){
        return;
    }
    Player *hitPlayer = getPlayer(event->author->id);
    if (hitPlayer->ready == 0){
        int i = 0;
        while (hitPlayer->bjHand[i].number != 0){
            i++;
        }
        hitPlayer->bjHand[i] = drawCard();
        hitPlayer->ready = 1;
        displayPlayers(client, event, 1);
    }
    checkBJWin(client,event,0);
}

void checkBJWin(struct discord *client, const struct discord_message *event, int allStay){
    if (readyCheck() == 1){
        do {
            char text[256];
            //displayPlayers(client,event,1);
            snprintf(text,256,"Dealer");
            int handSize = 0;
            for (int i=0; i<11;i++){
                if (dealerHand[i].number == 0){
                    if (handSize <= 17){
                        dealerHand[i] = drawCard();
                        int len = strlen(text);
                        snprintf(text+len,(sizeof text) - len,", :%s:%d",dealerHand[i].suite, dealerHand[i].number);
                    }
                    else {
                        allStay = 0;
                    }
                    break;
                }
                int len = strlen(text);
                snprintf(text+len,(sizeof text) - len,", :%s:%d",dealerHand[i].suite, dealerHand[i].number);
                handSize += dealerHand[i].number;
            }
            struct discord_create_message params = {.content = text};
            discord_create_message(client,event->channel_id,&params,NULL);
            if (handSize > 21){
                displayPlayers(client,event,1);
                return;
            }
        } while (allStay == 1);
    } 
    int handSize = 0;
    Player *currentPlayer = PlayerList;
    while (currentPlayer != NULL){
        handSize = 0;
        for (int i=0; i<11;i++){
            if (currentPlayer->bjHand[i].number == 0){
                break;
            }
            if (currentPlayer->bjHand[i].number > 10){
                handSize += 10;
            }
            else{
                handSize += currentPlayer->bjHand[i].number;
            }
        }
        if (handSize > 21){
            removePlayer(currentPlayer->userID);
        }
        currentPlayer = currentPlayer->next;
    }
    if (PlayerCount <= 0){
        PlayerCount = 0;
        gameRunning = 0;

        char text[256];
        displayPlayers(client,event,1);
        snprintf(text,256,"Dealer Wins!");
        struct discord_create_message params = {.content = text};
        discord_create_message(client,event->channel_id,&params,NULL);
    }
}

void runPokerCommand(struct discord *client, const struct discord_message *event){
    discord_delete_message(client,event->channel_id,event->id,NULL,NULL);
    resetDeck();
    gameRunning = 2;
    currentPot = 0;
    Player *currentPlayer = PlayerList;
    while (currentPlayer != NULL){
        printf("\n%d, %lu, ", currentPlayer->bet, currentPlayer->userID);
        currentPlayer->pokerCard1 = drawCard();
        currentPlayer->pokerCard2 = drawCard();
        printf("%d, %s, ", currentPlayer->pokerCard1.number, currentPlayer->pokerCard1.suite);
        printf("%d, %s\n", currentPlayer->pokerCard2.number, currentPlayer->pokerCard2.suite);
        currentPlayer = currentPlayer->next;
    }
    readyCheck();
    pokerRiver[0] = drawCard();
    pokerRiver[1] = drawCard();
    pokerRiver[2] = drawCard();
    printf("%d, %s, ", pokerRiver[0].number, pokerRiver[0].suite);
    printf("%d, %s, ", pokerRiver[1].number, pokerRiver[1].suite);
    printf("%d, %s\n", pokerRiver[2].number, pokerRiver[2].suite);

    readyCheck();
    pokerRiver[3] = drawCard();
    printf("%d, %s\n", pokerRiver[3].number, pokerRiver[3].suite);

    readyCheck();
    pokerRiver[4] = drawCard();
    printf("%d, %s\n", pokerRiver[4].number, pokerRiver[4].suite);

    readyCheck();
    checkPokerWinner();
}

void runBetCommand(struct discord *client, const struct discord_message *event){ //TODO: add ability for players to make bets
    int newBet = 0;
    int finished = 0;
    sscanf(event->content, "%d", &newBet);
    Player *currentPlayer = PlayerList;
    while (currentPlayer != NULL){
        if (currentPlayer->userID == event->author->id){
            if (getUserBalance(event->author->id)->balance < newBet){
                newBet = getUserBalance(event->author->id)->balance;
            }
            currentPlayer->bet += newBet;
            getUserBalance(currentPlayer->userID)->balance -= newBet;
            currentPot += newBet;
        }
        currentPlayer = currentPlayer->next;
    }
    displayPlayers(client,event,0);
}

void addPlayer(u64snowflake userID, int bet){ //Adds player to user database if no entry is found, then adds player to current active players.
    //Mode determines which game the user will be added to. Where 1=poker, 2=blackj, and 3=roulette.
    if (getUserBalance(userID)==NULL){
        UserBalance *TempBalance = (UserBalance *)malloc(sizeof(UserBalance));
        TempBalance->userID = userID;
        TempBalance->balance = 1000;
        TempBalance->next = balanceList;
        balanceList = TempBalance;
    }
    Player *TempElement = (Player *)malloc(sizeof(Player));
    TempElement->userID = userID;
    TempElement->bet = bet;
    TempElement->ready = 0;
    TempElement->next = PlayerList;
    PlayerList = TempElement;
    PlayerCount += 1;
}

Player *getPlayer(u64snowflake userID){
    Player *currentPlayer = PlayerList;
    while (currentPlayer != NULL){
        if (currentPlayer->userID == userID){
            return currentPlayer;
        }
        currentPlayer = currentPlayer->next;
    }
    return NULL;
}

int readyCheck(){
    int waitFor = 0;
    Player *currentPlayer;
    currentPlayer = PlayerList;
    waitFor = 1;
    while(currentPlayer != NULL){
        if (currentPlayer->ready == 0){
            waitFor = 0;
            break;
        }
        currentPlayer = currentPlayer->next;
    }
    if (waitFor == 1){
        currentPlayer = PlayerList;
        while(currentPlayer != NULL){
            currentPlayer->ready = 0;
            currentPlayer = currentPlayer->next;
        }
        return 1;
    }
    return 0;
}

Card drawCard(){ //Draws one card from the deck and returns the struct
    int cardIndex = getRand()%(cardsLeft);
    Card picked = Deck[cardIndex];
    for (int i=cardIndex+1;i<cardsLeft;i++){
        Deck[i-1]=Deck[i];
    }
    cardsLeft -= 1;
    return picked;
}

int getRand(){ //Generates a random int based off the current time
    srand((unsigned int)time(NULL));
    return rand();
}

void resetDeck(){ //Returns all the cards to the deck
    memcpy(Deck, baseDeck, sizeof(Deck));
}

void checkPokerWinner(){ //Checks to see which player has the best hand
    //TODO: check for player with winning hand
    return;
}

UserBalance *getUserBalance(u64snowflake userID){ //Gets given user's balance
    UserBalance *currentBalance = balanceList;
    while (currentBalance != NULL){
        if (currentBalance->userID == userID){
            return currentBalance;
        }
        currentBalance = currentBalance->next;
    }
    return NULL;
}

void readUserData(){
    dataFile = fopen("userData.bin","rb");
    printf("\nReading user data\n");
    UserBalance *currentBalance;
    while (fread(&currentBalance,sizeof(UserBalance),1,dataFile)==1){
        printf("\nUID: %lu, Balance: %d, ", currentBalance->userID, currentBalance->balance);
        currentBalance->next = balanceList;
        balanceList = currentBalance;
    }
    fclose(dataFile);
}

void writeUserData(){
    dataFile = fopen("userData.bin","wb");
    UserBalance *currentBalance = balanceList;
    while (currentBalance != NULL){
        printf("\n%lu, %d, ", currentBalance->userID, currentBalance->balance);
        fwrite(&currentBalance,sizeof(UserBalance),1,dataFile);
        currentBalance = currentBalance->next;
    }
    printf("\nWriting complete\n");
    fclose(dataFile);
}

void getBotKey(){
    botKeyFile = fopen("botKey.txt","r");
    fseek(botKeyFile,0,SEEK_END);
    int keyLength = ftell(botKeyFile)+1;
    fseek(botKeyFile,0,SEEK_SET);
    botKey=malloc(keyLength);
    fgets(botKey,keyLength,botKeyFile);
    fclose(botKeyFile);
}