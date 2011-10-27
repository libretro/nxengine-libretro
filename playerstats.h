#ifndef _PLAYER_STATS_H
#define _PLAYER_STATS_H

void GetWeapon(int wpn, int ammo);
void LoseWeapon(int wpn);
void TradeWeapon(int oldwpn, int newwpn, int ammo);
void AddInventory(int item);
void DelInventory(int item);
int FindInventory(int item);
void AddHealth(int hp);
void RefillAllAmmo(void);

#endif
