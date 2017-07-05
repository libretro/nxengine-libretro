
#include "../nx.h"
#include "dialog.h"
#include "dialog.fdh"
using namespace Options;
extern FocusStack optionstack;

#define REPEAT_WAIT	30
#define REPEAT_RATE	4

Dialog::Dialog()
{
	
	if (widescreen)
	{
		DLG_X = ((SCREEN_WIDTH / 2) - 110);
		DLG_Y = ((SCREEN_HEIGHT / 2) - 90);
		DLG_W = 240;
		DLG_H = 180;
	}
	else
	{
		DLG_X = ((SCREEN_WIDTH / 2) - 88);
		DLG_Y = ((SCREEN_HEIGHT / 2) - 90);
		DLG_W = 190;
		DLG_H = 180;
	}

	onclear = NULL;
	ondismiss = NULL;
	
	fCoords.x = DLG_X;
	fCoords.y = DLG_Y;
	fCoords.w = DLG_W;
	fCoords.h = DLG_H;
	fTextX = (fCoords.x + 48);
	
	fCurSel = 0;
	fNumShown = 0;
	fRepeatTimer = 0;
	
	optionstack.AddItem(this);
}

Dialog::~Dialog()
{
	ODItem *item;
	for(int i=0; item = ItemAt(i);i++)
		delete item;
	
	optionstack.RemoveItem(this);
}

void Dialog::SetSize(int w, int h)
{
	fCoords.w = w;
	fCoords.h = h;
	fCoords.x = ((DLG_W / 2) - (w / 2)) + DLG_X;
	fCoords.y = ((DLG_H / 2) - (h / 2)) + DLG_Y;
	fTextX = (fCoords.x + 34);
}

void Dialog::offset(int xd, int yd)
{
	fCoords.x += xd;
	fCoords.y += yd;
	fTextX += xd;
}

/*
void c------------------------------() {}
*/

ODItem *Dialog::AddItem(const char *text, \
						void (*activate)(ODItem *, int), \
						void (*update)(ODItem *),\
						int id, int type)
{
	ODItem *item = new ODItem;
	memset(item, 0, sizeof(ODItem));
	
	strcpy(item->text, text);
	
	item->activate = activate;
	item->update = update;
	item->id = id;
	item->type = type;
	
	fItems.AddItem(item);
	
	if (update)
		(*update)(item);
	
	return item;
}

ODItem *Dialog::AddSeparator()
{
	return AddItem("", NULL, NULL, -1, OD_SEPARATOR);
}

ODItem *Dialog::AddDismissalItem(const char *text)
{
	if (!text) text = "Return";
	return AddItem(text, NULL, NULL, -1, OD_DISMISS);
}

/*
void c------------------------------() {}
*/

void Dialog::Draw()
{
	TextBox::DrawFrame(fCoords.x, fCoords.y, fCoords.w, fCoords.h);
	
	int x = fTextX;
	int y = (fCoords.y + 18);
	for(int i=0;;i++)
	{
		ODItem *item = (ODItem *)fItems.ItemAt(i);
		if (!item) break;
		
		if (i < fNumShown)
			DrawItem(x, y, item);
		
		if (i == fCurSel)
			draw_sprite(x - 16, y, SPR_WHIMSICAL_STAR, 1);
		
		y += GetFontHeight();
	}
	
	if (fNumShown < 99)
		fNumShown++;
}

void Dialog::DrawItem(int x, int y, ODItem *item)
{
char text[132];

	strcpy(text, item->text);
	strcat(text, item->suffix);
	
	font_draw(x, y, text, 0);
	
	// for key remaps
	if (item->righttext[0])
	{
		font_draw((fCoords.x + fCoords.w) - 62, y, item->righttext, 0);
	}
}


/*
void c------------------------------() {}
*/

void Dialog::RunInput()
{
	if (inputs[UPKEY] || inputs[DOWNKEY])
	{
		int dir = (inputs[DOWNKEY]) ? 1 : -1;
		
		if (!fRepeatTimer)
		{
			fRepeatTimer = (lastinputs[UPKEY] || lastinputs[DOWNKEY]) ? REPEAT_RATE : REPEAT_WAIT;
			sound(SND_MENU_MOVE);
			
			int nitems = fItems.CountItems();
			for(;;)
			{
				fCurSel += dir;
				if (fCurSel < 0) fCurSel = (nitems - 1);
							else fCurSel %= nitems;
				
				ODItem *item = ItemAt(fCurSel);
				if (item && item->type != OD_SEPARATOR) break;
			}
		}
		else fRepeatTimer--;
	}
	else fRepeatTimer = 0;
	
	
	if (buttonjustpushed() || justpushed(RIGHTKEY) || justpushed(LEFTKEY))
	{
		int dir = (!inputs[LEFTKEY] || buttonjustpushed() || justpushed(RIGHTKEY)) ? 1 : -1;
		
		ODItem *item = ItemAt(fCurSel);
		if (item)
		{
			if (item->type == OD_DISMISS)
			{
				if (dir > 0)
				{
					sound(SND_MENU_MOVE);
					if (ondismiss) (*ondismiss)();
					return;
				}
			}
			else if (item->activate)
			{
				(*item->activate)(item, dir);
				
				if (item->update)
					(*item->update)(item);
			}
		}
	}
	
	/*if (justpushed(ESCKEY))
	{
		Dismiss();
		return;
	}*/
}

void Dialog::SetSelection(int sel)
{
	if (sel < 0) sel = fItems.CountItems();
	if (sel >= fItems.CountItems()) sel = (fItems.CountItems() - 1);
	
	fCurSel = sel;
}


void Dialog::Dismiss()
{
	delete this;
}

void Dialog::Refresh()
{
	ODItem *item;
	for(int i=0; item = ItemAt(i);i++)
	{
		if (item->update)
			(*item->update)(item);
	}
}

void Dialog::Clear()
{
	if (onclear)
		(*onclear)();
	
	ODItem *item;
	for(int i=0; item = ItemAt(i);i++)
		delete item;
	
	fItems.MakeEmpty();
	fNumShown = 0;
	fCurSel = 0;
}




