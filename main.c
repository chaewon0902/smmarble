//
//  main.c
//  SMMarble
//
//  Created by Juyeop Kim on 2023/11/05.
//

#include <time.h>
#include <string.h>
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
int turn = 0;

typedef struct player {
        int energy;
        int position;
        char name[MAX_CHARNAME];
        int accumCredit;
        int flag_graduate;
        int experience;
    	char *enrolledCourses[MAX_ENROLLED_COURSES]; // �� �÷��̾ ������ ���� ����� �����ϴ� �迭
    	int numEnrolledCourses; // �� �÷��̾ ������ ���� ���� �����ϴ� ����
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


void opening(){ //������ ��Ʈ��  �߰��ߴ�. 
	printf("------------------------------------------------------------------------------\n");
	printf("-----------Sookmyung Marble !! Let's Graduate (total credit : 30)!!-----------\n");
	printf("------------------------------------------------------------------------------\n");
}



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

void printPlayerStatus(void) //�÷��̾� ���� ������ ������ ��ġ ���¸� ��Ÿ���� ���� �ڵ� �ۼ� 
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

int initial_position[MAX_PLAYER]; //�ʱ���ġ ����
void generatePlayers(int n, int initEnergy) //generate a new player
{
     int i;
     char c;
     //n time loop
     for (i=0;i<n;i++)
     {
         //input name
         printf("Input player %i's name:", i); 
         scanf("%19s", cur_player[i].name);
         
         
         //set position
         //player_position[i] = 0;
         initial_position[i] = 0;
         
         //set energy
         //player_energy[i] = initEnergy;
         cur_player[i].energy = initEnergy;
         cur_player[i].accumCredit = 0;
         cur_player[i].flag_graduate = 0;
         
         while ((c = getchar()) != '\n' && c != EOF);
     }
}


int rolldie(int player)
{
    char c;
    int die_result;
    printf(" Press any key to roll a die (press g to see grade): ");
    c = getchar();
    
    
#if 1
    if (c == 'g')
        printGrades(player);
#endif
    
     while (getchar() != '\n'); // ���ۿ� ���� ���� ���ڸ� ó���Ͽ� �Է� ��Ʈ���� ���ϴ�.

    return (rand() % MAX_DIE + 1);
}



int checkAlreadyEnrolled(char **enrolledCourses, int numEnrolledCourses, char *lectureName) {
    int i;
    for (i = 0; i < numEnrolledCourses; i++) {
        if (strcmp(enrolledCourses[i], lectureName) == 0) {
            // �̹� ������ ������ ���
            return 1;
        }
    }
    // ������ ���ǰ� �ƴ� ���
    return 0;
}



void playPlayerTurn(int currentPlayer) {
    printf("Press any key to roll a die:");
    getchar(); // ����ڰ� Ű�� �Է��� ������ �����
    getchar();
}



//action code when a player stays at a node
void actionNode(int player) 
{
    void *boardPtr = smmdb_getData(LISTNO_NODE, cur_player[player].position);
    int type = smmObj_getNodeType(boardPtr);
    char *name = smmObj_getNodeName(boardPtr);
    void *gradePtr;


    	switch (type) {
        case SMMNODE_TYPE_LECTURE: // ���� ��忡�� ���Ǹ� ��� ��� 
		{
    	int energySpent = smmObj_getNodeEnergy(boardPtr);
    		printf(" -> Lecture %s (credit:%d, energy:%d) starts! Are you going to join or drop? :", smmObj_getNodeName(boardPtr), smmObj_getNodeCredit(boardPtr), energySpent);

    	char choice[10];
    	int invalidChoice = 1;

    	while (invalidChoice) {
        scanf("%9s", choice); // ������� ������ �Է¹޴´� 

        if (strcmp(choice, "join") == 0) {
            // ����� �������� �ִ��� Ȯ���մϴ�.
            if (energySpent <= cur_player[player].energy) {
                cur_player[player].energy -= energySpent; // ��忡�� ����� �������� �� ���� ����ϱ� ����

                char grades[] = {'A', 'B', 'C', 'D', 'F'}; // ���: A~F
                int randomIndex = rand() % 5; // �������� ����� ����մϴ�.
                float randomGrade = (float)(rand() % 4) + (float)(rand() % 10) / 10; // 0.0���� 4.9������ ������ ����� �����մϴ�.
                printf(" -> %s successfully takes the lecture %s with grade %c+ (average : %.6f), remained energy : %d)\n", cur_player[player].name, smmObj_getNodeName(boardPtr), grades[randomIndex], randomGrade, cur_player[player].energy);

                cur_player[player].accumCredit += smmObj_getNodeCredit(boardPtr); // credit �߰�
                invalidChoice = 0;
            } else {
                // �������� �����Ͽ� ���� �Ұ����� ���
                printf("%s is too hungry to take the lecture %s\n", cur_player[player].name, smmObj_getNodeName(boardPtr));
                invalidChoice = 0;

                // �̹� ������ ������ ��� �ߺ����� ���� �� ����
                char *lectureName = smmObj_getNodeName(boardPtr);
                if (checkAlreadyEnrolled(cur_player[player].enrolledCourses, cur_player[player].numEnrolledCourses, lectureName)) {
                    printf("%s has already taken the lecture %s\n", cur_player[player].name, lectureName);
                }
            }
        	} else if (strcmp(choice, "drop") == 0) {
            // drop ���� �� ó��

            invalidChoice = 0;
        	} else {
            // join �Ǵ� drop �̿��� ������ �޾��� �� �����ðڽ��ϱ� ��Ʈ  �ٽ� ��� 
            	printf("Please enter 'join' or 'drop'.\n");
            	printf("Are you going to join or drop? :");
        	}

        // �Է� ���� ����
        	while (getchar() != '\n');
    		}
    		break;
	}
			
		case SMMNODE_TYPE_HOME: //���� ������ ���
			if (cur_player[player].position == initial_position[player]) { 
			cur_player[player].energy += 18; // ���� �����ϸ� �������� 18��ŭ ����
			printf("--> returned to HOME!"); 
            printf("energy charged by 18 (total : %d)\n", cur_player[player].energy); // ������ ���� Ȯ���� ���� ���
        	}
        	
            
            break;
            
            
            
       case SMMNODE_TYPE_RESTAURANT: // �Ĵ翡 ������ ���
       		{
       		char *restaurantName = smmObj_getNodeName(boardPtr); //������� �̸� �������� (ī��, ������) 
       		int energyGained = smmObj_getNodeEnergy(boardPtr);
    		cur_player[player].energy += smmObj_getNodeEnergy(boardPtr);
    		
			printf("Let's eat in %s and charge %d energies (remained energy : %d)\n", restaurantName, energyGained, cur_player[player].energy);
    		break;
        }
            
            
            
        case SMMNODE_TYPE_GOTOLAB: // ����Ƿ� ���� ���
    		printf("OMG! This is experiment time!! Player %s goes to the lab.\n", cur_player[player].name);
    		cur_player[player].experience = 1; // �÷��̾��� ���� ���� ���� �÷��� ����
    		cur_player[player].position = SMMNODE_TYPE_LABORATORY; // ����Ƿ� �̵�
    		turn = (turn + 1) % player_nr; // �� �� �ѱ��
	
	


		int player_attempt = 0; // ���� �õ� Ƚ���� ������ ����

		case SMMNODE_TYPE_LABORATORY:
    		if (cur_player[player].experience == 1) {
        	int threshold = rand() % 6 + 1; // 1���� 6������ ������ ���ذ� ����
        
        	printf("-> Experiment time! Let's see if you can satisfy the professor (threshold: %d)\n ", threshold);

        while (player_attempt < 1) { // ���� �ݺ��� ���� ���� (�ִ� 1ȸ �õ�)
            printf("Press any key to roll a die: ");
            getchar(); // ���� ����
            getchar(); // ����� �Է� ���

            int diceResult = rand() % 6 + 1; // 1���� 6������ ������ �ֻ��� �� ����

            if (diceResult >= threshold) {
                printf("Success!\n");
                break; // �����ϸ� �ݺ� ����
            } else {
                printf("Experiment failed! You need more experiment...\n");
                cur_player[player].experience = 0;
                player_attempt++; // ���� �õ� Ƚ�� ����
            }
        	}
        	turn = (turn + 1) % player_nr; // ���� �ݺ� �� ���� ������ �̵�
        	player_attempt = 0; // �õ� Ƚ�� �ʱ�ȭ
    	} else {
        	printf("This is not experiment time. You can go through this lab.\n");
        	turn = (turn + 1) % player_nr; // ���� �������� ���� ��쿡�� �� ����
    	}
    	break;

            
        case SMMNODE_TYPE_FOODCHANCE: //���� ������ ���� ��� 
        	printf("%s gets a food chance! press any key to pick a food card : ",cur_player[player].name); 
    		getchar(); // ���� ����
        	getchar(); // ����� �Է� ��ٸ���
        
        	//�������� ���� ī�� �߿��� �� �� �̴´� 
        	int randomFoodIndex = rand() % food_nr; 
        	void *foodCard = smmdb_getData(LISTNO_FOODCARD, randomFoodIndex);
        
        	int foodEnergy = smmObj_getNodeEnergy(foodCard);
        	printf(" -> %s picks %s and charges %d (remained energy : %d)\n", cur_player[player].name, smmObj_getNodeName(foodCard), foodEnergy, cur_player[player].energy + foodEnergy);
        	cur_player[player].energy += foodEnergy;
        
       		break;
        
    
		case SMMNODE_TYPE_FESTIVAL: //���� ĭ�� ������ ��� 
    		printf("%s gets a festival chance! Press any key to pick a festival card:", cur_player[player].name);
    		getchar(); // �Է� ���� ����
    		getchar(); // ����� �Է� ���
    		
    		
    		//�������� ���� ī�� �߿��� �� �� �̴´� 
    		int randomFestivalIndex = rand() % festival_nr; // ������ ���� ī�� �ε��� ����
    		void *festivalCard = smmdb_getData(LISTNO_FESTCARD, randomFestivalIndex); // ������ ī�� ��������
    
    		printf("-> %s picks %s\n", cur_player[player].name, smmObj_getNodeName(festivalCard)); // ī�� �̸� ���
    		break;
		
		
	
        default:
        	
            break;
    }
}



void goForward(int player, int step) { //�÷��̾ �̵��ϴ� ���� ���� 
    void *boardPtr;
    int i;

    printf("\n --> result : %d\n", step);

    for (i = 0; i < step; i++) {
        cur_player[player].position++;
        if (cur_player[player].position >= board_nr) {
            cur_player[player].position = 0; // ó�� ��ġ�� ���ư����� ����
        }

        boardPtr = smmdb_getData(LISTNO_NODE, cur_player[player].position);

        printf("=> Jump to %s\n", smmObj_getNodeName(boardPtr)); // �ֻ��� ���� ����ŭ jump�ϴ� ���
    
		
    }

	}



float convertGradeToPoint(const char* grade) {  //���� ��� ���� ������ ������ ��ȯ�ϱ� ���� �ڵ� �ۼ� 
    
    float point = 0.0;

	//���� ���� �ش��ϴ� ���� �ο��ߴ� 
    if (strcmp(grade, "A") == 0) {
        point = 4.3;
    } else if (strcmp(grade, "B") == 0) {
        point = 3.3;
    } else if (strcmp(grade, "C") == 0) {
        point = 2.3;
    } else if (strcmp(grade, "D") == 0) {
        point = 1.3;
    } else if (strcmp(grade, "F") == 0) {
        point = 0.3;
    }

    return point;
}

float calcAverageGrade(int player) { //������ ���ǵ鿡 ���� ��� ������ ����ϴ� �Լ���, ���� ���� �� ���� ����� ����Ѵ�. 
    int i;
    float totalGrade = 0.0; //�ʱ�ȭ 

	//�� ������ ���ǵ��� �ݺ��ϸ鼭 converGradeToPoint �Լ��� �̿��� ������ ������ ��ȯ�ϰ�, totalGrade�� ������ ��, ������ ���� ���� ����� ��� ������ ��� 
    for (i = 0; i < cur_player[player].numEnrolledCourses; i++) {
        totalGrade += convertGradeToPoint(cur_player[player].enrolledCourses[i]);
    }

    return totalGrade / cur_player[player].numEnrolledCourses;
}
		
		
		
//���� ���� ���� Ȯ���� ���� �ڵ� 
 int isGameOver() {
    // �÷��̾ 30 ������ ��� ����ϰ� ���� �������� �� ���� �����ϵ��� ��
    int i;
    for (i = 0; i < player_nr; ++i) {
        if (cur_player[i].accumCredit >= 30 && cur_player[i].position == initial_position[i]) {
            printf("Congratulations! Player %s has graduated!\n", cur_player[i].name);
            printf("Course Grades for Player %s:\n", cur_player[i].name);
            printGrades(i); // �ش� �÷��̾ ���� ���� �̸��� ���� ���
            float averageGrade = calcAverageGrade(i); // ��� ���� ���
            printf("Average Grade for Player %s: %.2f\n", cur_player[i].name, averageGrade); // ��� ���� ���

            return 1; // ���� ���� ���� �� 
        }
    }
    return 0; // ���� ���� ���� ������
}


int main(int argc, const char * argv[]) {
    
    FILE* fp;
    char name[MAX_CHARNAME];
    int type;
    int credit;
    int energy;
    int i;
    int initEnergy;
    
    board_nr = 0;
    food_nr = 0;
    festival_nr = 0;
    
    srand(time(NULL));
    
    
    //0. opening--------------------------------------------------------------------------------------------
    opening();
    
    
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
printf("Total number of festival cards : %i\n", festival_nr);
   
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
        
       
	   
	//4. game play-----------------------------------------------------------------------------------------------
	    
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
    
    
    if (isGameOver()) {
        printf("Game Over!\n");
        break; // ���� ���� ���� ���� �� ������ �����մϴ�.
    } 
    
    
    turn = (turn + 1) % player_nr;
	}
    free(cur_player);
    return 0;
}

	
 

