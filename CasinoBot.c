#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
//#include <concord/discord.h>

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

FILE *dataFile;
FILE *botKey;

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
void runPoker();
void addPlayer(int, int);
int playerCount;

int main(){
    dataFile = fopen("userData.bin","wb+");
    botKey = fopen("botKey.bin","rb");
    resetDeck();
    runPoker();
}

void runPoker(){
    pokerPot = 0;
    printf("How many players?");
    scanf("%d",&playerCount);
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
    int finished = 0;
    int newBet = 0;
    Player *currentPlayer = playerList;
    while (finished != 1){
        currentPlayer->bet += newBet;
        pokerPot += newBet;
    }
}

void addPlayer(int userID, int bet){
    Player *TempElement = (Player *)malloc(sizeof(Player));
    TempElement->userID = userID;
    TempElement->bet = bet;
    TempElement->next = playerList;
    playerList = TempElement;
    playerCount += 1;
}

Card drawCard(){
    int cardIndex = getRand()%(cardsLeft);
    Card picked = Deck[cardIndex];
    for (int i=cardIndex+1;i<cardsLeft;i++){
        Deck[i-1]=Deck[i];
    }
    cardsLeft -= 1;
    return picked;
}

int getRand(){
    srand((unsigned int)time(NULL));
    return rand();
}

void resetDeck(){
    memcpy(Deck, baseDeck, sizeof(Deck));
}

void checkPokerWinner(){
    //TODO: check for player with winning hand
}