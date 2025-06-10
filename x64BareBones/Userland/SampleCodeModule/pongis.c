#include "syscalls.h"
#include <stdint.h>
#include "shell.h"

#define SYS_WRITE 0
#define SYS_READ 1
#define SYS_CLEAR_SCREEN 2
#define SYS_SLEEP 3
#define SYS_DRAW_RECT 4
#define SYS_PLAY_BEEP 8

#define ESC 27

#define SCREEN_WIDTH 1024
#define SCREEN_HEIGHT 768

#define PADDLE_SIZE 40
#define BALL_SIZE 16
#define HOLE_SIZE 24

#define PADDLE_SPEED 10
#define BALL_SPEED 8

typedef struct {
    int x;
    int y;
} vec2i;

static int str_len(const char *s) {
    int l = 0;
    while (s[l]) l++;
    return l;
}

static int int_to_str(int v, char *buf) {
    char tmp[16];
    int i = 0;
    if (v == 0) {
        buf[0] = '0';
        buf[1] = 0;
        return 1;
    }
    int neg = 0;
    if (v < 0) { neg = 1; v = -v; }
    while (v > 0) {
        tmp[i++] = '0' + (v % 10);
        v /= 10;
    }
    int len = 0;
    if (neg) buf[len++] = '-';
    while (i--) buf[len++] = tmp[i];
    buf[len] = 0;
    return len;
}

static void draw_score(int p1, int p2) {
    char buf[32];
    _sys_write(SYS_WRITE, "P1: ", 4);
    int len = int_to_str(p1, buf);
    _sys_write(SYS_WRITE, buf, len);
    _sys_write(SYS_WRITE, "    P2: ", 8);
    len = int_to_str(p2, buf);
    _sys_write(SYS_WRITE, buf, len);
    _sys_write(SYS_WRITE, "\n", 1);
}

static int rect_overlap(int x1,int y1,int w1,int h1,int x2,int y2,int w2,int h2){
    return !(x1+w1 < x2 || x2+w2 < x1 || y1+h1 < y2 || y2+h2 < y1);
}

static void clamp(int *v, int min, int max){
    if(*v < min) *v = min;
    if(*v > max) *v = max;
}

void pongis_game(){
    vec2i p1 = { 100, SCREEN_HEIGHT/2 - PADDLE_SIZE/2 };
    vec2i p2 = { SCREEN_WIDTH-100-PADDLE_SIZE, SCREEN_HEIGHT/2 - PADDLE_SIZE/2 };
    vec2i ball = { SCREEN_WIDTH/2 - BALL_SIZE/2, SCREEN_HEIGHT/2 - BALL_SIZE/2 };
    vec2i hole = { SCREEN_WIDTH/2 - HOLE_SIZE/2, 60 };
    int vx = 0, vy = 0;
    int last = 0;
    int score1 = 0, score2 = 0;

    char running = 1;
    while(running){
        char c = 0;
        if(_sys_read(SYS_READ,0,&c,1)>0){
            if(c==ESC || c=='q') break;
            if(c=='w') p1.y -= PADDLE_SPEED;
            if(c=='s') p1.y += PADDLE_SPEED;
            if(c=='a') p1.x -= PADDLE_SPEED;
            if(c=='d') p1.x += PADDLE_SPEED;
            if(c=='i') p2.y -= PADDLE_SPEED;
            if(c=='k') p2.y += PADDLE_SPEED;
            if(c=='j') p2.x -= PADDLE_SPEED;
            if(c=='l') p2.x += PADDLE_SPEED;
        }

        clamp(&p1.x,0,SCREEN_WIDTH-PADDLE_SIZE);
        clamp(&p1.y,0,SCREEN_HEIGHT-PADDLE_SIZE);
        clamp(&p2.x,0,SCREEN_WIDTH-PADDLE_SIZE);
        clamp(&p2.y,0,SCREEN_HEIGHT-PADDLE_SIZE);

        ball.x += vx;
        ball.y += vy;
        if(ball.x <=0 || ball.x+BALL_SIZE>=SCREEN_WIDTH){ vx=-vx; ball.x += vx; }
        if(ball.y <=0 || ball.y+BALL_SIZE>=SCREEN_HEIGHT){ vy=-vy; ball.y += vy; }

        if(rect_overlap(ball.x,ball.y,BALL_SIZE,BALL_SIZE,p1.x,p1.y,PADDLE_SIZE,PADDLE_SIZE)){
            if(ball.x+p1.x+PADDLE_SIZE < ball.x+BALL_SIZE+p1.x){} //dummy
            if(ball.x < p1.x) vx = -BALL_SPEED; else vx = BALL_SPEED;
            if(ball.y < p1.y) vy = -BALL_SPEED; else vy = BALL_SPEED;
            last = 1;
            _sys_playBeep(SYS_PLAY_BEEP, 900, 80);
        }
        if(rect_overlap(ball.x,ball.y,BALL_SIZE,BALL_SIZE,p2.x,p2.y,PADDLE_SIZE,PADDLE_SIZE)){
            if(ball.x < p2.x) vx = -BALL_SPEED; else vx = BALL_SPEED;
            if(ball.y < p2.y) vy = -BALL_SPEED; else vy = BALL_SPEED;
            last = 2;
            _sys_playBeep(SYS_PLAY_BEEP, 900, 80);
        }

        if(rect_overlap(ball.x,ball.y,BALL_SIZE,BALL_SIZE,hole.x,hole.y,HOLE_SIZE,HOLE_SIZE)){
            _sys_playBeep(SYS_PLAY_BEEP, 600, 200);
            if(last==1) score1++; else if(last==2) score2++;
            ball.x = SCREEN_WIDTH/2 - BALL_SIZE/2;
            ball.y = SCREEN_HEIGHT/2 - BALL_SIZE/2;
            vx = vy = 0;
        }

        _sys_clearScreen(SYS_CLEAR_SCREEN);
        draw_score(score1, score2);

        _sys_drawRect(SYS_DRAW_RECT,0x00FF00,p1.x,p1.y,PADDLE_SIZE,PADDLE_SIZE);
        _sys_drawRect(SYS_DRAW_RECT,0xFF0000,p2.x,p2.y,PADDLE_SIZE,PADDLE_SIZE);
        _sys_drawRect(SYS_DRAW_RECT,0xFFFFFF,ball.x,ball.y,BALL_SIZE,BALL_SIZE);
        _sys_drawRect(SYS_DRAW_RECT,0x0000FF,hole.x,hole.y,HOLE_SIZE,HOLE_SIZE);

        _sys_sleep(SYS_SLEEP,2);
    }
}

