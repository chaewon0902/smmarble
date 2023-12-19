//
//  main.c
//  SMMarble
//
//  Created by Juyeop Kim on 2023/11/05.
//

#include <time.h>
#include <string.h>
#include <stdlib.h>
#include "smm_object.h"
#include "smm_database.h"
#include "smm_common.h"

#define BOARDFILEPATH "marbleBoardConfig.txt"
#define FOODFILEPATH "marbleFoodConfig.txt"
#define FESTFILEPATH "marbleFestivalConfig.txt"


#define MAX_PLAYER 10
#define MAX_CHARNAME 20
#define MAX_DIE 6
#define MAX_NAME	50

//board configuration parameters
static int board_nr;
static int food_nr;
static int festival_nr;

static int player_nr;
int turn = 0;

typedef struct player {
        int energy;
        int position;
        char name[MAX_CHARNAME];
        int accumCredit;
        int flag_graduate;
        int experience;
} player_t;

static player_t *cur_player;
//static player_t cur_player[MAX_PLAYER];





#if 0
static int player_energy[MAX_PLAYER];
static int player_position[MAX_PLAYER];
static char player_name[MAX_PLAYER][MAX_CHARNAME];
#endif

//function prototypes
#if 0
int isGraduated(void); //check if any player is graduated
void printGrades(int player); //print grade history of the player
void goForward(int player, int step); //make player go "step" steps on the board (check if player is graduated)
void printPlayerStatus(void); //print all player status at the beginning of each turn
float calcAverageGrade(int player); //calculate average grade of the player
smmGrade_e takeLecture(int player, char *lectureName, int credit); //take the lecture (insert a grade of the player)
void* findGrade(int player, char *lectureName); //find the grade from the player's grade history
void printGrades(int player); //print all the grade history of the player
#endif



void printGrades(int player)
{
     int i;
     void *gradePtr;
     for (i=0;i<smmdb_len(LISTNO_OFFSET_GRADE + player);i++)
     {
         gradePtr = smmdb_getData(LISTNO_OFFSET_GRADE + player, i);
         printf("%s : %i\n", smmObj_getNodeName(gradePtr), smmObj_getNodeGrade(gradePtr));
     }
}

void printPlayerStatus(void)
{
     int i;
     
     for (i=0;i<player_nr;i++)
     {
     	printf("-------------------------------------------------------\n");
         printf("%s : credit %i, energy %i, position %i\n", 
                      cur_player[i].name,
                      cur_player[i].accumCredit,
                      cur_player[i].energy,
                      cur_player[i].position);
        printf("-------------------------------------------------------\n");
     }
}

void generatePlayers(int n, int initEnergy) //generate a new player
{
     int i;
     //n time loop
     for (i=0;i<n;i++)
     {
         //input name
         printf("Input player %i's name:", i); //¾E³≫ ¹®±¸ 
         scanf("%19s", cur_player[i].name);
         fflush(stdin);
         
         //set position
         //player_position[i] = 0;
         cur_player[i].position = 0;
         
         //set energy
         //player_energy[i] = initEnergy;
         cur_player[i].energy = initEnergy;
         cur_player[i].accumCredit = 0;
         cur_player[i].flag_graduate = 0;
     }
}


int rolldie() {
    return (rand() % MAX_DIE + 1);
}

void playPlayerTurn(int player) {
    printf("Player %s, press any key to roll the die (press g to see grade) :", cur_player[player].name);
    getchar(); // 사용자의 아무 키나 입력받음
    int die_result = rolldie(); // 주사위를 굴립니다.
    printf("Player %s rolled a %d.\n", cur_player[player].name, die_result);

    goForward(player, die_result); // 플레이어를 주사위 결과만큼 이동시킵니다.
    actionNode(player); // 도착 지점에서의 행동을 수행합니다.
}



//action code when a player stays at a node
void actionNode(int player) {
    void *boardPtr = smmdb_getData(LISTNO_NODE, cur_player[player].position);
    int type = smmObj_getNodeType(boardPtr);
    char *name = smmObj_getNodeName(boardPtr);
    void *gradePtr;


    	switch (type) {
        case SMMNODE_TYPE_LECTURE:
    	cur_player[player].accumCredit += smmObj_getNodeCredit(boardPtr);
    	int energySpent = smmObj_getNodeEnergy(boardPtr);

    	printf(" -> Lecture %s (credit:%d, energy:%d) starts! Are you going to join or drop? :",
        smmObj_getNodeName(boardPtr), smmObj_getNodeCredit(boardPtr), energySpent);

    	char choice[10];
    	scanf("%9s", choice); // 사용자의 선택을 입력받습니다.

    	if (strcmp(choice, "join") == 0) {
        // 충분한 에너지가 있는지 확인합니다.
        if (energySpent <= cur_player[player].energy) {
            cur_player[player].energy -= energySpent; // 노드에서 사용한 에너지를 뺀 값을 출력하기 위해 

            char grades[] = {'A', 'B', 'C', 'D', 'F'}; // 등급: A~F 
            int randomIndex = rand() % 5; // 랜덤으로 등급을 출력합니다. 
            float randomGrade = (float)(rand() % 4) + (float)(rand() % 10) / 10; // 0.0부터 4.9까지의 랜덤한 등급을 생성합니다.
            printf(" -> %s successfully takes the lecture %s with grade %c+ (average : %.6f), remained energy : %d)\n",
                cur_player[player].name, smmObj_getNodeName(boardPtr), grades[randomIndex], randomGrade, cur_player[player].energy);
        } else {
            // 에너지가 부족하여 수강 불가능한 경우
            printf("%s is too hungry to take the lecture %s\n", cur_player[player].name, smmObj_getNodeName(boardPtr));
        }
    	}
   		 break;

            // grade generation

            smmObj_genObject(name, smmObjType_grade, 0, smmObj_getNodeCredit(boardPtr), 0, 0);
            smmdb_addTail(LISTNO_OFFSET_GRADE + player, gradePtr);
            
			break;
			
		case SMMNODE_TYPE_HOME:
			cur_player[player].energy += 18; // 집에 도착하면 에너지를 18만큼 받음
			printf("--> returned to HOME!"); 
            printf("energy charged by 18 (total : %d)\n", cur_player[player].energy); // 에너지 증가 확인을 위한 출력
            
            break;
            
       case SMMNODE_TYPE_RESTAURANT:
    if (strcmp(name, "cafe") == 0) {
        cur_player[player].energy += 2; // 카페에 도착했을 때 2만큼의 에너지를 얻음
        printf("(remained energy : %d)\n", cur_player[player].energy);
    } else if (strcmp(name, "burger_joint") == 0) {
        cur_player[player].energy += 6; // 버거집에 도착했을 때 6만큼의 에너지를 얻음
        printf("(remained energy : %d)\n", cur_player[player].energy);
    }
    break;
            
            
        case SMMNODE_TYPE_GOTOLAB:
        	printf("OMG! This is experiment time!! Player %s goes to the lab.\n", cur_player[player].name);
        	
        	// 실험 성공 기준을 주사위를 굴려서 4 이상이 나올 때까지 반복합니다.
    	
		int diceResult;
            do {
                printf("Player %s is conducting an experiment...\n", cur_player[player].name);
                diceResult = rolldie(); // 주사위를 굴립니다.
                printf("Player %s rolled a %d.\n", cur_player[player].name, diceResult);

                if (diceResult >= 4) {
                    printf("Experiment successful!\n");
                    break; // 주사위 결과가 4 이상이면 실험 성공으로 종료합니다.
                } else {
                    printf("Experiment failed! Player %s will continue the experiment next turn.\n", cur_player[player].name);
                    printf("Next player's turn...\n");
                    turn = (turn + 1) % player_nr; // 실험 실패 시 다음 플레이어의 턴으로 넘어갑니다.
                }
            } while (1);

            break;
            
    

            
        case SMMNODE_TYPE_FOODCHANCE:
        	printf("%s gets a food chance! press any key to pick a food card : ",cur_player[player].name);
    		getchar(); // 버퍼 비우기
        	getchar(); // 사용자 입력 기다리기
        
        	int randomFoodIndex = rand() % food_nr; 
        	void *foodCard = smmdb_getData(LISTNO_FOODCARD, randomFoodIndex);
        
        	int foodEnergy = smmObj_getNodeEnergy(foodCard);
        	printf(" -> %s picks %s and charges %d (remained energy : %d)\n", cur_player[player].name, smmObj_getNodeName(foodCard), foodEnergy, cur_player[player].energy + foodEnergy);
        	cur_player[player].energy += foodEnergy;
        
       		break;
        
    
		case SMMNODE_TYPE_FESTIVAL:
    		printf("%s gets a festival chance! Press any key to pick a festival card:", cur_player[player].name);
    		getchar(); // 입력 버퍼 비우기
    		getchar(); // 사용자 입력 대기
    
    		int randomFestivalIndex = rand() % festival_nr; // 랜덤한 축제 카드 인덱스 선택
    		void *festivalCard = smmdb_getData(LISTNO_FESTCARD, randomFestivalIndex); // 선택한 카드 가져오기
    
    		printf("-> %s picks %s\n", cur_player[player].name, smmObj_getNodeName(festivalCard)); // 카드 이름 출력
    		break;
		
		
		case SMMNODE_TYPE_LABORATORY:
    	if (cur_player[player].experience) {
        printf("Experiment time! Let's see if you can satisfy the professor (threshold: 5)\n");
        printf("Press any key to roll a die: ");
        getchar(); // 버퍼 비우기
        getchar(); // 사용자 입력 대기
        
        int diceResult = rolldie(); // 주사위 굴리기
        printf("You rolled a %d.\n", diceResult);

        if (diceResult >= 5) {
            printf("Success!\n");
        } else {
            printf("Experiment failed! You need more experiment...\n");
            printf("Next player's turn...\n");
            turn = (turn + 1) % player_nr;
        }
    	} else {
        printf("This is not experiment time. You can go through this lab.\n");
        turn = (turn + 1) % player_nr;
   		}
    	break;




        default:
        	printf("Unknown node type.\n");
            break;
    }
}



void goForward(int player, int step)
{
    void *boardPtr;
    int i;
    
    printf("\n --> result : %d\n", step);

    for (i = 0; i < step; i++) {
        cur_player[player].position++;
        boardPtr = smmdb_getData(LISTNO_NODE, cur_player[player].position);

        // 이동한 후의 칸에 대해 행동을 수행합니다.
     if (boardPtr == NULL) {
            printf("[ERROR] Invalid node. \n");
            cur_player[player].position--;
            break;
        } else {
            printf("=> Jump to %s\n", smmObj_getNodeName(boardPtr)); //주사위 나온 값만큼 jump하는 방식 
        }
        
	}
}
		
	


int main(int argc, const char * argv[]) {
    
    FILE* fp;
    char name[MAX_CHARNAME];
    int type;
    int credit;
    int energy;
    int i;
    int initEnergy;
    int turn=0;
    
    board_nr = 0;
    food_nr = 0;
    festival_nr = 0;
    
    srand(time(NULL));
    
    
    //1. import parameters ---------------------------------------------------------------------------------
    //1-1. boardConfig 
    if ((fp = fopen(BOARDFILEPATH,"r")) == NULL)
    {
        printf("[ERROR] failed to open %s. This file should be in the same directory of SMMarble.exe.\n", BOARDFILEPATH);
        getchar();
        return -1;
    }
    
     
    printf("Reading board component......\n");
    while ( fscanf(fp, "%s %i %i %i", name, &type, &credit, &energy) == 4 ) //read a node parameter set
    {
        //store the parameter set
        //(char* name, smmObjType_e objType, int type, int credit, int energy, smmObjGrade_e grade)
        void *boardObj = smmObj_genObject(name, smmObjType_board, type, credit, energy, 0);
        
		smmdb_addTail(LISTNO_NODE, boardObj);
        
        if (type == SMMNODE_TYPE_HOME)
           initEnergy = energy;
        board_nr++;
    }
    fclose(fp);
    
    
    
    for (i = 0;i<board_nr;i++)
    {
        void *boardObj = smmdb_getData(LISTNO_NODE, i);
        
        printf("node %i : %s, %i(%s), credit %i, energy %i\n", 
                     i, smmObj_getNodeName(boardObj), 
                     smmObj_getNodeType(boardObj), smmObj_getTypeName(smmObj_getNodeType(boardObj)),
                     smmObj_getNodeCredit(boardObj), smmObj_getNodeEnergy(boardObj));
    }
    printf("Total number of board nodes : %i\n", board_nr);
    //printf("(%s)", smmObj_getTypeName(SMMNODE_TYPE_LECTURE));
    
    
    
   // 2. food card config 
if ((fp = fopen(FOODFILEPATH,"r")) == NULL)
{
    printf("[ERROR] failed to open %s. This file should be in the same directory of SMMarble.exe.\n", FOODFILEPATH);
    return -1;
}

printf("\n\nReading food card component......\n");
while (fscanf(fp, "%s %i", name, &energy) == 2) // read a food parameter set
{
    // store the parameter set
    void *foodObj = smmObj_genObject(name, smmObjType_food, 0, 0, energy, 0); // board와 같은 방식으로 작성

    // Add foodObj to the correct list (LISTNO_FOODCARD)
    smmdb_addTail(LISTNO_FOODCARD, foodObj); // Fix this line to add foodObj to the correct list
    
    if (smmObj_getNodeType(foodObj) == SMMNODE_TYPE_HOME)
    {
        initEnergy = energy;
    }
    food_nr++;
}
fclose(fp);


// Print the food cards to verify
for (i = 0; i < food_nr; i++)
{
    void *foodObj = smmdb_getData(LISTNO_FOODCARD, i);

    printf("node %i : %s, energy %i\n",
             i, smmObj_getNodeName(foodObj),
             smmObj_getNodeEnergy(foodObj));
}
printf("Total number of food cards : %i\n", food_nr);



   // 3. festival card config 

if ((fp = fopen(FESTFILEPATH, "r")) == NULL)
{
    printf("[ERROR] failed to open %s. This file should be in the same directory of SMMarble.exe.\n", FESTFILEPATH);
    return -1;
}

printf("\n\nReading festival card component......\n");
char festivalName[MAX_NAME]; // 변수 크기를 설정합니다.

while (fscanf(fp, "%s", festivalName) == 1) // 축제 카드 문자열을 읽습니다.
{
    // 파라미터 세트 저장
    void *festivalObj = smmObj_genObject(festivalName, smmObjType_festival, 0, 0, 0, 0);
    
    if (festivalObj != NULL) {
        // 축제 카드를 LISTNO_FESTCARD에 추가합니다.
        if (smmdb_addTail(LISTNO_FESTCARD, festivalObj) == -1) {
            printf("[ERROR] Failed to add festival card to the list.\n");
            return -1;
        }
        festival_nr++;
    } else {
        printf("[ERROR] Failed to generate festival object.\n");
    }
}
fclose(fp);

printf("Total number of festival cards : %i\n", festival_nr);

// 추가한 축제 카드를 확인합니다.
for (i = 0; i < festival_nr; i++)
{
    void *festivalObj = smmdb_getData(LISTNO_FESTCARD, i);

    if (festivalObj != NULL) {
        printf("node %i : %s\n", i, smmObj_getNodeName(festivalObj));
    } else {
        printf("[ERROR] Failed to retrieve festival card from the list.\n");
    }
}

   
    //2. Player configuration ---------------------------------------------------------------------------------
    
    do
    {
        //input player number to player_nr
        printf("input player number.:");
        scanf("%d", &player_nr);
        fflush(stdin);
    }
    while (player_nr < 0 || player_nr >  MAX_PLAYER);
    
    cur_player = (player_t*)malloc(player_nr*sizeof(player_t));
    generatePlayers(player_nr, initEnergy);
    
    
    
    
    
    //3. SM Marble game starts ---------------------------------------------------------------------------------
    while (1) //is anybody graduated?
    {
        int die_result;
        
        
        //4-1. initial printing
        printPlayerStatus();
        
        //4-2. die rolling (if not in experiment)        
        die_result = rolldie(turn);
        
        //4-3. go forward
        goForward(turn, die_result);

		//4-4. take action at the destination node of the board
        actionNode(turn);
        
        //4-5. next turn
        playPlayerTurn(turn);
        turn = (turn + 1)%player_nr;
    
    
}
    free(cur_player);
    system("PAUSE");
    return 0;
}

