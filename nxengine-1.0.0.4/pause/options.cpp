
#include "../nx.h"
#include "../replay.h"
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
	
	int selected_replay;
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
	
	settings_save();
}

/*
void c------------------------------() {}
*/

void options_tick()
{
int i;
FocusHolder *fh;

	if (justpushed(F3KEY))
	{
		game.pause(0);
		return;
	}
	
	ClearScreen(BLACK);
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


/*
void c------------------------------() {}
*/

void DialogDismissed()
{
	if (opt.InMainMenu)
	{
		memset(inputs, 0, sizeof(inputs));
		game.pause(false);
	}
	else
	{
		EnterMainMenu();
	}
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
	dlg->AddItem("Replay", EnterReplayMenu);
	
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

#if 0
void _debug_change(ODItem *item, int dir)
{
	settings->enable_debug_keys ^= 1;
	sound(SND_MENU_SELECT);
}

void _debug_get(ODItem *item)
{
	static const char *strs[] = { "", " =" };
	strcpy(item->suffix, strs[settings->enable_debug_keys]);
}

void _save_change(ODItem *item, int dir)
{
	settings->multisave ^= 1;
	sound(SND_MENU_MOVE);
}

void _save_get(ODItem *item)
{
	static const char *strs[] = { "1", "5" };
	strcpy(item->suffix, strs[settings->multisave]);
}
#endif


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

/*
void c------------------------------() {}
*/

static void EnterReplayMenu(ODItem *item, int dir)
{
Dialog *dlg = opt.dlg;
ReplaySlotInfo slot;
bool have_replays = false;

	dlg->Clear();
	sound(SND_MENU_MOVE);
	
	for(int i=0;i<MAX_REPLAYS;i++)
	{
		Replay::GetSlotInfo(i, &slot);
		
		if (slot.status != RS_UNUSED)
		{
			const char *mapname = map_get_stage_name(slot.hdr.stageno);
			dlg->AddItem(mapname, EnterReplaySubmenu, _upd_replay, i);
			have_replays = true;
		}
	}
	
	if (!have_replays)
		dlg->AddDismissalItem("[no replays yet]");
	
	dlg->AddSeparator();
	dlg->AddDismissalItem();
}

void _upd_replay(ODItem *item)
{
ReplaySlotInfo slot;

	Replay::GetSlotInfo(item->id, &slot);
	
	Replay::FramesToTime(slot.hdr.total_frames, &item->raligntext[1]);
	item->raligntext[0] = (slot.hdr.locked) ? '=':' ';
}

/*
void c------------------------------() {}
*/

void EnterReplaySubmenu(ODItem *item, int dir)
{
	opt.selected_replay = item->id;
	sound(SND_MENU_MOVE);
	
	opt.subdlg = new Dialog;
	opt.subdlg->SetSize(80, 60);
	
	opt.subdlg->AddItem("Play", _play_replay);
	opt.subdlg->AddItem("Keep", _keep_replay);
}


void _keep_replay(ODItem *item, int dir)
{
        char fname_tmp[1024];
	char fname[MAXPATHLEN];
	ReplayHeader hdr;

	GetReplayName(opt.selected_replay, fname);

	retro_create_path_string(fname_tmp, sizeof(fname_tmp), g_dir, fname);
	
	if (Replay::LoadHeader(fname_tmp, &hdr))
	{
		new Message("Failed to load header.");
		sound(SND_GUN_CLICK);
		opt.dismiss_on_focus = opt.subdlg;
		return;
	}
	
	hdr.locked ^= 1;
	
	if (Replay::SaveHeader(fname_tmp, &hdr))
	{
		new Message("Failed to write header.");
		sound(SND_GUN_CLICK);
		opt.dismiss_on_focus = opt.subdlg;
		return;
	}
	
	sound(SND_MENU_MOVE);
	opt.subdlg->Dismiss();
	opt.dlg->Refresh();
}


void _play_replay(ODItem *item, int dir)
{
	game.switchstage.param = opt.selected_replay;
	game.switchmap(START_REPLAY);
	
	game.setmode(GM_NORMAL);
	game.pause(false);
}

/*
void c------------------------------() {}
*/

static void _edit_control(ODItem *item, int dir)
{
Message *msg;

	opt.remapping_key = item->id;
	opt.new_sdl_key = -1;
	
	msg = new Message("Press new key for:", input_get_name(opt.remapping_key));
	msg->rawKeyReturn = &opt.new_sdl_key;
	msg->on_dismiss = _finish_control_edit;
	
	sound(SND_DOOR);
}

static void _finish_control_edit(Message *msg)
{
int inputno = opt.remapping_key;
int new_sdl_key = opt.new_sdl_key;
int i;

	if (input_get_mapping(inputno) == new_sdl_key)
	{
		sound(SND_MENU_MOVE);
		return;
	}
	
	// check if key is already in use
	for(i=0;i<INPUT_COUNT;i++)
	{
		if (i != inputno && input_get_mapping(i) == new_sdl_key)
		{
			new Message("Key already in use by:", input_get_name(i));
			sound(SND_GUN_CLICK);
			return;
		}
	}
	
	input_remap(inputno, new_sdl_key);
	sound(SND_MENU_SELECT);
	opt.dlg->Refresh();
}














