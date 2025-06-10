#include "syscalls.h"
#include <stdint.h>
#include "shell.h"

#define SYS_WRITE 0
#define SYS_READ 1
#define SYS_CLEAR_SCREEN 2
#define SYS_SLEEP 3
#define SYS_DRAW_RECT 4
#define SYS_PLAY_BEEP 8
#define SYS_FONT_SIZE 9

#define ESC 27

#define SCREEN_WIDTH 1024
#define SCREEN_HEIGHT 768

#define PADDLE_SIZE 40
#define BALL_SIZE 16
#define HOLE_SIZE 48

#define L1_OB_W 100
#define L1_OB_H 100
#define L1_OBS 2

#define L2_OB_W 60
#define L2_OB_H 60
#define L2_OBS 4

#define PADDLE_SPEED 10
#define BALL_SPEED 8

typedef struct {
    int x;
    int y;
} vec2i;

static vec2i *obstacles;
static int obstacle_count;
static int obstacle_w;
static int obstacle_h;

// Obstacles positions on screen
static vec2i level1_obstacles[L1_OBS] = {
    {SCREEN_WIDTH/3 - L1_OB_W/2, SCREEN_HEIGHT/2 - L1_OB_H/2},
    {2*SCREEN_WIDTH/3 - L1_OB_W/2, SCREEN_HEIGHT/2 - L1_OB_H/2}
};

static vec2i level2_obstacles[L2_OBS] = {
    {SCREEN_WIDTH/2 - L2_OB_W/2, SCREEN_HEIGHT/2 - 150},
    {SCREEN_WIDTH/4 - L2_OB_W/2, SCREEN_HEIGHT/3},
    {3*SCREEN_WIDTH/4 - L2_OB_W/2, SCREEN_HEIGHT/3},
    {SCREEN_WIDTH/2 - L2_OB_W/2, SCREEN_HEIGHT/2 + 80}
};

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

static int ask_players(){
    const char *msg = "Jugadores (1/2): ";
    _sys_write(SYS_WRITE, msg, str_len(msg));
    char c = 0;
    while(_sys_read(SYS_READ,0,&c,1)<=0 || (c!='1' && c!='2')){}
    _sys_write(SYS_WRITE, "\n", 1);
    return c-'0';
}

static void show_victory(int last, int players){
    _sys_clearScreen(SYS_CLEAR_SCREEN);
    _sys_changeFontSize(SYS_FONT_SIZE, 3);
    const char *msg;
    if(players==1){
        msg = "GANASTE!";
    }else{
        msg = last==1 ? "EL JUGADOR 1 GANO" : "EL JUGADOR 2 GANO";
    }
    _sys_write(SYS_WRITE, msg, str_len(msg));
    _sys_write(SYS_WRITE, "\n", 1);
    _sys_playBeep(8, 349.23, 400);  // F4 – negra
    _sys_playBeep(8, 349.23, 300);  // F4 – corchea puntillo (200+100)
    _sys_playBeep(8, 349.23, 100);  // F4 – fusa
    _sys_playBeep(8, 466.16, 400);  // Bb4 – negra
    _sys_playBeep(8, 466.16, 400);  // Bb4 – negra
    _sys_playBeep(8, 523.25, 400);  // C5 – negra
    _sys_playBeep(8, 523.25, 400);  // C5 – negra
    _sys_playBeep(8, 349.23, 600);  // F4 – negra puntillo (400+200)
    _sys_playBeep(8, 587.33, 200);  // D5 – corchea
    _sys_playBeep(8, 466.16, 400);  // Bb4 – negra

    // Continuación de la frase
    _sys_playBeep(8, 466.16, 400);  // Bb4 – negra
    _sys_playBeep(8, 587.33, 300);  // D5 – corchea puntillo
    _sys_playBeep(8, 466.16, 100);  // Bb4 – fusa
    _sys_playBeep(8, 392.00, 400);  // G4 – negra
    _sys_playBeep(8, 622.25, 800);  // Eb5 – blanca
    _sys_playBeep(8, 523.25, 300);  // C5 – corchea puntillo
    _sys_playBeep(8, 440.00, 100);  // A4 – fusa
    _sys_changeFontSize(SYS_FONT_SIZE, 1);
    _sys_sleep(SYS_SLEEP, 60);
}

static int rect_overlap(int x1,int y1,int w1,int h1,int x2,int y2,int w2,int h2){
    return !(x1+w1 < x2 || x2+w2 < x1 || y1+h1 < y2 || y2+h2 < y1);
}

static void clamp(int *v, int min, int max){
    if(*v < min) *v = min;
    if(*v > max) *v = max;
}

void pongis_game(){
    int players = ask_players();
    vec2i p1 = { 100, SCREEN_HEIGHT/2 - PADDLE_SIZE/2 };
    vec2i p2 = { SCREEN_WIDTH-100-PADDLE_SIZE, SCREEN_HEIGHT/2 - PADDLE_SIZE/2 };
    int score1 = 0, score2 = 0;
    int last = 0;

    char running = 1;
    for(int level=1; level<=2 && running; level++){
        if(level==1){
            obstacles = level1_obstacles;
            obstacle_count = L1_OBS;
            obstacle_w = L1_OB_W;
            obstacle_h = L1_OB_H;
        }else{
            obstacles = level2_obstacles;
            obstacle_count = L2_OBS;
            obstacle_w = L2_OB_W;
            obstacle_h = L2_OB_H;
        }
        vec2i ball = { SCREEN_WIDTH/2 - BALL_SIZE/2, SCREEN_HEIGHT/2 - BALL_SIZE/2 };
        vec2i hole = { SCREEN_WIDTH/2 - HOLE_SIZE/2, 60 };
        int vx = 0, vy = 0;
        running = 1;
        char levelWon = 0;
        while(running && !levelWon){
            // process all input characters available this frame
            char c = 0;
            while(_sys_read(SYS_READ,0,&c,1)>0){
                if(c==ESC || c=='q')
                    running = 0; // exit game
                if(c=='w') p1.y -= PADDLE_SPEED;
                if(c=='s') p1.y += PADDLE_SPEED;
                if(c=='a') p1.x -= PADDLE_SPEED;
                if(c=='d') p1.x += PADDLE_SPEED;
                if(players==2){
                    if(c=='i') p2.y -= PADDLE_SPEED;
                    if(c=='k') p2.y += PADDLE_SPEED;
                    if(c=='j') p2.x -= PADDLE_SPEED;
                    if(c=='l') p2.x += PADDLE_SPEED;
                }
            }
            if(!running) break;

            // store previous positions in case we collide with obstacles
            vec2i prev_p1 = p1;
            vec2i prev_p2 = p2;

        clamp(&p1.x,0,SCREEN_WIDTH-PADDLE_SIZE);
        clamp(&p1.y,0,SCREEN_HEIGHT-PADDLE_SIZE);
        clamp(&p2.x,0,SCREEN_WIDTH-PADDLE_SIZE);
        clamp(&p2.y,0,SCREEN_HEIGHT-PADDLE_SIZE);

        // prevent paddles from entering obstacles
        for(int i=0;i<obstacle_count;i++){
            if(rect_overlap(p1.x,p1.y,PADDLE_SIZE,PADDLE_SIZE,
                            obstacles[i].x,obstacles[i].y,obstacle_w,obstacle_h))
                p1 = prev_p1;
            if(rect_overlap(p2.x,p2.y,PADDLE_SIZE,PADDLE_SIZE,
                            obstacles[i].x,obstacles[i].y,obstacle_w,obstacle_h))
                p2 = prev_p2;
        }

        ball.x += vx;
        ball.y += vy;
        if(ball.x <=0 || ball.x+BALL_SIZE>=SCREEN_WIDTH){ vx=-vx; ball.x += vx; }
        if(ball.y <=0 || ball.y+BALL_SIZE>=SCREEN_HEIGHT){ vy=-vy; ball.y += vy; }

        // bounce off obstacles
        for(int i=0;i<obstacle_count;i++){
            if(rect_overlap(ball.x,ball.y,BALL_SIZE,BALL_SIZE,
                           obstacles[i].x,obstacles[i].y,obstacle_w,obstacle_h)){
                if(ball.x+BALL_SIZE <= obstacles[i].x || ball.x >= obstacles[i].x+obstacle_w)
                    vx = -vx;
                if(ball.y+BALL_SIZE <= obstacles[i].y || ball.y >= obstacles[i].y+obstacle_h)
                    vy = -vy;
                ball.x += vx;
                ball.y += vy;
            }
        }

        if(rect_overlap(ball.x,ball.y,BALL_SIZE,BALL_SIZE,p1.x,p1.y,PADDLE_SIZE,PADDLE_SIZE)){
            if(ball.x+p1.x+PADDLE_SIZE < ball.x+BALL_SIZE+p1.x){} //dummy
            if(ball.x < p1.x) vx = -BALL_SPEED; else vx = BALL_SPEED;
            if(ball.y < p1.y) vy = -BALL_SPEED; else vy = BALL_SPEED;
            last = 1;
            _sys_playBeep(SYS_PLAY_BEEP, 40, 80);
        }
        if(rect_overlap(ball.x,ball.y,BALL_SIZE,BALL_SIZE,p2.x,p2.y,PADDLE_SIZE,PADDLE_SIZE)){
            if(ball.x < p2.x) vx = -BALL_SPEED; else vx = BALL_SPEED;
            if(ball.y < p2.y) vy = -BALL_SPEED; else vy = BALL_SPEED;
            last = 2;
            _sys_playBeep(SYS_PLAY_BEEP, 40, 80);
        }

        if(rect_overlap(ball.x,ball.y,BALL_SIZE,BALL_SIZE,hole.x,hole.y,HOLE_SIZE,HOLE_SIZE)){
            _sys_playBeep(SYS_PLAY_BEEP, 600, 200);
            if(last==1) score1++; else if(last==2) score2++;
            levelWon = 1;
        }

        _sys_clearScreen(SYS_CLEAR_SCREEN);
        draw_score(score1, score2);

        _sys_drawRect(SYS_DRAW_RECT,0x00FF00,p1.x,p1.y,PADDLE_SIZE,PADDLE_SIZE);
        _sys_drawRect(SYS_DRAW_RECT,0xFF0000,p2.x,p2.y,PADDLE_SIZE,PADDLE_SIZE);
        _sys_drawRect(SYS_DRAW_RECT,0xFFFFFF,ball.x,ball.y,BALL_SIZE,BALL_SIZE);
        _sys_drawRect(SYS_DRAW_RECT,0x0000FF,hole.x,hole.y,HOLE_SIZE,HOLE_SIZE);
        for(int i=0;i<obstacle_count;i++)
            _sys_drawRect(SYS_DRAW_RECT,0x888888,obstacles[i].x,obstacles[i].y,obstacle_w,obstacle_h);

        _sys_sleep(SYS_SLEEP,2);
        }
        if(!running) break;
    }
    if(!running) return;
    show_victory(last, players);
}