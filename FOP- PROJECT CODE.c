#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include <unistd.h>

#define SIZE 8
#define NUM_SHIPS 5

int ship_sizes[NUM_SHIPS] = {5,4,3,3,2};

// Colors
#define RED "\033[1;31m"
#define GREEN "\033[1;32m"
#define BLUE "\033[1;34m"
#define WHITE "\033[1;37m"
#define RESET "\033[0m"

typedef enum {
    WATER='~', SHIP='S', HIT='X', MISS='O'
} Cell;

typedef struct {
    int size;
    int hits;
    int sunk;
} Ship;

// Initialize board
void init(Cell b[SIZE][SIZE]){
    for(int i=0;i<SIZE;i++)
        for(int j=0;j<SIZE;j++)
            b[i][j]=WATER;
}

// Pretty print
void print(Cell b[SIZE][SIZE], int hide){
    printf("\n   ");
    for(int i=1;i<=SIZE;i++) printf(" %d ",i);
    printf("\n");

    for(int i=0;i<SIZE;i++){
        printf(" %c ", 'A'+i);
        for(int j=0;j<SIZE;j++){
            char ch=b[i][j];

            if(hide && ch>='A' && ch<'A'+NUM_SHIPS) ch='~';

            if(ch=='X') printf(RED "[X]" RESET);
            else if(ch=='O') printf(WHITE "[O]" RESET);
            else if(ch>='A') printf(GREEN "[S]" RESET);
            else printf(BLUE "[~]" RESET);
        }
        printf("\n");
    }
}

// Placement check
int canPlace(Cell b[SIZE][SIZE],int r,int c,int len,int dir){
    if(dir==0 && c+len>SIZE) return 0;
    if(dir==1 && r+len>SIZE) return 0;

    for(int i=0;i<len;i++)
        if(b[r+(dir?i:0)][c+(dir?0:i)]!=WATER) return 0;

    return 1;
}

// Place ships
void placeShips(Cell b[SIZE][SIZE], Ship ships[]){
    for(int i=0;i<NUM_SHIPS;i++){
        int r,c,d;
        ships[i].size=ship_sizes[i];
        ships[i].hits=0;
        ships[i].sunk=0;

        do{
            r=rand()%SIZE;
            c=rand()%SIZE;
            d=rand()%2;
        }while(!canPlace(b,r,c,ship_sizes[i],d));

        for(int j=0;j<ship_sizes[i];j++)
            b[r+(d?j:0)][c+(d?0:j)]='A'+i;
    }
}

// Fire
int fire(Cell b[SIZE][SIZE], Ship ships[], int r,int c){
    if(b[r][c]>='A' && b[r][c]<'A'+NUM_SHIPS){
        int id=b[r][c]-'A';
        b[r][c]=HIT;
        ships[id].hits++;

        if(ships[id].hits==ships[id].size && !ships[id].sunk){
            ships[id].sunk=1;
            printf(GREEN "🔥 You sunk a ship of size %d!\n" RESET, ships[id].size);
        }
        return 1;
    }
    else if(b[r][c]==WATER){
        b[r][c]=MISS;
        return 0;
    }
    return -1;
}

// Check win
int allSunk(Ship ships[]){
    for(int i=0;i<NUM_SHIPS;i++)
        if(!ships[i].sunk) return 0;
    return 1;
}

// NEW: Ships remaining
int shipsRemaining(Ship ships[]){
    int count = 0;
    for(int i=0;i<NUM_SHIPS;i++)
        if(!ships[i].sunk) count++;
    return count;
}

// NEW: Cells remaining
int cellsRemaining(Ship ships[]){
    int total = 0;
    for(int i=0;i<NUM_SHIPS;i++)
        total += (ships[i].size - ships[i].hits);
    return total;
}

// AI move
void aiMove(Cell player[SIZE][SIZE], Ship ships[], int level){
    int r,c,res;

    if(level==1){
        do{
            r=rand()%SIZE;
            c=rand()%SIZE;
        }while(player[r][c]==HIT || player[r][c]==MISS);
    }
    else if(level==2){
        for(int i=0;i<100;i++){
            r=rand()%SIZE;
            c=rand()%SIZE;
            if(player[r][c]!=HIT && player[r][c]!=MISS) break;
        }
    }
    else{
        for(int i=0;i<SIZE;i++){
            for(int j=0;j<SIZE;j++){
                if(player[i][j]>='A' && player[i][j]<'A'+NUM_SHIPS){
                    r=i; c=j;
                    goto found;
                }
            }
        }
        found:;
    }

    res=fire(player,ships,r,c);
    printf("\n🤖 Computer fires at %c%d: %s\n",
           'A'+r,c+1,res==1?GREEN "HIT!" RESET:BLUE "MISS!" RESET);
}

// Player move
void playerMove(Cell enemy[SIZE][SIZE], Ship ships[]){
    char row; int col,r,c,res;

    while(1){
        printf("\n🎯 Enter target (e.g. B4): ");
        if(scanf(" %c%d",&row,&col)!=2){
            while(getchar()!='\n');
            continue;
        }

        row=toupper(row);
        r=row-'A';
        c=col-1;

        if(r<0||r>=SIZE||c<0||c>=SIZE){
            printf("Invalid!\n");
            continue;
        }

        res=fire(enemy,ships,r,c);

        if(res==1){ printf(GREEN "🔥 HIT!\n" RESET); break; }
        else if(res==0){ printf(BLUE "💧 MISS!\n" RESET); break; }
        else printf("Already fired!\n");
    }
}

int main(){
    srand(time(0));

    Cell p1[SIZE][SIZE], p2[SIZE][SIZE];
    Ship s1[NUM_SHIPS], s2[NUM_SHIPS];

    init(p1); init(p2);

    int mode, difficulty;

    printf("====================================\n");
    printf("        🚢 BATTLESHIP GAME 🚢\n");
    printf("====================================\n");

    printf("1. Single Player\n2. Multiplayer\nChoose: ");
    scanf("%d",&mode);

    if(mode==1){
        printf("Select Difficulty: 1-Easy 2-Medium 3-Hard: ");
        scanf("%d",&difficulty);
    }

    placeShips(p1,s1);
    placeShips(p2,s2);

    while(1){
        printf("\033[2J\033[1;1H");

        printf("\n====================================\n");
        printf("        🎯 PLAYER 1 TURN\n");
        printf("====================================\n");

        printf("\nEnemy Board:\n");
        print(p2,1);

        printf("\n🚢 Enemy Ships Remaining: %d", shipsRemaining(s2));
        printf("\n🧩 Enemy Cells Remaining: %d\n", cellsRemaining(s2));

        playerMove(p2,s2);

        if(allSunk(s2)){
            printf("\n🎉 PLAYER 1 WINS!\n");
            break;
        }

        if(mode==2){
            printf("\n====================================\n");
            printf("        🎯 PLAYER 2 TURN\n");
            printf("====================================\n");

            printf("\nEnemy Board:\n");
            print(p1,1);

            printf("\n🚢 Enemy Ships Remaining: %d", shipsRemaining(s1));
            printf("\n🧩 Enemy Cells Remaining: %d\n", cellsRemaining(s1));

            playerMove(p1,s1);

            if(allSunk(s1)){
                printf("\n🎉 PLAYER 2 WINS!\n");
                break;
            }
        } else {
            sleep(1);
            aiMove(p1,s1,difficulty);

            printf("\n📊 Your Status:");
            printf("\n🚢 Your Ships Remaining: %d", shipsRemaining(s1));
            printf("\n🧩 Your Cells Remaining: %d\n", cellsRemaining(s1));

            if(allSunk(s1)){
                printf("\n💀 COMPUTER WINS!\n");
                break;
            }
        }
    }

    printf("\nFinal Boards:\nPlayer 1:\n");
    print(p1,0);

    printf("\nPlayer 2:\n");
    print(p2,0);

    return 0;
}