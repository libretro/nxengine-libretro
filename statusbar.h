
#ifndef _STATUSBAR_H
#define _STATUSBAR_H

struct PercentBar
{
	int displayed_value;
	int dectimer;
};

struct StatusBar
{
	int xpflashcount;
	int xpflashstate;
};

extern StatusBar statusbar;
void niku_draw(int value, bool force_white=false);

void stat_PrevWeapon(bool quiet=false);
void stat_NextWeapon(bool quiet=false);
void weapon_introslide(void);
void DrawStatusBar(void);
void niku_run(void);
bool statusbar_init(void);
void DrawWeaponLevel(int x, int y, int wpn);
void DrawWeaponAmmo(int x, int y, int wpn);
void weapon_slide(int dir, int newwpn);

#endif
