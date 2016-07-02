#include <graphics.h>
#include <iostream>
#include <time.h>
#include<stdlib.h>
#include<stdio.h>
#include <conio.h>
#include<algorithm>
#include<math.h>
#include <map>

using namespace std;

#define WINDOWS_X 1200	//窗口大小X
#define WINDOWS_Y 800	//窗口大小Y

#define PX 280			//棋盘偏移量X
#define PY 80			//棋盘偏移量Y
#define BBLACK 80		//空格大小

#define CHESSSIZE 25	//棋子大小
#define SIZE 8			//棋盘格数

#define ESC 27
#define ESCEXIT (_kbhit()&&_getch()==ESC)

#pragma comment (lib,"ws2_32.lib")						// 引用 Windows Multimedia API
#pragma comment(lib,"Winmm.lib")

const int black = 1;	//黑棋
const int white = 0;	//白棋

int mapp[SIZE][SIZE];		//棋盘矩阵存储
const int MOVE[8][2] = { { -1, 0 },{ 1, 0 },{ 0, -1 },{ 0, 1 },{ -1, -1 },{ 1, -1 },{ 1, 1 },{ -1, 1 } };	//方位
const int MAPPOINTCOUNT[8][8] =												//棋盘各点权值估值
{
	{90,-60,10,10,10,10,-60,90},
	{-60,-80,5,5,5,5,-80,-60},
	{10,5,1,1,1,1,5,10},
	{10,5,1,1,1,1,5,10},
	{10,5,1,1,1,1,5,10},
	{10,5,1,1,1,1,5,10},
	{-60,-80,5,5,5,5,-80,-60},
	{90,-60,10,10,10,10,-60,90}
};
IMAGE MAPIMAGE[SIZE][SIZE];
IMAGE COUNT[2];

int expect[SIZE][SIZE];

SOCKET sockSer;
SOCKET sockConn;

bool TOINTERNET;						//是否有网络数据
int MYCOLOR;							//我的颜色 1黑色 0白色 -1其他
int NOWCOLOR;							//当前执子颜色
bool TIANEYES;							//天眼模式

void gameStart();						//函数声明
class POINT2
{
public:
	void WIN2MAP(POINT2 &MAP)			//建立棋盘与矩阵的映射关系
	{
		MAP.x = (x - PX) / BBLACK;
		MAP.y = (y - PY) / BBLACK;
	}
	void MAP2WIN(POINT2 &WIN)			//建立矩阵与棋盘的映射关系
	{
		WIN.x = PX + x*BBLACK + BBLACK / 2;
		WIN.y = PY + y*BBLACK + BBLACK / 2;
	}
	void INIT(int x, int y)				//插入元素
	{
		this->x = x;
		this->y = y;
	}
	void ADD(int x, int y)				//计算和
	{
		this->x += x;
		this->y += y;
	}
	int MAP2WINX()
	{
		return PX + x*BBLACK + BBLACK / 2;
	}
	int MAP2WINY()
	{
		return PY + y*BBLACK + BBLACK / 2;
	}
	int WIN2MAPX()
	{
		return (x - PX) / BBLACK;
	}
	int WIN2MAPY()
	{
		return (y - PY) / BBLACK;
	}
	int x, y;
};

POINT2 LASTCH;								//上一步棋子方位

void mappadd(int x, int y, int color,int MAP[SIZE][SIZE])		//向地图中添加棋子
{
	POINT2 WINDOWS2, MAP2;
	WINDOWS2.INIT(x, y);
	WINDOWS2.WIN2MAP(MAP2);
	MAP[MAP2.x][MAP2.y] = color ? 1 : -1;
}

void printcircle(int x, int y, int color,int MAP[SIZE][SIZE])				//绘制棋子
{
	mappadd(x, y, color,MAP);
																			//注释中为特效二
	/*POINT2 X;
	X.INIT(x, y);
	X.WIN2MAP(X);
	POINT2 ZS, YS, ZX, YX, ZZS, ZZX, ZYS, ZYX;
	ZS.INIT(x - BBLACK / 2+1, y - BBLACK / 2+1);
	ZX.INIT(x - BBLACK / 2+1, y + BBLACK / 2 - 1);
	YS.INIT(x + BBLACK / 2-1, y - BBLACK / 2+1);
	YX.INIT(x + BBLACK / 2 - 1, y + BBLACK / 2 - 1);
	ZZS.INIT(x, y - BBLACK / 2+1);
	ZZX.INIT(x, y + BBLACK / 2 - 1);
	ZYS = ZZS, ZYX = ZZX;

	for (int i = 0; i < BBLACK; i++)
	{
	POINT points1[4] = { { ZS.x,ZS.y },{ ZZS.x,ZZS.y },{ ZZX.x,ZZX.y },{ ZX.x,ZX.y }};			//定义点
	POINT points2[4] = { { YS.x,YS.y },{ ZYS.x,ZYS.y },{ ZYX.x,ZYX.y },{ YX.x,YX.y }};			//定一点
	setfillcolor(LIGHTRED);																		//翻转背景颜色
	putimage(ZS.x - 1, ZS.y - 1, &MAPIMAGE[X.x][X.y]);											//使用原图覆盖
	solidpolygon(points1, 4);
	solidpolygon(points2, 4);
	if (ZZS.x <= x - BBLACK / 2 + 1)break;
	ZZS.x-=3;
	++ZZS.y;
	++ZYS.y;
	ZYS.x += 3;
	--ZYX.y;
	ZYX.x += 3;
	ZZX.x-=3;
	--ZZX.y;

	Sleep(2);
	}*/
	setfillcolor(color ? BLACK : WHITE);					//从中间放大变换
	for (int i = 0; i <= CHESSSIZE; ++i)
	{
		solidcircle(x, y, i);
		Sleep(1);
	}
}

void init()															//绘制棋盘
{
	memset(mapp, 0, sizeof(mapp));									//初始化
	memset(expect, 0, sizeof(expect));
	TIANEYES = false;												//关闭天眼模式
	MYCOLOR = -1;
	LASTCH.INIT(0, 0);

	settextstyle(15, 0, "宋体");
	loadimage(NULL, "1.jpg");										//背景图片
	for (int x = PX; x < PX + BBLACK*(SIZE + 1); x += BBLACK)		//绘制棋盘 横线
	{
		if ((x / BBLACK % 2) == 0)setlinecolor(BLACK);
		else setlinecolor(RED);
		line(x, PY, x, PY + BBLACK*SIZE);
	}
	for (int y = PY; y <PY + BBLACK*(SIZE + 1); y += BBLACK)		//绘制棋盘 竖线
	{
		if (y / BBLACK % 2 == 0)setlinecolor(BLACK);
		else setlinecolor(RED);
		line(PX, y, PX + BBLACK*SIZE, y);
	}
	for (int i = PX; i <= PX + BBLACK*SIZE; i += BBLACK)			//获取每部分图片保存在图片指针中
	{
		for (int j = PY; j <= PY + BBLACK*SIZE; j += BBLACK)
		{
			POINT2 DATA;
			DATA.INIT(i, j);
			DATA.WIN2MAP(DATA);
			getimage(&MAPIMAGE[DATA.x][DATA.y], i, j, BBLACK, BBLACK);
		}
	}

	printcircle(PX + (SIZE / 2 - 1)* BBLACK + BBLACK / 2, PY + (SIZE / 2 - 1) * BBLACK + BBLACK / 2, white,mapp);		//初始的四枚棋子
	printcircle(PX + (SIZE / 2 - 1) * BBLACK + BBLACK / 2, PY + (SIZE / 2) * BBLACK + BBLACK / 2, black, mapp);
	printcircle(PX + (SIZE / 2) * BBLACK + BBLACK / 2, PY + (SIZE / 2) * BBLACK + BBLACK / 2, white, mapp);
	printcircle(PX + (SIZE / 2) * BBLACK + BBLACK / 2, PY + (SIZE / 2 - 1) * BBLACK + BBLACK / 2, black, mapp);


	getimage(COUNT, WINDOWS_X / 30, WINDOWS_Y - WINDOWS_Y / 6, 230, 40);											//成绩所在区域背景图片指针
	getimage(COUNT + 1, WINDOWS_X - WINDOWS_X / 5, WINDOWS_Y - WINDOWS_Y / 6, 230, 40);

	LOGFONT f;
	gettextstyle(&f);												// 获取字体样式
	f.lfHeight = 35;												// 设置字体高度
	strcpy_s(f.lfFaceName, _T("方正姚体"));							// 设置字体
	f.lfQuality = ANTIALIASED_QUALITY;								// 设置输出效果为抗锯齿  
	settextstyle(&f);												// 设置字体样式

	settextcolor(WHITE);
	outtextxy(BBLACK / 4, BBLACK / 2, "你所执子");
	outtextxy(BBLACK / 4, BBLACK / 4 + BBLACK, "当前执子");
}

int Judge(int x, int y, int color,int MAP[SIZE][SIZE])									//预判当前位置能否下子
{
	if (MAP[x][y])return 0;																//如果当前位置已经有棋子
	int me = color ? 1 : -1;															//准备落棋棋子颜色
	POINT2 star;
	int count = 0, flag;																//count为该位置可以转换对手棋子个数
	for (int i = 0; i < SIZE; ++i)														//搜索
	{
		flag = 0;
		star.INIT(x + MOVE[i][0], y + MOVE[i][1]);
		while (star.x >= 0 && star.x < SIZE&&star.y >= 0 && star.y < SIZE&&MAP[star.x][star.y])
		{
			if (MAP[star.x][star.y] == -me)flag++;
			else
			{
				count += flag;
				break;
			}
			star.ADD(MOVE[i][0], MOVE[i][1]);
		}
	}
	return count;																		//返回该点转换对方棋子个数
}

void Change(POINT2 NOW,int MAP[SIZE][SIZE],bool F)												//落子后改变棋盘状态 F为是否输出到屏幕
{
	int me = MAP[NOW.x][NOW.y];																	//当前落子棋子颜色
	bool flag;
	POINT2 a, b;
	for (int i = 0; i<SIZE; ++i)																//搜索
	{
		flag = false;
		a.INIT(NOW.x + MOVE[i][0], NOW.y + MOVE[i][1]);
		while (a.x >= 0 && a.x<SIZE&&a.y >= 0 && a.y<SIZE&&MAP[a.x][a.y])
		{
			if (MAP[a.x][a.y] == -me)flag = true;
			else
			{
				if (flag)
				{
					a.ADD(-MOVE[i][0], -MOVE[i][1]);
					b.INIT(NOW.x + MOVE[i][0], NOW.y + MOVE[i][1]);
					while (((NOW.x <= b.x && b.x <= a.x) || (a.x <= b.x && b.x <= NOW.x)) && ((NOW.y <= b.y && b.y <= a.y) || (a.y <= b.y && b.y <= NOW.y)))
					{
						if(F)printcircle(b.MAP2WINX(), b.MAP2WINY(), (me == 1) ? black : white,MAP);	//改变棋子
						if (!F)mappadd(b.MAP2WINX(), b.MAP2WINY(), (me == 1) ? black : white, MAP);		//如果不输出到屏幕，改变地图数组
						b.ADD(MOVE[i][0], MOVE[i][1]);
					}
				}
				break;
			}
			a.ADD(MOVE[i][0], MOVE[i][1]);
		}
	}
}

int Statistics(int color)														//预判每个位置可以转化对手棋子个数
{
	int NOWEXPECT = 0;															//总的转化棋子个数
	for (int i = 0; i < SIZE; ++i)												//遍历
		for (int j = 0; j < SIZE; ++j)
		{
			expect[i][j] = Judge(i, j, color,mapp);									//存储该位置可以转化棋子个数
			if (expect[i][j])
			{
				++NOWEXPECT;

				POINT2 a;
				a.INIT(i, j);
				setfillcolor(RGB(96, 96, 96));
				circle(a.MAP2WINX(), a.MAP2WINY(), CHESSSIZE / 4);
				circle(a.MAP2WINX(), a.MAP2WINY(), CHESSSIZE / 4 - 1);

				if (TIANEYES)														//如果开启天眼模式
				{
					settextstyle(15, 0, "宋体");
					TCHAR s[20];													//天眼模式
					_stprintf_s(s, _T("%d"), expect[i][j]);
					outtextxy(a.MAP2WINX(), a.MAP2WINY() + 10, s);
				}
			}
		}
	return NOWEXPECT;
}

void CleanStatistics()										//清除期望点提示
{
	for (int i = 0; i < SIZE; ++i)
	{
		for (int j = 0; j < SIZE; ++j)
		{
			if (expect[i][j] && !mapp[i][j])				//如果当前点没有棋子或者有期望
			{
				POINT2 a;
				a.INIT(i, j);								//记录坐标
				putimage(a.MAP2WINX() - BBLACK / 2, a.MAP2WINY() - BBLACK / 2, &MAPIMAGE[i][j]);	//输出局部背景
			}
		}
	}
}

string INTTOCHI(int num, int color)											//当前局势成绩输出汉字
{
	string number[10] = { "","一","二","三","四","五","六","七","八","九" };
	string data = "";
	if (num >= 10)
	{
		data += number[num / 10 % 10];
		data += "十";
	}
	data += number[num % 10];
	return (color ? "黑棋：" : "白棋：") + data;							//num>=0&&num<=64
}

void Printcount(int balckcount, int whitecount, int nowcolor)		//输出当前分数
{
	settextcolor(DARKGRAY);											//更改字体颜色
	settextstyle(35, 0, "方正姚体");

	putimage(WINDOWS_X / 30, WINDOWS_Y - WINDOWS_Y / 6, COUNT);		//擦出原来痕迹
	putimage(WINDOWS_X - WINDOWS_X / 5, WINDOWS_Y - WINDOWS_Y / 6, COUNT + 1);

	outtextxy(WINDOWS_X / 30, WINDOWS_Y - WINDOWS_Y / 6, INTTOCHI(whitecount, white).data());	//输出当前成绩
	outtextxy(WINDOWS_X - WINDOWS_X / 5, WINDOWS_Y - WINDOWS_Y / 6, INTTOCHI(balckcount, black).data());

	setfillcolor(MYCOLOR == 1 ? BLACK : MYCOLOR == 0 ? WHITE : LIGHTCYAN);						//从中间放大变换
	solidcircle(BBLACK * 2 + 10, BBLACK * 3 / 4, CHESSSIZE * 3 / 4);
	setfillcolor((!nowcolor || balckcount + whitecount == 4) ? BLACK : WHITE);
	NOWCOLOR = (!nowcolor || balckcount + whitecount == 4) ? black : white;						//记录当前执子
	for (int i = 0; i <= CHESSSIZE * 3 / 4; ++i)
	{
		solidcircle(BBLACK * 2 + 10, BBLACK * 3 / 2, i);
		Sleep(1);
	}
}

void WIN(int YOURCOLOR, int balckcount, int whitecount)			//判断输赢
{
	HWND wnd = GetHWnd();										//获取窗口句柄
	if (balckcount>whitecount)
	{
		MessageBox(wnd, YOURCOLOR == black ? "恭喜你，胜利啦~" : YOURCOLOR == white ? "输了哎~，不过别灰心，下次一定可以赢的！" : "黑方得胜~", "Result", MB_OK);
	}
	else if (balckcount<whitecount)
	{
		MessageBox(wnd, YOURCOLOR == white ? "恭喜你，胜利啦~" : YOURCOLOR == black ? "输了哎~，不过别灰心，下次一定可以赢的！" : "白方得胜", "Result", MB_OK);
	}
	else
	{
		MessageBox(wnd, "噫~平局哎，要不要再来一次呢！", "Result", MB_OK);
	}
}

void HL(int NOWWJ)
{
	if (NOWWJ != -1)
	{
		HWND wnd = GetHWnd();										//获取窗口句柄
		MessageBox(wnd, NOWWJ == MYCOLOR ? "你没有可以下的子！" : "对方已无子可下！", "回合跳过", MB_OK);
	}
}

POINT2 Easy()										//人机对战简单AI
{
	POINT2 MAX;										//定义以及初始化最优解
	MAX.INIT(0, 0);
	int maxx = 0;
	for (int i = 0; i < SIZE; ++i)
		for (int j = 0; j < SIZE; ++j)
		{
			if (expect[i][j] >= maxx)				//寻找可以转化棋子最多的点作为最优解
			{
				maxx = expect[i][j];
				MAX.INIT(i, j);
			}
		}
	if (ESCEXIT)gameStart();
	Sleep(800);										//间歇
	return MAX;										//返回最优解
}

void copymap(int one[SIZE][SIZE], int last[SIZE][SIZE])						//拷贝地图
{
	for (int i = 0; i < SIZE; i++)
		for (int j = 0; j < SIZE; j++)
			one[i][j] = last[i][j];
}

POINT2 Middle()										//人机对战中等AI
{
	int ME = 0;										//AI权值
	int maxx = 0;

	struct _ADD
	{
		int x;										//X坐标
		int y;										//Y坐标
		int w;										//权值
		void init(int x, int y, int w)
		{
			this->x = x;
			this->y = y;
			this->w = w;
		}
		bool operator < (_ADD a)					//重载比较运算符
		{
			return w>a.w;
		}
		POINT2 INTOPOINT2()							//转换为POINT2类型
		{
			POINT2 data;
			data.INIT(x, y);
			return data;
		}
	}WEA[SIZE*SIZE];

	int expectnow[SIZE][SIZE],mapnow[SIZE][SIZE];	
	if (ESCEXIT)gameStart();												//按ESC退出
	Sleep(800);																//间歇0.8S
	
	for (int i = 0; i < SIZE; ++i)
		for (int j = 0; j < SIZE; ++j)
		{
			if (expect[i][j])												//如果当前点可以走棋
			{
				ME = MAPPOINTCOUNT[i][j]+expect[i][j];						//计算本方在该点权值
				copymap(mapnow, mapp);
				mapnow[i][j] = NOWCOLOR ? 1 : -1;							//模拟走棋
				POINT2 NOWPOINT;
				NOWPOINT.INIT(i, j);
				if ((i == 0 && j == 0 )||( i == 0 && j == SIZE - 1) ||( i == SIZE - 1 && j == SIZE - 1) ||( i == SIZE - 1 && j == 0))
				{
					return NOWPOINT;										//如果在角，返回角坐标
				}

				Change(NOWPOINT, mapnow,false);								//模拟走棋后虚拟改变地图
				int YOU = -1050;											//探知对手行动力与局势
				for (int k = 0; k < SIZE; ++k)
					for (int l = 0; l < SIZE; ++l)
					{
						expectnow[k][l] = Judge(k, l, !NOWCOLOR, mapnow);	//判断对手期望
						if (expectnow[k][l])
						{
							YOU = YOU < MAPPOINTCOUNT[k][l] + expectnow[k][l] ? MAPPOINTCOUNT[k][l] + expectnow[k][l] : YOU;
						}
					}
				WEA[maxx++].init(i, j, ME - YOU);							//入结构体数组
			}
		}
	sort(WEA, WEA + maxx);													//按照权值排序
	for (int i = 0; i < maxx; ++i)
	{
		if ((WEA[i].x < 2 && WEA[i].y < 2) || (WEA[i].x < 2 && SIZE - WEA[i].y - 1 < 2) || (SIZE - 1 - WEA[i].x < 2 && WEA[i].y < 2) || (SIZE - 1 - WEA[i].x < 2 && SIZE - 1 - WEA[i].y < 2))continue;
		return WEA[i].INTOPOINT2();											//返回非角边最优解
	}
	return WEA[0].INTOPOINT2();												//返回角边最优解
}


int difai(int x,int y,int mapnow[SIZE][SIZE],int expectnow[SIZE][SIZE],int depin,int depinmax)						//极大极小搜索
{
	if (depin >= depinmax)return 0;											//递归出口

	int maxx = -10005;														//最大权值
	POINT2 NOW;
	int expectnow2[SIZE][SIZE] , mapnow2[SIZE][SIZE],mapnext[SIZE][SIZE],expectlast[SIZE][SIZE];					//定义临时数组

	copymap(mapnow2, mapnow);												//复制当前棋盘

	mapnow2[x][y] = NOWCOLOR ? 1 : -1;										//模拟在当前棋盘上下棋
	int ME = MAPPOINTCOUNT[x][y] + expectnow[x][y];							//当前棋子权
	NOW.INIT(x,y);

	Change(NOW, mapnow2, false);											//改变棋盘AI结束

	int MAXEXPECT = 0, LINEEXPECT = 0, COUNT = 0;
	for (int i = 0; i < SIZE; ++i)
		for (int j = 0; j < SIZE; ++j)
		{
			expectnow2[i][j] = Judge(i, j, !NOWCOLOR, mapnow2);				//预判对方是否可以走棋
			if (expectnow2[i][j])
			{
				++MAXEXPECT;
				if ((i == 0 && j == 0) || (i == 0 && j == SIZE - 1) || (i == SIZE - 1 && j == SIZE - 1) || (i == SIZE - 1 && j == 0))return -1800;	//如果对方有占角的可能
				if ((i < 2 && j < 2) || (i < 2 && SIZE - j - 1 < 2) || (SIZE - 1 - i < 2 && j < 2) || (SIZE - 1 - i < 2 && SIZE - 1 - j < 2))++LINEEXPECT;
			}
		}
	if (LINEEXPECT * 10 > MAXEXPECT * 7)return 1400;						//如果对方走到坏点状态较多 剪枝

	for (int i = 0; i < SIZE; i++)
		for (int j = 0; j < SIZE; j++)
			if (expectnow2[i][j])											//如果对方可以走棋
			{
				int YOU = MAPPOINTCOUNT[i][j] + expectnow2[i][j];			//当前权值
				copymap(mapnext, mapnow2);									//拷贝地图
				mapnext[i][j] = (!NOWCOLOR) ? 1 : -1;						//模拟对方走棋
				NOW.INIT(i, j);
				Change(NOW, mapnext, false);								//改变棋盘

				for (int k = 0; k < SIZE; k++)
					for (int l = 0; l < SIZE; l++)
						expectlast[k][l] = Judge(k, l, NOWCOLOR, mapnext);	//寻找AI可行点

				for (int k = 0; k < SIZE; k++)
					for (int l = 0; l < SIZE;l++)
						if (expectlast[k][l])
						{
							int nowm = ME - YOU + difai(k, l, mapnext, expectlast, depin + 1, depinmax);
							maxx = maxx < nowm ? nowm : maxx;
						}
			}
	return maxx;
}

/*
POINT2 MIDDLE()									//人机对战中等AI
{
	POINT2 MAX;
	int maxx = -10005;
	MAX.INIT(0, 0);
	for (int i = 0; i < SIZE; i++)
		for (int j = 0; j < SIZE; j++)
		{
			if (expect[i][j])
			{
				if ((i == 0 && j == 0) || (i == 0 && j == SIZE - 1) || (i == SIZE - 1 && j == SIZE - 1) || (i == SIZE - 1 && j == 0))
				{
					MAX.INIT(i, j);
					return MAX;										//如果在角，返回角坐标
				}
				int k = difai(i, j, mapp, expect, 0, 1);					//递归搜索 搜索三层
				if (k >= maxx)
				{
					maxx = k;
					MAX.INIT(i, j);
				}
			}
		}
	return MAX;
}
*/

POINT2 Difficult()									//人机对战困难AI
{
	POINT2 MAX;
	int maxx = -10005;
	MAX.INIT(0, 0);
	for (int i = 0; i < SIZE; i++)
		for (int j = 0; j < SIZE; j++)
		{
			if (expect[i][j])
			{
				if ((i == 0 && j == 0) || (i == 0 && j == SIZE - 1) || (i == SIZE - 1 && j == SIZE - 1) || (i == SIZE - 1 && j == 0))
				{
					MAX.INIT(i, j);
					return MAX;										//如果在角，返回角坐标
				}
				int k = difai(i,j,mapp,expect,0,3);					//递归搜索 搜索三层
				if (k >= maxx)
				{
					maxx = k;
					MAX.INIT(i, j);
				}
			}
		}
	return MAX;
}

POINT2 MOUSE()										//鼠标事件
{
	MOUSEMSG m;
	while (true)
	{
		m = GetMouseMsg();							//获取鼠标信息
		switch (m.uMsg)
		{
		case(WM_LBUTTONDOWN) :						//当鼠标左键按下时
		{
			POINT2 NOWMOUSE;
			NOWMOUSE.INIT(m.x, m.y);
			if (TOINTERNET)							//如果处于联网对战状态 发送当前数据
			{
				char Toyou[50] = { 0 };
				sprintf_s(Toyou, "%d,%d", m.x, m.y);
				send(sockConn, Toyou, sizeof(Toyou) , 0);		//发送数据
			}
			return NOWMOUSE;						//返回鼠标坐标
			break;
		}
		case(WM_MOUSEMOVE) :								//调试 输出鼠标位置
		{
			if (ESCEXIT)gameStart();
			break;
		}
		case(WM_RBUTTONDOWN) :								//如果鼠标右键点下时
		{
			TIANEYES = !TIANEYES;							//开启OR关闭天眼模式
			break;
		}
		}
	}
}

bool putmouse(POINT2 &m)									//重定向鼠标坐标
{
	bool flag = true;
	int mouseinx[SIZE + 1], mouseiny[SIZE + 1];
	for (int i = 0; i < SIZE + 1; ++i)						//精确坐标打表
	{
		mouseinx[i] = PX + i*BBLACK;
		mouseiny[i] = PY + i*BBLACK;
	}
	if (m.x < PX || m.x>PX + SIZE*BBLACK || m.y < PY || m.y>PY + SIZE*BBLACK)flag = false;	//如果点击在棋盘外
	else
	{
		for (int i = 0; i<SIZE; ++i)
		{
			if (m.x >= mouseinx[i] && m.x <= mouseinx[i + 1])
			{
				if (m.x - mouseinx[i]>BBLACK / 8 && mouseinx[i + 1] - m.x>BBLACK / 8)		//重定向X
				{
					m.x = (mouseinx[i] + mouseinx[i + 1]) / 2;
				}
				else flag = false;
			}
		}
		for (int i = 0; i<SIZE; ++i)
		{
			if (m.y >= mouseiny[i] && m.y <= mouseiny[i + 1])
			{
				if (m.y - mouseiny[i]>BBLACK / 8 && mouseiny[i + 1] - m.y > BBLACK / 8)		//重定向Y
				{
					m.y = (mouseiny[i] + mouseiny[i + 1]) / 2;
				}
				else flag = false;
			}
		}
	}
	return flag;											//返回当前位置能否落子
}

void CleanLast(POINT2 WINDOWS2, int F)						//记录上一步走棋位置
{
	if (LASTCH.x > SIZE&&LASTCH.y > SIZE)					//以下取消上一步填充
	{
		setfillcolor(getpixel(LASTCH.x, LASTCH.y));			//获取原来棋子颜色
		putimage(LASTCH.x - BBLACK / 2, LASTCH.y - BBLACK / 2, &MAPIMAGE[LASTCH.WIN2MAPX()][LASTCH.WIN2MAPY()]);		
		solidcircle(LASTCH.x, LASTCH.y, CHESSSIZE);
	}

	setfillcolor(RGB(49, 153, 182));						//以下为填充当前走棋
	LASTCH.INIT(WINDOWS2.x, WINDOWS2.y);

	solidrectangle(WINDOWS2.x - BBLACK / 2 + 2, WINDOWS2.y - BBLACK / 2 + 2, WINDOWS2.x + BBLACK / 2 - 2, WINDOWS2.y + BBLACK / 2 - 2);		//背景矩形
	setfillcolor(F ? BLACK : WHITE);
	solidcircle(WINDOWS2.x, WINDOWS2.y, CHESSSIZE);			//画棋子
}

int Playchess(int F, POINT2 WINDOWS2, int &balckcount, int &whitecount)	//开始
{
	//F 黑方为1  白方为0
	POINT2 MAP2;

	if (WINDOWS2.x < SIZE&&WINDOWS2.y < SIZE)					//如果传入的坐标为矩阵坐标
	{
		MAP2 = WINDOWS2;
		WINDOWS2.MAP2WIN(WINDOWS2);								//将其转换成实际展示坐标
	}
	else
	{
		if (!putmouse(WINDOWS2))return 0;						//鼠标输入坐标重定向
		WINDOWS2.WIN2MAP(MAP2);									//存储重定向之后的矩阵坐标
	}
	if (expect[MAP2.x][MAP2.y])									//有位置可行
	{
		CleanStatistics();										//清除屏幕提示
		if (F)													//判断如果为黑棋得分
		{
			balckcount += expect[MAP2.x][MAP2.y] + 1;
			whitecount -= expect[MAP2.x][MAP2.y];
		}
		else
		{
			whitecount += expect[MAP2.x][MAP2.y] + 1;
			balckcount -= expect[MAP2.x][MAP2.y];
		}

		printcircle(WINDOWS2.x, WINDOWS2.y, F,mapp);			//画棋子 mapp为输入数组
		CleanLast(WINDOWS2, F);									//当前走棋棋子提示
		Change(MAP2,mapp,true);									//翻转棋子 true为显示在屏幕
		Printcount(balckcount, whitecount, F);	//打印分数

		if (balckcount + whitecount >= SIZE*SIZE || !balckcount || !whitecount)return 3;	//如果胜负已分
		if (!Statistics(!F))									//如果对方无法走棋
		{
			if (Statistics(F))									//判断自己是否可以走棋
			{
				HL(!F);											//自己可走棋输出对方无法走棋信息
				return 2;
			}
			else return 3;										//双方都无法走棋
		}
		return 1;
	}
	return 0;
}

void STARTVS(int YOURCOLOR, POINT2 P1(), POINT2 P2())			//开始对战
{
	int balckcount = 2, whitecount = 2;							//初始化活着的棋子

	MYCOLOR = YOURCOLOR;

	Printcount(balckcount, whitecount, black);					//当前形势
	Statistics(black);
	while (true)
	{
	CX1:														//本回合忽略
		int PD = Playchess(black, (*P1)(), balckcount, whitecount);
		switch (PD)
		{
		case 0:
			goto CX1;											//输入失误或无输入
			break;
		case 1:
			break;												//正常结束
		case 2:
			goto CX1;											//忽略对方
			break;
		case 3:
			goto ED;											//棋局结束
			break;
		}
	CX2:															//本回合忽略
		PD = Playchess(!black, (*P2)(), balckcount, whitecount);
		switch (PD)
		{
		case 0:
			goto CX2;
			break;
		case 1:
			break;
		case 2:
			goto CX2;
			break;
		case 3:
			goto ED;
			break;
		}
	}
ED:																	//结束
	WIN(YOURCOLOR, balckcount, whitecount);
	_getch();
	if (TOINTERNET)
	{
		closesocket(sockSer);
		closesocket(sockConn);
	}
	gameStart();
}

char *ip;												//定义IP地址变量 
void Get_ip()											//获取本机IP地址
{
	WSADATA wsaData;
	char name[255];										//定义用于存放获得的主机名的变量
	PHOSTENT hostinfo;									//获得Winsock版本的正确值
	if (WSAStartup(MAKEWORD(2, 0), &wsaData) == 0)		//在是加载Winsock库，如果WSAStartup（）函数返回值为0，说明加载成功，程序可以继续
	{
		if (gethostname(name, sizeof(name)) == 0)		//成功地将本地主机名存放入由name参数指定的缓冲区中
		{
			if ((hostinfo = gethostbyname(name)) != NULL) //这是获取主机名
			{
				settextstyle(BBLACK / 2, 0, "方正姚体");
				outtextxy(WINDOWS_X / 2 - 5 * BBLACK / 2, BBLACK * 2, "墨攻棋阵 服务器已创建");
				settextstyle(BBLACK / 4, 0, "楷体");
				ip = inet_ntoa(*(struct in_addr *)*hostinfo->h_addr_list);   //调用inet_ntoa（）函数，将hostinfo结构变量中的h_addr_list转化为标准的点分表示的IP
				char c[250] = { "1.告诉你的同学这个ip地址就可以联机啦~" };
				strcat_s(c, ip);
				outtextxy(WINDOWS_X / 2 - 3 * BBLACK, BBLACK * 7 / 2, c);
				outtextxy(WINDOWS_X / 2 - 3 * BBLACK, BBLACK * 4, "2.不过要先确保你们在同一个局域网下哦！");
			}
		}
		WSACleanup();										//卸载Winsock库，并释放所有资源
	}
}

POINT2 OURCLASS()									//解析对方发送的数据
{
	POINT2 YOU;
	char data[50] = { 0 };
	int x = 0, y = 0;
	int p = recv(sockConn, data, 50, 0);
	if (p == SOCKET_ERROR)
	{
		HWND wnd = GetHWnd();
		MessageBox(wnd, "对方已中断程序或已掉线,请重启程序", "连接中断", MB_OK | MB_ICONWARNING);
		exit(0);
	}
	sscanf_s(data, "%d,%d", &x, &y);				//保存在变量中
	YOU.INIT(x, y);
	return YOU;
}

void TOI(bool FUORKE)								//联机模式
{
	WORD wVersionRequested;
	WSADATA wsaData;
	int err, res = 0;
	wVersionRequested = MAKEWORD(1, 1);
	err = WSAStartup(wVersionRequested, &wsaData);
	if (err != 0) return;
	if (LOBYTE(wsaData.wVersion) != 1 || HIBYTE(wsaData.wVersion) != 1)
	{
		WSACleanup();
		return;
	}
	SOCKADDR_IN addrSer;
	if (FUORKE)
	{
		Get_ip();										//创建服务器
		sockSer = socket(AF_INET, SOCK_STREAM, 0);
	}
	else
	{
		settextstyle(BBLACK / 2, 0, "方正姚体");
		outtextxy(WINDOWS_X / 2 - 3 * BBLACK / 2, BBLACK * 3 / 2, "墨攻棋阵的连接");
		settextstyle(BBLACK / 4, 0, "楷体");
		outtextxy(WINDOWS_X / 2 - 2 * BBLACK, BBLACK * 5 / 2, "1、请确保服务端已正常启动");
		outtextxy(WINDOWS_X / 2 - 2 * BBLACK, BBLACK * 3, "2、请确保你和朋友在同一局域网下");
		ip = (char*)malloc(sizeof(char) * 50);
		InputBox(ip, 50, "请输入服务端的IP地址");
		sockConn = socket(AF_INET, SOCK_STREAM, 0);
	}
	addrSer.sin_family = AF_INET;
	addrSer.sin_port = htons(5050);
	addrSer.sin_addr.S_un.S_addr = inet_addr(ip);
	if (FUORKE)											//如果为服务端
	{
		SOCKADDR_IN addrCli;
		bind(sockSer, (SOCKADDR*)&addrSer, sizeof(SOCKADDR));
		listen(sockSer, 5);
		int len = sizeof(SOCKADDR);
		settextstyle(BBLACK / 4, 0, "楷体");
		outtextxy(WINDOWS_X / 2 - 2 * BBLACK, BBLACK * 5, "服务器已创建，等待连接中...");
		sockConn = accept(sockSer, (SOCKADDR*)&addrCli, &len);
	}
	else res = connect(sockConn, (SOCKADDR*)&addrSer, sizeof(SOCKADDR));
	if (sockConn == INVALID_SOCKET || res)
	{
		outtextxy(2 * BBLACK, BBLACK * 6, "连接失败！");
		_getch();
		gameStart();									//返回主界面
	}
	else
	{
		outtextxy(2 * BBLACK, BBLACK * 6, "连接成功！点击任意键进入游戏~");
		_getch();
	}
}

void gameStart()
{
	IMAGE MM[11] = { 0 }, MB[3] = { 0 };
	initgraph(WINDOWS_X, WINDOWS_Y);
	setbkmode(TRANSPARENT);					//透明字体

	HWND hwnd = GetHWnd();					// 设置窗口标题文字
	SetWindowText(hwnd, "墨攻棋阵 --- 千千");
	loadimage(NULL, "0.jpg");

	const int bblack = 10;

	LOGFONT f;
	gettextstyle(&f);												// 获取字体样式
	f.lfHeight = BBLACK;												// 设置字体高度
	strcpy_s(f.lfFaceName, _T("方正姚体"));								// 设置字体
	f.lfQuality = ANTIALIASED_QUALITY;								// 设置输出效果为抗锯齿  
	settextstyle(&f);												// 设置字体样式
	RECT r1 = { 0, 0, WINDOWS_X, WINDOWS_Y / 3 };
	drawtext("墨 攻 棋 阵", &r1, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

	settextstyle(BBLACK / 4, 0, "方正姚体");
	RECT r2 = { WINDOWS_X / 2 - BBLACK,WINDOWS_Y / 3,WINDOWS_X / 2 + BBLACK ,WINDOWS_Y / 3 + BBLACK / 2 };
	rectangle(WINDOWS_X / 2 - BBLACK, WINDOWS_Y / 3, WINDOWS_X / 2 + BBLACK, WINDOWS_Y / 3 + BBLACK / 2);
	drawtext("单人模式", &r2, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

	RECT r3 = { WINDOWS_X / 2 - BBLACK,WINDOWS_Y / 3 + BBLACK / 2 + bblack,WINDOWS_X / 2 + BBLACK,WINDOWS_Y / 3 + BBLACK + bblack };
	rectangle(WINDOWS_X / 2 - BBLACK, WINDOWS_Y / 3 + BBLACK / 2 + bblack, WINDOWS_X / 2 + BBLACK, WINDOWS_Y / 3 + BBLACK + bblack);
	drawtext("双人模式", &r3, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

	RECT r4 = { WINDOWS_X / 2 - BBLACK,WINDOWS_Y / 3 + BBLACK + 2 * bblack,WINDOWS_X / 2 + BBLACK,(int)(WINDOWS_Y / 3 + BBLACK*1.5 + 2 * bblack) };
	rectangle(WINDOWS_X / 2 - BBLACK, WINDOWS_Y / 3 + BBLACK + 2 * bblack, WINDOWS_X / 2 + BBLACK, (int)(WINDOWS_Y / 3 + BBLACK*1.5 + 2 * bblack));
	drawtext("联机对战", &r4, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

	RECT r5 = { WINDOWS_X / 2 - BBLACK,(int)(WINDOWS_Y / 3 + BBLACK*1.5 + 3 * bblack),WINDOWS_X / 2 + BBLACK,WINDOWS_Y / 3 + BBLACK * 2 + 3 * bblack };
	rectangle(WINDOWS_X / 2 - BBLACK, (int)(WINDOWS_Y / 3 + BBLACK*1.5 + 3 * bblack), WINDOWS_X / 2 + BBLACK, WINDOWS_Y / 3 + BBLACK * 2 + 3 * bblack);
	drawtext("观战模式", &r5, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

	RECT r6 = { WINDOWS_X / 2 - BBLACK,WINDOWS_Y / 3 + BBLACK * 2 + 4 * bblack,WINDOWS_X / 2 + BBLACK,(int)(WINDOWS_Y / 3 + BBLACK * 2.5 + 4 * bblack) };
	rectangle(WINDOWS_X / 2 - BBLACK, WINDOWS_Y / 3 + BBLACK * 2 + 4 * bblack, WINDOWS_X / 2 + BBLACK, (int)(WINDOWS_Y / 3 + BBLACK * 2.5 + 4 * bblack));
	drawtext("游戏介绍", &r6, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

	RECT r7 = { WINDOWS_X / 2 - BBLACK,(int)(WINDOWS_Y / 3 + BBLACK * 2.5 + 5 * bblack),WINDOWS_X / 2 + BBLACK,WINDOWS_Y / 3 + BBLACK * 3 + 5 * bblack };
	rectangle(WINDOWS_X / 2 - BBLACK, (int)(WINDOWS_Y / 3 + BBLACK * 2.5 + 5 * bblack), WINDOWS_X / 2 + BBLACK, WINDOWS_Y / 3 + BBLACK * 3 + 5 * bblack);
	drawtext("操作说明", &r7, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

	RECT r8 = { WINDOWS_X / 2 - BBLACK,WINDOWS_Y / 3 + BBLACK * 3 + 6 * bblack,WINDOWS_X / 2 + BBLACK,(int)(WINDOWS_Y / 3 + BBLACK * 3.5 + 6 * bblack) };
	rectangle(WINDOWS_X / 2 - BBLACK, WINDOWS_Y / 3 + BBLACK * 3 + 6 * bblack, WINDOWS_X / 2 + BBLACK, (int)(WINDOWS_Y / 3 + BBLACK * 3.5 + 6 * bblack));
	drawtext("关    于", &r8, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

	RECT r9 = { WINDOWS_X / 2 - BBLACK,(int)(WINDOWS_Y / 3 + BBLACK * 3.5 + 7 * bblack),WINDOWS_X / 2 + BBLACK,WINDOWS_Y / 3 + BBLACK * 4 + 7 * bblack };
	rectangle(WINDOWS_X / 2 - BBLACK, (int)(WINDOWS_Y / 3 + BBLACK * 3.5 + 7 * bblack), WINDOWS_X / 2 + BBLACK, WINDOWS_Y / 3 + BBLACK * 4 + 7 * bblack);
	drawtext("退出游戏", &r9, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

	for (int i = 0; i < 8; i++)																						//保存按钮图片
		getimage(MM + i, WINDOWS_X / 2 - BBLACK, WINDOWS_Y / 3 + i*bblack + i*BBLACK / 2, 2 * BBLACK, BBLACK / 2);
	getimage(MM + 8, WINDOWS_X - BBLACK, WINDOWS_Y - BBLACK / 2, BBLACK - 11, BBLACK / 2 - 11);

	bool _HOME = true, _INTRODUCTION = false, _OPERATION = false, _ABOUT = false, _TOINTERNET = false, _DRMS = false;			//TRUE表示处于当前页面
	MOUSEMSG m;
	while (_HOME)
	{
		BeginBatchDraw();
		m = GetMouseMsg();
		switch (m.uMsg)
		{
		case WM_LBUTTONDOWN:												//当鼠标左键击下时
			EndBatchDraw();
			if (m.x>WINDOWS_X / 2 - BBLACK && m.x<WINDOWS_X / 2 + BBLACK && m.y>WINDOWS_Y / 3 && m.y<WINDOWS_Y / 3 + BBLACK / 2 && _HOME  && !_INTRODUCTION  && !_OPERATION&&!_ABOUT&&!_TOINTERNET&&!_DRMS)//单人模式
			{
				_DRMS = true;									//离开HOME界面
				cleardevice();
				loadimage(NULL, "0.jpg");				//背景
				rectangle(BBLACK, BBLACK, WINDOWS_X - BBLACK, WINDOWS_Y - BBLACK);

				RECT r1 = { WINDOWS_X / 2 - BBLACK,WINDOWS_Y / 3,WINDOWS_X / 2 + BBLACK ,WINDOWS_Y / 3 + BBLACK / 2 };
				rectangle(WINDOWS_X / 2 - BBLACK, WINDOWS_Y / 3, WINDOWS_X / 2 + BBLACK, WINDOWS_Y / 3 + BBLACK / 2);
				drawtext("简    单", &r1, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

				RECT r2 = { WINDOWS_X / 2 - BBLACK,WINDOWS_Y / 3 + BBLACK + 2 * bblack,WINDOWS_X / 2 + BBLACK,(int)(WINDOWS_Y / 3 + BBLACK*1.5 + 2 * bblack) };
				rectangle(WINDOWS_X / 2 - BBLACK, WINDOWS_Y / 3 + BBLACK + 2 * bblack, WINDOWS_X / 2 + BBLACK, (int)(WINDOWS_Y / 3 + BBLACK*1.5 + 2 * bblack));
				drawtext("中    等", &r2, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

				RECT r3 = { WINDOWS_X / 2 - BBLACK,WINDOWS_Y / 3 + BBLACK * 2 + 4 * bblack,WINDOWS_X / 2 + BBLACK,(int)(WINDOWS_Y / 3 + BBLACK * 2.5 + 4 * bblack) };
				rectangle(WINDOWS_X / 2 - BBLACK, WINDOWS_Y / 3 + BBLACK * 2 + 4 * bblack, WINDOWS_X / 2 + BBLACK, (int)(WINDOWS_Y / 3 + BBLACK * 2.5 + 4 * bblack));
				drawtext("困    难", &r3, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

				for (int i = 0; i < 3; i++)
				{
					getimage(MB + i, WINDOWS_X / 2 - BBLACK, WINDOWS_Y / 3 + BBLACK*i + 2 * i * bblack, 2 * BBLACK, BBLACK / 2);
				}

				RECT R = { WINDOWS_X - BBLACK,WINDOWS_Y - BBLACK / 2,WINDOWS_X - 10,WINDOWS_Y - 10 };
				rectangle(WINDOWS_X - BBLACK, WINDOWS_Y - BBLACK / 2, WINDOWS_X - 10, WINDOWS_Y - 10);
				drawtext("返回", &R, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
				break;
			}
			else if (m.x>WINDOWS_X / 2 - BBLACK && m.x<WINDOWS_X / 2 + BBLACK && m.y>WINDOWS_Y / 3 && m.y<WINDOWS_Y / 3 + BBLACK / 2 && _DRMS)			//简单
			{
				_HOME = false;
				cleardevice();
				init();
				STARTVS(black, MOUSE, Easy);							//单人简单模式
				break;
			}
			else if (m.x>WINDOWS_X / 2 - BBLACK && m.x<WINDOWS_X / 2 + BBLACK && m.y>WINDOWS_Y / 3 + BBLACK + 2 * bblack && m.y<(int)(WINDOWS_Y / 3 + BBLACK*1.5 + 2 * bblack) && _DRMS)			//中等
			{
				_HOME = false;
				cleardevice();
				init();
				STARTVS(black, MOUSE, Middle);							//单人中等模式
				break;
			}
			else if (m.x>WINDOWS_X / 2 - BBLACK && m.x<WINDOWS_X / 2 + BBLACK && m.y> WINDOWS_Y / 3 + BBLACK * 2 + 4 * bblack && m.y<(int)(WINDOWS_Y / 3 + BBLACK * 2.5 + 4 * bblack) && _DRMS)			//困难
			{
				_HOME = false;
				cleardevice();
				init();
				STARTVS(black, MOUSE, Difficult);							//单人困难模式
				break;
			}
			else if (m.x>WINDOWS_X / 2 - BBLACK && m.x<WINDOWS_X / 2 + BBLACK && m.y>WINDOWS_Y / 3 + BBLACK / 2 + bblack && m.y<WINDOWS_Y / 3 + BBLACK + bblack && _HOME  && !_INTRODUCTION  && !_OPERATION&&!_ABOUT&&!_TOINTERNET&&!_DRMS)//双人模式
			{
				_HOME = false;									//离开HOME界面
				init();
				STARTVS(-1, MOUSE, MOUSE);							//双人模式
				break;
			}
			else if (m.x>WINDOWS_X / 2 - BBLACK && m.x<WINDOWS_X / 2 + BBLACK && m.y>WINDOWS_Y / 3 + BBLACK + 2 * bblack && m.y<(int)(WINDOWS_Y / 3 + BBLACK*1.5 + 2 * bblack) && _HOME   && !_OPERATION&&!_ABOUT && !_INTRODUCTION&&!_TOINTERNET&&!_DRMS)//联机对战
			{
				_TOINTERNET = true;
				cleardevice();
				loadimage(NULL, "0.jpg");				//背景
				rectangle(BBLACK, BBLACK, WINDOWS_X - BBLACK, WINDOWS_Y - BBLACK);
				RECT R1 = { WINDOWS_X / 4, WINDOWS_Y / 2 + BBLACK, WINDOWS_X / 4 + 2 * BBLACK , WINDOWS_Y / 2 + 2 * BBLACK };
				RECT R3 = { WINDOWS_X - WINDOWS_X / 4 - 2 * BBLACK, WINDOWS_Y / 2 + BBLACK, WINDOWS_X - WINDOWS_X / 4, WINDOWS_Y / 2 + 2 * BBLACK };
				rectangle(WINDOWS_X / 4, WINDOWS_Y / 2 + BBLACK, WINDOWS_X / 4 + 2 * BBLACK, WINDOWS_Y / 2 + 2 * BBLACK);
				rectangle(WINDOWS_X - WINDOWS_X / 4 - 2 * BBLACK, WINDOWS_Y / 2 + BBLACK, WINDOWS_X - WINDOWS_X / 4, WINDOWS_Y / 2 + 2 * BBLACK);
				drawtext("我要创建", &R1, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
				drawtext("我要连接", &R3, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
				getimage(MM + 9, WINDOWS_X / 4, WINDOWS_Y / 2 + BBLACK, 2 * BBLACK, BBLACK);
				getimage(MM + 10, WINDOWS_X - WINDOWS_X / 4 - 2 * BBLACK, WINDOWS_Y / 2 + BBLACK, 2 * BBLACK, BBLACK);

				outtextxy(WINDOWS_X / 3 - 50, 150, "连接说明：");
				outtextxy(WINDOWS_X / 3, 185, "1、点击“我要创建”创建服务器");
				outtextxy(WINDOWS_X / 3, 220, "2、让你的朋友进入“我要连接”");
				outtextxy(WINDOWS_X / 3, 255, "3、输入服务器ip地址");
				outtextxy(WINDOWS_X / 3, 290, "4、进入游戏");
				outtextxy(WINDOWS_X / 3, 325, "*请确保你们在同一个局域网下哦~");
				RECT R2 = { WINDOWS_X - BBLACK,WINDOWS_Y - BBLACK / 2,WINDOWS_X - 10,WINDOWS_Y - 10 };
				rectangle(WINDOWS_X - BBLACK, WINDOWS_Y - BBLACK / 2, WINDOWS_X - 10, WINDOWS_Y - 10);
				drawtext("返回", &R2, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
				break;
			}
			else if (m.x>WINDOWS_X / 4 && m.x<WINDOWS_X / 4 + 2 * BBLACK  && m.y>WINDOWS_Y / 2 + BBLACK && m.y<WINDOWS_Y / 2 + 2 * BBLACK && _TOINTERNET)//我要创建
			{
				TOINTERNET = true;
				cleardevice();
				loadimage(NULL, "0.jpg");				//背景
				rectangle(BBLACK, BBLACK, WINDOWS_X - BBLACK, WINDOWS_Y - BBLACK);
				TOI(true);								//创建服务器
				init();
				STARTVS(white, OURCLASS, MOUSE);
				break;
			}
			else if (m.x>WINDOWS_X - WINDOWS_X / 4 - 2 * BBLACK && m.x<WINDOWS_X - WINDOWS_X / 4 && m.y>WINDOWS_Y / 2 + BBLACK && m.y<WINDOWS_Y / 2 + 2 * BBLACK && _TOINTERNET)//我要连接
			{
				TOINTERNET = true;
				cleardevice();
				loadimage(NULL, "0.jpg");				//背景
				rectangle(BBLACK, BBLACK, WINDOWS_X - BBLACK, WINDOWS_Y - BBLACK);
				TOI(false);								//连接服务器
				init();
				STARTVS(black, MOUSE, OURCLASS);
				break;
			}
			else if (m.x>WINDOWS_X / 2 - BBLACK && m.x<WINDOWS_X / 2 + BBLACK && m.y>(int)(WINDOWS_Y / 3 + BBLACK*1.5 + 3 * bblack) && m.y<WINDOWS_Y / 3 + BBLACK * 2 + 3 * bblack && _HOME  &&!_OPERATION&&!_ABOUT && !_INTRODUCTION&&!_TOINTERNET&&!_DRMS)		//观战模式
			{
				_HOME = false;										//离开HOME界面
				init();
				STARTVS(-1, Middle,Difficult );								//观战模式
				break;
			}
			else if (m.x>WINDOWS_X / 2 - BBLACK && m.x<WINDOWS_X / 2 + BBLACK && m.y> WINDOWS_Y / 3 + BBLACK * 2 + 4 * bblack && m.y<(int)(WINDOWS_Y / 3 + BBLACK * 2.5 + 4 * bblack) && _HOME &&!_INTRODUCTION&&!_ABOUT&&!_OPERATION&&!_TOINTERNET&&!_DRMS)	//游戏介绍
			{
				_INTRODUCTION = true;
				cleardevice();
				loadimage(NULL, "0.jpg");				//背景
				string data[16] = {
					"游戏介绍：" ,
					"五步之内，百人不当",
					"十年磨剑，一孤侠道",
					"千里挥戈，万众俯首",
					"四海江湖，百世王道",
					"每一个来到墨问的人 都会面临选择",
					"天下皆白 唯我独黑",
					"民生涂炭 奈之若何",
					"墨门绝术 克而不攻",
					"八横八纵 兼爱平生",
					"墨家主张非攻兼爱 要获得胜利",
					"并非一定要通过杀戮 攻城为下 攻心为上",
					"墨攻棋局 棋子虽然不多",
					"但是敌我双方的转化 却是千变万化 步步惊心",
				};

				rectangle(BBLACK, BBLACK, WINDOWS_X - BBLACK, WINDOWS_Y - BBLACK);
				settextstyle(BBLACK / 2 - 5, 0, "方正姚体");
				settextcolor(RGB(0, 255, 255));
				outtextxy(WINDOWS_X / 3 - 100, 90, data[0].data());
				settextstyle(BBLACK / 4, 0, "方正姚体");
				settextcolor(WHITE);
				int LEFT, TOP = 115;
				for (int i = 1; i < 16; i++)
				{
					LEFT = WINDOWS_X / 3;
					if (i == 5 || i >= 10)LEFT -= 25;
					if (i == 5 || i == 10 || i == 6)TOP += 30;
					outtextxy(LEFT, TOP + 35 * i, data[i].data());
				}
				RECT R1 = { WINDOWS_X - BBLACK,WINDOWS_Y - BBLACK / 2,WINDOWS_X - 10,WINDOWS_Y - 10 };
				rectangle(WINDOWS_X - BBLACK, WINDOWS_Y - BBLACK / 2, WINDOWS_X - 10, WINDOWS_Y - 10);
				drawtext("返回", &R1, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
				break;
			}
			else if (m.x>WINDOWS_X / 2 - BBLACK && m.x<WINDOWS_X / 2 + BBLACK && m.y>(int)(WINDOWS_Y / 3 + BBLACK * 2.5 + 5 * bblack) && m.y<WINDOWS_Y / 3 + BBLACK * 3 + 5 * bblack && _HOME &&!_INTRODUCTION &&!_OPERATION&&!_ABOUT&&!_TOINTERNET&&!_DRMS)	//操作说明
			{
				_OPERATION = true;
				cleardevice();
				loadimage(NULL, "0.jpg");				//背景
				string data[16] = {
					"操作说明：" ,
					"班大师：",
					"王道之室中 不是普通的棋局",
					"而是根据本门绝学精髓设计而成的墨攻棋阵",
					"墨攻棋阵与围棋明显的不同就是",
					"墨攻棋局中不会有任何棋子被杀死",
					"当一方的棋子被另一方棋子前后围堵",
					"那这些棋子就转化成另一方",
					"当然 如果这些棋子又被围堵时",
					"还可以再次转化",
					"最后六十四格棋盘布满时就看双方谁的棋子数量多",
					"哪一方就获胜",
					"墨攻棋局 每一次落子必须要形成转换",
					"如果对方没有可被转换的棋子时",
					"这种情况 本方就只能放弃这一轮出手",
					"能够把对手逼入这种困境 就叫作破阵 是最厉害的招数",
				};

				rectangle(BBLACK, BBLACK, WINDOWS_X - BBLACK, WINDOWS_Y - BBLACK);
				settextstyle(BBLACK / 2 - 5, 0, "方正姚体");
				settextcolor(RGB(0, 255, 255));
				outtextxy(WINDOWS_X / 3 - 100, 90, data[0].data());
				settextstyle(BBLACK / 4, 0, "方正姚体");
				settextcolor(WHITE);
				int LEFT, TOP = 115;
				for (int i = 1; i < 16; i++)
				{
					LEFT = WINDOWS_X / 3;
					if (i == 1)LEFT -= 50;
					if (i == 6 || i == 10)LEFT -= 25;
					if (i == 12)TOP += 30;
					outtextxy(LEFT, TOP + 35 * i, data[i].data());
				}
				RECT R3 = { WINDOWS_X - BBLACK,WINDOWS_Y - BBLACK / 2,WINDOWS_X - 10,WINDOWS_Y - 10 };
				rectangle(WINDOWS_X - BBLACK, WINDOWS_Y - BBLACK / 2, WINDOWS_X - 10, WINDOWS_Y - 10);
				drawtext("返回", &R3, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
				break;
			}
			else if (m.x > WINDOWS_X / 2 - BBLACK && m.x<WINDOWS_X / 2 + BBLACK && m.y>WINDOWS_Y / 3 + BBLACK * 3 + 6 * bblack && m.y < (int)(WINDOWS_Y / 3 + BBLACK * 3.5 + 6 * bblack) && _HOME  && !_INTRODUCTION  && !_OPERATION&&!_ABOUT&&!_TOINTERNET&&!_DRMS)//关于
			{
				_ABOUT = true;
				cleardevice();
				loadimage(NULL, "0.jpg");				//背景
				string data[16] = {
					"关于:" ,
					"感觉主动进入这个页面的人都是关心 千千 的人哟！",
					"怎么说千千也都是新人那！",
					"懵懂无知感觉时间过得真的好快，不知不觉就要度过大一啦~",
					"只是不想在考试之后看到自己会挂科 o(╯□╰)o",
					"每次更换头像都会找很久很久惹",
					"千千的梦想呢？不会说出来的~ 因为自己也不知道",
					"想让身边的每个人开心~ 毕竟他们也曾经让我开心过~",
					"#More 哒哒……",
					"千千是我啦！不是千玺~",
					"毕竟我是让班里唯一一个喜欢千玺的女孩更换称呼的人惹~",
					"当然继续叫千千也没事啦~ 我不会介意的╮(╯▽╰)╭[害羞]  @蛋蛋",
					"千千是个90后，噫~",
					"不能这么说啦，98后~",
					"千千的生日是新年的第四天 n(*≧▽≦*)n",
					"对我说元旦快乐的同时也可以Happy  birthday  to  me!",
				};

				rectangle(BBLACK, BBLACK, WINDOWS_X - BBLACK, WINDOWS_Y - BBLACK);
				settextstyle(BBLACK / 2 - 5, 0, "方正姚体");
				settextcolor(RGB(0, 255, 255));
				outtextxy(WINDOWS_X / 3 - 100, 90, data[0].data());
				settextstyle(BBLACK / 4, 0, "方正姚体");
				settextcolor(WHITE);
				int LEFT, TOP = 115;
				for (int i = 1; i < 16; i++)
				{
					LEFT = WINDOWS_X / 3;
					if (i == 1 || i == 9)LEFT -= 50;
					else if (i == 2 || i == 8)LEFT -= 25;
					if (i == 9)TOP += 30;
					outtextxy(LEFT, TOP + 35 * i, data[i].data());
				}
				RECT R3 = { WINDOWS_X - BBLACK,WINDOWS_Y - BBLACK / 2,WINDOWS_X - 10,WINDOWS_Y - 10 };
				rectangle(WINDOWS_X - BBLACK, WINDOWS_Y - BBLACK / 2, WINDOWS_X - 10, WINDOWS_Y - 10);
				drawtext("返回", &R3, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
				settextstyle(BBLACK / 4, 0, "微软雅黑");
				string author[5] = {
					"Author: 千千",
					"Age: 18",
					"Constellation: Capricorn",
					"Email: qian1335661317@qq.com",
					"Domain:https://www.dreamwings.cn"
				};
				for (int i = 0; i < 5; i++)
				{
					outtextxy(5 * BBLACK / 4, WINDOWS_Y - (7 - i) * BBLACK / 2, author[i].data());
				}
				settextstyle(BBLACK / 4, 0, "方正姚体");
				break;
			}
			else if (m.x>WINDOWS_X - BBLACK && m.x<WINDOWS_X - 10 && m.y>WINDOWS_Y - BBLACK / 2 && m.y<WINDOWS_Y - 10 && (_INTRODUCTION || _OPERATION || _ABOUT || _TOINTERNET || _DRMS))					//返回
			{
				cleardevice();
				_HOME = false, _INTRODUCTION = false, _OPERATION = false, _ABOUT = false, _TOINTERNET = false, _DRMS = false;
				gameStart();
			}
			else if (m.x>WINDOWS_X / 2 - BBLACK && m.x<WINDOWS_X / 2 + BBLACK && m.y>(int)(WINDOWS_Y / 3 + BBLACK*3.5 + 7 * bblack) && m.y<WINDOWS_Y / 3 + BBLACK * 4 + 7 * bblack && _HOME  && !_INTRODUCTION  && !_OPERATION&&!_ABOUT&&!_TOINTERNET&&!_DRMS)//退出游戏
			{
				exit(0);
			}
			else break;
		case WM_MOUSEMOVE:									//移动鼠标
			RECT r;
			if (_INTRODUCTION || _OPERATION || _ABOUT || _TOINTERNET || _DRMS)				//如果当前处于游戏介绍 操作说明 或者关于界面 或者联机对战界面 或者单人模式界面
			{
				if (ESCEXIT)gameStart();							//部分界面按ESC退出
				if (m.x>WINDOWS_X - BBLACK && m.x<WINDOWS_X - 10 && m.y>WINDOWS_Y - BBLACK / 2 && m.y<WINDOWS_Y - 10)
				{
					r.left = WINDOWS_X - BBLACK;
					r.top = WINDOWS_Y - BBLACK / 2;
					r.right = WINDOWS_X - 10;
					r.bottom = WINDOWS_Y - 10;
					POINT points[8] = { { r.left,r.top },{ r.right,r.top },{ r.right,r.bottom },{ r.left,r.bottom } };
					setfillcolor(RED);
					fillpolygon(points, 4);
					setbkmode(TRANSPARENT);
					drawtext("返回", &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
				}
				else
				{
					if (getpixel(WINDOWS_X - BBLACK + 1, WINDOWS_Y - BBLACK / 2 + 1) == RED)
					{
						putimage(WINDOWS_X - BBLACK + 1, WINDOWS_Y - BBLACK / 2 + 1, MM + 8);
						setbkmode(TRANSPARENT);
						drawtext("返回", &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
					}
				}
				if (_DRMS)
				{
					for (int i = 0; i < 3; i++)
					{
						if (m.x > WINDOWS_X / 2 - BBLACK && m.x<WINDOWS_X / 2 + BBLACK && m.y>WINDOWS_Y / 3 + BBLACK*i + 2 * i * bblack && m.y < WINDOWS_Y / 3 + BBLACK*i + 2 * i * bblack + BBLACK / 2)
						{
							r.left = WINDOWS_X / 2 - BBLACK;
							r.top = WINDOWS_Y / 3 + BBLACK*i + 2 * i * bblack;
							r.right = WINDOWS_X / 2 + BBLACK;
							r.bottom = WINDOWS_Y / 3 + BBLACK*i + 2 * i * bblack + BBLACK / 2;
							POINT points[8] = { { r.left,r.top },{ r.right,r.top },{ r.right,r.bottom },{ r.left,r.bottom } };
							setfillcolor(RED);
							fillpolygon(points, 4);
							setbkmode(TRANSPARENT);
							switch (i)
							{
							case 0:
								drawtext("简    单", &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
								break;
							case 1:
								drawtext("中    等", &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
								break;
							case 2:
								drawtext("困    难", &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
								break;
							}
						}
						else
						{
							if (getpixel(WINDOWS_X / 2 - BBLACK + 1, WINDOWS_Y / 3 + BBLACK*i + 2 * i * bblack + 1) == RED)
							{
								putimage(WINDOWS_X / 2 - BBLACK, WINDOWS_Y / 3 + BBLACK*i + 2 * i * bblack, MB + i);	//输出原来图片
							}
						}
					}
				}
				if (_TOINTERNET)											//处于联机栏
				{
					if (m.x > WINDOWS_X / 4 && m.x<WINDOWS_X / 4 + 2 * BBLACK  && m.y>WINDOWS_Y / 2 + BBLACK && m.y < WINDOWS_Y / 2 + 2 * BBLACK)
					{
						r.left = WINDOWS_X / 4;
						r.top = WINDOWS_Y / 2 + BBLACK;
						r.right = WINDOWS_X / 4 + 2 * BBLACK;
						r.bottom = WINDOWS_Y / 2 + 2 * BBLACK;
						POINT points[8] = { { r.left,r.top },{ r.right,r.top },{ r.right,r.bottom },{ r.left,r.bottom } };
						setfillcolor(RED);
						fillpolygon(points, 4);
						setbkmode(TRANSPARENT);
						drawtext("我要创建", &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
					}
					else
					{
						if (getpixel(WINDOWS_X / 4 + 1, WINDOWS_Y / 2 + BBLACK + 1) == RED)
						{
							putimage(WINDOWS_X / 4, WINDOWS_Y / 2 + BBLACK, MM + 9);
						}
					}
					if (m.x>WINDOWS_X - WINDOWS_X / 4 - 2 * BBLACK && m.x<WINDOWS_X - WINDOWS_X / 4 && m.y>WINDOWS_Y / 2 + BBLACK && m.y<WINDOWS_Y / 2 + 2 * BBLACK)
					{
						r.left = WINDOWS_X - WINDOWS_X / 4 - 2 * BBLACK;
						r.top = WINDOWS_Y / 2 + BBLACK;
						r.right = WINDOWS_X - WINDOWS_X / 4;
						r.bottom = WINDOWS_Y / 2 + 2 * BBLACK;
						POINT points[8] = { { r.left,r.top },{ r.right,r.top },{ r.right,r.bottom },{ r.left,r.bottom } };
						setfillcolor(RED);
						fillpolygon(points, 4);
						setbkmode(TRANSPARENT);
						drawtext("我要连接", &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
					}
					else
					{
						if (getpixel(WINDOWS_X - WINDOWS_X / 4 - 2 * BBLACK + 1, WINDOWS_Y / 2 + BBLACK + 1) == RED)
						{
							putimage(WINDOWS_X - WINDOWS_X / 4 - 2 * BBLACK, WINDOWS_Y / 2 + BBLACK, MM + 10);
						}
					}
				}
			}
			else
			{
				for (int i = 0; i<8; i++)
				{
					if (m.x>WINDOWS_X / 2 - BBLACK && m.x<WINDOWS_X / 2 + BBLACK && m.y>WINDOWS_Y / 3 + i*bblack + i*BBLACK / 2 && m.y<WINDOWS_Y / 3 + i*bblack + i*BBLACK / 2 + BBLACK / 2)
					{
						r.left = WINDOWS_X / 2 - BBLACK;
						r.top = WINDOWS_Y / 3 + i*bblack + i*BBLACK / 2;
						r.right = WINDOWS_X / 2 + BBLACK;
						r.bottom = WINDOWS_Y / 3 + i*bblack + i*BBLACK / 2 + BBLACK / 2;
						POINT points[8] = { { r.left,r.top },{ r.right,r.top },{ r.right,r.bottom },{ r.left,r.bottom } };
						setfillcolor(RED);
						fillpolygon(points, 4);
						setbkmode(TRANSPARENT);
						switch (i)
						{
						case 0:
							drawtext("单人模式", &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
							break;
						case 1:
							drawtext("双人模式", &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
							break;
						case 2:
							drawtext("联机对战", &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
							break;
						case 3:
							drawtext("观战模式", &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
							break;
						case 4:
							drawtext("游戏介绍", &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
							break;
						case 5:
							drawtext("操作说明", &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
							break;
						case 6:
							drawtext("关    于", &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
							break;
						case 7:
							drawtext("退出游戏", &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
							break;
						}
					}
					else
					{
						if (getpixel(WINDOWS_X / 2 - BBLACK + 1, WINDOWS_Y / 3 + i*bblack + i*BBLACK / 2 + 1) == RED)
						{
							putimage(WINDOWS_X / 2 - BBLACK, WINDOWS_Y / 3 + i*bblack + i*BBLACK / 2, MM + i);	//输出原来图片
						}
					}
				}
			}
			FlushBatchDraw();
			break;
		default: break;
		}
	}
	gameStart();
}

int main()
{
	gameStart();					//主界面
	closegraph();					//关闭图形化界面
	return 0;
}
