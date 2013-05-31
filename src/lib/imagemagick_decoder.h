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

#include "video_decoder.h"

namespace Magick {
	class Image;
}

class ImageMagickContent;

class ImageMagickDecoder : public VideoDecoder
{
public:
	ImageMagickDecoder (boost::shared_ptr<const Film>, boost::shared_ptr<const ImageMagickContent>);

	/* Decoder */

	void pass ();
	void seek (Time);
	void seek_back ();
	void seek_forward ();
	Time next () const;
	bool done () const;

	/* VideoDecoder */

	float video_frame_rate () const;
	libdcp::Size video_size () const;
	ContentVideoFrame video_length () const;

	/* ImageMagickDecoder */

	boost::shared_ptr<const ImageMagickContent> content () const {
		return _imagemagick_content;
	}

private:
	boost::shared_ptr<const ImageMagickContent> _imagemagick_content;
	boost::shared_ptr<Image> _image;
	mutable boost::optional<libdcp::Size> _video_size;
};
