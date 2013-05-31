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

/** @file src/lib/audio_decoder.h
 *  @brief Parent class for audio decoders.
 */

#ifndef DCPOMATIC_AUDIO_DECODER_H
#define DCPOMATIC_AUDIO_DECODER_H

#include "audio_source.h"
#include "decoder.h"
extern "C" {
#include <libswresample/swresample.h>
}

class AudioContent;

/** @class AudioDecoder.
 *  @brief Parent class for audio decoders.
 */
class AudioDecoder : public AudioSource, public virtual Decoder
{
public:
	AudioDecoder (boost::shared_ptr<const Film>, boost::shared_ptr<const AudioContent>);
	~AudioDecoder ();

protected:

	void audio (boost::shared_ptr<const AudioBuffers>, Time);
	bool audio_done () const;

	Time _next_audio;
	boost::shared_ptr<const AudioContent> _audio_content;

private:	
	SwrContext* _swr_context;
};

#endif
