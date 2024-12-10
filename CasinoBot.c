#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <concord/discord.h>
#include <unistd.h>

typedef struct Card{
    int number; // 1 is ace, 11 jack, 12 queen, 13 king
    char *suite; //d, s, h, c
} Card;

typedef struct Player{
    u64snowflake userID;
    int bet;
    Card bjHand[11];
    int bjHandCount;
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
Card baseDealerHand[11];
Card dealerHand[11];
int dealerHandCount;
int cardsLeft=52;

int getRand();
Card drawCard();
int getCardPoints(Card);
void resetDeck();

int gameRunning = 0; //Indicates if another game is currently running
int takingBets = 0; //Indicates when the game is waiting for players to bet
void runBetCommand(struct discord *client, const struct discord_message *event);
void runBJCommand(struct discord *client, const struct discord_message *event);
void runHitCommand(struct discord *client, const struct discord_message *event);
void runStayCommand(struct discord *client, const struct discord_message *event);
void checkBJWin(struct discord *client, const struct discord_message *event);
void runJoinCommand(struct discord *client, const struct discord_message *event);
void displayPlayers(struct discord *client, const struct discord_message *event);
void displayBalances(struct discord *client, const struct discord_message *event);
void continueBJ(struct discord *client, const struct discord_message *event);

void addPlayer(u64snowflake, int);
Player *getPlayer(u64snowflake);
void removePlayer(u64snowflake);
void payout(int);
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
    discord_set_on_command(client, "!join", &runJoinCommand);
    discord_set_on_command(client, "!bet", &runBetCommand);
    discord_set_on_command(client, "!hit", &runHitCommand);
    discord_set_on_command(client, "!stay", &runStayCommand);

    discord_run(client);

    discord_cleanup(client);
    ccord_global_cleanup();
}

void runJoinCommand(struct discord *client, const struct discord_message *event){
    addPlayer(event->author->id,0);
    displayPlayers(client, event);
}

void runBJCommand(struct discord *client, const struct discord_message *event){
    resetDeck();
    gameRunning = 1;
    memcpy(dealerHand, baseDealerHand, sizeof(baseDealerHand));
    Player *currentPlayer = PlayerList;
    while (currentPlayer != NULL){
        memcpy(currentPlayer->bjHand,baseDealerHand, sizeof baseDealerHand);
        currentPlayer->ready = 0;
        currentPlayer->bjHand[0] = drawCard();
        currentPlayer->bjHandCount = getCardPoints(currentPlayer->bjHand[0]);
        currentPlayer->bjHand[1] = drawCard();
        currentPlayer->bjHandCount += getCardPoints(currentPlayer->bjHand[1]);
        currentPlayer->bJStay = 0;
        currentPlayer = currentPlayer->next;
    }
    displayPlayers(client, event);
    dealerHand[0] = drawCard();
    dealerHandCount = getCardPoints(dealerHand[0]);
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

void displayPlayers(struct discord *client, const struct discord_message *event){
    Player *currentPlayer = PlayerList;
    while (currentPlayer != NULL){
        char text[256];
        int len;
        snprintf(text,256,"%lu | $%d",currentPlayer->userID, currentPlayer->bet);
        for (int i=0; i<11;i++){
            if (currentPlayer->bjHand[i].number == 0){
                break;
            }
            len = strlen(text);
            snprintf(text+len,(sizeof text) - len,", :%s:%d",currentPlayer->bjHand[i].suite, currentPlayer->bjHand[i].number);
        }
        len = strlen(text);
        snprintf(text+len,(sizeof text) - len,", HandSize = %d", currentPlayer->bjHandCount);
        struct discord_create_message params = {.content = text};
        discord_create_message(client,event->channel_id,&params,NULL);
        currentPlayer = currentPlayer->next;
    }
}

void displayBalances(struct discord *client, const struct discord_message *event){
    UserBalance *currentBalance = balanceList;
    while (currentBalance != NULL){
        char text[256];
        int len;
        snprintf(text,256,"%lu | $%d",currentBalance->userID, currentBalance->balance);
        struct discord_create_message params = {.content = text};
        discord_create_message(client,event->channel_id,&params,NULL);
        currentBalance = currentBalance->next;
    }
}

void runStayCommand(struct discord *client, const struct discord_message *event){
    if (gameRunning != 1){
        return;
    }
    Player *stayPlayer = getPlayer(event->author->id);
    stayPlayer->bJStay = 1;
    continueBJ(client,event);
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
        hitPlayer->bjHandCount += getCardPoints(hitPlayer->bjHand[i]);
        hitPlayer->ready = 1;
        displayPlayers(client, event);
        if (hitPlayer->bjHandCount > 21){
            removePlayer(hitPlayer->userID);
        }
    }
    continueBJ(client,event);
}

void continueBJ(struct discord *client, const struct discord_message *event){
    if (readyCheck() == 1){
        if (dealerHandCount <= 17){
            do {
                char text[256];
                snprintf(text,256,"Dealer");
                for (int i=0; i<11;i++){
                    int len = strlen(text);
                    if (dealerHand[i].number == 0){
                        if (dealerHandCount <= 17){
                            dealerHand[i] = drawCard();
                            dealerHandCount += getCardPoints(dealerHand[i]);
                            snprintf(text+len,(sizeof text) - len,", :%s:%d",dealerHand[i].suite, dealerHand[i].number);
                        }
                        break;
                    }
                    snprintf(text+len,(sizeof text) - len,", :%s:%d",dealerHand[i].suite, dealerHand[i].number);
                }
                struct discord_create_message params = {.content = text};
                discord_create_message(client,event->channel_id,&params,NULL);
            } while (readyCheck() == 1 && dealerHandCount <= 17);
        }
        checkBJWin(client,event);
    }
}

void checkBJWin(struct discord *client, const struct discord_message *event){
    //Check each player's hand total
    if (PlayerCount <= 0){ //Ends game if no more players
        PlayerCount = 0;
        gameRunning = 0;
        PlayerList = NULL;

        char text[256];
        displayPlayers(client,event);
        snprintf(text,256,"Dealer Wins!");
        struct discord_create_message params = {.content = text};
        discord_create_message(client,event->channel_id,&params,NULL);
        printf("Game end: No players");
        return;
    }
    else {
        if (readyCheck() == 1){
            if (dealerHandCount <= 21){
                Player *currentPlayer = PlayerList;
                Player *currentNext;
                while (currentPlayer != NULL){
                    currentNext = currentPlayer->next;
                    if (currentPlayer->bjHandCount < dealerHandCount){
                        removePlayer(currentPlayer->userID);
                    }
                    currentPlayer = currentNext;
                }
            }
        if (PlayerCount <= 0){
            char text[256];
            snprintf(text,256,"Dealer Wins!");
            struct discord_create_message params = {.content = text};
            discord_create_message(client,event->channel_id,&params,NULL);
        }
        else{
            displayPlayers(client, event);
        }
        payout(2);
        displayBalances(client, event);
        }
    } 
}

void runBetCommand(struct discord *client, const struct discord_message *event){
    int newBet = 0;
    int finished = 0;
    sscanf(event->content, "%d", &newBet);
    readUserData();
    Player *currentPlayer = PlayerList;
    while (currentPlayer != NULL){
        if (currentPlayer->userID == event->author->id){
            if (getUserBalance(event->author->id)->balance < newBet){
                newBet = getUserBalance(event->author->id)->balance;
                getUserBalance(currentPlayer->userID)->balance = 0;
            }
            currentPlayer->bet += newBet;
            getUserBalance(currentPlayer->userID)->balance -= newBet;
        }
        currentPlayer = currentPlayer->next;
    }
    writeUserData();
    displayPlayers(client,event);
}

void addPlayer(u64snowflake userID, int bet){ //Adds player to user database if no entry is found, then adds player to current active players.
    readUserData();
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
    TempElement->next = PlayerList;
    TempElement->bJStay = 0;
    TempElement->bjHandCount = 0;
    memcpy(TempElement->bjHand,baseDealerHand, sizeof baseDealerHand);
    PlayerList = TempElement;
    PlayerCount += 1;
    writeUserData();
}

int getCardPoints(Card input){
    if (input.number > 10){
        return 10;
    }
    else if (input.number <= 0){
        return 0;
    }
    else {
        return input.number;
    }
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
    int waitFor = 1;
    Player *currentPlayer;
    currentPlayer = PlayerList;
    while(currentPlayer != NULL){
        if (currentPlayer->ready == 0 && currentPlayer->bJStay == 0){
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

void payout(int multiplier){
    readUserData();
    Player *currentPlayer = PlayerList;
    Player *nextPlayer;    
    while (currentPlayer != NULL){
        getUserBalance(currentPlayer->userID)->balance += currentPlayer->bet*multiplier;
        nextPlayer = currentPlayer->next;
        removePlayer(currentPlayer->userID);
        currentPlayer = nextPlayer;
    }
    writeUserData();
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
    cardsLeft = 52;
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
    balanceList = NULL;
    printf("\nReading user data\n");
    UserBalance currentBalance;
    while (fread(&currentBalance,sizeof(UserBalance),1,dataFile)==1){
        printf("\nUID: %lu, Balance: %d, ", currentBalance.userID, currentBalance.balance);
        UserBalance *TempBalance = (UserBalance *)malloc(sizeof(UserBalance));
        TempBalance->userID = currentBalance.userID;
        TempBalance->balance = currentBalance.balance;
        TempBalance->next = balanceList;
        balanceList = TempBalance;
    }
    fclose(dataFile);
}

void writeUserData(){
    dataFile = fopen("userData.bin","wb");
    UserBalance *currentBalance = balanceList;
    while (currentBalance != NULL){
        printf("\n%lu, %d, ", currentBalance->userID, currentBalance->balance);
        fwrite(currentBalance,sizeof(UserBalance),1,dataFile);
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