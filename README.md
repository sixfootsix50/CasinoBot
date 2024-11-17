# CasinoBot
## Concept
Project will allow users to play several casino games against other players. Program will be written in ANSI C and will run on either a linux computer or microprocessor with access to the internet. Players will have a balance of currency that will all be stored in a linked list for each user’s account. Each game will allow players to wager some of their currency into a pot that the winner will get to claim. Player currency will be reset to a minimum value each day to allow bankrupt players to keep playing. The deck of dealt cards will be an array containing all 52 standard playing cards, and cards will be randomly dealt by randomly accessing an index in said array. Cards that have been drawn will then be marked to prevent re-drawing.
## Games
The program will be able to play three different standard casino games, Texas Hold’em Poker, Blackjack, and Roulette. When the program detects the start keyword in the specified message channel, it will begin the requested game.
## Interface
All program interfaces will be generated within the messaging platform Discord through its provided API. Discord buttons will be used to handle player inputs and Discord user ids will be used to track balances.
## BlackJack
- Users will be dealt a card that is displayed in a message with their name. The program will also show the dealer’s hand in the same message. 
- Players will be able to wager their currency on the current match.
- Each player will then have an option to hit or stay
- If the user hits, the program will then deal them an additional card. 
- Play continues until all players stay or exceed 21. 
## Poker
- Players will be sent a hidden message that shows their current hand.
- The game will then mark two users to make big blind and small blind bets.
- Then all users will be asked to fold, call, or raise.
- After all users have either folded or called, the program will start revealing the river.
- All current players will then be asked to fold, call, or raise again.
- This cycle repeats until either one player is left, or the whole river is revealed.
- If the game ends with more than one player in play, the program will then reveal each player’s hand and declare a winner.
- If only one player remains, they will have the option to reveal their hand or keep it secret.
## Roulette
- Players will be able to place bets on specific numbers
- Once all players confirm or a set time has passed, bets will be locked
- Program will then calculate which roulette number wins
- Players will be shown the winning number and payouts will be distributed
- Program repeats unless no players place bets within the time frame
