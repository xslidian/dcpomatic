/*
    Copyright (C) 2013-2015 Carl Hetherington <cth@carlh.net>

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

/** @file  src/wx/audio_mapping_view.cc
 *  @brief AudioMappingView class and helpers.
 */

#include "lib/audio_mapping.h"
#include "lib/util.h"
#include "lib/raw_convert.h"
#include "audio_mapping_view.h"
#include "wx_util.h"
#include "audio_gain_dialog.h"
#include <dcp/types.h>
#include <wx/wx.h>
#include <wx/renderer.h>
#include <wx/grid.h>
#include <boost/lexical_cast.hpp>

using std::cout;
using std::list;
using std::string;
using std::max;
using std::vector;
using boost::shared_ptr;
using boost::lexical_cast;

#define INDICATOR_SIZE 16

enum {
	ID_off = 1,
	ID_full = 2,
	ID_minus6dB = 3,
	ID_edit = 4
};

class NoSelectionStringRenderer : public wxGridCellStringRenderer
{
public:
	void Draw (wxGrid& grid, wxGridCellAttr& attr, wxDC& dc, const wxRect& rect, int row, int col, bool)
	{
		wxGridCellStringRenderer::Draw (grid, attr, dc, rect, row, col, false);
	}
};

/** @class ValueRenderer
 *  @brief wxGridCellRenderer for a gain value.
 */
class ValueRenderer : public wxGridCellRenderer
{
public:

	void Draw (wxGrid& grid, wxGridCellAttr &, wxDC& dc, const wxRect& rect, int row, int col, bool)
	{
		dc.SetPen (*wxThePenList->FindOrCreatePen (wxColour (255, 255, 255), 1, wxPENSTYLE_SOLID));
		dc.SetBrush (*wxTheBrushList->FindOrCreateBrush (wxColour (255, 255, 255), wxBRUSHSTYLE_SOLID));
		dc.DrawRectangle (rect);

		int const xo = (rect.GetWidth() - INDICATOR_SIZE) / 2;
		int const yo = (rect.GetHeight() - INDICATOR_SIZE) / 2;

		dc.SetPen (*wxThePenList->FindOrCreatePen (wxColour (0, 0, 0), 1, wxPENSTYLE_SOLID));
		dc.SetBrush (*wxTheBrushList->FindOrCreateBrush (wxColour (255, 255, 255), wxBRUSHSTYLE_SOLID));
		dc.DrawRectangle (wxRect (rect.GetLeft() + xo, rect.GetTop() + yo, INDICATOR_SIZE, INDICATOR_SIZE));

		float const value = lexical_cast<float> (wx_to_std (grid.GetCellValue (row, col)));
		float const value_dB = 20 * log10 (value);
		int const range = 18;
		int height = 0;
		if (value_dB > -range) {
			height = INDICATOR_SIZE * (1 + value_dB / range);
		}

		height = max (0, height);
		
		if (value > 0) {
			/* Make sure we get a little bit of the marker if there is any gain */
			height = max (3, height);
		}

		dc.SetBrush (*wxTheBrushList->FindOrCreateBrush (wxColour (0, 255, 0), wxBRUSHSTYLE_SOLID));
		dc.DrawRectangle (wxRect (rect.GetLeft() + xo, rect.GetTop() + yo + INDICATOR_SIZE - height, INDICATOR_SIZE, height));
	}

	wxSize GetBestSize (wxGrid &, wxGridCellAttr &, wxDC &, int, int)
	{
		return wxSize (INDICATOR_SIZE + 4, INDICATOR_SIZE + 4);
	}
	
	wxGridCellRenderer* Clone () const
	{
		return new ValueRenderer;
	}
};


AudioMappingView::AudioMappingView (wxWindow* parent)
	: wxPanel (parent, wxID_ANY)
	, _menu_row (0)
	, _menu_column (1)
	, _last_tooltip_row (0)
	, _last_tooltip_column (0)
{
	_grid = new wxGrid (this, wxID_ANY);

	_grid->CreateGrid (0, MAX_DCP_AUDIO_CHANNELS + 1);
	_grid->HideRowLabels ();
	_grid->DisableDragRowSize ();
	_grid->DisableDragColSize ();
	_grid->EnableEditing (false);
	_grid->SetCellHighlightPenWidth (0);
	_grid->SetDefaultRenderer (new NoSelectionStringRenderer);
	_grid->AutoSize ();

	_sizer = new wxBoxSizer (wxVERTICAL);
	_sizer->Add (_grid, 1, wxEXPAND | wxALL);
	SetSizerAndFit (_sizer);

	Bind (wxEVT_GRID_CELL_LEFT_CLICK, boost::bind (&AudioMappingView::left_click, this, _1));
	Bind (wxEVT_GRID_CELL_RIGHT_CLICK, boost::bind (&AudioMappingView::right_click, this, _1));
	_grid->GetGridWindow()->Bind (wxEVT_MOTION, boost::bind (&AudioMappingView::mouse_moved, this, _1));

	_menu = new wxMenu;
	_menu->Append (ID_off, _("Off"));
	_menu->Append (ID_full, _("Full"));
	_menu->Append (ID_minus6dB, _("-6dB"));
	_menu->Append (ID_edit, _("Edit..."));

	Bind (wxEVT_COMMAND_MENU_SELECTED, boost::bind (&AudioMappingView::off, this), ID_off);
	Bind (wxEVT_COMMAND_MENU_SELECTED, boost::bind (&AudioMappingView::full, this), ID_full);
	Bind (wxEVT_COMMAND_MENU_SELECTED, boost::bind (&AudioMappingView::minus6dB, this), ID_minus6dB);
	Bind (wxEVT_COMMAND_MENU_SELECTED, boost::bind (&AudioMappingView::edit, this), ID_edit);
}

/** Called when any gain value has changed */
void
AudioMappingView::map_values_changed ()
{
	update_cells ();
	Changed (_map);
	_last_tooltip_column = -1;
}	

void
AudioMappingView::left_click (wxGridEvent& ev)
{
	if (ev.GetCol() == 0) {
		return;
	}

	int const d = ev.GetCol() - 1;
	
	if (_map.get (ev.GetRow(), d) > 0) {
		_map.set (ev.GetRow(), d, 0);
	} else {
		_map.set (ev.GetRow(), d, 1);
	}

	map_values_changed ();
}

void
AudioMappingView::right_click (wxGridEvent& ev)
{
	if (ev.GetCol() == 0) {
		return;
	}

	_menu_row = ev.GetRow ();
	_menu_column = ev.GetCol ();
	PopupMenu (_menu, ev.GetPosition ());
}

void
AudioMappingView::off ()
{
	_map.set (_menu_row, _menu_column - 1, 0);
	map_values_changed ();
}

void
AudioMappingView::full ()
{
	_map.set (_menu_row, _menu_column - 1, 1);
	map_values_changed ();
}

void
AudioMappingView::minus6dB ()
{
	_map.set (_menu_row, _menu_column - 1, pow (10, -6.0 / 20));
	map_values_changed ();
}

void
AudioMappingView::edit ()
{
	int const d = _menu_column - 1;
	
	AudioGainDialog* dialog = new AudioGainDialog (this, _menu_row, _menu_column - 1, _map.get (_menu_row, d));
	if (dialog->ShowModal () == wxID_OK) {
		_map.set (_menu_row, d, dialog->value ());
		map_values_changed ();
	}
	
	dialog->Destroy ();
}

void
AudioMappingView::set (AudioMapping map)
{
	_map = map;
	update_cells ();
}

void
AudioMappingView::set_input_channels (vector<string> const & names)
{
	for (int i = 0; i < _grid->GetNumberRows(); ++i) {
		_grid->SetCellValue (i, 0, std_to_wx (names[i]));
	}
}

void
AudioMappingView::set_output_channels (vector<string> const & names)
{
	int const o = names.size() + 1;
	if (o < _grid->GetNumberCols ()) {
		_grid->DeleteCols (o, _grid->GetNumberCols() - o);
	} else if (o > _grid->GetNumberCols ()) {
		_grid->InsertCols (_grid->GetNumberCols(), o - _grid->GetNumberCols());
	}

	_grid->SetColLabelValue (0, _("Content"));

	for (size_t i = 0; i < names.size(); ++i) {
		_grid->SetColLabelValue (i + 1, std_to_wx (names[i]));
	}

	update_cells ();
}

void
AudioMappingView::update_cells ()
{
	vector<string> row_names;
	for (int i = 0; i < _grid->GetNumberRows (); ++i) {
		row_names.push_back (wx_to_std (_grid->GetCellValue (i, 0)));
	}
	
	if (_grid->GetNumberRows ()) {
		_grid->DeleteRows (0, _grid->GetNumberRows ());
	}

	_grid->InsertRows (0, _map.input_channels ());

	for (int i = 0; i < _map.input_channels(); ++i) {
		for (int j = 0; j < _map.output_channels(); ++j) {
			_grid->SetCellRenderer (i, j + 1, new ValueRenderer);
		}
	}
	
	for (int i = 0; i < _map.input_channels(); ++i) {
		if (i < int (row_names.size ())) {
			_grid->SetCellValue (i, 0, std_to_wx (row_names[i]));
		}
		for (int j = 1; j < _grid->GetNumberCols(); ++j) {
			_grid->SetCellValue (i, j, std_to_wx (raw_convert<string> (_map.get (i, j - 1))));
		}
	}

	_grid->AutoSize ();
}

void
AudioMappingView::mouse_moved (wxMouseEvent& ev)
{
	int xx;
	int yy;
	_grid->CalcUnscrolledPosition (ev.GetX(), ev.GetY(), &xx, &yy);

	int const row = _grid->YToRow (yy);
	int const column = _grid->XToCol (xx);

	if (row < 0 || column < 1) {
		_grid->GetGridWindow()->SetToolTip ("");
		_last_tooltip_row = row;
		_last_tooltip_column = column;
	}

	if (row != _last_tooltip_row || column != _last_tooltip_column) {

		wxString s;
		float const gain = _map.get (row, column - 1);
		if (gain == 0) {
			s = wxString::Format (_("No audio will be passed from content channel %d to DCP channel %d."), row + 1, column);
		} else if (gain == 1) {
			s = wxString::Format (_("Audio will be passed from content channel %d to DCP channel %d unaltered."), row + 1, column);
		} else {
			float const dB = 20 * log10 (gain);
			s = wxString::Format (_("Audio will be passed from content channel %d to DCP channel %d with gain %.1fdB."), row + 1, column, dB);
		}
		
		_grid->GetGridWindow()->SetToolTip (s + " " + _("Right click to change gain."));
		_last_tooltip_row = row;
		_last_tooltip_column = column;
	}

        ev.Skip ();
}
