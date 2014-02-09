/* Copyright (c) 2014 Fabian Schuiki */
#pragma once
#include "Jockey.hpp"
#include <boost/smart_ptr.hpp>
#include <gtkmm.h>
#include <vlc/vlc.h>

namespace aurisradio {

class MainWindow : public Gtk::Window
{
public:
	MainWindow();

	void on_action_file_open();
	void on_action_file_quit();
	void on_clicked_like();
	void on_clicked_dislike();
	void on_clicked_skip();
	bool on_update_timer();
	void on_time_slider_changed();

private:
	Glib::RefPtr<Gtk::ActionGroup> action_group;
	Glib::RefPtr<Gtk::UIManager> ui_manager;
	Gtk::Box root_box;

	Gtk::Button like_button, dislike_button, skip_button;
	Gtk::Label title_label, artist_label, album_label, id_label;
	Gtk::ButtonBox button_box;
	Gtk::Box label_box, main_box, control_box;
	Gtk::Label time_played_label, time_left_label;
	Gtk::Scale time_slider;
	bool ignore_slider;

	boost::scoped_ptr<Jockey> jockey;
	void play_next();
	Jockey::Track* current_track;
	libvlc_instance_t* vlc_inst;
	libvlc_media_player_t* vlc_mp;
	bool track_liked;
};

} // namespace aurisradio