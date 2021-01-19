#include "pch.h"
#include <graphics.h>
#include <iostream>
#include "conio.h"
#include <easyx.h>
#include <time.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")
#pragma warning(disable:4996)
using namespace std;

#define WIDTH 560 //画面宽度
#define HEIGHT 760 //画面高度
#define MaxBulletNum 100 //最多子弹个数
#define PI 3.1415926

void sleep(DWORD ms) { //精确延时函数
	static DWORD oldtime = GetTickCount();
	while (GetTickCount() - oldtime < ms)
		Sleep(1);
	oldtime = GetTickCount();
}

void PlayMusicOnce(TCHAR fileName[80]) { //播放一次音乐函数
	TCHAR cmdString1[50];
	_stprintf(cmdString1, _T("open %s alias tmpmusic"), fileName); //生成命令字符串
	mciSendString(_T("close tmpmusic"), NULL, 0, NULL); //先把前面一次的音乐关闭
	mciSendString(cmdString1, NULL, 0, NULL); //打开音乐
	mciSendString(_T("play tmpmusic"), NULL, 0, NULL); //仅播放一次
}

class Spaceship { //定义飞船类
public:
	IMAGE im_spaceship1; //飞船图像
	IMAGE im_spaceship2;
	IMAGE im_blowup1; //爆炸图像
	IMAGE im_blowup2;
	IMAGE im_life1; //生命图像
	IMAGE im_life2;
	IMAGE im_inv; //无敌图像
	float x, y; //飞船坐标
	float width, height; //飞船图片的宽度和高度
	int liveSecond; //飞船存活了多长时间
	int life; //飞船有几条命
	int invincible = 0; //飞船无敌状态

	void draw() { //显示飞船相关信息
		//显示飞船
		putimage(
			x - width / 2, y - height / 2, //显示的位置
			im_spaceship2.getwidth(), im_spaceship2.getheight(),//图片的大小
			&im_spaceship2, //选择需要贴的图
			0, 0, //从该图片的起始位置开始显示
			SRCAND); //绘制出的像素颜色 = 屏幕颜色 & 图像颜色
		putimage(
			x - width / 2, y - height / 2, //显示的位置
			im_spaceship1.getwidth(), im_spaceship1.getheight(),//图片的大小
			&im_spaceship1, //选择需要贴的图
			0, 0, //从该图片的起始位置开始显示
			SRCPAINT); //绘制出的像素颜色 = 屏幕颜色 | 图像颜色
		//窗口左上角显示life个生命图片，表示飞船生命数
		for (int i = 0; i < life; i++) {
			putimage(
				i * width * 0.9 + 10, 10, //显示的位置
				im_life2.getwidth(), im_life2.getheight(),//图片的大小
				&im_life2, //选择需要贴的图
				0, 0, //从该图片的起始位置开始显示
				SRCAND); //绘制出的像素颜色 = 屏幕颜色 & 图像颜色
			putimage(
				i * width * 0.9 + 10, 10, //显示的位置
				im_life1.getwidth(), im_life1.getheight(),//图片的大小
				&im_life1, //选择需要贴的图
				0, 0, //从该图片的起始位置开始显示
				SRCPAINT); //绘制出的像素颜色 = 屏幕颜色 & 图像颜色
		}
		//窗口正上方显示坚持了多少秒
		TCHAR s[20];
		setbkmode(TRANSPARENT); //文字字体透明
		_stprintf(s, _T("%d秒"), liveSecond);
		settextcolor(WHITE); //设定文字颜色
		settextstyle(40, 0, _T("黑体")); //设定文字大小样式
		outtextxy(WIDTH * 0.85, 20, s); //输出文字内容

		if (life > 0) {
			putimage(
				x - width / 2, y - height / 2,
				im_spaceship2.getwidth(), im_spaceship2.getheight(),
				&im_spaceship2,
				0, 0,
				SRCAND);
			putimage(
				x - width / 2, y - height / 2,
				im_spaceship1.getwidth(), im_spaceship1.getheight(),
				&im_spaceship1,
				0, 0,
				SRCPAINT);
		}
			else {
			putimage(
				x - width / 2, y - height / 2,
				im_blowup2.getwidth(), im_blowup2.getheight(),
				&im_blowup2,
				0, 0,
				SRCAND);
			putimage(
				x - width / 2, y - height / 2,
				im_blowup1.getwidth(), im_blowup1.getheight(),
				&im_blowup1,
				0, 0,
				SRCPAINT);
			}
	}

	void update(float mx, float my) { //根据输入的坐标更新飞船的位置
		x = mx;
		y = my;
	}

	void updateWhenLifeLost() { //当飞船减命时执行的操作
		if (invincible == 0) {
			TCHAR cmdString1[50];
			_stprintf(cmdString1, _T("open %s alias tmpmusic"), "explode.mp3"); //生成命令字符串
			mciSendString(_T("close tmpmusic"), NULL, 0, NULL); //先把前面一次的音乐关闭
			mciSendString(_T("open explode.mp3 alias tmpmusic"), NULL, 0, NULL); //打开音乐
			mciSendString(_T("play tmpmusic"), NULL, 0, NULL); //仅播放一次
			life--; //生命减少 
		}
	}
};

class Bullet {
public:
	IMAGE im_bullet1, im_bullet2; //子弹图像
	float x, y; //子弹坐标
	float vx, vy; //子弹速度
	float radius; //接近球体的子弹半径的大小

	void draw() { //显示子弹
		putimage(
			x - radius, y - radius,
			im_bullet2.getwidth(), im_bullet2.getheight(),
			&im_bullet2,
			0, 0,
			SRCAND);
		putimage(
			x - radius, y - radius,
			im_bullet1.getwidth(), im_bullet1.getheight(),
			&im_bullet1,
			0, 0,
			SRCPAINT);
	}

	void update() { //更新子弹的位置和速度
		x += vx;
		y += vy;
		if (x <= 0 || x >= WIDTH) //实现反弹
			vx = -vx;
		if (y <= 0 || y >= HEIGHT)
			vy = -vy;
	}

	int isCollideSpaceship(Spaceship spaceship) {
		float distance_x = abs(spaceship.x - x);
		float distance_y = abs(spaceship.y - y);
			if (distance_x < spaceship.width / 2 && distance_y < spaceship.height / 2) {
				return 1; //碰撞返回1
			}
			else {
				return 0; //不碰撞返回0
			}
	}
};

class UFO : public Bullet
{
	public:
		void updateVelforTarge(Spaceship targetspaceship) {
			float scalar = 1 * rand() / double(RAND_MAX) + 1; //速度大小有一定的随机性
			if (targetspaceship.x > x) //目标在飞碟左边，飞碟x方向速度向右
				vx = scalar;
			else if (targetspaceship.x < x) //目标在飞碟右边，飞碟x方向速度向左
				vx = -scalar;
			if (targetspaceship.y > y) //目标在飞碟下方，飞碟y方向速度向下
				vy = scalar;
			else if (targetspaceship.y < y) //目标在飞碟上方，飞碟y方向速度向上
				vy = -scalar;
		}
};

class Stone : public Bullet
{
	public:
};

class Props : public Bullet
{
	public:
		int count;
};

IMAGE im_background, im_bullet1, im_bullet2, im_spaceship1, im_spaceship2, im_life1, im_life2, 
im_blowup1, im_blowup2, im_ufo1, im_ufo2, im_stone11, im_stone12, im_stone21, im_stone22, 
im_inv, im_ready, im_addlife1, im_addlife2; //定义图像对象
Bullet bullet[MaxBulletNum]; //定义子弹对象数组
Stone stone[2]; //定义陨石对象
Spaceship spaceship; //定义飞船对象
UFO ufo; //定义飞碟数组
Props addlife;
int bulletNum = 0; //已有子弹个数


void startup() { //初始化函数
	mciSendString(_T("open game_music.mp3 alias bkmusic"), NULL, 0, NULL); //打开背景音乐
	mciSendString(_T("play bkmusic repeat"), NULL, 0, NULL); //循环播放

	srand(time(0)); //初始化随机种子
	loadimage(&im_background, _T("background.png")); //导入背景图片
	loadimage(&im_bullet1, _T("bullet1.png")); //导入子弹图片
	loadimage(&im_bullet2, _T("bullet2.png")); 
	loadimage(&im_spaceship1, _T("spaceship1.png")); //导入飞船图片
	loadimage(&im_spaceship2, _T("spaceship2.png"));
	loadimage(&im_life1, _T("life1.png")); //导入生命图片
	loadimage(&im_life2, _T("life2.png"));
	loadimage(&im_blowup1, _T("blowup1.png")); //导入爆炸图片
	loadimage(&im_blowup2, _T("blowup2.png"));
	loadimage(&im_ufo1, _T("ufo1.png")); //导入飞碟图片
	loadimage(&im_ufo2, _T("ufo2.png"));
	loadimage(&im_stone11, _T("stone11.png")); //导入陨石1图片
	loadimage(&im_stone12, _T("stone12.png"));
	loadimage(&im_stone21, _T("stone21.png")); //导入陨石2图片
	loadimage(&im_stone22, _T("stone22.png"));
	loadimage(&im_inv, _T("inv.png")); 
	loadimage(&im_addlife1, _T("addlife1.png")); 
	loadimage(&im_addlife2, _T("addlife2.png"));
	//对飞碟一些成员变量的初始化
	ufo.x = WIDTH / 2;
	ufo.y = 10;
	ufo.im_bullet1 = im_ufo1; //设置飞碟图片
	ufo.im_bullet2 = im_ufo2;
	ufo.radius = im_ufo1.getwidth() / 2; //设置飞碟半径大小
	ufo.updateVelforTarge(spaceship); //更新飞碟的速度

	//对spaceship一些成员变量初始化
	spaceship.im_spaceship1 = im_spaceship1; //设置飞船图片
	spaceship.im_spaceship2 = im_spaceship2;
	spaceship.im_blowup1 = im_blowup1; //设置爆炸图片
	spaceship.im_blowup2 = im_blowup2;
	spaceship.im_life1 = im_life1; //设置生命图片
	spaceship.im_life2 = im_life2;
	spaceship.im_inv = im_inv; //设置无敌图片
	spaceship.width = im_spaceship1.getwidth(); //设置飞船宽度
	spaceship.height = im_spaceship1.getheight(); //设置飞船高度
	spaceship.life = 5; //飞船初始5条命
	initgraph(WIDTH, HEIGHT); //新开一个画面
	BeginBatchDraw(); //开始批量绘制
}

void show() { //绘制函数
	putimage(0, 0, &im_background); //显示背景
	ufo.draw(); //显示飞碟
	for (int i = 0; i < bulletNum; i++)
		bullet[i].draw(); //显示已有子弹
	for (int i = 0; i < 2; i++) {
		stone[i].draw(); //显示陨石
	}
	spaceship.draw(); //显示飞船及相关信息
	addlife.draw();
	FlushBatchDraw(); //批量复制
	Sleep(10); //暂停
}

int flag = 0;
void updateWithoutInput() { //和输入无关的更新
	HWND hwnd = GetHWnd();
	if (spaceship.life <= 0 || flag == 1) { //飞船没命了,不处理
		MessageBox(hwnd, "Defeat！", "Universe Escaping", MB_OK);
		flag = 1;
		exit(0);
	}
	static int lastSecond1 = 0, lastSecond2 = 0; //记录前一次程序运行了多少秒
	static int nowSecond = 0; //记录当前程序运行了多少秒
	static clock_t start = clock(); //记录第一次运行时刻
	//static clock_t collide = start;
	clock_t now = clock(); //获得当前时刻
	//计算程序目前一共运行了多少秒
	nowSecond = (int(now - start) / CLOCKS_PER_SEC);
	spaceship.liveSecond = nowSecond; //飞船生成了多少秒赋值
	if (nowSecond == lastSecond1 + 1) {//时间过1秒，更新飞碟的速度
		lastSecond1 = nowSecond;
		ufo.updateVelforTarge(spaceship); //飞碟的速度方向瞄准飞船
	}
	if (nowSecond == lastSecond2 + 2) { //时间过了2秒钟，新增一颗子弹
		lastSecond2 = nowSecond; //更新lastSecond变量
		//如果没有超出最大子弹数目的限制，增加一颗子弹
		if (bulletNum < MaxBulletNum) {
			bullet[bulletNum].x = WIDTH / 2; //子弹初始位置
			bullet[bulletNum].y = 10;
			float angle = (rand() / double(RAND_MAX) - 0.5)*0.9*PI;
			float scalar = 2 * rand() / double(RAND_MAX) + 2;
			bullet[bulletNum].vx = scalar * sin(angle); //子弹随机速度
			bullet[bulletNum].vy = scalar * cos(angle);
			bullet[bulletNum].im_bullet1 = im_bullet1; //设置子弹图像
			bullet[bulletNum].im_bullet2 = im_bullet2;
			bullet[bulletNum].radius = im_bullet1.getwidth() / 2; //子弹半径为图片宽度一半
		}
		bulletNum++; //子弹数目加一
	}
	for (int i = 0; i < bulletNum; i++) { //对于所有的子弹
		bullet[i].update(); //更新子弹的位置和速度
		if (bullet[i].isCollideSpaceship(spaceship)) { //判断子弹飞船是否相撞
			static clock_t collide = clock();
			spaceship.updateWhenLifeLost(); //飞船减命操作 
			if ((int(now - collide) / CLOCKS_PER_SEC) <  1) { //1秒内无敌
				spaceship.invincible = 1;
			}
			else {
				collide = now;
				spaceship.invincible = 0;
			}
			break; //飞船已经炸了，不用再和其他子弹比较了
		}
	}
	
	ufo.update(); //更新飞碟的位置和速度
	if (ufo.isCollideSpaceship(spaceship)) { //判断飞碟是否和火箭相撞
		static clock_t collide = clock();
		spaceship.updateWhenLifeLost(); //当飞船减命时执行相关的操作
		if ((int(now - collide) / CLOCKS_PER_SEC) < 1) { //1秒内无敌
			spaceship.invincible = 1;
		}
		else {
			collide = now;
			spaceship.invincible = 0;
		}
	}

	if (nowSecond == 40) {
		//对陨石一些成员变量的初始化
		stone[0].x = WIDTH / 2;
		stone[0].y = 10;
		float angle = (rand() / double(RAND_MAX) - 0.5)*0.9*PI;
		float scalar = 1;
		stone[0].vx = scalar * sin(angle);
		stone[0].vy = scalar * cos(angle);
		stone[0].im_bullet1 = im_stone11;
		stone[0].im_bullet2 = im_stone12;
		stone[0].radius = im_stone11.getwidth() / 2;
	}
	if (nowSecond == 80) {
		//对道具的初始化
		addlife.x = WIDTH / 2;
		addlife.y = 10;
		float angle = (rand() / double(RAND_MAX) - 0.5)*0.9*PI;
		float scalar = 1;
		addlife.vx = scalar * sin(angle);
		addlife.vy = scalar * cos(angle);
		addlife.im_bullet1 = im_addlife1;
		addlife.im_bullet2 = im_addlife2;
		addlife.radius = im_addlife1.getwidth() / 2;
		addlife.count = 1;
	}
	if (nowSecond == 70) {
		//对陨石一些成员变量的初始化
		stone[1].x = WIDTH / 2;
		stone[1].y = 10;
		float angle = (rand() / double(RAND_MAX) - 0.5)*0.9*PI;
		float scalar = 1;
		stone[1].vx = scalar * sin(angle);
		stone[1].vy = scalar * cos(angle);
		stone[1].im_bullet1 = im_stone21;
		stone[1].im_bullet2 = im_stone22;
		stone[1].radius = im_stone21.getwidth() / 2;
	}
	if (nowSecond >= 40) {
		stone[0].update();
		if (stone[0].isCollideSpaceship(spaceship)) {
			spaceship.updateWhenLifeLost(); //飞船减命操作
			static clock_t collide = clock();
			if ((int(now - collide) / CLOCKS_PER_SEC) < 1) { //1秒内无敌
				spaceship.invincible = 1;
			}
			else {
				collide = now;
				spaceship.invincible = 0;
			}
		}
	}
	if (nowSecond >= 80) {
		addlife.update();
		if (addlife.isCollideSpaceship(spaceship) && addlife.count > 0) {
			spaceship.life++;
			addlife.count--;
		}
	}
	if (nowSecond >= 70) {
		stone[1].update();
		if (stone[1].isCollideSpaceship(spaceship)) {
			spaceship.updateWhenLifeLost(); //飞船减命操作
			static clock_t collide = clock();
			if ((int(now - collide) / CLOCKS_PER_SEC) < 1) { //1秒内无敌
				spaceship.invincible = 1;
			}
			else {
				collide = now;
				spaceship.invincible = 0;
			}
		}
	}

	if (nowSecond >= 100) {
		MessageBox(hwnd, "Victory！", "Universe Escaping", MB_OK);
		flag = 1;
		exit(0);
	}
}

void updateWithInput() { //和输入相关的更新
	if (spaceship.life <= 0) { //飞船没命了 不处理
		return; //直接返回
	}

	MOUSEMSG m; //定义鼠标消息
	while (MouseHit()) { //检测当前是否有鼠标消息
		m = GetMouseMsg();
		if (m.uMsg == WM_MOUSEMOVE) //鼠标移动时
			spaceship.update(m.x, m.y); //飞船的位置等于鼠标所在的位置
	}
}

int main()
{
	
	initgraph(WIDTH, HEIGHT);
	loadimage(&im_background, _T("background.png")); //导入图片
	loadimage(&im_ready, _T("ready.png"));
	putimage(0, 0, &im_background); //显示
	putimage(WIDTH*0.40, HEIGHT*0.9, &im_ready);
	while (1) {
		MOUSEMSG m;
		m = GetMouseMsg();
		if (m.x >= WIDTH * 0.40 && m.x <= WIDTH * 0.40 + im_ready.getwidth() && m.y >= HEIGHT * 0.9 && m.y <= HEIGHT * 0.9 + im_ready.getheight()) {
			if (m.uMsg == WM_LBUTTONDOWN) {
				startup(); //初始化
				while (1) {
					show(); //绘制
					updateWithoutInput(); //和输入无关的更新
					updateWithInput(); //和输入相关的更新
				}
			}
		}
	}
}

