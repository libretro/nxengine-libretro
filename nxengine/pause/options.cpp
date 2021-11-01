/* vim: set shiftwidth=3 tabstop=3 textwidth=80 expandtab: */
#include "../nx.h"
#include "options.h"
#include "dialog.h"
#include "message.h"
using namespace Options;
#include "options.fdh"
#include "libretro_shared.h"
FocusStack optionstack;

#define SLIDE_SPEED				32
#define SLIDE_DIST				(3 * SLIDE_SPEED)

static struct
{
	Dialog *dlg, *subdlg;
	Dialog *dismiss_on_focus;
	int mm_cursel;
	bool InMainMenu;
	int xoffset;
	
	int remapping_key, new_sdl_key;
} opt;


bool options_init(int retmode)
{
   memset(&opt, 0, sizeof(opt));
   Options::init_objects();
   opt.dlg = new Dialog;

   opt.xoffset = SLIDE_DIST;
   opt.dlg->offset(-SLIDE_DIST, 0);

   EnterMainMenu();
   opt.dlg->ondismiss = DialogDismissed;
   opt.dlg->ShowFull();

   inputs[F3KEY] = 0;
   sound(SND_MENU_MOVE);
   return 0;
}

void options_close()
{
	Options::close_objects();
	while(FocusHolder *fh = optionstack.ItemAt(0))
		delete fh;
	
	settings_save(NULL);
}

void options_tick()
{
   int i;
   FocusHolder *fh;

   if (justpushed(F3KEY))
   {
      game.pause(0);
      return;
   }

   Graphics::ClearScreen(BLACK);
   Options::run_and_draw_objects();

   fh = optionstack.ItemAt(optionstack.CountItems() - 1);
   if (fh)
   {
      fh->RunInput();
      if (game.paused != GP_OPTIONS) return;

      fh = optionstack.ItemAt(optionstack.CountItems() - 1);
      if (fh == opt.dismiss_on_focus && fh)
      {
         opt.dismiss_on_focus = NULL;
         delete fh;
      }
   }

   for(i=0;;i++)
   {
      fh = optionstack.ItemAt(i);
      if (!fh) break;

      fh->Draw();
   }

   if (opt.xoffset > 0)
   {
      opt.dlg->offset(SLIDE_SPEED, 0);
      opt.xoffset -= SLIDE_SPEED;
   }
}

void DialogDismissed()
{
   if (opt.InMainMenu)
   {
      game.pause(false);
      /*
       * This leaks inputs to other modes.
       * How did nxengine-evo avoid leaking inputs to other modes with this?
       */
      /* memset(inputs, 0, sizeof(inputs)); */
   }
   else
      EnterMainMenu();
}

void _60hz_change(ODItem *item, int dir)
{
   extern bool retro_60hz;
   retro_60hz ^= 1;
}

void _60hz_get(ODItem *item)
{
   extern bool retro_60hz;
   static const char *strs[] = { "50fps", "60fps" };
   strcpy(item->suffix, strs[retro_60hz]);
}

/*
void c------------------------------() {}
*/

static void EnterMainMenu()
{
   Dialog *dlg = opt.dlg;

   dlg->Clear();

   dlg->AddItem("Framerate: ", _60hz_change, _60hz_get);

   dlg->AddSeparator();

   //dlg->AddItem("Enable Debug Keys", _debug_change, _debug_get);
   //dlg->AddItem("Save Slots: ", _save_change, _save_get);

   dlg->AddSeparator();

   dlg->AddItem("Music: ", _music_change, _music_get);
   dlg->AddItem("Sound: ", _sound_change, _sound_get);

   dlg->AddSeparator();
   dlg->AddDismissalItem();

   dlg->SetSelection(opt.mm_cursel);
   dlg->onclear = LeavingMainMenu;
   opt.InMainMenu = true;
}

void LeavingMainMenu()
{
	opt.mm_cursel = opt.dlg->GetSelection();
	opt.dlg->onclear = NULL;
	opt.InMainMenu = false;
}

void _sound_change(ODItem *item, int dir)
{
	settings->sound_enabled ^= 1;
	sound(SND_MENU_SELECT);
}

void _sound_get(ODItem *item)
{
	static const char *strs[] = { "Off", "On" };
	strcpy(item->suffix, strs[settings->sound_enabled]);
}

void _music_change(ODItem *item, int dir)
{
	music_set_enabled((settings->music_enabled + 1) % 3);
	sound(SND_MENU_SELECT);
}

void _music_get(ODItem *item)
{
	static const char *strs[] = { "Off", "On", "Boss Only" };
	strcpy(item->suffix, strs[settings->music_enabled]);
}
