#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <conio.h>
#include <windows.h>
#include <stdint.h>
#include <stdbool.h>

const char* const INFO_AUTHOR = "GeForce GTX 982 Ti";
const char* const INFO_VERSION = "1.8 c"; 


// ������״
enum { TETRIS_I = 0, TETRIS_T, TETRIS_L, TETRIS_J, TETRIS_Z, TETRIS_S, TETRIS_O };

// =============================================================================
// 7�ַ����4��ת״̬��4λΪһ�У�
static const uint16_t gs_uTetrisTable[7][4] =
{
    { 0x00F0U, 0x2222U, 0x00F0U, 0x2222U },  // I��
    { 0x0072U, 0x0262U, 0x0270U, 0x0232U },  // T��
    { 0x0223U, 0x0074U, 0x0622U, 0x0170U },  // L��
    { 0x0226U, 0x0470U, 0x0322U, 0x0071U },  // J��
    { 0x0063U, 0x0264U, 0x0063U, 0x0264U },  // Z��
    { 0x006CU, 0x0462U, 0x006CU, 0x0462U },  // S��
    { 0x0660U, 0x0660U, 0x0660U, 0x0660U }   // O��
};

// =============================================================================
// ��ʼ״̬����Ϸ��
// ÿ��Ԫ�ر�ʾ��Ϸ�ص�һ�У��±�������Ϸ�صײ�
// ���˸���2��1���ײ�2ȫ��Ϊ1�����ڽ�����ײ���
// �������ֻ��2��1��Ҫ��֤���з����x�������0������λ����ʱ���BUG
// ����һ����Ϸ�صĿ��Ϊ12��
// �����Ҫ��ͳ��10�У�ֻ���������1���ɣ�0xE007������Ȼ��ʾ��ز���ҲҪ��֮�Ķ�
// ��ĳ��Ԫ��Ϊ0xFFFFUʱ��˵�������ѱ�����
// ����4�����ڸ����飬����ʾ����
// �ٳ�ȥ�ײ�2�У���ʾ��������Ϸ�ظ߶�Ϊ22��
static const uint16_t gs_uInitialTetrisPool[28] =
{
    0xC003U, 0xC003U, 0xC003U, 0xC003U, 0xC003U, 0xC003U, 0xC003U,
    0xC003U, 0xC003U, 0xC003U, 0xC003U, 0xC003U, 0xC003U, 0xC003U,
    0xC003U, 0xC003U, 0xC003U, 0xC003U, 0xC003U, 0xC003U, 0xC003U,
    0xC003U, 0xC003U, 0xC003U, 0xC003U, 0xC003U, 0xFFFFU, 0xFFFFU
};

#define COL_BEGIN 2
#define COL_END 14
#define ROW_BEGIN 4
#define ROW_END 26

// =============================================================================
typedef struct TetrisManager  // ����ṹ��洢��Ϸ�������
{
    uint16_t pool[28];  // ��Ϸ��
    int8_t x;  // ��ǰ����x���꣬�˴�����Ϊ�������Ͻ�����
    int8_t y;  // ��ǰ����y����
    int8_t type[3];  // ��ǰ����һ��������һ����������
    int8_t orientation[3];  // ��ǰ����һ��������һ��������ת״̬
} TetrisManager;

// =============================================================================
typedef struct TetrisControl  // ����ṹ��洢�����������
{
    // ��Ϸ����ÿ�����ɫ
    // ���ڴ˰汾�ǲ�ɫ�ģ�������Ϸ�������޷��洢��ɫ��Ϣ
    // ��Ȼ�����ֻʵ�ֵ�ɫ��ģ���û��Ҫ�����������
    int8_t color[28][16];
    bool dead;  // ��
    bool pause;  // ��ͣ
    bool clockwise;  // ��ת����˳ʱ��Ϊtrue
    int8_t direction;  // �ƶ�����0�����ƶ� 1�����ƶ�
    bool model;  // �Ƿ���AIģʽ 
    uint16_t frameRate;  // ��֮֡���ʱ�� 
    unsigned score;  // �÷�
    unsigned erasedCount[4];  // ������
    unsigned erasedTotal;  // ��������
    unsigned tetrisCount[7];  // ��������
    unsigned tetrisTotal;  // ��������
} TetrisControl;

HANDLE g_hConsoleOutput;  // ����̨���
struct{
	int8_t model;  // ��Ϸ����ģʽ
	int8_t benchmark;  // �����ܷ�ģʽ 
	double startTime;  // ���ü�ʱ�� 
	bool resetFpsCounter; 
} general; 

unsigned long fpsRate = 800; 
// =============================================================================
// �ʵ���
// [0] ��ʦ����ˣ����
// [1] л���ϲ��Ҷ��
// [2] Turbo Boost
// [3] ��Ƶ 
#define easterNum 4
int8_t easterEgg[easterNum] = {0}; 

// =============================================================================
// ��������
// ���ʹ��ȫ�ֱ�����ʽʵ�֣���û��Ҫ������
void autoRun(TetrisManager *manager, TetrisControl *control);  // �Զ�����
void benchmark(TetrisManager *manager, TetrisControl *control);  // �ܷ�����
signed long benchmarkRun(TetrisManager *manager, TetrisControl *control);  // �ֺܷ��� 
signed long calcFPS(void);  // ����FPS������Benchmarkģʽ��������� 
bool checkCollision(const TetrisManager *manager);  // ��ײ���
bool checkErasing(TetrisManager *manager, TetrisControl *control);  // ���м��
void clrscr(void);  // ���� 
double getTime(void);  // �߾��ȼ�ʱ�� 
void dropDownTetris(TetrisManager *manager, TetrisControl *control);  // ����ֱ�����
void giveTetris(TetrisManager *manager, TetrisControl *control);  // ��һ������
void gotoxyInPool(short x, short y); // ��λ����Ϸ�� 
void gotoxyWithFullwidth(short x, short y);  // ��ȫ�Ƕ�λ��ĳ��
void horzMoveTetris(TetrisManager *manager, TetrisControl *control);  // ˮƽ�ƶ�����
void initGame(TetrisManager *manager, TetrisControl *control, bool model);  // ��ʼ����Ϸ
void insertTetris(TetrisManager *manager);  // ���뷽��
void keydownControl(TetrisManager *manager, TetrisControl *control, int key);  // ������
int mainMenu(void);  // ���˵�
void moveDownTetris(TetrisManager *manager, TetrisControl *control);  // �����ƶ�����
void pause(void);  // ��ͣ 
void printCurrentTetris(const TetrisManager *manager, const TetrisControl *control);  // ��ʾ��ǰ����
void printNextTetris(const TetrisManager *manager);  // ��ʾ��һ��������һ������
void printPoolBorder();  // ��ʾ��Ϸ�ر߽�
void printPrompting(const TetrisControl *control);  // ��ʾ��ʾ��Ϣ
void printScore(const TetrisManager *manager, const TetrisControl *control);  // ��ʾ�÷���Ϣ
void printTetrisPool(const TetrisManager *manager, const TetrisControl *control);  // ��ʾ��Ϸ��
void removeTetris(TetrisManager *manager);  // �Ƴ�����
void rotateTetris(TetrisManager *manager, TetrisControl *control);  // ��ת����
void runGame(TetrisManager *manager, TetrisControl *control);  // ������Ϸ
void setPoolColor(const TetrisManager *manager, TetrisControl *control);  // ������ɫ

// =============================================================================
// ������
int main(int argc, char* argv[])
{
	// �����������ʱ����˸������ 
	if (argc > 1)
	{
		char optionEx[64]; 
		for (int i = 1;i < argc;i++)
		{
			if (argv[i][0] == '/' || argv[i][0] == '-')
			{
				argv[i]++;
				// �Ѳ���ת����Сд 
				char *a = argv[i];
				while (*a != '\0' && *a != ':')
				{
					if (*a >= 'A' && *a <= 'Z')
					{
						*a += 0x20;
					}
					a++;
				}
				if (*a == ':')
				{
					*a = '\0';
					a++;
					strcpy(optionEx, a);
				}
				
				if (strcmp(argv[i], "c16") == 0)
				{
					easterEgg[1] = 1;
				}
				else if (strcmp(argv[i], "boost") == 0 || strcmp(argv[i], "turbo") == 0)
				{
					easterEgg[2] = 1;
				}
				else if (strcmp(argv[i], "fpsrate") == 0)
				{
					long newfps = atol(optionEx);
					if (newfps > 0){
						fpsRate = newfps;
						if (fpsRate > 5000)
						{
							fpsRate = 5000;
						}
						easterEgg[3] = 1;
					}
				}
				else
				{
					printf("Error: Unknown command line option \"%s\"\n", argv[i]);
					return 1; 
				}
			}
		}
	}
	
	// ��ʼ������̨���� 
    TetrisManager tetrisManager;
    TetrisControl tetrisControl;
    g_hConsoleOutput = GetStdHandle(STD_OUTPUT_HANDLE);  // ��ȡ����̨������
    CONSOLE_CURSOR_INFO cursorInfo = { 1, FALSE };  // �����Ϣ
    CONSOLE_SCREEN_BUFFER_INFO screenInfo;  // ����̨��Ļ��Ϣ 
	
	GetConsoleScreenBufferInfo(g_hConsoleOutput, &screenInfo);
	COORD bufferSize;
	bufferSize.X = 1 + screenInfo.srWindow.Right;
	bufferSize.Y = 1 + screenInfo.srWindow.Bottom;
	SetConsoleScreenBufferSize(g_hConsoleOutput, bufferSize); 
	clrscr();
	
    SetConsoleCursorInfo(g_hConsoleOutput, &cursorInfo);  // ���ù������
    char conTitle[64] = {0};
    snprintf(conTitle, sizeof(conTitle), "���ܶ���˹����Ver %s",INFO_VERSION);
    SetConsoleTitleA(conTitle);
    
    DeleteMenu(GetSystemMenu(GetConsoleWindow(),0),SC_MAXIMIZE,MF_DISABLED);
	
    do
    {
        general.model = mainMenu();
        if (general.model != 2)
        {
        	general.benchmark = 0;
        }
        else
        {
        	general.benchmark = 1;
        }
        SetConsoleTextAttribute(g_hConsoleOutput, 0x07);
        clrscr(); 
        initGame(&tetrisManager, &tetrisControl, general.model == 0);  // ��ʼ����Ϸ
        printPrompting(&tetrisControl);  // ��ʾ��ʾ��Ϣ
        printPoolBorder();  // ��ʾ��Ϸ�ر߽�
		
		int8_t prevEE2;
        switch (general.model)
        {
        case 0: 
        	tetrisControl.frameRate = 450;
            runGame(&tetrisManager, &tetrisControl);  // ������Ϸ
            break;
		case 1: 
            autoRun(&tetrisManager, &tetrisControl);  // �Զ�����
            break;
        case 2:
        	prevEE2 = easterEgg[2];
			easterEgg[2] = 1; 
    		
        	SetConsoleTextAttribute(g_hConsoleOutput, 0x07);
        	clrscr(); 
        	initGame(&tetrisManager, &tetrisControl, 0);  // ��ʼ����Ϸ
        	printPrompting(&tetrisControl);  // ��ʾ��ʾ��Ϣ
        	printPoolBorder();  // ��ʾ��Ϸ�ر߽�
			signed long mark = benchmarkRun(&tetrisManager, &tetrisControl);
			
			clrscr();
		    SetConsoleTextAttribute(g_hConsoleOutput, 0x0F);
    		gotoxyWithFullwidth(14, 5);
    		printf("������������������������");
    		gotoxyWithFullwidth(14, 6);
    		printf("��%3s%s%3s��", "", "���ܶ���˹����", "");
    		gotoxyWithFullwidth(14, 7);
    		printf("��%6s%s%6s��", "", "�ܷ�ģʽ", "");
    		gotoxyWithFullwidth(14, 8);
    		printf("������������������������");
    		gotoxyWithFullwidth(14, 9);
			printf("���ߣ�%s", INFO_AUTHOR); 
    		gotoxyWithFullwidth(15, 11);
			printf("��ĵ������֣�%6d", mark);
			
			pause();
			easterEgg[2] = prevEE2;
			continue; 
			break;
		default:
			return -1; 
        }

        SetConsoleTextAttribute(g_hConsoleOutput, 0xF0);
        gotoxyWithFullwidth(12, 9);
		printf("%20s\n","");
        gotoxyWithFullwidth(12, 10);
        printf("%2s%s%2s\n","",easterEgg[1] ? " л���ϲ��Ҷ�� " : "������������˵�","");
        gotoxyWithFullwidth(12, 11);
		printf("%20s\n","");
        SetConsoleTextAttribute(g_hConsoleOutput, 0x07);
        pause();
        clrscr(); 

    } while (1);

    gotoxyWithFullwidth(0, 0);
    CloseHandle(g_hConsoleOutput);
    return 0;
}

// =============================================================================
// ��ʼ����Ϸ
void initGame(TetrisManager *manager, TetrisControl *control, bool model)
{
    memset(manager, 0, sizeof(TetrisManager));  // ȫ����0

    // ��ʼ����Ϸ��
    memcpy(manager->pool, gs_uInitialTetrisPool, sizeof(uint16_t [28]));
    if (general.benchmark)
    {
    	srand(2014530982);  // �ܷ�ģʽ��ʹ��Ĭ��������� 
    }
    else
    {
    	srand((unsigned)time(NULL));  // �����������
    }

    manager->type[1] = rand() % 7;  // ��һ��
    manager->orientation[1] = rand() & 3;
    manager->type[2] = rand() % 7;  // ����һ��
    manager->orientation[2] = rand() & 3;
    if (general.benchmark)
    {
    	manager->type[1] = 0;
    	manager->type[2] = 1;
    }

    memset(control, 0, sizeof(TetrisControl));  // ȫ����0
    control->model = model;

    giveTetris(manager, control);  // ����һ������
    setPoolColor(manager, control);  // ������ɫ
    printScore(manager, control);  // ��ʾ�÷���Ϣ
    printTetrisPool(manager, control);  // ���Sync�����ı������� 
    general.startTime = getTime();
    general.resetFpsCounter = true;
    calcFPS();
}

// =============================================================================
// ��һ������
void giveTetris(TetrisManager *manager, TetrisControl *control)
{
    static uint16_t tetris;
	static uint16_t num;
	
    manager->type[0] = manager->type[1];  // ��һ��������Ϊ��ǰ
    manager->orientation[0] = manager->orientation[1];

    manager->type[1] = manager->type[2];// ����һ���÷���Ϊ��һ��
    manager->orientation[1] = manager->orientation[2];
	
	// �����ֽţ���I��������΢��һЩ 
	num = rand();
	if (num >= 32200)
	{
		manager->type[2] = 0;// ��Щ�ֽ� 
	}
	else
	{
    	manager->type[2] = rand() % 7;// �����������һ������
    }
    if (general.benchmark)
    {
    	manager->type[2] = (manager->type[1] + 1) % 7;
    }
    manager->orientation[2] = rand() & 3;

    tetris = gs_uTetrisTable[manager->type[0]][manager->orientation[0]];  // ��ǰ����

    // ���õ�ǰ����y���꣬��֤�ո���ʱֻ��ʾ����������һ��
    // ����ʵ��ʹ����ҿ����Ժܿ���ٶȽ��������ڲ���ʾ�����Ķ���4����
    if (tetris & 0xF000)
    {
        manager->y = 0;
    }
    else
    {
        manager->y = (tetris & 0xFF00) ? 1 : 2;
    }
    manager->x = 6;  // ���õ�ǰ����x����

    if (checkCollision(manager))  // ��⵽��ײ
    {
        control->dead = true;  // �����Ϸ����
    }
    else  // δ��⵽��ײ
    {
        insertTetris(manager);  // ����ǰ���������Ϸ��
    }

    ++control->tetrisTotal;  // ��������
    ++control->tetrisCount[manager->type[0]];  // ��Ӧ������

    printNextTetris(manager);  // ��ʾ��һ������
}

// =============================================================================
// ��ײ���
bool checkCollision(const TetrisManager *manager)
{
    // ��ǰ����
    uint16_t tetris = gs_uTetrisTable[manager->type[0]][manager->orientation[0]];
    uint16_t dest = 0U;

    // ��ȡ��ǰ��������Ϸ���е�����
    // ��Ϸ������x y��С������Ϣ�����͵��ߴ����16λ�޷�������
    dest |= (((manager->pool[manager->y + 0] >> manager->x) << 0x0) & 0x000F);
    dest |= (((manager->pool[manager->y + 1] >> manager->x) << 0x4) & 0x00F0);
    dest |= (((manager->pool[manager->y + 2] >> manager->x) << 0x8) & 0x0F00);
    dest |= (((manager->pool[manager->y + 3] >> manager->x) << 0xC) & 0xF000);

    // ����ǰ������Ŀ����������ص�����ײ������λ��Ľ����Ϊ0
    return ((dest & tetris) != 0);
}

// =============================================================================
// ���뷽��
void insertTetris(TetrisManager *manager)
{
    // ��ǰ����
    uint16_t tetris = gs_uTetrisTable[manager->type[0]][manager->orientation[0]];

    // ��ǰ����ÿ4λȡ����λ����Ϸ����Ӧλ�ã�����ɲ��뷽��
    manager->pool[manager->y + 0] |= (((tetris >> 0x0) & 0x000F) << manager->x);
    manager->pool[manager->y + 1] |= (((tetris >> 0x4) & 0x000F) << manager->x);
    manager->pool[manager->y + 2] |= (((tetris >> 0x8) & 0x000F) << manager->x);
    manager->pool[manager->y + 3] |= (((tetris >> 0xC) & 0x000F) << manager->x);
}

// =============================================================================
// �Ƴ�����
void removeTetris(TetrisManager *manager)
{
    // ��ǰ����
    uint16_t tetris = gs_uTetrisTable[manager->type[0]][manager->orientation[0]];

    // ��ǰ����ÿ4λȡ������λȡ����λ�뵽��Ϸ����Ӧλ�ã�������Ƴ�����
    manager->pool[manager->y + 0] &= ~(((tetris >> 0x0) & 0x000F) << manager->x);
    manager->pool[manager->y + 1] &= ~(((tetris >> 0x4) & 0x000F) << manager->x);
    manager->pool[manager->y + 2] &= ~(((tetris >> 0x8) & 0x000F) << manager->x);
    manager->pool[manager->y + 3] &= ~(((tetris >> 0xC) & 0x000F) << manager->x);
}

// =============================================================================
// ������ɫ
void setPoolColor(const TetrisManager *manager, TetrisControl *control)
{
    // ������ʾ��Ϸ��ʱ����Ҫ����Ϸ�����ж�ĳһ�����з������ʾ��Ӧ�������ɫ
    // ����ֻ�����ü��ɣ�û��Ҫ���
    // ���ƶ�������һ������ʱ����

    int8_t i, x, y;

    // ��ǰ����
    uint16_t tetris = gs_uTetrisTable[manager->type[0]][manager->orientation[0]];

    for (i = 0; i < 16; ++i)
    {
        y = (i >> 2) + manager->y;  // �����õ���
        if (y > ROW_END)  // �����ײ�����
        {
            break;
        }
        x = (i & 3) + manager->x;  // �����õ���
        if ((tetris >> i) & 1)  // ���ĵ�С�������ڵ�ǰ��������
        {
            control->color[y][x] = (manager->type[0] | 8);  // ������ɫ
        }
    }
}

// =============================================================================
// ��ת����
void rotateTetris(TetrisManager *manager, TetrisControl *control)
{
    int8_t ori = manager->orientation[0];  // ��¼ԭ��ת״̬

    removeTetris(manager);  // ���ߵ�ǰ����

    // ˳/��ʱ����ת
    manager->orientation[0] = (control->clockwise) ? ((ori + 1) & 3) : ((ori + 3) & 3);

    if (checkCollision(manager))  // ��⵽��ײ
    {
        manager->orientation[0] = ori;  // �ָ�Ϊԭ��ת״̬
        insertTetris(manager);  // ���뵱ǰ���顣����״̬û�ı䣬����Ҫ������ɫ
    }
    else
    {
        insertTetris(manager);  // ���뵱ǰ����
        setPoolColor(manager, control);  // ������ɫ
        if (!easterEgg[2] || control->model == 0)
        {
        	printCurrentTetris(manager, control);  // ��ʾ��ǰ����
        }
    }
}

// =============================================================================
// ˮƽ�ƶ�����
void horzMoveTetris(TetrisManager *manager, TetrisControl *control)
{
    int x = manager->x;  // ��¼ԭ��λ��

    removeTetris(manager);  // ���ߵ�ǰ����
    control->direction == 0 ? (--manager->x) : (++manager->x);  // ��/���ƶ�

    if (checkCollision(manager))  // ��⵽��ײ
    {
        manager->x = x;  // �ָ�Ϊԭ��λ��
        insertTetris(manager);  // ���뵱ǰ���顣����λ��û�ı䣬����Ҫ������ɫ
    }
    else
    {
        insertTetris(manager);  // ���뵱ǰ����
        setPoolColor(manager, control);  // ������ɫ
        if (!easterEgg[2] || !control->model)
        {
        	printCurrentTetris(manager, control);  // ��ʾ��ǰ����
        }
    }
}

// =============================================================================
// �����ƶ�����
void moveDownTetris(TetrisManager *manager, TetrisControl *control)
{
    int8_t y = manager->y;  // ��¼ԭ��λ��

    removeTetris(manager);  // ���ߵ�ǰ����
    ++manager->y;  // �����ƶ�

    if (checkCollision(manager))  // ��⵽��ײ
    {
        manager->y = y;  // �ָ�Ϊԭ��λ��
        insertTetris(manager);  // ���뵱ǰ���顣����λ��û�ı䣬����Ҫ������ɫ
        if (checkErasing(manager, control))  // ��⵽����
        {
			if (control->frameRate > 0)
			{
				Sleep(2 * control->frameRate);
    			calcFPS();  // Ϊ�˱�֤FPS��׼ȷ�� 
			}
            printTetrisPool(manager, control);  // ��ʾ��Ϸ��
        }
    }
    else
    {
        insertTetris(manager);  // ���뵱ǰ����
        setPoolColor(manager, control);  // ������ɫ
        printCurrentTetris(manager, control);  // ��ʾ��ǰ����
    }
}

// =============================================================================
// ����ֱ�����
void dropDownTetris(TetrisManager *manager, TetrisControl *control)
{
    removeTetris(manager);  // ���ߵ�ǰ����

    // �������¼��
    // ע�����ﲻ�ܴ������ϣ��������ַ��鴩��������ն���BUG
    for (; manager->y < ROW_END; ++manager->y)
    {
        if (checkCollision(manager))  // ��⵽��ײ
        {
            break;
        }
    }
    --manager->y;  // ����һ��Ȼû����ײ

    insertTetris(manager);  // ���뵱ǰ����
    setPoolColor(manager, control);  // ������ɫ

    printTetrisPool(manager, control);  // ����ʾ��Ϸ��
    if (checkErasing(manager, control) && control->frameRate > 0)  // ������в����� 
	{
		Sleep(2 * control->frameRate);
    	printTetrisPool(manager, control);  // ����ʾһ����Ϸ��
    	calcFPS();  // Ϊ�˱�֤FPS��׼ȷ�� 
	}
}

// =============================================================================
// ���м��
bool checkErasing(TetrisManager *manager, TetrisControl *control)
{
    static const unsigned scores[5] = { 0, 10, 30, 90, 150};  // ���е÷�
    static const unsigned scoreRatios[5] = { 0, 1, 5, 15, 25};  // ���и߶Ƚ�����
    int8_t lowest = 0;
    int8_t count = 0;
    int8_t k = 0, y = manager->y + 3;

    do  // �������ϼ��
    {
        if (y < ROW_END && manager->pool[y] == 0xFFFFU)  // ��Ч��������һ��������
        {
            count++;
            // ��¼��������
			lowest = ROW_END - y; 
            // ����һ�з���
            memmove(manager->pool + 1, manager->pool, sizeof(uint16_t) * y);
            // ��ɫ�����Ԫ����֮�ƶ�
            memmove(control->color[1], control->color[0], sizeof(int8_t [16]) * y);
        }
        else
        {
            --y;
            ++k;
        }
    } while (y >= manager->y && k < 4);
	
	lowest--;
    control->erasedTotal += count;  // ��������
    control->score += scores[count] + lowest * scoreRatios[count] + 1;  // �÷�

    if (count > 0)
    {
        ++control->erasedCount[count - 1];  // ����
    }

    giveTetris(manager, control);  // ����һ������
    setPoolColor(manager, control);  // ������ɫ
    printScore(manager, control);  // ��ʾ�÷���Ϣ

    return (count > 0);
}

// =============================================================================
// ������
void keydownControl(TetrisManager *manager, TetrisControl *control, int key)
{
	static signed char lastAction;
	// if (general.model == 1)
	// {
		calcFPS();
	// }
	
	if (general.model == 0 && lastAction >= 0)
	{
		gotoxyWithFullwidth(27, 10 + lastAction);
		SetConsoleTextAttribute(g_hConsoleOutput, 0x0B);
		printf("��");
	}
	
    if (key == 13)  // ��ͣ/�����ͣ
    {
    	lastAction = 6;
    	
        control->pause = !control->pause;
        
        gotoxyWithFullwidth(27, 10 + lastAction);
		SetConsoleTextAttribute(g_hConsoleOutput, 0x0B);
		if(control->pause)
		{
			printf("��");
		}
		else
		{
			printf("��");
		}
    }

    if (control->pause)  // ��ͣ״̬����������
    {
        return;
    }

    switch (key)
    {
    case 'w': case 'W': case 72:  // ��
    	lastAction = 3;
        control->clockwise = true;  // ˳ʱ����ת
        rotateTetris(manager, control);  // ��ת����
        break;
    case 'a': case 'A': case 75:  // ��
    	lastAction = 0;
        control->direction = 0;  // �����ƶ�
        horzMoveTetris(manager, control);  // ˮƽ�ƶ�����
        break;
    case 'd': case 'D': case 77:  // ��
    	lastAction = 1;
        control->direction = 1;  // �����ƶ�
        horzMoveTetris(manager, control);  // ˮƽ�ƶ�����
        break;
    case 's': case 'S': case 80:  // ��
    	lastAction = 2;
        moveDownTetris(manager, control);  // �����ƶ�����
        break;
    case ' ':  // ֱ�����
    	lastAction = 5;
        dropDownTetris(manager, control);
        break;
    case 'x': case 'X': case '0':  // ��ת
    	lastAction = 4;
        control->clockwise = false;  // ��ʱ����ת
        rotateTetris(manager, control);  // ��ת����
        break;
    default:
    	lastAction = -1;
        break;
    }
    
    if (general.model == 0 && lastAction >= 0)
	{
    	gotoxyWithFullwidth(27, 10 + lastAction);
		SetConsoleTextAttribute(g_hConsoleOutput, 0x0B);
		printf("��");
	}
}

// =============================================================================
// ��ȫ�Ƕ�λ��ĳ��
inline void gotoxyWithFullwidth(short x, short y)
{
    static COORD cd;

    cd.X = (short)(x << 1);
    cd.Y = y;
    SetConsoleCursorPosition(g_hConsoleOutput, cd);
}

// =============================================================================
// ���˵�
int mainMenu()
{
	static int indexTotal = 3; 
	if (easterEgg[3])
	{
		indexTotal = 2; // ��������˵�Ƶ�������ܷ�ģʽ 
	}
    static const char *const modelItem[] = { "1.�Լ����ֵ���������", "2.�Կ�������������", "3.�޵�������ܷ�ģʽ" };
    #define secretNum 3
    static const char *const secretCode[secretNum] = {"xzsyw", "xwhxhyz", "boost"};
    static char secretKey[1024];
    signed long int index = 0, secretIndex = 0, ch;
    
    SetConsoleTextAttribute(g_hConsoleOutput, 0x0F);
    gotoxyWithFullwidth(14, 5);
    printf("������������������������");
    gotoxyWithFullwidth(14, 6);
    printf("��%3s%s%3s��", "", "���ܶ���˹����", "");
    gotoxyWithFullwidth(14, 7);
    printf("��%3sVersion: %s", "", INFO_VERSION);
    gotoxyWithFullwidth(25, 7);
    printf("��");
    gotoxyWithFullwidth(14, 8);
    printf("������������������������");
	gotoxyWithFullwidth(14, 9);
	printf("���ߣ�%s", INFO_AUTHOR); 
	
    SetConsoleTextAttribute(g_hConsoleOutput, 0xF0);
	for (int i = 0;i < indexTotal;i++)
	{
    	gotoxyWithFullwidth(14, 14 + 2 * i);
    	printf("%2s%s%2s", "", modelItem[i], "");
    	SetConsoleTextAttribute(g_hConsoleOutput, 0x0F);
    }
    
    do
    {
        ch = _getch();
        switch (ch)
        {
        /* 
        // Ϊ��SecretCodeֻ�������� 
        case 'w': case 'W': case '8': case 72:  // ��
        case 'a': case 'A': case '4': case 75:  // ��
        case 'd': case 'D': case '6': case 77:  // ��
        case 's': case 'S': case '2': case 80:  // ��
        */
        
        //===========================================
        case 72: case 75:  // ���� 
		case 77: case 80:  // ���� 
            SetConsoleTextAttribute(g_hConsoleOutput, 0x0F);
            gotoxyWithFullwidth(14, 14 + 2 * index);
            printf("%2s%s%2s", "", modelItem[index], "");
			if (ch == 72 || ch == 75)
			{
                index--;
                if (index < 0)
				{
					index += indexTotal;
				}
            }
			if (ch == 77 || ch == 80)
			{
                index++;
                if (index >= indexTotal)
				{
					index -= indexTotal;
				}
            }
            SetConsoleTextAttribute(g_hConsoleOutput, 0xF0);
            gotoxyWithFullwidth(14, 14 + 2 * index);
            printf("%2s%s%2s", "", modelItem[index], "");
            break;
        case ' ': case 13:  // �ո��Enter 
            return index;
        case 27:  // Esc��ֱ���˳� 
        	exit(0);
        	return 0;
        case 8:  // Backspace���������������ܴ��� 
        	memset(secretKey, 0, sizeof(secretKey));
        	secretIndex = 0;
        	break;
        default:
        	if (ch >= 'A' && ch <= 'Z')
        	{
        		ch += 0x20;
        	}
        	if (ch < 'a' || ch > 'z')
        	{
        		break;
        	}
        	secretKey[secretIndex] = ch;
        	secretIndex++;
        	for (int i = 0;i < secretNum;i++)
			{
				if (strcmp(secretKey, secretCode[i]) == 0)
				{
					easterEgg[i] = !easterEgg[i];
        			memset(secretKey, 0, sizeof(secretKey));
        			secretIndex = 0;
        		}
        	}
        	break;
        }
    } while (1);
}

// =============================================================================
// ��ʾ��Ϸ�ر߽�
void printPoolBorder()
{
    int8_t y;

    SetConsoleTextAttribute(g_hConsoleOutput, 0xF0);
    for (y = ROW_BEGIN; y < ROW_END; ++y)  // ����ʾ����4�к͵ײ�2��
    {
        gotoxyWithFullwidth(10, y - 3);
        printf("%2s", "");
        gotoxyWithFullwidth(23, y - 3);
        printf("%2s", "");
    }

    gotoxyWithFullwidth(10, y - 3);  // �ײ��߽�
    printf("%28s", "");
}

// ��λ����Ϸ���еķ���
inline void gotoxyInPool(short x, short y)
{
	gotoxyWithFullwidth(x + 9, y - 3);
}

// =============================================================================
// ��ʾ��Ϸ��
// �Կ��������񼣣�Sync��ʾ�����ع飡
static uint16_t ppool[16][28] = {0}; 

void printTetrisPool(const TetrisManager *manager, const TetrisControl *control)
{
    int8_t x, y;
    printNextTetris(manager); 

    for (y = ROW_BEGIN; y < ROW_END; ++y)  // ����ʾ����4�к͵ײ�2��
    {
        for (x = COL_BEGIN; x < COL_END; ++x)  // ����ʾ���ұ߽�
        {
            if ((manager->pool[y] >> x) & 1)  // ��Ϸ�ظ÷����з���
            {
        		if (control->color[y][x] == ppool[x][y])continue; // ����δ�仯������ 
                // ����Ӧ��ɫ����ʾһ��ʵ�ķ���
				gotoxyInPool(x, y);
                SetConsoleTextAttribute(g_hConsoleOutput, control->color[y][x]);
                printf("��");
                ppool[x][y] = control->color[y][x];
            }
            else  // û�з��飬��ʾ�հ�
            {
				if (ppool[x][y] == 0)continue; 
                // ����SetConsoleTextAttribute(g_hConsoleOutput, 0);
				gotoxyInPool(x, y);
                printf("%2s", "");
                ppool[x][y] = 0;
            }
        }
    }
}

// =============================================================================
// ��ʾ��ǰ����
void printCurrentTetris(const TetrisManager *manager, const TetrisControl *control)
{
    int8_t x, y;

    // ��ʾ��ǰ���������ƶ�����õģ�Ϊ��ȥ�ƶ�ǰ�ķ��飬��Ҫ��չ��ʾ����
    // ���ڲ����������ƶ����ʲ���Ҫ������չ
    y = (manager->y > ROW_BEGIN) ? (manager->y - 1) : ROW_BEGIN;  // ������չһ��
    for (; y < ROW_END && y < manager->y + 4; ++y)
    {
        x = (manager->x > COL_BEGIN) ? (manager->x - 1) : COL_BEGIN;  // ������չһ��
        for (; x < COL_END && x < manager->x + 5; ++x)  // ������չһ��
        {
            gotoxyInPool(x, y);  // ���㵽��Ϸ���еķ���
            if ((manager->pool[y] >> x) & 1)  // ��Ϸ�ظ÷����з���
            {
                // ����Ӧ��ɫ����ʾһ��ʵ�ķ���
                SetConsoleTextAttribute(g_hConsoleOutput, control->color[y][x]);
                printf("��");
                ppool[x][y] = control->color[y][x];
                
            }
            else  // û�з��飬��ʾ�հ�
            {
            	if (ppool[x][y] = 0)continue;
                // ����SetConsoleTextAttribute(g_hConsoleOutput, 0);
                printf("%2s", "");
                ppool[x][y] = 0;
            }
        }
    }
}

// =============================================================================
// ��ʾ��һ��������һ������
void printNextTetris(const TetrisManager *manager)
{
    int8_t i;
    uint16_t tetris;


    // ��һ��������Ӧ��ɫ��ʾ
    tetris = gs_uTetrisTable[manager->type[1]][manager->orientation[1]];
    SetConsoleTextAttribute(g_hConsoleOutput, manager->type[1] | 8);
    for (i = 0; i < 16; ++i)
    {
        gotoxyWithFullwidth((i & 3) + 27, (i >> 2) + 2);
        ((tetris >> i) & 1) ? printf("��") : printf("%2s", "");
    }
	
	if (general.benchmark)
	{
		// �����Benchmark�ܷ�ģʽ�Ͳ���ʾ����һ��ֱ������ 
		return;
	}
	else
	{
    // ����һ��������ʾ��ɫ
    	tetris = gs_uTetrisTable[manager->type[2]][manager->orientation[2]];
    	SetConsoleTextAttribute(g_hConsoleOutput, 8);
    	for (i = 0; i < 16; ++i)
    	{
    	    gotoxyWithFullwidth((i & 3) + 32, (i >> 2) + 2);
    	    ((tetris >> i) & 1) ? printf("��") : printf("%2s", "");
    	}
    }
}

// =============================================================================
// ��ʾ�÷���Ϣ
void printScore(const TetrisManager *manager, const TetrisControl *control)
{
	static TetrisControl sync; // ��Ч��Syncʵ�ַ��������ˣ�
	static bool flag; 
    int8_t i;

    SetConsoleTextAttribute(g_hConsoleOutput, 0x0E);
	
	if (control->score != sync.score)
	{ 
    	gotoxyWithFullwidth(5, 6);
    	printf("%u", control->score);
    }
	
	flag = false;
    for (i = 0; i < 4; ++i)
    {
    	if (control->erasedCount[i] != sync.erasedCount[i])
    	{
        	gotoxyWithFullwidth(6, 10 + i);
        	printf("%u", control->erasedCount[i]);
        	flag = true;
        }
    }
    if (flag)
    {
    	gotoxyWithFullwidth(7, 8);
    	printf("%u", control->erasedTotal);
    }
	
	flag = false;
    for (i = 0; i < 7; ++i)
    {
    	if (control->tetrisCount[i] != sync.tetrisCount[i])
    	{
        	gotoxyWithFullwidth(6, 17 + i);
        	printf("%u", control->tetrisCount[i]);
        	flag = true;
        }
    }
    if (flag)
    {
    	gotoxyWithFullwidth(7, 15);
		printf("%u", control->tetrisTotal);
	}
    
    
	sync = *control;
}

// =============================================================================
// ��ʾ��ʾ��Ϣ
void printPrompting(const TetrisControl *control)
{
	// ������ʾ��Ϣ 
	static const char* const modelName[] = {"�Լ�������������", "�Կ�����������"};
	static const char* const easterEggName = "��ʦ������ˣ����";
    static const char* const tetrisName = "ITLJZSO";
    SetConsoleTextAttribute(g_hConsoleOutput, 0x0E);
    
    gotoxyWithFullwidth(1, 1);
    if (general.benchmark)
    {
    	printf("��Benchmark�ܷ�");
    }
    else
    {
    	if (easterEgg[0])
		{
    		SetConsoleTextAttribute(g_hConsoleOutput, 0x0C);
			printf("��%s", easterEggName);
    		SetConsoleTextAttribute(g_hConsoleOutput, 0x0E);
		}
    	printf("��%s", control->model ? modelName[0] : modelName[1]);
    }
    gotoxyWithFullwidth(1, 3);
    printf("����Esc�����˵�");
    
    gotoxyWithFullwidth(1, 6);
    printf("���÷֣�%u", control->score);
    gotoxyWithFullwidth(1, 8);
    printf("������������%u", control->erasedTotal);
    
    int8_t i;
    for (i = 0; i < 4; ++i)
    {
        gotoxyWithFullwidth(2, 10 + i);
        printf("����%1s%d��%u", "", i + 1, control->erasedCount[i]);
    }
    gotoxyWithFullwidth(1, 15);
    printf("������������%u", control->tetrisTotal);
    
    for (i = 0; i < 7; ++i)
    {
        gotoxyWithFullwidth(2, 17 + i);
        printf("��%1s%c�Σ�%u", "", tetrisName[i], control->tetrisCount[i]);
    }
    
    // ��һ������ı߿�
    SetConsoleTextAttribute(g_hConsoleOutput, 0xF);
    if (general.benchmark)
	{
		// Benchmark�ܷ�ģʽ����ʾ�������� 
    	gotoxyWithFullwidth(26, 1);
    	printf("������������");
    	gotoxyWithFullwidth(26, 2);
    	printf("��%8s��", "");
    	gotoxyWithFullwidth(26, 3);
    	printf("��%8s��", "");
    	gotoxyWithFullwidth(26, 4);
    	printf("��%8s��", "");
    	gotoxyWithFullwidth(26, 5);
    	printf("��%8s��", "");
    	gotoxyWithFullwidth(26, 6);
    	printf("������������");	
	}
	else
	{
    	gotoxyWithFullwidth(26, 1);
    	printf("�����������ש���������");
    	gotoxyWithFullwidth(26, 2);
    	printf("��%8s��%8s��", "", "");
    	gotoxyWithFullwidth(26, 3);
    	printf("��%8s��%8s��", "", "");
    	gotoxyWithFullwidth(26, 4);
    	printf("��%8s��%8s��", "", "");
    	gotoxyWithFullwidth(26, 5);
    	printf("��%8s��%8s��", "", "");
    	gotoxyWithFullwidth(26, 6);
    	printf("�����������ߩ���������");
    }
    
    SetConsoleTextAttribute(g_hConsoleOutput, 0xB);
    if (!control->model) // AI����ģʽ 
    {
    
    	if (general.benchmark)
		{ 
    		gotoxyWithFullwidth(26, 8);
    		printf("���ܷ�ģʽ��");
    		gotoxyWithFullwidth(27, 10);
    		printf("���˳���Esc");
    		
    		SetConsoleTextAttribute(g_hConsoleOutput, 0x0B);
    		gotoxyWithFullwidth(26, 20);
    		printf("����ǰ���֣�%6u", 100 * (unsigned)0.0);
    		gotoxyWithFullwidth(26, 21);
    		printf("���������֣�%6u", 100 * (unsigned)0.0);
    	}
    	else
    	{
    		gotoxyWithFullwidth(26, 8);
    		printf("���ۿ�ģʽ��");
    		gotoxyWithFullwidth(27, 10);
    		printf("�����٣��� W +");
    		gotoxyWithFullwidth(27, 11);
    		printf("�����٣��� S -");
    		gotoxyWithFullwidth(27, 12);
    		printf("����ͣ��Space Enter");
    		gotoxyWithFullwidth(27, 13);
    		printf("���˳���Esc");
    		
    		gotoxyWithFullwidth(26, 21);
    		SetConsoleTextAttribute(g_hConsoleOutput, 0x0B);
    		printf("��ʵʱ֡Ƶ��%6.2lf fps", 0.0);
    	}
    }
    else
    {
    	gotoxyWithFullwidth(26, 8);
    	printf("����Ϸģʽ��");
    	gotoxyWithFullwidth(27, 10);
    	printf("�������ƶ����� A 4");
    	gotoxyWithFullwidth(27, 11);
    	printf("�������ƶ����� D 6");
    	gotoxyWithFullwidth(27, 12);
    	printf("�������ƶ����� S 2");
    	gotoxyWithFullwidth(27, 13);
    	printf("��˳ʱ��ת���� W 8");
    	gotoxyWithFullwidth(27, 14);
    	printf("����ʱ��ת��   X 0");
    	gotoxyWithFullwidth(27, 15);
    	printf("��ֱ����أ�Space");
    	gotoxyWithFullwidth(27, 16);
    	printf("����ͣ��Ϸ��Enter");
    	gotoxyWithFullwidth(27, 17);
    	printf("��������Ϸ��Esc");
    }
    
    gotoxyWithFullwidth(25, 23);
    printf("��By: %s", INFO_AUTHOR);
}

// =============================================================================
// ������Ϸ
void runGame(TetrisManager *manager, TetrisControl *control)
{
    int ch;
    static double clockLast, clockNow;
    control->model = 0; 

    clockLast = getTime();  // ��ʱ
    printTetrisPool(manager, control);  // ��ʾ��Ϸ��

    while (!control->dead)  // û��
    {
        while (_kbhit())  // �м�����
        {
            ch = _getch();
            if (ch == 27)  // Esc��
            {
                return;
            }
            keydownControl(manager, control, ch);  // ������
        }

        if (!control->pause)  // δ��ͣ
        {
            clockNow = getTime();  // ��ʱ
            // ���μ�ʱ�ļ������0.45��
            if (clockNow - clockLast > control->frameRate)
            {
                clockLast = clockNow;
                moveDownTetris(manager, control);  // ����������
            }
        }
    }
}

/*******************************************************************************
����������˼���������磬�㷨������ΪGeForce GTX 982Ti����ԭ�� 

�Ľ��ķ�����ֻ���ǵ�ǰ���飩

��Сѧ���������˶�����
 
һ�������ŶԵ�ǰ���ӵ�ÿһ����ת�任�������ҵذڷţ��������аڷ���

������ÿһ�ְڷ��������ۡ����۰�������6��ָ�꣺

    1.����߶ȣ�Landing Height����
        ��ǰ��������ȥ֮�󣬷����е��ײ��ķ�����
        ��ʵ�ϣ������е�Ҳ�ǿ��Եģ������ַ

    2.��������Rows Eliminated��
        ���в����뵱ǰ���鹱�׳��ķ������˻�

    3.�б任��Row Transitions����
        �����ң����߷����������һ�У���������ĳ��������з��鵽�޷��飨���޷��鵽�з��飩��
        ��Ϊһ�α任����Ϸ�ر߽������з��顣�б任��һ���̶��Ϸ�ӳ��һ�е�ƽ���̶ȣ�Խƽ��ֵԽС
        ��ָ��Ϊ�����еı任��֮��
        ��ͼ������ʾ�з��飬����ʾ�ո���Ϸ�ر߽�δ������
        ������������������������ �任��Ϊ6
        ������������������������ �任��Ϊ8
        ������������������������ �任��Ϊ2
        ������������������������ �任��Ϊ0

    4.�б任��Column Transitions��������ͬ��
        �б任��һ���̶��Ϸ�ӳ��һ���пն��ļ��г̶ȣ��ն�Խ����ֵԽС

    5.�ն�����Number of Holes��
        ������

    6.�����ܺͣ�Well Sums����
        ��ָ���߽��з���Ŀ��С���ָ��Ϊ���о���������ӵ�1�����ܺ�
        ע��һ���п����ж��������ͼ��
        ������
        ������
        ������
        ������
        ������
        ������
        ������
        �м�һ��Ϊ����������ӵ�һ�ĺ�Ϊ (2+1)+(3+2+1)=9

    ����ָ��Ȩ�ؾ���ֵ��
    1	-4.500158825082766
    2	3.4181268101392694
    3	-3.2178882868487753
    4	-9.348695305445199
    5	-7.899265427351652
    6	-3.3855972247263626

�����Ƚ�ÿһ�ְڷ������֣�ȡ����ߡ���������ͬʱ���Ƚ����ȶ�
    ���㹫ʽ

        �������İڷ���100 * ˮƽƽ�Ƹ����� + 10 + ��ת����;

        �����Ҳ�İڷ���100 * ˮƽƽ�Ƹ����� + ��ת����;
*******************************************************************************/

typedef struct _AIPlacing  // �ڷ�
{
    uint16_t action;  // ��������8λ����ת��������8λ��Ŀ����
    uint16_t priority;  // ���ȶ�
    double value;  // ��ֵ
} AIPlacing;

void putDownTetris(TetrisManager *manager);  // �������䵽��
int calcLanding(TetrisManager *manager);  // ����߶Ⱥ�������
int calcTrans(const TetrisManager *manager);  // ���б任
int calcStatus(const TetrisManager *manager);  // �ն��;�
int evaluate(TetrisManager *manager);  // ��ֵ
uint16_t getBestPlacing(const TetrisManager *manager);  // ��ȡ��ðڷ�

// =============================================================================
// �������䵽��
void putDownTetris(TetrisManager *manager)
{
    removeTetris(manager);  // ���ߵ�ǰ����
    for (; manager->y < ROW_END; ++manager->y)  // ��������
    {
        if (checkCollision(manager))  // ��⵽��ײ
        {
            break;
        }
    }
    --manager->y;  // ����һ��Ȼû����ײ
    insertTetris(manager);  // ���뵱ǰ����
}

// =============================================================================
// ����߶Ⱥ�������
int calcLanding(TetrisManager *manager)
{
    int8_t x, y, k, count, cells;
    int8_t height = 25 - manager->y;  // ����߶�
    uint16_t tetris = gs_uTetrisTable[manager->type[0]][manager->orientation[0]];

    if ((tetris & 0xF000) == 0)  // ��ǰ��������һ��û�з���
    {
        --height;
    }
    if ((tetris & 0xFF00) == 0)  // ��ǰ����ڶ���û�з���
    {
        --height;
    }
    if ((tetris & 0xFFF0) == 0)  // ��ǰ���������û�з���
    {
        --height;
    }

    cells = 0;
    count = 0;
    k = 0;
    y = manager->y + 3;  // �������ϼ��
    do
    {
        if (y < ROW_END && manager->pool[y] == 0xFFFFU)  // ��Ч��������һ��������
        {
            for (x = 0; x < 4; ++x)  // ��⵱ǰ����Ķ�Ӧ��
            {
                if (((tetris >> (k << 2)) >> x) & 1)  // ��һ���з���
                {
                    ++cells;  // ���й��׵ķ�����
                }
            }
            ++count;
            // ����һ�з���
            memmove(manager->pool + 1, manager->pool, sizeof(uint16_t) * y);
        }
        else
        {
            --y;
            ++k;
        }
    } while (y >= manager->y && k < 4);

    height -= count;  // �ٽ�������߶�

    return count * cells * 34 - 45 * height;
}

// =============================================================================
// ���б任
int calcTrans(const TetrisManager *manager)
{
    int8_t x, y;
    int rowTrans = 0, colTrans = 0;
    int filled, test;

    // �б任
    for (y = ROW_BEGIN; y < ROW_END; ++y)  // ���μ�����
    {
        filled = 1;  // ��Ϸ�ر߽����������״̬
        for (x = COL_BEGIN; x < COL_END; ++x)
        {
            test = (manager->pool[y] >> x) & 1;  // ��������״̬
            if (filled != test)  // �任
            {
                ++rowTrans;
                filled = test;
            }
        }
        if (filled != 1)  // ��Ϸ�ر߽�
        {
            ++rowTrans;
        }
    }

    // �б任
    for (x = COL_BEGIN ; x < COL_END; ++x)  // ���μ�����
    {
        filled = 1;  // ��Ϸ�ر߽����������״̬
        for (y = ROW_BEGIN; y < ROW_END; ++y)
        {
            test = (manager->pool[y] >> x) & 1;  // ��������״̬
            if (filled != test)  // �任
            {
                ++colTrans;
                filled = test;
            }
        }
        if (filled != 1)  // ��Ϸ�ر߽�
        {
            ++colTrans;
        }
    }

    return 32 * rowTrans + 93 * colTrans;
}

// =============================================================================
// �ն��;�
int calcStatus(const TetrisManager *manager)
{
    static const int wellDepthTable[29] =
    {
        0, 1, 3, 6, 10, 15, 21, 28, 36, 45, 55, 66, 78,
        91, 105, 120, 136, 153, 171, 190, 210, 231, 253,
        276, 300, 325, 351, 378, 406
    };
    int8_t x, y;
    int holeCount = 0, wellDepthSum, depth;

    for (x = COL_BEGIN; x < COL_END; ++x)  // ��
    {
        for (y = ROW_BEGIN; y < ROW_END; ++y)  // ��
        {
            if ((manager->pool[y] >> x) & 1)  // �з���
            {
                break;
            }
        }
        while (y < 26)
        {
            if (!((manager->pool[y] >> x) & 1))  // �޷���
            {
                ++holeCount;  // ���ĸ���
            }
            ++y;
        }
    }

    wellDepthSum = 0;
    for (x = COL_BEGIN; x < COL_END; ++x)  // ��
    {
        depth = 0;
        for (y = ROW_END - 1; y >= ROW_BEGIN; --y)  // �У���������
        {
            if (!((manager->pool[y] >> x) & 1))  // �޷���
            {
                // �������߶��з���
                if (((manager->pool[y - 1] >> x) & 1) && ((manager->pool[y + 1] >> x) & 1))
                {
                    ++depth;
                }
            }
            else
            {
                wellDepthSum += wellDepthTable[depth];
                depth = 0;
            }
        }
        wellDepthSum += wellDepthTable[depth];
    }

    return 79 * holeCount + 34 * wellDepthSum;
}

// =============================================================================
// ��ֵ
inline int evaluate(TetrisManager *manager)
{
    putDownTetris(manager);  // �������䵽��

    return calcLanding(manager) - calcTrans(manager) - calcStatus(manager);
}

// =============================================================================
// ��ȡ��ðڷ�
uint16_t getBestPlacing(const TetrisManager *manager)
{
    int8_t i, j, count, type, ori, rotateLimit, deltaX;
    static AIPlacing placing[48];
    AIPlacing *best = NULL;
    static TetrisManager backup;

    memset(placing, 0, sizeof(placing));  // ��0���аڷ�

    type = manager->type[0];
    ori = manager->orientation[0];

    switch (type)  // ��ǰ��������
    {
    case TETRIS_I: case TETRIS_Z: case TETRIS_S:  // I�Ρ�Z�Ρ�S�Σ�������ת״̬
        rotateLimit = 2;
        break;
    case TETRIS_T: case TETRIS_L: case TETRIS_J:  // T�Ρ�L�Ρ�J�Σ�4����ת״̬
        rotateLimit = 4;
        break;
    case TETRIS_O:  // O�Σ�1����ת״̬
        rotateLimit = 1;
        break;
    default:
        rotateLimit = 0;
        break;
    }

    // ʵ��δ��������һЩ���ƽ������ն���ֻ����Ӷ���ֱ�����µ����аڷ�
    count = 0;
    for (i = 0; i < rotateLimit; ++i)  // ���Ը�����ת״̬
    {
        for (j = 0; j < 13; ++j)  // ����ÿһ��
        {
            memcpy(&backup, manager, sizeof(TetrisManager));  // ��Ϸ���ݱ���
            removeTetris(&backup);  // �Ƴ���ǰ����
            backup.orientation[0] = (i + ori) & 3;  // ������ת״̬
            backup.x = j;  // ����Ҫ�������

            // �����⵽��ײ��˵����������Ʋ�����һ��ȥ
            if (!checkCollision(&backup))  // δ��⵽��ײ���õ�һ�ְڷ�
            {
                placing[count].action = i;
                placing[count].action <<= 8;
                placing[count].action |= j;  // ��8λΪ��ת״̬����8λΪҪ���õ���
                placing[count].value = 1000000 + evaluate(&backup);  // ��ֵ
                deltaX = j - manager->x;  // ƽ�Ƶĸ����� ��Ϊ���ƣ���Ϊ����
                if (deltaX > 0)  // �����Ҳ�İڷ�
                {
                    placing[count].priority = 100 * deltaX + i;  // ���ȶ�
                }
                else  // �������İڷ�
                {
                    placing[count].priority = 100 * (-deltaX) + 10 + i;  // ���ȶ�
                }
                ++count;
            }
        }
    }

    // �㷨ֻ���ǵ�ǰ���飬���ݹ�
    best = placing;
    for (i = 1; i < count; ++i)
    {
        if (placing[i].value > best->value)  // ȡ��ֵ�����
        {
            best = placing + i;
        }
        else if (placing[i].value == best->value)  // ��ֵ��ͬ
        {
            if (placing[i].priority > best->priority)  // ȡ���ȶȸ���
            {
                best = placing + i;
            }
        }
    }

    return best->action;  // ���ذڷ�
}

// =============================================================================
// �Զ�����
void autoRun(TetrisManager *manager, TetrisControl *control)
{
    uint16_t best;
    int8_t i, rotate, destX, deltaX;
    signed long delayTime = 100;
    if (easterEgg[2])
	{
		delayTime = 0;
	}
	control->model = 1; 
    control->frameRate = delayTime; 
    int ch;

    printTetrisPool(manager, control);  // ��ʾ��Ϸ��
	
	// ��ʼ֮ǰ�Ե�һ����Կ��ø���� 
	Sleep(1000);
    while (!control->dead)  // û��
    {
        while (_kbhit())  // ������
        {
            ch = _getch();
            switch (ch)
            {
            case 27:  // Esc��
                return;
            case 32: case 13:  // �ո�ͻس���
                control->pause = !control->pause;  // �л���ͣ״̬
                gotoxyWithFullwidth(27, 12);
                SetConsoleTextAttribute(g_hConsoleOutput, 0x0B);
                if (control->pause)
                {
                	printf("��");
				}
				else
				{
					printf("��");
				}
                break;
            case 72: case 'W': case '+': // ��
                delayTime /= 2;
                if (delayTime < 0)
                {
                    delayTime = 0;
                }
                control->frameRate = delayTime;
                break;
            case 80: case 'S': case '-':  // ��
            	if (easterEgg[2])
            	{
            		break;
            	}
                delayTime = (delayTime + 1) * 2;
                if (delayTime > 300)
                {
                    delayTime = 300;
                }
                control->frameRate = delayTime;
                break;
            default:
				break; 
            }
        }

        if (control->pause)  // ��ͣ״̬
        {
            continue;
        }

        for (i = 0; i < 3; ++i)  // �����ķ������ƣ�����۲�
        {
            moveDownTetris(manager, control);
        }

        best = getBestPlacing(manager);  // ��ȡ��ðڷ�
        rotate = (best >> 8);  // ��8λ����ת����
        destX = (best & 0x0F);  // ��8λ��Ŀ����
        deltaX = destX - manager->x;  // �ƶ��ĸ��������Ϊ�������ƣ���������
		
		// ׼������ 
        for (i = 0; i < rotate; ++i)  // ��ת
        {
			Sleep(delayTime);
			calcFPS();  // Ϊ�˱�֤FPS׼ȷ�Բ����� 
			Sleep(delayTime);
            keydownControl(manager, control, 'w');
        }

		Sleep(delayTime);
		
        if (deltaX > 0)
        {
            for (i = 0; i < deltaX; ++i)  // ������
            {
                keydownControl(manager, control, 'd');
                Sleep(delayTime);
            }
        }
        else if (deltaX < 0)
        {
            for (i = 0; i < -deltaX; ++i)  // ������
            {
                keydownControl(manager, control, 'a');
                Sleep(delayTime);
            }
        }

        keydownControl(manager, control, ' ');  // �ƶ��ú�ֱ�����
        Sleep(delayTime);
    }
}

signed long benchmarkRun(TetrisManager *manager, TetrisControl *control)
{
    uint16_t best;
    int8_t i, rotate, destX, deltaX;
    int ch;
    signed long mark; 

	control->model = 1; 
    printTetrisPool(manager, control);  // ��ʾ��Ϸ��
	
    while (!control->dead)  // û��
    {
        mark = calcFPS();
        if (mark < 0)
        {
            return -mark;
        }
        while (_kbhit())  // ������
        {
            ch = _getch();
            switch (ch)
            {
            case 27: // Esc��
                return mark < 0 ? -mark : mark;
            }
        }

        for (i = 0; i < 3; ++i)  // �����ķ������ƣ�����۲�
        {
            moveDownTetris(manager, control);
        }

        best = getBestPlacing(manager);  // ��ȡ��ðڷ�
        rotate = (best >> 8);  // ��8λ����ת����
        destX = (best & 0x0F);  // ��8λ��Ŀ����
        deltaX = destX - manager->x;  // �ƶ��ĸ��������Ϊ�������ƣ���������
		
        for (i = 0; i < rotate; ++i)  // ��ת
        {
            keydownControl(manager, control, 'w');
        }

        if (deltaX > 0)
        {
            for (i = 0; i < deltaX; ++i)  // ������
            {
                keydownControl(manager, control, 'd');
            }
        }
        else if (deltaX < 0)
        {
            for (i = 0; i < -deltaX; ++i)  // ������
            {
                keydownControl(manager, control, 'a');
            }
        }

        keydownControl(manager, control, ' ');  // �ƶ��ú�ֱ�����
    }
}

inline double getTime(void)
{
	static LARGE_INTEGER freq, counter; 
    QueryPerformanceFrequency(&freq);   
    QueryPerformanceCounter(&counter);  
    return 1.0e3 * counter.QuadPart / freq.QuadPart;
}

inline signed long calcFPS(void)
{
	static double fraps; // �����ñ���
	static unsigned long framesElapsed = 0; // ���㷨ʹ�ñ��� 
	static unsigned long totalFramesElapsed = 0; // Benchmarkģʽʹ�ñ��� 
	static unsigned long mark, markMax = 0, markMaxLesser = 0; // Benchmark���֣�ע�ⷵ�صڶ���ֵ���� 
	static unsigned long markMaxDuration = 0; // ����50�μ���û�и���markMax�ͽ���Benchmarkģʽ 
	
	static double lastRecordedTime = 0.0; // ��Ϸ���ü�ʱ��
	static double lastDisplayTime = 0.0; // ����һ�����һ��
	
	if (general.resetFpsCounter){
		general.resetFpsCounter = false;
		framesElapsed = 0; 
		totalFramesElapsed = 0;
		markMax = 0;
		markMaxLesser = 0; 
		markMaxDuration = 0;
	}
	
    framesElapsed++;
    totalFramesElapsed++; 
    lastRecordedTime = getTime();
    
    // �Լ�����ģʽ�������Ҳ���� 
    if (general.model == 0)return 0;
    
    if (lastRecordedTime - (double)fpsRate > lastDisplayTime){
    	fraps = framesElapsed * 1000.0 / (lastRecordedTime - lastDisplayTime);
		mark = (unsigned long)(totalFramesElapsed * 100000.0 / (lastRecordedTime - general.startTime));
    	
    	SetConsoleTextAttribute(g_hConsoleOutput, 0x0B);
    	if (general.benchmark)
    	{
			gotoxyWithFullwidth(32, 20);
    		printf("%6u%4s", mark, "");
    		if (mark > markMax)
			{ 
				markMaxDuration = 0;
				markMaxLesser = markMax;
				markMax = mark; 
				gotoxyWithFullwidth(32, 21);
    			printf("%6u%4s", markMaxLesser, "");
    		}
    		else if (mark > markMaxLesser){
    			markMaxDuration++;
    			markMaxLesser = mark;
				gotoxyWithFullwidth(32, 21);
    			printf("%6u%4s", markMaxLesser, "");
    		}
    		else
    		{
    			markMaxDuration++;
    			if (markMaxDuration > 15)
    			{
    				return -(signed long)markMaxLesser; // ��ֹBenchmarkģʽ���ź� 
    			}
    		}
    	}
    	else if (fraps > 999.48)
    	{
    		SetConsoleTextAttribute(g_hConsoleOutput, 0x0C);
			gotoxyWithFullwidth(32, 21);
    		printf("Maxed Out!");
    	}
		else
		{
			gotoxyWithFullwidth(32, 21);
    		printf("%6.2lf fps", fraps);
    	}
    	lastDisplayTime = lastRecordedTime;
    	framesElapsed = 0;
    }
    return markMax;
}

inline void pause(void)
{
	while(_kbhit())_getch();
	while(!_kbhit());
	while(_kbhit())_getch();
}

inline void clrscr(void){
	static DWORD clsCount; 
	static COORD startPos = {0, 0};
	static CONSOLE_SCREEN_BUFFER_INFO consoleScreenBufferInfo;
	static DWORD consoleScreenBufferSize;
	
	// ������Ļ��������С 
	GetConsoleScreenBufferInfo(g_hConsoleOutput,&consoleScreenBufferInfo);
	consoleScreenBufferSize = consoleScreenBufferInfo.dwSize.X * consoleScreenBufferInfo.dwSize.Y;
	
	// ���� 
	FillConsoleOutputCharacter(g_hConsoleOutput,0x20,consoleScreenBufferSize,startPos,&clsCount);
	FillConsoleOutputAttribute(g_hConsoleOutput,0x0F,consoleScreenBufferSize,startPos,&clsCount);
	SetConsoleCursorPosition(g_hConsoleOutput,startPos);
}

