
/*
	The save select box (for multiple save files).
*/

#include "../nx.h"
#include "../profile.h"
#include "../inventory.h"
#include "TextBox.h"	// for textbox coordinates; MSG_W etc
#include "SaveSelect.h"
#include "SaveSelect.fdh"

// moved here as static data so that the compiler will shut up about a circular dependency
// that happens if you try to include profile.h from SaveSelect.h.
static Profile fProfiles[MAX_SAVE_SLOTS];
static bool fHaveProfile[MAX_SAVE_SLOTS];
int fPicXOffset;

TB_SaveSelect::TB_SaveSelect()
{
}

/*
void c------------------------------() {}
*/

void TB_SaveSelect::ResetState()
{
	fVisible = false;
}

void TB_SaveSelect::SetVisible(bool enable, bool saving)
{
	fVisible = enable;
	if (!enable) return;
	game.showmapnametime = 0;
	
	fCoords.w = 244;
	fCoords.h = 152;

	if (widescreen)
	{
		fCoords.x = (SCREEN_WIDTH / 2) - (fCoords.w / 2);
		fCoords.y = 30;
	}
	else
	{
		fCoords.x = 38;
		fCoords.y = 8;
	}

	fNumFiles = MAX_SAVE_SLOTS;
	fSaving = saving;
	
	fCurSel = settings->last_save_slot;
	fPicXOffset = -24;
	
	// load all profiles
	memset(fHaveProfile, 0, sizeof(fHaveProfile));
	for(int i=0;i<fNumFiles;i++)
	{
		if (!profile_load(GetProfileName(i), &fProfiles[i]))
			fHaveProfile[i] = true;
	}
	
	textbox.ClearText();
}

bool TB_SaveSelect::IsVisible()
{
	return fVisible;
}

/*
void c------------------------------() {}
*/

void TB_SaveSelect::Run_Input()
{
int start;

	if (justpushed(DOWNKEY))
	{
		start = fCurSel;
		for(;;)
		{
			fCurSel++;
			if (fCurSel >= fNumFiles) fCurSel = 0;
			
			if (fSaving) break;
			if (fHaveProfile[fCurSel]) break;
			if (fCurSel == start) break;
		}
		
		sound(SND_MENU_MOVE);
		fPicXOffset = -24;
	}
	
	if (justpushed(UPKEY))
	{
		start = fCurSel;
		for(;;)
		{
			fCurSel--;
			if (fCurSel < 0) fCurSel = fNumFiles - 1;
			
			if (fSaving) break;
			if (fHaveProfile[fCurSel]) break;
			if (fCurSel == start) break;
		}
		
		sound(SND_MENU_MOVE);
		fPicXOffset = -24;
	}
	
	if (buttonjustpushed())
	{
		if (fSaving)
			game_save(fCurSel);

		settings->last_save_slot = fCurSel;
		settings_save();		// record new save/load slot
		
		SetVisible(false);
		
		// when the script hit the <SVP, it froze itself in an artifical <WAI9999
		// waiting for us to complete. Now unfreeze it so can say "Game saved.",
		// or for loading, just end.
		ScriptInstance *s = GetCurrentScriptInstance();
		if (s) s->delaytimer = 0;
	}
}


void TB_SaveSelect::DrawProfile(int x, int y, int index)
{
Profile *p = &fProfiles[index];
const int w = fCoords.w - 33;

	int sidewd = sprites[SPR_SAVESELECTOR_SIDES].w;
	int repeatwd = w - (sidewd * 2);
	int frame = (index == fCurSel) ? 0 : 1;
	
	draw_sprite(x, y, SPR_SAVESELECTOR_SIDES, frame, LEFT);
	draw_sprite_repeating_x(x+sidewd, y, SPR_SAVESELECTOR_MIDDLE, frame, repeatwd);
	draw_sprite(x+sidewd+repeatwd, y, SPR_SAVESELECTOR_SIDES, frame, RIGHT);
	
	y += 4;
	const int FONT_SPACING = 5;
	
	if (fHaveProfile[index])
	{
		const char *stage = map_get_stage_name(p->stage);
		font_draw(x+8, y-1, stage, FONT_SPACING);
		
		// draw health.
		DrawHealth(x+w, y, p);
	}
	else if (fCurSel == index)
	{
		const char *str = "available";
		int fx = (w / 2) - (GetFontWidth(str, FONT_SPACING) / 2);
		font_draw(x+fx, y-1, str, FONT_SPACING);
	}
}


void TB_SaveSelect::DrawExtendedInfo()
{
Profile *p = &fProfiles[fCurSel];
int x, y, s;
	
	if (fPicXOffset < 0)
	{
		fPicXOffset += 8;
		set_clip_rect(MSG_X+4, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	}
	
	// player pic
	draw_sprite((MSG_X+8) + fPicXOffset, MSG_NORMAL_Y+8, SPR_SELECTOR_ARMS);
	
	x = (MSG_X + 12) + fPicXOffset;
	y = MSG_NORMAL_Y + 12;
	s = (p->equipmask & EQUIP_MIMIGA_MASK) ? SPR_MYCHAR_MIMIGA : SPR_MYCHAR;
	
	draw_sprite(x, y, s, 0, RIGHT);
	
	// player gun
	if (p->curWeapon != WPN_NONE && p->curWeapon != WPN_BLADE)
	{
		int spr, frame;
		GetSpriteForGun(p->curWeapon, 0, &spr, &frame);
		
		draw_sprite_at_dp(x + sprites[s].frame[0].dir[RIGHT].actionpoint.x, \
						  y + sprites[s].frame[0].dir[RIGHT].actionpoint.y, \
						  spr, frame, RIGHT);
	}
	
	clear_clip_rect();
	
	// whimsical stars
	if (p->equipmask & EQUIP_WHIMSTAR)
	{
		x = MSG_X + 12;
		
		for(int i=0;i<3;i++)
		{
			static int frames[] = { 1, 0, 2 };
			draw_sprite(x, y+20, SPR_WHIMSICAL_STAR, frames[i]);
			x += 10;
		}
	}
	
	// WEAPONS:
	x = MSG_X + 64;
	y = MSG_NORMAL_Y + 8;
	
	// weapon list
	for(int i=0;i<WPN_COUNT;i++)
	{
		if (p->weapons[i].hasWeapon)
		{
			draw_sprite(x, y, SPR_ARMSICONS, i);
			x += 20;
		}
	}
	
	// xp of current weapon
	if (p->curWeapon != WPN_NONE)
	{
		int xb = MSG_X + 64;
		int yb = MSG_NORMAL_Y + 26;
		
		int level = p->weapons[p->curWeapon].level;
		int curxp = p->weapons[p->curWeapon].xp;
		int maxxp = player->weapons[p->curWeapon].max_xp[level];
		
		draw_sprite(xb, yb, SPR_XPLEVELICON); xb += 16;
		draw_sprite(xb, yb, SPR_WHITENUMBERS, level+1); xb += 8;
		draw_sprite(xb, yb, SPR_XPBAR);
		
		if ((curxp == maxxp) && level == 2)
			draw_sprite(xb, yb, SPR_XPBAR, 3);		// MAX
		else
			DrawPercentage(xb, yb, SPR_XPBAR, 1, curxp, maxxp, sprites[SPR_XPBAR].w);
	}
	
	// ITEMS:
	x = (MSG_X + 64) - 10;
	y = MSG_NORMAL_Y + 40;
	
	// Booster
	// items list. I generally tried to put the ones that are temporary and indicate a
	// quantity of stage completion at the front so they'll be more likely to be visible.
	static int items[] = {
		ITEM_BOOSTER08,
		ITEM_BOOSTER20,
		ITEM_LIFE_POT,
		ITEM_PUPPY,
		ITEM_JELLYFISH_JUICE,
		ITEM_CHARCOAL,
		ITEM_GUM_BASE,
		ITEM_EXPLOSIVE,
		ITEM_SPRINKLER,
		ITEM_CONTROLLER,
		ITEM_MA_PIGNON,
		ITEM_LITTLE_MAN,
		-1
	};
	for(int i=0;items[i] != -1;i++)
	{
		if (CheckInventoryList(items[i], p->inventory, p->ninventory) != -1)
		{
			draw_sprite(x, y, SPR_ITEMIMAGE, items[i]);
			x += 28;
			
			if (x + sprites[SPR_ITEMIMAGE].w > (MSG_X + MSG_W) - 8)
				break;
		}
	}
	
	// health
	DrawHealth((MSG_X+MSG_W) - 4, MSG_NORMAL_Y+8, p);
	
}


// I don't think it's possible to get 3-digit max life
// without hacking, but I accounted for it anyway.
static void DrawHealth(int xright, int y, Profile *p)
{
int hx, len;
	
	hx = xright - 28 - 24;
	if (p->maxhp > 99) { hx -= 8; }
	len = (p->hp > 99) ? 8 : 0;
	draw_sprite(hx-len, y-1, SPR_SS_HEALTH_ICON, 0, 0);
	DrawNumberRAlign(hx+24, y, SPR_WHITENUMBERS, p->hp);
	
	hx = xright - 4;
	len = (p->maxhp > 99) ? 32 : 24;
	draw_sprite(hx-len, y, SPR_WHITENUMBERS, 11);		// '/' character
	DrawNumberRAlign(hx, y, SPR_WHITENUMBERS, p->maxhp);
}



void TB_SaveSelect::Draw(void)
{
	if (!fVisible)
		return;
	
	// handle user input
	Run_Input();
	
	// draw frame
	TextBox::DrawFrame(fCoords.x, fCoords.y, fCoords.w, fCoords.h);
	
	// draw selectors/options
	int x = fCoords.x + 16;
	int y = fCoords.y + 15;
	
	for(int i=0;i<fNumFiles;i++)
	{
		DrawProfile(x, y, i);
		y += (sprites[SPR_SAVESELECTOR_MIDDLE].h + 10);
	}
	
	// draw extended info for current selection
	if (fHaveProfile[fCurSel])
		DrawExtendedInfo();
}











