/* Copyright (c) 2014 Fabian Schuiki */
#include "MainWindow.hpp"
#include <gtkmm.h>

using namespace aurisradio;

int main(int argc, char *argv[])
{
	Glib::RefPtr<Gtk::Application> app = Gtk::Application::create(argc, argv, "org.auris.aurium");
	MainWindow window;
	return app->run(window);
}