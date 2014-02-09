/* Copyright (c) 2014 Fabian Schuiki */
#include "Jockey.hpp"
#include <boost/lexical_cast.hpp>
#include <cstdlib>

using namespace aurisradio;

void Jockey::load()
{
	auris::db::ObjectBuffer<auris::db::file::Index> index(dbs);
	index.maybe_ref("index");
	tracks.clear();

	time_t now = time(NULL);
	srand(now);
	float pmax = 0;
	for (map<string,string>::const_iterator it = index.tracks.begin(); it != index.tracks.end(); it++) {
		Track& t = tracks[it->first];
		t.id = it->first;
		t.hash = it->second;
		load_track(t);
		update_probability(t, now);
		if (t.probability > pmax)
			pmax = t.probability;
	}
	std::cout << "highest probability = " << pmax << '\n';
}

void Jockey::load_track(Track& t)
{
	auris::db::ObjectBuffer<auris::db::file::Track> track(dbs);
	track.read(t.hash);
	t.title = track.md["title"];
	t.artist = track.md["artist"];
	t.album = track.md["album"];
	t.rating = atoi(track.md["rating"].c_str());
	t.likes = atoi(track.md["likes"].c_str());
	t.dislikes = atoi(track.md["dislikes"].c_str());
	t.added = atoi(track.md["added"].c_str());
	t.play_date = atoi(track.md["play-date"].c_str());
	t.skip_date = atoi(track.md["skip-date"].c_str());
	t.play_count = atoi(track.md["play-count"].c_str());
	t.skip_count = atoi(track.md["skip-count"].c_str());
}

void Jockey::update_probability(Track& t, time_t now)
{
	if (now == 0)
		now = time(NULL);
	float p = 1;

	time_t interaction_date = 0;
	if (t.play_date <= now) interaction_date = t.play_date;
	if (t.skip_date <= now && t.skip_date > interaction_date) interaction_date = t.skip_date;
	if (interaction_date) {
		int64_t dt = (int64_t)now - interaction_date;
		p *= std::min(1.0, dt / 60.0 / 60.0 / 12); // decrease replay probability within 12h of last playback
	}
	if (t.added <= now) {
		int64_t dt = (int64_t)now - t.added;
		p *= std::max(1.0, 10.0 - dt / 60.0 / 60.0 / 24 / 120); // boost probability if added within last 120d
	}
	if (t.dislikes) p /= t.dislikes;
	if (t.likes) p *= t.likes;
	float rating = t.rating * 0.01 * 0.95 + 0.05;
	p *= rating*rating;
	t.probability = p;
}

Jockey::Track* Jockey::pick()
{
	Tracks::iterator it;
	float total_probability = 0;
	for (it = tracks.begin(); it != tracks.end(); it++) {
		total_probability += it->second.probability;
	}

	float P = rand() * total_probability / RAND_MAX;
	for (it = tracks.begin(); it != tracks.end() && P > it->second.probability; it++) {
		P -= it->second.probability;
	}

	// in case we picked past the end of the track list due to float inaccuracies
	if (it == tracks.end())
		it = tracks.begin();

	return &it->second;
}

void Jockey::notify_played(Track* t)
{
	auris::db::ObjectBuffer<auris::db::file::Track> track(dbs);
	track.read(t->hash);
	time_t now = time(NULL);
	track.md["play-date"] = boost::lexical_cast<string>(now);
	track.md["play-count"] = boost::lexical_cast<string>(t->play_count + 1);
	track.write();

	t->hash = track.hash_out;
	auris::db::ObjectBuffer<auris::db::file::Index> index(dbs);
	index.maybe_ref("index");
	index.tracks[t->id] = t->hash;
	index.write();

	load_track(*t);
	update_probability(*t);
}

void Jockey::notify_skipped(Track* t)
{
	auris::db::ObjectBuffer<auris::db::file::Track> track(dbs);
	track.read(t->hash);
	time_t now = time(NULL);
	track.md["skip-date"] = boost::lexical_cast<string>(now);
	track.md["skip-count"] = boost::lexical_cast<string>(t->skip_count + 1);
	track.write();

	t->hash = track.hash_out;
	auris::db::ObjectBuffer<auris::db::file::Index> index(dbs);
	index.maybe_ref("index");
	index.tracks[t->id] = t->hash;
	index.write();

	load_track(*t);
	update_probability(*t);
}

void Jockey::notify_liked(Track* t)
{
	auris::db::ObjectBuffer<auris::db::file::Track> track(dbs);
	track.read(t->hash);
	time_t now = time(NULL);
	track.md["likes"] = boost::lexical_cast<string>(t->likes + 1);
	track.write();

	t->hash = track.hash_out;
	auris::db::ObjectBuffer<auris::db::file::Index> index(dbs);
	index.maybe_ref("index");
	index.tracks[t->id] = t->hash;
	index.write();

	load_track(*t);
	update_probability(*t);
}

void Jockey::notify_disliked(Track* t)
{
	auris::db::ObjectBuffer<auris::db::file::Track> track(dbs);
	track.read(t->hash);
	time_t now = time(NULL);
	track.md["dislikes"] = boost::lexical_cast<string>(t->dislikes + 1);
	track.write();

	t->hash = track.hash_out;
	auris::db::ObjectBuffer<auris::db::file::Index> index(dbs);
	index.maybe_ref("index");
	index.tracks[t->id] = t->hash;
	index.write();

	load_track(*t);
	update_probability(*t);
}
