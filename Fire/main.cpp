#include <iostream>
#include <graphics.h>
#include <vector>
#include <conio.h>
#include <tchar.h>
using namespace std;

constexpr auto screen_width = 600;
constexpr auto screen_height = 1100;
constexpr unsigned int SHP = 4;
constexpr auto hurttime = 1000;

bool IfPointInRect(int x, int y, RECT& r) {
    return (r.left <= x && x <= r.right && r.top <= y && y <= r.bottom);
}
bool RectDuangRect(RECT& r1, RECT& r2) {
    RECT r;
    r.left = r1.left - (r2.right - r2.left);
    r.right = r1.right;
    r.top = r1.top - (r2.bottom - r2.top);
    r.bottom = r1.bottom;
    return (r.left < r2.left && r2.left <= r.right && r.top <= r2.top && r2.top <= r.bottom);
}

void menu() {
    LPCTSTR title = _T("ACE COMBAT");
    LPCTSTR tplay = _T("START");
    LPCTSTR texit = _T("QUIT");

    RECT tplayr, texitr;
    BeginBatchDraw();
    setbkcolor(WHITE);
    cleardevice();
    settextstyle(60, 0, _T("黑体"));
    settextcolor(BLACK);
    outtextxy(screen_width / 2 - textwidth(title) / 2, screen_height / 5, title);

    settextstyle(40, 0, _T("黑体"));
    tplayr.left = screen_width / 2 - textwidth(tplay) / 2;
    tplayr.right = tplayr.left + textwidth(tplay);
    tplayr.top = screen_height / 5 * 2.5;
    tplayr.bottom = tplayr.top + textheight(tplay);

    texitr.left = screen_width / 2 - textwidth(texit) / 2;
    texitr.right = texitr.left + textwidth(texit);
    texitr.top = screen_height / 5 * 3;
    texitr.bottom = texitr.top + textheight(texit);

    outtextxy(tplayr.left, tplayr.top, tplay);
    outtextxy(texitr.left, texitr.top, texit);
    EndBatchDraw();

    while (true) {
        ExMessage mess;
        getmessage(&mess, EM_MOUSE);
        if (mess.lbutton) {
            if (IfPointInRect(mess.x, mess.y, tplayr)) return;
            else if (IfPointInRect(mess.x, mess.y, texitr)) exit(0);
        }
    }
}

void Over(unsigned long long& kill) {
    TCHAR str[128];
    _stprintf_s(str, 128, _T("RESULT：%llu"), kill);
    settextcolor(RED);
    settextstyle(40, 0, _T("黑体"));
    outtextxy(screen_width / 2 - textwidth(str) / 2, screen_height / 5, str);

    LPCTSTR info = _T("按Enter返回");
    settextstyle(20, 0, _T("黑体"));
    outtextxy(screen_width - textwidth(info), screen_height - textheight(info), info);

    while (true) {
        ExMessage mess;
        getmessage(&mess, EM_KEY);
        if (mess.vkcode == 0x0D) return;
    }
}

// 得分  关卡 
inline void ShowHUD(unsigned long long kill, int level) {
    setbkmode(TRANSPARENT);
    settextcolor(WHITE);
    settextstyle(24, 0, _T("黑体"));

    TCHAR scorebuf[64];
    _stprintf_s(scorebuf, _T("SCORE：%llu"), kill);
    outtextxy(10, 10, scorebuf);

    TCHAR lvlbuf[64];
    _stprintf_s(lvlbuf, _T("STAGE：%02d"), level);
    outtextxy(10, 40, lvlbuf);

    setbkmode(OPAQUE);
}

// 背景、英雄、敌机、子弹 
class background {
public:
    background(IMAGE& img) : img(img), y(-screen_height) {}
    void Show() {
        if (y == 0) y = -screen_height;
        y += 4;
        putimage(0, y, &img);
    }
private:
    IMAGE& img; int y;
};

class hero {
public:
    hero(IMAGE& img) : img(img), HP(SHP) {
        rect.left = screen_width / 2 - img.getwidth() / 2;
        rect.top = screen_height - img.getheight();
        rect.right = rect.left + img.getwidth();
        rect.bottom = screen_height;
    }
    void Show() {
        setlinecolor(RED);
        setlinestyle(PS_SOLID, 4);
        putimage(rect.left, rect.top, &img);
        line(rect.left, rect.top - 5, rect.left + (img.getwidth() / SHP * HP), rect.top - 5);
    }
    void Control() {
        ExMessage mess;
        if (peekmessage(&mess, EM_MOUSE)) {
            rect.left = mess.x - img.getwidth() / 2;
            rect.top = mess.y - img.getheight() / 2;
            rect.right = rect.left + img.getwidth();
            rect.bottom = rect.top + img.getheight();
        }
    }
    bool hurt() { HP--; return (HP == 0) ? false : true; }
    RECT& GetRect() { return rect; }
private:
    IMAGE& img; RECT rect; unsigned int HP;
};

class enemy {
public:
    enemy(IMAGE& img, int x, IMAGE*& boom)
        : img(img), isdie(false), boomsum(0) {
        selfboom[0] = boom[0];
        selfboom[1] = boom[1];
        selfboom[2] = boom[2];
        rect.left = x;
        rect.right = rect.left + img.getwidth();
        rect.top = -img.getheight();
        rect.bottom = 0;
    }
    bool Show() {
        if (isdie) {
            if (boomsum == 3) return false;
            putimage(rect.left, rect.top, selfboom + boomsum);
            boomsum++;
            return true;
        }
        if (rect.top >= screen_height) return false;
        rect.top += 4; rect.bottom += 4;
        putimage(rect.left, rect.top, &img);
        return true;
    }
    void Isdie() { isdie = true; }
    RECT& GetRect() { return rect; }
private:
    IMAGE& img; RECT rect; IMAGE selfboom[3]; bool isdie; int boomsum;
};

class enemy2 {
public:
    enemy2(IMAGE& img, int x, IMAGE*& boom)
        : img(img), isdie(false), boomsum(0), hp(2) {
        selfboom[0] = boom[0];
        selfboom[1] = boom[1];
        selfboom[2] = boom[2];
        rect.left = x;
        rect.right = rect.left + img.getwidth();
        rect.top = -img.getheight();
        rect.bottom = 0;
    }
    bool Show() {
        if (isdie) {
            if (boomsum == 3) return false;
            putimage(rect.left, rect.top, selfboom + boomsum);
            boomsum++;
            return true;
        }
        if (rect.top >= screen_height) return false;
        rect.top += 4; rect.bottom += 4;
        putimage(rect.left, rect.top, &img);
        return true;
    }
    bool Hit() {
        if (isdie) return false;
        hp--;
        if (hp <= 0) { isdie = true; return true; }
        return false;
    }
    RECT& GetRect() { return rect; }
private:
    IMAGE& img; RECT rect; IMAGE selfboom[3]; bool isdie; int boomsum; int hp;
};

class bullet {
public:
    bullet(IMAGE& img, RECT pr) : img(img) {
        rect.left = pr.left + (pr.right - pr.left) / 2 - img.getwidth() / 2;
        rect.right = rect.left + img.getwidth();
        rect.top = pr.top - img.getheight();
        rect.bottom = rect.top + img.getheight();
    }
    bool Show() {
        if (rect.bottom <= 0) return false;
        rect.top -= 3; rect.bottom -= 3;
        putimage(rect.left, rect.top, &img);
        return true;
    }
    RECT& GetRect() { return rect; }
protected:
    IMAGE& img; RECT rect;
};

class EBullet : public bullet {
public:
    EBullet(IMAGE& img, RECT pr, int cx_offset = 0) : bullet(img, pr) {
        int cx = pr.left + (pr.right - pr.left) / 2 + cx_offset;
        rect.left = cx - img.getwidth() / 2;
        rect.right = rect.left + img.getwidth();
        rect.top = pr.bottom;
        rect.bottom = rect.top + img.getheight();
    }
    bool Show() {
        if (rect.top >= screen_height) return false;
        rect.top += 5; rect.bottom += 5;
        putimage(rect.left, rect.top, &img);
        return true;
    }
};

//生成敌人
bool AddEnemy(vector<enemy*>& es, IMAGE& enemyimg, IMAGE* boom) {
    enemy* e = new enemy(enemyimg, abs(rand()) % (screen_width - enemyimg.getwidth()), boom);
    for (auto& i : es) {
        if (RectDuangRect(i->GetRect(), e->GetRect())) {
            delete e; return false;
        }
    }
    es.push_back(e);
    return true;
}

bool AddEnemy2(vector<enemy2*>& es2, IMAGE& enemy2img, IMAGE* boom2) {
    enemy2* e = new enemy2(enemy2img, abs(rand()) % (screen_width - enemy2img.getwidth()), boom2);
    for (auto& i : es2) {
        if (RectDuangRect(i->GetRect(), e->GetRect())) {
            delete e; return false;
        }
    }
    es2.push_back(e);
    return true;
}
//关卡的推进
void ResetLevel(vector<enemy*>& es, vector<enemy2*>& es2, vector<bullet*>& bs, vector<EBullet*>& ebs,
    IMAGE& enemyimg, IMAGE* eboom, IMAGE& enemy2img, IMAGE* e2boom, int spawn_num = 5) {
    for (auto p : es) delete p; es.clear();
    for (auto p : es2) delete p; es2.clear();
    for (auto p : bs) delete p; bs.clear();
    for (auto p : ebs) delete p; ebs.clear();
    for (int i = 0; i < spawn_num; i++) {
        if (i % 2 == 0) AddEnemy2(es2, enemy2img, e2boom);
        else AddEnemy(es, enemyimg, eboom);
    }
}

//游戏主循环
bool game() {
    setbkcolor(WHITE);
    cleardevice();
    bool is_play = true;

    IMAGE heroimg, enemyimg, enemy2img, bkimg, bulletimg;
    IMAGE eboom[3], e2boom[3];

    loadimage(&heroimg, _T("../images/me1.png"));
    loadimage(&enemyimg, _T("../images/enemy1.png"));
    loadimage(&enemy2img, _T("../images/enemy2.png"));
    loadimage(&bkimg, _T("../images/bk2.png"), screen_width, screen_height * 2);
    loadimage(&bulletimg, _T("../images/bullet1.png"));
    loadimage(&eboom[0], _T("../images/enemy1_down2.png"));
    loadimage(&eboom[1], _T("../images/enemy1_down3.png"));
    loadimage(&eboom[2], _T("../images/enemy1_down4.png"));
    loadimage(&e2boom[0], _T("../images/enemy2_down2.png"));
    loadimage(&e2boom[1], _T("../images/enemy2_down3.png"));
    loadimage(&e2boom[2], _T("../images/enemy2_down4.png"));

    background bk = background(bkimg);
    hero hp = hero(heroimg);

    vector<enemy*> es;
    vector<enemy2*> es2;
    vector<bullet*> bs;
    vector<EBullet*> ebs;
    int bsing = 0;

    clock_t hurtlast = clock();

    unsigned long long kill = 0;   // 当前关的击杀/得分
    int level = 1;                 // 关卡：1,2,3
    const int KILL_TO_NEXT = 10;   // 每关需要的击杀数

    for (int i = 0; i < 5; i++) {
        if (i % 2 == 0) AddEnemy2(es2, enemy2img, e2boom);
        else AddEnemy(es, enemyimg, eboom);
    }

    while (is_play) {
        bsing++;
        if (bsing % 10 == 0) bs.push_back(new bullet(bulletimg, hp.GetRect()));
        if (bsing == 60) {
            bsing = 0;
            for (auto& i : es) ebs.push_back(new EBullet(bulletimg, i->GetRect(), 0));
            for (auto& j : es2) {
                ebs.push_back(new EBullet(bulletimg, j->GetRect(), -15));
                ebs.push_back(new EBullet(bulletimg, j->GetRect(), +15));
            }
        }

        BeginBatchDraw();

        bk.Show();
        Sleep(2);
        flushmessage();
        Sleep(2);
        hp.Control();

        if (_kbhit()) {
            char v = _getch();
            if (v == 0x20) {
                Sleep(500);
                while (true) {
                    if (_kbhit()) {
                        v = _getch();
                        if (v == 0x20) break;
                    }
                    Sleep(16);
                }
            }
        }
        hp.Show();

        for (auto it = bs.begin(); it != bs.end();) {
            if (!(*it)->Show()) it = bs.erase(it);
            else ++it;
        }
        for (auto it = ebs.begin(); it != ebs.end();) {
            if (!(*it)->Show()) it = ebs.erase(it);
            else {
                if (RectDuangRect((*it)->GetRect(), hp.GetRect())) {
                    if (clock() - hurtlast >= hurttime) {
                        is_play = hp.hurt();
                        hurtlast = clock();
                    }
                }
                ++it;
            }
        }

        for (auto it = es.begin(); it != es.end();) {
            if (RectDuangRect((*it)->GetRect(), hp.GetRect())) {
                if (clock() - hurtlast >= hurttime) {
                    is_play = hp.hurt();
                    hurtlast = clock();
                }
            }
            bool killed_now = false;
            for (auto bit = bs.begin(); bit != bs.end(); ) {
                if (RectDuangRect((*bit)->GetRect(), (*it)->GetRect())) {
                    (*it)->Isdie();
                    delete (*bit);
                    bit = bs.erase(bit);
                    kill++;
                    killed_now = true;
                    break;
                }
                else ++bit;
            }
            if (!(*it)->Show()) {
                delete (*it);
                it = es.erase(it);
                continue;
            }
            ++it;

            if (killed_now && kill >= (unsigned)KILL_TO_NEXT) {
                level++;
                if (level <= 3) {
                    kill = 0;
                    ResetLevel(es, es2, bs, ebs, enemyimg, eboom, enemy2img, e2boom, 5);
                }
                else {
                    is_play = false;
                }
                break;
            }
        }

        for (auto it = es2.begin(); it != es2.end();) {
            if (RectDuangRect((*it)->GetRect(), hp.GetRect())) {
                if (clock() - hurtlast >= hurttime) {
                    is_play = hp.hurt();
                    hurtlast = clock();
                }
            }
            bool killed_now = false;
            for (auto bit = bs.begin(); bit != bs.end(); ) {
                if (RectDuangRect((*bit)->GetRect(), (*it)->GetRect())) {
                    bool dead = (*it)->Hit();
                    delete (*bit);
                    bit = bs.erase(bit);
                    if (dead) {
                        kill++;
                        killed_now = true;
                    }
                    break;
                }
                else ++bit;
            }
            if (!(*it)->Show()) {
                delete (*it);
                it = es2.erase(it);
                continue;
            }
            ++it;

            if (killed_now && kill >= (unsigned)KILL_TO_NEXT) {
                level++;
                if (level <= 3) {
                    kill = 0;
                    ResetLevel(es, es2, bs, ebs, enemyimg, eboom, enemy2img, e2boom, 5);
                }
                else {
                    is_play = false;
                }
                break;
            }
        }

        while ((int)es.size() + (int)es2.size() < 5) {
            if (((int)es.size() + (int)es2.size()) % 2 == 0) AddEnemy2(es2, enemy2img, e2boom);
            else AddEnemy(es, enemyimg, eboom);
        }

        ShowHUD(kill, level);

        EndBatchDraw();
    }

    for (auto p : es) delete p;
    for (auto p : es2) delete p;
    for (auto p : bs) delete p;
    for (auto p : ebs) delete p;

    Over(kill);
    return true;
}

int main() {
    initgraph(screen_width, screen_height, EW_NOMINIMIZE | EW_SHOWCONSOLE);
    bool is_play = true;
    while (is_play) {
        menu();
        is_play = game();
    }
    return 0;
}
