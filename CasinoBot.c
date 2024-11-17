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
    Player *next;
} Player;
Player *playerList;

Card drawCard();
void resetDeck();
// Card *testDeck;
Card baseDeck[52] = {{1,'h'},{2,'h'},{3,'h'},{4,'h'},{5,'h'},{6,'h'},{7,'h'},{8,'h'},{9,'h'},{10,'h'},{11,'h'},{12,'h'},{13,'h'},
    {1,'d'},{2,'d'},{3,'d'},{4,'d'},{5,'d'},{6,'d'},{7,'d'},{8,'d'},{9,'d'},{10,'d'},{11,'d'},{12,'d'},{13,'d'},
    {1,'c'},{2,'c'},{3,'c'},{4,'c'},{5,'c'},{6,'c'},{7,'c'},{8,'c'},{9,'c'},{10,'c'},{11,'c'},{12,'c'},{13,'c'},
    {1,'s'},{2,'s'},{3,'s'},{4,'s'},{5,'s'},{6,'s'},{7,'s'},{8,'s'},{9,'s'},{10,'s'},{11,'s'},{12,'s'},{13,'s'}};
Card Deck[52];
int cardsLeft=52;

int getRand();
void addPlayer(int, int);
int playerCount;

int main(){
    resetDeck();
    for (int i=0;i<52;i++){
        Card tempCard = drawCard();
        printf("%d, %d, %c\n", i, tempCard.number, tempCard.suite);
    }
}

void runGame(){
    printf("How many players?");
    scanf("%d",&playerCount);
    for (int i=0;i<playerCount;i++){
        addPlayer(1,50);
        addPlayer(2,70);
        addPlayer(3,30);
    }
}

void takeBets(){
    int finished = 0;
    int newBet = 0;
    Player *currentPlayer = playerList;
    while (finished != 1){
        currentPlayer->bet += newBet;
    }
}

void addPlayer(int userID, int bet){
    Player *TempElement = (Player *)malloc(sizeof(Player));
    TempElement->userID = userID;
    TempElement->bet = bet;
    TempElement->next = playerList;
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
