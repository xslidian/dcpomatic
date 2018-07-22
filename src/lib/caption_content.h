/*
    Copyright (C) 2013-2018 Carl Hetherington <cth@carlh.net>

    This file is part of DCP-o-matic.

    DCP-o-matic is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    DCP-o-matic is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with DCP-o-matic.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef DCPOMATIC_CAPTION_CONTENT_H
#define DCPOMATIC_CAPTION_CONTENT_H

#include "content_part.h"
#include <libcxml/cxml.h>
#include <dcp/types.h>
#include <boost/signals2.hpp>

class Font;

class CaptionContentProperty
{
public:
	static int const X_OFFSET;
	static int const Y_OFFSET;
	static int const X_SCALE;
	static int const Y_SCALE;
	static int const USE;
	static int const BURN;
	static int const LANGUAGE;
	static int const FONTS;
	static int const COLOUR;
	static int const EFFECT;
	static int const EFFECT_COLOUR;
	static int const LINE_SPACING;
	static int const FADE_IN;
	static int const FADE_OUT;
	static int const OUTLINE_WIDTH;
	static int const TYPE;
};

/** @class CaptionContent
 *  @brief Description of how some text content should be presented.
 *
 *  There are `bitmap' subtitles and `plain' subtitles (plain text),
 *  and not all of the settings in this class correspond to both types.
 */
class CaptionContent : public ContentPart
{
public:
	CaptionContent (Content* parent, CaptionType original_type);
	CaptionContent (Content* parent, std::vector<boost::shared_ptr<Content> >);

	void as_xml (xmlpp::Node *) const;
	std::string identifier () const;
	void take_settings_from (boost::shared_ptr<const CaptionContent> c);

	void add_font (boost::shared_ptr<Font> font);

	void set_use (bool);
	void set_burn (bool);
	void set_x_offset (double);
	void set_y_offset (double);
	void set_x_scale (double);
	void set_y_scale (double);
	void set_language (std::string language);
	void set_colour (dcp::Colour);
	void unset_colour ();
	void set_effect (dcp::Effect);
	void unset_effect ();
	void set_effect_colour (dcp::Colour);
	void unset_effect_colour ();
	void set_line_spacing (double s);
	void set_fade_in (ContentTime);
	void unset_fade_in ();
	void set_fade_out (ContentTime);
	void set_outline_width (int);
	void unset_fade_out ();
	void set_type (CaptionType type);

	bool use () const {
		boost::mutex::scoped_lock lm (_mutex);
		return _use;
	}

	bool burn () const {
		boost::mutex::scoped_lock lm (_mutex);
		return _burn;
	}

	double x_offset () const {
		boost::mutex::scoped_lock lm (_mutex);
		return _x_offset;
	}

	double y_offset () const {
		boost::mutex::scoped_lock lm (_mutex);
		return _y_offset;
	}

	double x_scale () const {
		boost::mutex::scoped_lock lm (_mutex);
		return _x_scale;
	}

	double y_scale () const {
		boost::mutex::scoped_lock lm (_mutex);
		return _y_scale;
	}

	std::list<boost::shared_ptr<Font> > fonts () const {
		boost::mutex::scoped_lock lm (_mutex);
		return _fonts;
	}

	std::string language () const {
		boost::mutex::scoped_lock lm (_mutex);
		return _language;
	}

	boost::optional<dcp::Colour> colour () const {
		boost::mutex::scoped_lock lm (_mutex);
		return _colour;
	}

	boost::optional<dcp::Effect> effect () const {
		boost::mutex::scoped_lock lm (_mutex);
		return _effect;
	}

	boost::optional<dcp::Colour> effect_colour () const {
		boost::mutex::scoped_lock lm (_mutex);
		return _effect_colour;
	}

	double line_spacing () const {
		boost::mutex::scoped_lock lm (_mutex);
		return _line_spacing;
	}

	boost::optional<ContentTime> fade_in () const {
		boost::mutex::scoped_lock lm (_mutex);
		return _fade_in;
	}

	boost::optional<ContentTime> fade_out () const {
		boost::mutex::scoped_lock lm (_mutex);
		return _fade_out;
	}

	int outline_width () const {
		boost::mutex::scoped_lock lm (_mutex);
		return _outline_width;
	}

	CaptionType type () const {
		boost::mutex::scoped_lock lm (_mutex);
		return _type;
	}

	CaptionType original_type () const {
		boost::mutex::scoped_lock lm (_mutex);
		return _original_type;
	}

	static std::list<boost::shared_ptr<CaptionContent> > from_xml (Content* parent, cxml::ConstNodePtr, int version);

protected:
	/** subtitle language (e.g. "German") or empty if it is not known */
	std::string _language;

private:
	friend struct ffmpeg_pts_offset_test;

	CaptionContent (Content* parent, cxml::ConstNodePtr, int version);
	void font_changed ();
	void connect_to_fonts ();

	std::list<boost::signals2::connection> _font_connections;

	bool _use;
	bool _burn;
	/** x offset for placing subtitles, as a proportion of the container width;
	 * +ve is further right, -ve is further left.
	 */
	double _x_offset;
	/** y offset for placing subtitles, as a proportion of the container height;
	 *  +ve is further down the frame, -ve is further up.
	 */
	double _y_offset;
	/** x scale factor to apply to subtitles */
	double _x_scale;
	/** y scale factor to apply to subtitles */
	double _y_scale;
	std::list<boost::shared_ptr<Font> > _fonts;
	boost::optional<dcp::Colour> _colour;
	boost::optional<dcp::Effect> _effect;
	boost::optional<dcp::Colour> _effect_colour;
	/** scaling factor for line spacing; 1 is "standard", < 1 is closer together, > 1 is further apart */
	double _line_spacing;
	boost::optional<ContentTime> _fade_in;
	boost::optional<ContentTime> _fade_out;
	int _outline_width;
	/** what these captions will be used for in the output DCP (not necessarily what
	 *  they were originally).
	 */
	CaptionType _type;
	/** the original type of these captions in their content */
	CaptionType _original_type;
};

#endif