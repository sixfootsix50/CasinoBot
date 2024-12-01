#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <concord/discord.h>

typedef struct Card{
    int number; // 1 is ace, 11 jack, 12 queen, 13 king
    char suite; //d, s, h, c
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

Card baseDeck[52] = {{1,'h'},{2,'h'},{3,'h'},{4,'h'},{5,'h'},{6,'h'},{7,'h'},{8,'h'},{9,'h'},{10,'h'},{11,'h'},{12,'h'},{13,'h'},
    {1,'d'},{2,'d'},{3,'d'},{4,'d'},{5,'d'},{6,'d'},{7,'d'},{8,'d'},{9,'d'},{10,'d'},{11,'d'},{12,'d'},{13,'d'},
    {1,'c'},{2,'c'},{3,'c'},{4,'c'},{5,'c'},{6,'c'},{7,'c'},{8,'c'},{9,'c'},{10,'c'},{11,'c'},{12,'c'},{13,'c'},
    {1,'s'},{2,'s'},{3,'s'},{4,'s'},{5,'s'},{6,'s'},{7,'s'},{8,'s'},{9,'s'},{10,'s'},{11,'s'},{12,'s'},{13,'s'}};
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

int main(){
    readUserData();
    getBotKey();
    printf("%s",botKey);
    return;

    ccord_global_init();
    struct discord *client = discord_init(botKey);

    resetDeck();
    runPoker();
    writeUserData();
    readUserData();
}

void runPoker(){
    pokerPot = 0;
    printf("How many players?");
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
    botKeyFile = fopen("botKey.bin","r");
    fseek(botKeyFile,0,SEEK_END);
    int keyLength = ftell(botKeyFile);
    fseek(botKeyFile,0,SEEK_SET);
    botKey=malloc(keyLength);
    fread(botKey,1,keyLength,botKeyFile);
    fclose(botKeyFile);
}