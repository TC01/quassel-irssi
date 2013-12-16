#define _GNU_SOURCE
#include <stdio.h>
#include <module.h>
#include <channels-setup.h>
#include <channels.h>
#include <chat-protocols.h>
#include <chatnets.h>
#include <commands.h>
#include <net-sendbuffer.h>
#include <network.h>
#include <queries.h>
#include <recode.h>
#include <servers-setup.h>
#include <nicklist.h>
#include <servers.h>
#include <settings.h>
#include <levels.h>
#include <window-activity.h>
#include <signals.h>

//fe-common/core
#include <fe-windows.h>
#include <printtext.h>

#include "quassel-irssi.h"
#include "irssi/irssi-gui.h"

static void sig_item_activity(WI_ITEM_REC* wirec, int oldlevel) {
	(void) oldlevel;
	(void) wirec;
}

int quassel_buffer_displayed(uint32_t bufid);
static void sig_window_activity(WINDOW_REC* winrec, int oldlevel) {
	if(oldlevel<0 && winrec->data_level <= 1)
		winrec->data_level = oldlevel;
}

static void sig_created(WINDOW_REC *winrec, int automatic) {
	(void) automatic;
	if(!winrec)
	       return;
	if(!winrec->active) {
		return;
	}
	Quassel_CHANNEL_REC *chanrec = (Quassel_CHANNEL_REC*) channel_find(winrec->active_server, winrec->active->visible_name);
	if(chanrec->buffer_id == -1) {
		return;
	}

	if(!quassel_buffer_displayed(chanrec->buffer_id)) {
		window_activity(winrec, -1, "");
	}
}

static void hide_chan(const char *chan) {
	GSList *win = windows;
	while(win) {
		WINDOW_REC* w = (WINDOW_REC*)win->data;
		if(!w || !w->active || !w->active->visible_name)
			goto next;
		if(strcmp(w->active->visible_name, chan)==0) {
			w->data_level = -1;
		}
next:
		win = g_slist_next(win);
	}
}

static void sig_joined(Quassel_CHANNEL_REC* chanrec) {
	if(!chanrec)
		return;
	if(chanrec->buffer_id == -1)
		return;

	if(!quassel_buffer_displayed(chanrec->buffer_id)) {
		hide_chan(chanrec->name);
	}
}

void quassel_irssi_hide(void* arg, int net, const char* _chan) {
	Quassel_SERVER_REC *server = (Quassel_SERVER_REC*) arg;
	(void)server;
	char *chan = channame(net, _chan);
	hide_chan(chan);
	free(chan);
}

static void sig_changed(WINDOW_REC* new, WINDOW_REC* old) {
	(void)new;
	sig_created(old, 0);
}

static void sig_dehilight(WINDOW_REC *winrec) {
	if(winrec->data_level < 0)
		signal_stop();
}

void quassel_felevel_init(void) {
	signal_add_first("window item activity", (SIGNAL_FUNC) sig_item_activity);
	signal_add_first("window activity", (SIGNAL_FUNC) sig_window_activity);
	signal_add("window created", (SIGNAL_FUNC) sig_created);
	signal_add_last("window changed", (SIGNAL_FUNC) sig_changed);
	signal_add_first("window dehilight", (SIGNAL_FUNC) sig_dehilight);
	signal_add("channel joined", (SIGNAL_FUNC) sig_joined);
}

void quassel_felevel_deinit(void) {
	signal_remove("window item activity", (SIGNAL_FUNC) sig_item_activity);
	signal_remove("window activity", (SIGNAL_FUNC) sig_window_activity);
	signal_remove("window created", (SIGNAL_FUNC) sig_created);
}