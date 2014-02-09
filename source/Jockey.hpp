/* Copyright (c) 2014 Fabian Schuiki */
#pragma once
#include <auris/auris.hpp>
#include <boost/filesystem.hpp>
#include <map>
#include <string>
#include <ctime>

namespace aurisradio {

namespace fs = boost::filesystem;
using std::string;
using std::map;

class Jockey
{
public:
	const fs::path repo;
	auris::db::Structure dbs;

	struct Track
	{
		string id, hash, title, artist, album;
		int rating, likes, dislikes, play_count, skip_count;
		time_t added, play_date, skip_date;
		float probability;
	};

	Jockey(const fs::path& repo): repo(repo), dbs(repo) { load(); }
	void load();
	void load_track(Track& t);
	void update_probability(Track& t, time_t now = 0);

	Track* pick();
	void notify_played(Track* t);
	void notify_skipped(Track* t);
	void notify_liked(Track* t);
	void notify_disliked(Track* t);

protected:
	typedef map<string, Track> Tracks;
	typedef map<string, float> Probabilities;
	Tracks tracks;
	Probabilities probabilities;
};

} // namespace aurisradio