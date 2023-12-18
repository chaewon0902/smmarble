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




//board configuration parameters
static int board_nr;
static int food_nr;
static int festival_nr;

static int player_nr;


typedef struct player {
        int energy;
        int position;
        char name[MAX_CHARNAME];
        int accumCredit;
        int flag_graduate;
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
         printf("%s : credit %i, energy %i, position %i\n", 
                      cur_player[i].name,
                      cur_player[i].accumCredit,
                      cur_player[i].energy,
                      cur_player[i].position);
     }
}

void generatePlayers(int n, int initEnergy) //generate a new player
{
     int i;
     //n time loop
     for (i=0;i<n;i++)
     {
         //input name
         printf("Input player %i's name:", i); //��E���� ���硾�� 
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


int rolldie(int player)
{
    char c;
    printf(" Press any key to roll a die (press g to see grade): ");
    scanf("%c", &c); 
    

    if (c == 'g') {
        printGrades(player);
	}	
	
    
    return (rand()%MAX_DIE + 1);
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
            cur_player[player].energy -= smmObj_getNodeEnergy(boardPtr);

            // grade generation
            // int gradePtr; // �� �κ��� ���ŵǾ�� �մϴ�.
            smmObj_genObject(name, smmObjType_grade, 0, smmObj_getNodeCredit(boardPtr), 0, 0);
            smmdb_addTail(LISTNO_OFFSET_GRADE + player, gradePtr);
            // ��� ������ ������ ����� ó���� �ʿ��մϴ�.

            break;

        default:
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

        if (boardPtr == NULL) {
            printf("[ERROR] Invalid node. \n");
            cur_player[player].position--;
            break;
        } else {
            printf("=> Jump to %s\n", smmObj_getNodeName(boardPtr)); //�ֻ��� ���� ����ŭ jump�ϴ� ��� 
        }
    }
	//���� �����Ѵٸ� �ѹ��� �������Ƿ� ������ �߰��� ����
	 

    if (boardPtr != NULL) {
    	//���� ĭ�� �����ϸ� join�ϰڳİ� ����, ���� �����ϸ� ���� ��� �ڵ� ¥�� 
    	if (smmObj_getNodeType(boardPtr) == SMMNODE_TYPE_LECTURE) {
        printf(" -> Lecture %s (credit:%d, energy:%d) starts! are you going to join? or drop? :",
               smmObj_getNodeName(boardPtr), smmObj_getNodeCredit(boardPtr), smmObj_getNodeEnergy(boardPtr));

    char choice[10];
        scanf("%9s", choice); //������� ������ �Է¹޽��ϴ�.

        if (strcmp(choice, "join") == 0) {
            int energySpent = smmObj_getNodeEnergy(boardPtr); 
    		cur_player[player].energy -= energySpent; //��忡�� ����� �������� �� ���� ����ϱ� ���� 
			 
            char grades[] = {'A', 'B', 'C', 'D', 'F'}; //���: A~F 
            int randomIndex = rand() % 5; //�������� ����� ����Ѵ�. 
            float randomGrade = (float)(rand() % 4) + (float)(rand() % 10) / 10; // 0.0���� 4.9������ ������ ����� �����Ѵ�.
            printf(" -> %s successfully takes the lecture %s with grade %c+ (average : %.6f), remained energy : %d)\n",
                   cur_player[player].name, smmObj_getNodeName(boardPtr), grades[randomIndex], randomGrade, cur_player[player].energy);
 		       
		
		}
		}
		// �������� ����  ĭ�� �����ϸ� �������� ī�� ����, �װſ� �´� ������ ��� �ڵ� 
    else if (smmObj_getNodeType(boardPtr) == SMMNODE_TYPE_FOODCHANCE) {
    	printf("%s gets a food chance! press any key to pick a food card : ",cur_player[player].name);
    	getchar(); // ���� ����
        getchar(); // ����� �Է� ��ٸ���
        
        int randomFoodIndex = rand() % food_nr; 
        void *foodCard = smmdb_getData(LISTNO_FOODCARD, randomFoodIndex);
        
        int foodEnergy = smmObj_getNodeEnergy(foodCard);
        printf(" -> %s picks %s and charges %d (remained energy : %d)\n", cur_player[player].name, smmObj_getNodeName(foodCard), foodEnergy, cur_player[player].energy + foodEnergy);
        cur_player[player].energy += foodEnergy;
    }
    //���� ĭ�� �����ϸ� ����Ƿ� �̵��ϴ� �ڵ�  (���� ���� ���ذ� ����)
	else if (smmObj_getNodeType(boardPtr) == SMMNODE_TYPE_GOTOLAB) {
    printf("OMG! This is experiment time!! Player %s goes to the lab.\n", cur_player[player].name);
    }
	
	//�Ĵ翡 �����ϸ� �������� �����ϵ��� �ϴ� �ڵ� (ī��, ������)
	else if (smmObj_getNodeType(boardPtr) == SMMNODE_TYPE_RESTAURANT) {
    int energyGained = 0; // ȹ���� �������� ������ ���� �ʱ�ȭ �ؾ� �� 

    // ������� ��� ������ ���� ȹ���� ������ �ٸ��� ����
    int restaurantType = smmObj_getNodeSubType(boardPtr);
    if (restaurantType == SMMNODE_SUBTYPE_CAFE) {
        printf("Let's get in cafe and charges 2 energies \n");
        energyGained = 2;
    } else if (restaurantType == SMMNODE_SUBTYPE_BURGER_JOINT) {
        printf("Let's get in ������ and charges 6 energies \n");
        energyGained = 6;
    }

    cur_player[player].energy += energyGained; // ���� �������� ���� �÷��̾��� �������� �߰��մϴ�.
    printf("(remained energy : %d)\n", cur_player[player].energy); // ȹ���� �������� ���� ������ ���
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
    void *foodObj = smmObj_genObject(name, smmObjType_food, 0, 0, energy, 0); // board�� ���� ������� �ۼ�

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


#if 0
   // 3. festival card config 

if ((fp = fopen(FESTFILEPATH, "r")) == NULL)
{
    printf("[ERROR] failed to open %s. This file should be in the same directory of SMMarble.exe.\n", FESTFILEPATH);
    return -1;
}

printf("\n\nReading festival card component......\n");
char festivalName[MAX_NAME]; // ���� ũ�⸦ �����մϴ�.

while (fscanf(fp, "%s", festivalName) == 1) // ���� ī�� ���ڿ��� �н��ϴ�.
{
    // �Ķ���� ��Ʈ ����
    void *festivalObj = smmObj_genObject(festivalName, smmObjType_festival, 0, 0, 0, 0);
    
    if (festivalObj != NULL) {
        // ���� ī�带 LISTNO_FESTCARD�� �߰��մϴ�.
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

// �߰��� ���� ī�带 Ȯ���մϴ�.
for (i = 0; i < festival_nr; i++)
{
    void *festivalObj = smmdb_getData(LISTNO_FESTCARD, i);

    if (festivalObj != NULL) {
        printf("node %i : %s\n", i, smmObj_getNodeName(festivalObj));
    } else {
        printf("[ERROR] Failed to retrieve festival card from the list.\n");
    }
}
#endif
   
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
        turn = (turn + 1)%player_nr;
    }
    
    
    free(cur_player);
    system("PAUSE");
    return 0;
}

