/*
    Copyright (C) 2017-2018 Carl Hetherington <cth@carlh.net>

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

#include "active_captions.h"
#include "caption_content.h"
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

using std::list;
using std::pair;
using std::make_pair;
using boost::weak_ptr;
using boost::shared_ptr;
using boost::optional;

/** Get the subtitles that should be burnt into a given period.
 *  @param period Period of interest.
 *  @param always_burn_captions Always burn captions even if their content is not set to burn.
 */
list<PlayerCaption>
ActiveCaptions::get_burnt (DCPTimePeriod period, bool always_burn_captions) const
{
	list<PlayerCaption> ps;

	for (Map::const_iterator i = _data.begin(); i != _data.end(); ++i) {

		shared_ptr<const CaptionContent> caption = i->first.lock ();
		if (!caption) {
			continue;
		}

		if (!caption->use() || (!always_burn_captions && !caption->burn())) {
			/* Not burning this content */
			continue;
		}

		BOOST_FOREACH (Period j, i->second) {
			DCPTimePeriod test (j.from, j.to.get_value_or(DCPTime::max()));
			optional<DCPTimePeriod> overlap = period.overlap (test);
			if (overlap && overlap->duration() > DCPTime(period.duration().get() / 2)) {
				ps.push_back (j.subs);
			}
		}
	}

	return ps;
}

/** Remove subtitles that finish before a given time from our list.
 *  @param time Time to remove before.
 */
void
ActiveCaptions::clear_before (DCPTime time)
{
	Map updated;
	for (Map::const_iterator i = _data.begin(); i != _data.end(); ++i) {
		list<Period> as;
		BOOST_FOREACH (Period j, i->second) {
			if (!j.to || j.to.get() >= time) {
				as.push_back (j);
			}
		}
		if (!as.empty ()) {
			updated[i->first] = as;
		}
	}
	_data = updated;
}

/** Add a new subtitle with a from time.
 *  @param content Content that the subtitle is from.
 *  @param ps Subtitles.
 *  @param from From time for these subtitles.
 */
void
ActiveCaptions::add_from (weak_ptr<const CaptionContent> content, PlayerCaption ps, DCPTime from)
{
	if (_data.find(content) == _data.end()) {
		_data[content] = list<Period>();
	}
	_data[content].push_back (Period (ps, from));
}

/** Add the to time for the last subtitle added from a piece of content.
 *  @param content Content that the subtitle is from.
 *  @param to To time for the last subtitle submitted to add_from for this content.
 *  @return Return the corresponding subtitles and their from time.
 */
pair<PlayerCaption, DCPTime>
ActiveCaptions::add_to (weak_ptr<const CaptionContent> content, DCPTime to)
{
	DCPOMATIC_ASSERT (_data.find(content) != _data.end());

	_data[content].back().to = to;

	BOOST_FOREACH (TextCaption& i, _data[content].back().subs.text) {
		i.set_out (dcp::Time(to.seconds(), 1000));
	}

	return make_pair (_data[content].back().subs, _data[content].back().from);
}

/** @param content Some content.
 *  @return true if we have any active subtitles from this content.
 */
bool
ActiveCaptions::have (weak_ptr<const CaptionContent> content) const
{
	Map::const_iterator i = _data.find(content);
	if (i == _data.end()) {
		return false;
	}

	return !i->second.empty();
}

void
ActiveCaptions::clear ()
{
	_data.clear ();
}