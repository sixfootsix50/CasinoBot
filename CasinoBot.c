#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <concord/discord.h>

typedef struct Card{
    int number; // 1 is ace, 11 jack, 12 queen, 13 king
    char *suite; //d, s, h, c
} Card;

typedef struct Player{
    int userID;
    int bet;
    Card card1;
    Card card2;
    struct Player *next;
} Player;
Player *playerList;

typedef struct UserBalance{
    int userID;
    int balance;
    struct UserBalance *next;
} UserBalance;
UserBalance *balanceList;

FILE *dataFile;
FILE *botKeyFile;
char *botKey;

Card baseDeck[52] = {{1,"hearts"},{2,"hearts"},{3,"hearts"},{4,"hearts"},{5,"hearts"},{6,"hearts"},{7,"hearts"},{8,"hearts"},{9,"hearts"},{10,"hearts"},{11,"hearts"},{12,"hearts"},{13,"hearts"},
    {1,"diamonds"},{2,"diamonds"},{3,"diamonds"},{4,"diamonds"},{5,"diamonds"},{6,"diamonds"},{7,"diamonds"},{8,"diamonds"},{9,"diamonds"},{10,"diamonds"},{11,"diamonds"},{12,"diamonds"},{13,"diamonds"},
    {1,"clubs"},{2,"clubs"},{3,"clubs"},{4,"clubs"},{5,"clubs"},{6,"clubs"},{7,"clubs"},{8,"clubs"},{9,"clubs"},{10,"clubs"},{11,"clubs"},{12,"clubs"},{13,"clubs"},
    {1,"spades"},{2,"spades"},{3,"spades"},{4,"spades"},{5,"spades"},{6,"spades"},{7,"spades"},{8,"spades"},{9,"spades"},{10,"spades"},{11,"spades"},{12,"spades"},{13,"spades"}};
Card Deck[52];
Card pokerRiver[5];
int cardsLeft=52;
int pokerPot;

int getRand();
Card drawCard();
void resetDeck();

void takeBets();
void runPoker();
void checkPokerWinner();

void addPlayer(int, int);
int playerCount=0;
UserBalance *getUserBalance(int);

void readUserData();
void writeUserData();
void getBotKey();

void runPokerCommand(struct discord *client, const struct discord_message *event);

u64snowflake g_app_id;

int main(){
    readUserData();
    getBotKey();

    ccord_global_init();
    struct discord *client = discord_init(botKey);

    discord_set_on_command(client, "!poker", &runPokerCommand);

    discord_run(client);

    resetDeck();
    runPoker();
    writeUserData();
    readUserData();

    discord_cleanup(client);
    ccord_global_cleanup();
}

void runPokerCommand(struct discord *client, const struct discord_message *event){
    char[256] text;
    snprintf(":clubs:77, :%s:%d, :%s:%d",baseDeck[1]->suite,baseDeck[1]->number,baseDeck[3]->suite,baseDeck[3]->number);
    struct discord_create_message params = {.content = text};
    discord_create_message(client,event->channel_id,&params,NULL);
}

void on_ready(struct discord *client, const struct discord_ready *event){
    g_app_id = event->application->id;
}

void runPoker(){
    resetDeck();
    pokerPot = 0;
    addPlayer(1,50);
    addPlayer(2,70);
    addPlayer(3,30);
    addPlayer(4,10000);
    Player *currentPlayer = playerList;
    while (currentPlayer != NULL){
        printf("\n%d, %d, ", currentPlayer->bet, currentPlayer->userID);
        currentPlayer->card1 = drawCard();
        currentPlayer->card2 = drawCard();
        printf("%d, %c, ", currentPlayer->card1.number, currentPlayer->card1.suite);
        printf("%d, %c\n", currentPlayer->card2.number, currentPlayer->card2.suite);
        currentPlayer = currentPlayer->next;
    }
    takeBets();
    pokerRiver[0] = drawCard();
    pokerRiver[1] = drawCard();
    pokerRiver[2] = drawCard();
    printf("%d, %c, ", pokerRiver[0].number, pokerRiver[0].suite);
    printf("%d, %c, ", pokerRiver[1].number, pokerRiver[1].suite);
    printf("%d, %c\n", pokerRiver[2].number, pokerRiver[2].suite);

    takeBets();
    pokerRiver[3] = drawCard();
    printf("%d, %c\n", pokerRiver[3].number, pokerRiver[3].suite);

    takeBets();
    pokerRiver[4] = drawCard();
    printf("%d, %c\n", pokerRiver[4].number, pokerRiver[4].suite);

    takeBets();
    checkPokerWinner();
}

void takeBets(){ //TODO: add ability for players to make bets
    return;
    int finished = 0;
    int newBet = 0;
    Player *currentPlayer = playerList;
    while (finished != 1){
        currentPlayer->bet += newBet;
        getUserBalance(currentPlayer->userID)->balance -= newBet;
        pokerPot += newBet;
    }
}

void addPlayer(int userID, int bet){ //Adds player to user database if no entry is found, then adds player to current active players
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
    TempElement->next = playerList;
    playerList = TempElement;
    playerCount += 1;
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

UserBalance *getUserBalance(int userID){ //Gets given user's balance
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
        printf("\nUID: %d, Balance: %d, ", currentBalance->userID, currentBalance->balance);
        currentBalance->next = balanceList;
        balanceList = currentBalance;
    }
    fclose(dataFile);
}

void writeUserData(){
    dataFile = fopen("userData.bin","wb");
    UserBalance *currentBalance = balanceList;
    while (currentBalance != NULL){
        printf("\n%d, %d, ", currentBalance->userID, currentBalance->balance);
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