/*
    Copyright (C) 2012 Carl Hetherington <cth@carlh.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

*/

/** @file  src/film_viewer.cc
 *  @brief A wx widget to view `thumbnails' of a Film.
 */

#include <iostream>
#include <iomanip>
#include "lib/film.h"
#include "lib/format.h"
#include "lib/util.h"
#include "lib/thumbs_job.h"
#include "lib/job_manager.h"
#include "lib/film_state.h"
#include "lib/options.h"
#include "film_viewer.h"

using namespace std;
using namespace boost;

class ThumbPanel : public wxPanel
{
public:
	ThumbPanel (wxPanel* parent, Film* film)
		: wxPanel (parent)
		, _film (film)
		, _image (0)
		, _bitmap (0)
	{
	}

	void paint_event (wxPaintEvent& ev)
	{
		if (!_bitmap) {
			return;
		}

		wxPaintDC dc (this);
		dc.DrawBitmap (*_bitmap, 0, 0, false);
	}

	void size_event (wxSizeEvent &)
	{
		if (!_image) {
			return;
		}

		resize ();
	}

	void resize ()
	{
		int vw, vh;
		GetSize (&vw, &vh);

		float const target = _film->format()->ratio_as_float ();

		delete _bitmap;
		if ((float (vw) / vh) > target) {
			/* view is longer (horizontally) than the ratio; fit height */
			_bitmap = new wxBitmap (_image->Scale (vh * target, vh));
		} else {
			/* view is shorter (horizontally) than the ratio; fit width */
			_bitmap = new wxBitmap (_image->Scale (vw, vw / target));
		}

		Refresh ();
	}

	void load (string f)
	{
		delete _image;
		_image = new wxImage (wxString (f.c_str(), wxConvUTF8));
		resize ();
	}

	void clear ()
	{
		delete _bitmap;
		_bitmap = 0;
		delete _image;
		_image = 0;
	}

	DECLARE_EVENT_TABLE ();

private:
	Film* _film;
	wxImage* _image;
	wxBitmap* _bitmap;
};

BEGIN_EVENT_TABLE (ThumbPanel, wxPanel)
EVT_PAINT (ThumbPanel::paint_event)
EVT_SIZE (ThumbPanel::size_event)
END_EVENT_TABLE ()

FilmViewer::FilmViewer (Film* f, wxWindow* p)
	: wxPanel (p)
	, _film (f)
{
	_sizer = new wxBoxSizer (wxVERTICAL);
	SetSizer (_sizer);
	
	_thumb_panel = new ThumbPanel (this, f);
	_sizer->Add (_thumb_panel, 1, wxEXPAND);

	int const max = f ? f->num_thumbs() : 0;
	_slider = new wxSlider (this, wxID_ANY, 0, 0, max);
	_sizer->Add (_slider, 0, wxEXPAND | wxLEFT | wxRIGHT);
	load_thumbnail (0);

	_slider->Connect (wxID_ANY, wxEVT_COMMAND_SLIDER_UPDATED, wxCommandEventHandler (FilmViewer::slider_changed), 0, this);

	set_film (_film);
}

void
FilmViewer::load_thumbnail (int n)
{
	if (_film == 0 || _film->num_thumbs() <= n) {
		return;
	}

	int const left = _film->left_crop ();
	int const right = _film->right_crop ();
	int const top = _film->top_crop ();
	int const bottom = _film->bottom_crop ();

	_thumb_panel->load (_film->thumb_file(n));
}

void
FilmViewer::reload_current_thumbnail ()
{
	load_thumbnail (_slider->GetValue ());
}

void
FilmViewer::slider_changed (wxCommandEvent &)
{
	reload_current_thumbnail ();
}

string
FilmViewer::format_position_slider_value (double v) const
{
#if 0	
	stringstream s;

	if (_film && int (v) < _film->num_thumbs ()) {
		int const f = _film->thumb_frame (int (v));
		s << f << " " << seconds_to_hms (f / _film->frames_per_second ());
	} else {
		s << "-";
	}
	
	return s.str ();
#endif	
}

void
FilmViewer::film_changed (Film::Property p)
{
#if 0	
	if (p == Film::LEFT_CROP || p == Film::RIGHT_CROP || p == Film::TOP_CROP || p == Film::BOTTOM_CROP) {
		reload_current_thumbnail ();
	} else if (p == Film::THUMBS) {
		if (_film && _film->num_thumbs() > 1) {
			_position_slider.set_range (0, _film->num_thumbs () - 1);
		} else {
			_image.clear ();
			_position_slider.set_range (0, 1);
		}
		
		_position_slider.set_value (0);
		reload_current_thumbnail ();
	} else if (p == Film::FORMAT) {
		reload_current_thumbnail ();
	} else if (p == Film::CONTENT) {
		setup_visibility ();
		_film->examine_content ();
		update_thumbs ();
	}
#endif	
}

void
FilmViewer::set_film (Film* f)
{
	_film = f;

	if (!_film) {
		_thumb_panel->clear ();
		return;
	}

//	_film->Changed.connect (sigc::mem_fun (*this, &FilmViewer::film_changed));

	film_changed (Film::THUMBS);
}

pair<int, int>
FilmViewer::scaled_pixbuf_size () const
{
#if 0	
	if (_film == 0) {
		return make_pair (0, 0);
	}
	
	int const cw = _film->size().width - _film->left_crop() - _film->right_crop(); 
	int const ch = _film->size().height - _film->top_crop() - _film->bottom_crop();

	float ratio = 1;
	if (_film->format()) {
		ratio = _film->format()->ratio_as_float() * ch / cw;
	}

	Gtk::Allocation const a = _scroller.get_allocation ();
	float const zoom = min (float (a.get_width()) / (cw * ratio), float (a.get_height()) / cw);
	return make_pair (cw * zoom * ratio, ch * zoom);
#endif	
}
	
void
FilmViewer::update_scaled_pixbuf ()
{
#if 0	
	pair<int, int> const s = scaled_pixbuf_size ();

	if (s.first > 0 && s.second > 0 && _cropped_pixbuf) {
		_scaled_pixbuf = _cropped_pixbuf->scale_simple (s.first, s.second, Gdk::INTERP_HYPER);
		_image.set (_scaled_pixbuf);
	}
#endif	
}

void
FilmViewer::update_thumbs ()
{
#if 0	
	if (!_film) {
		return;
	}

	_film->update_thumbs_pre_gui ();

	shared_ptr<const FilmState> s = _film->state_copy ();
	shared_ptr<Options> o (new Options (s->dir ("thumbs"), ".tiff", ""));
	o->out_size = _film->size ();
	o->apply_crop = false;
	o->decode_audio = false;
	o->decode_video_frequency = 128;
	
	shared_ptr<Job> j (new ThumbsJob (s, o, _film->log ()));
	j->Finished.connect (sigc::mem_fun (_film, &Film::update_thumbs_post_gui));
	JobManager::instance()->add (j);
#endif	
}

void
FilmViewer::setup_visibility ()
{
#if 0	
	if (!_film) {
		return;
	}

	ContentType const c = _film->content_type ();
	_position_slider.property_visible() = (c == VIDEO);
#endif	
}
