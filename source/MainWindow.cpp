/* Copyright (c) 2014 Fabian Schuiki */
#include "MainWindow.hpp"
#include "detail.hpp"
#include <iostream>
#include <map>
#include <string>
#include <cstring>
#include <cerrno>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


using namespace aurisradio;

MainWindow::MainWindow(): Gtk::Window(), current_track(NULL), vlc_inst(NULL), vlc_mp(NULL), time_slider(Gtk::ORIENTATION_HORIZONTAL), ignore_slider(false)
{
	vlc_inst = libvlc_new(0, NULL);
	set_default_size(500,-1);
	set_title("Auris Radio");
	jockey.reset(new Jockey(home()/"Music"/"Auris"));

	action_group = Gtk::ActionGroup::create();
	action_group->add(Gtk::Action::create("menu_file", "_File"));
	action_group->add(Gtk::Action::create("open", Gtk::Stock::OPEN), sigc::mem_fun(*this, &MainWindow::on_action_file_open));
	action_group->add(Gtk::Action::create("quit", Gtk::Stock::QUIT), sigc::mem_fun(*this, &MainWindow::on_action_file_quit));

	ui_manager = Gtk::UIManager::create();
	ui_manager->insert_action_group(action_group);
	add_accel_group(ui_manager->get_accel_group());

	Glib::ustring ui_info =
	"<ui>"
		"<menubar name='menubar'>"
			"<menu action='menu_file'>"
				"<menuitem action='open'/>"
				"<separator/>"
				"<menuitem action='quit'/>"
			"</menu>"
		"</menubar>"
		"<toolbar name='toolbar'>"
			"<toolitem action='open'/>"
			"<toolitem action='quit'/>"
		"</toolbar>"
	"</ui>";
	ui_manager->add_ui_from_string(ui_info);

	title_label.set_text("<title>");
	artist_label.set_text("<artist>");
	album_label.set_text("<album>");
	id_label.set_text("<id>");

	title_label.set_alignment(0, 0.5);
	artist_label.set_alignment(0, 0.5);
	album_label.set_alignment(0, 0.5);
	id_label.set_alignment(0, 0.5);

	title_label.set_ellipsize(Pango::ELLIPSIZE_END);
	artist_label.set_ellipsize(Pango::ELLIPSIZE_END);
	album_label.set_ellipsize(Pango::ELLIPSIZE_END);
	id_label.set_ellipsize(Pango::ELLIPSIZE_END);

	like_button.set_label("Like");
	like_button.signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::on_clicked_like));
	dislike_button.set_label("Dislike");
	dislike_button.signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::on_clicked_dislike));
	skip_button.set_label("Skip");
	skip_button.signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::on_clicked_skip));

	Gtk::Widget* menubar = ui_manager->get_widget("/menubar");
	Gtk::Widget* toolbar = ui_manager->get_widget("/toolbar");
	root_box.set_orientation(Gtk::ORIENTATION_VERTICAL);
	root_box.pack_start(*menubar, Gtk::PACK_SHRINK);
	// root_box.pack_start(*toolbar, Gtk::PACK_SHRINK);

	time_played_label.set_text("0:00");
	time_left_label.set_text("0:00");
	time_slider.set_draw_value(false);
	time_played_label.set_valign(Gtk::ALIGN_CENTER);
	time_left_label.set_valign(Gtk::ALIGN_CENTER);
	time_slider.set_valign(Gtk::ALIGN_CENTER);
	time_slider.set_adjustment(Gtk::Adjustment::create(0.0, 0.0, 1.0, 0.01, 0.1));
	time_slider.signal_value_changed().connect(sigc::mem_fun(*this, &MainWindow::on_time_slider_changed));

	control_box.set_orientation(Gtk::ORIENTATION_HORIZONTAL);
	control_box.set_spacing(5);
	control_box.pack_start(time_played_label, Gtk::PACK_SHRINK);
	control_box.pack_start(time_slider, Gtk::PACK_EXPAND_WIDGET);
	control_box.pack_start(time_left_label, Gtk::PACK_SHRINK);

	label_box.set_orientation(Gtk::ORIENTATION_VERTICAL);
	label_box.pack_start(title_label);
	label_box.pack_start(artist_label);
	label_box.pack_start(album_label);
	label_box.pack_start(id_label);
	label_box.pack_start(control_box);

	button_box.set_orientation(Gtk::ORIENTATION_VERTICAL);
	button_box.set_valign(Gtk::ALIGN_CENTER);
	button_box.add(like_button);
	button_box.add(dislike_button);
	button_box.add(skip_button);

	main_box.set_valign(Gtk::ALIGN_CENTER);
	main_box.set_orientation(Gtk::ORIENTATION_HORIZONTAL);
	main_box.set_spacing(10);
	main_box.set_margin_left(10);
	main_box.set_margin_right(10);
	main_box.set_margin_top(10);
	main_box.set_margin_bottom(10);
	main_box.pack_start(label_box, Gtk::PACK_EXPAND_WIDGET);
	main_box.pack_start(button_box, Gtk::PACK_SHRINK);
	root_box.pack_start(main_box, Gtk::PACK_SHRINK);

	add(root_box);
	show_all_children();
	play_next();

	Glib::signal_timeout().connect(sigc::mem_fun(*this, &MainWindow::on_update_timer), 750);
}

void MainWindow::on_action_file_open()
{
}

void MainWindow::on_action_file_quit()
{
	hide();
}

void MainWindow::on_clicked_like()
{
	if (!jockey || track_liked) return;
	if (current_track)
		jockey->notify_liked(current_track);
	track_liked = true;
}

void MainWindow::on_clicked_dislike()
{
	if (!jockey) return;
	if (current_track)
		jockey->notify_disliked(current_track);
	play_next();
}

void MainWindow::on_clicked_skip()
{
	if (!jockey) return;
	if (current_track)
		jockey->notify_skipped(current_track);
	play_next();
}

static string format_time(libvlc_time_t T)
{
	int t = T/1000;
	char buf[32];
	snprintf(buf, 32, "%i:%02i", t / 60, t % 60);
	return buf;
}

bool MainWindow::on_update_timer()
{
	if (!vlc_mp) return true;

	libvlc_time_t length = libvlc_media_player_get_length(vlc_mp);
	libvlc_time_t current = libvlc_media_player_get_time(vlc_mp);
	libvlc_time_t left = length-current;

	if (left <= 5) {
		if (current_track)
			jockey->notify_played(current_track);
		play_next();
	}

	time_played_label.set_text(format_time(current));
	time_left_label.set_text("-" + format_time(left));
	ignore_slider = true;
	time_slider.set_value(libvlc_media_player_get_position(vlc_mp));
	ignore_slider = false;

	return true;
}

void MainWindow::on_time_slider_changed()
{
	if (!vlc_mp || ignore_slider) return;
	libvlc_media_player_set_position(vlc_mp, time_slider.get_value());
}

void MainWindow::play_next()
{
	if (vlc_mp) {
		libvlc_media_player_stop(vlc_mp);
		libvlc_media_player_release(vlc_mp);
		vlc_mp = NULL;
	}

	if (!jockey) return;
	current_track = jockey->pick();
	track_liked = false;

	if (!current_track) {
		title_label.set_text("no track");
		artist_label.set_text("");
		album_label.set_text("");
		id_label.set_text("");
		return;
	}

	std::cout << "picked probability = " << current_track->probability << " (rating = " << current_track->rating << ")\n";
	title_label.set_text(current_track->title);
	artist_label.set_text(current_track->artist);
	album_label.set_text(current_track->album);
	id_label.set_text(current_track->id);

	auris::db::ObjectBuffer<auris::db::file::Track> track(jockey->dbs);
	track.read(current_track->hash);
	if (track.blobs.empty()) {
		std::cerr << "track " << current_track->id << " has no blob that can be player\n";
		return;
	}
	string blob = (*track.blobs.begin()).blob_ref;

	std::ifstream fobj(jockey->dbs.object(blob).path.c_str());
	if (!fobj.good()) {
		std::cerr << "cannot open blob " << blob << " for reading\n";
		return;
	}

	const char* tmp = "/tmp/aurisplayer-buffer";
	auris::db::file::Object object;
	object.read(fobj);
	auris::aux::mapfile::write(tmp, fobj);

	libvlc_media_t* m = libvlc_media_new_path(vlc_inst, tmp);
	if (!m) {
		std::cerr << "unable to open blob " << blob << " using libvlc\n";
		return;
	}

	vlc_mp = libvlc_media_player_new_from_media(m);
	libvlc_media_release(m); m = NULL;
	libvlc_media_player_play(vlc_mp);
}
