/*
Copyright (c) 2011-2012, Brandon Aubie
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the <organization> nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef UIFILTERFRAME_H
#define UIFILTERFRAME_H

#include <gtkmm.h>
#include <vector>


class uiFilterFrame : public Gtk::Frame {

	public:
		uiFilterFrame();
        virtual ~uiFilterFrame();

		int minFiles();
		void minFiles(int set);

		Glib::ustring tag();
		void updateTagCompletion(std::vector<Glib::ustring> tags);

		bool showHidden();
		void showHidden(bool set);

		double minSpikeTime();
		double maxSpikeTime();
		bool absoluteSpikeTime();

		int XVar();

		typedef sigc::signal<void> type_signal_fileFilter_changed;
		type_signal_fileFilter_changed signal_fileFilter_changed();

		typedef sigc::signal<void> type_signal_spikeFilter_changed;
		type_signal_spikeFilter_changed signal_spikeFilter_changed();

		bool on_spikeTimeFocusLost(GdkEventFocus *e);


	protected:
		type_signal_fileFilter_changed m_signal_fileFilter_changed;
		type_signal_spikeFilter_changed m_signal_spikeFilter_changed;

		/**
		 * Parent window
		 */
		Gtk::Object *parent;

		/**
		 * Tree model columns for the EntryCompletion's filter model.
		 */
		class ModelColumns : public Gtk::TreeModel::ColumnRecord
		{
			public:
				ModelColumns()
				{ add(m_col_name); }
				Gtk::TreeModelColumn<Glib::ustring> m_col_name;
		} m_tagColumns;
		Glib::RefPtr<Gtk::ListStore> mrp_CompletionModel;
		void on_tag_changed();


		/**
		 * Minimum number of files spin box
		 */
		Gtk::SpinButton* mp_sbMinFiles;
		Gtk::Adjustment m_adjMinFiles;
		void on_adjMinFiles_changed();

		/**
		 * X-Variable Type Filter ComboBox
		 */
		Gtk::ComboBoxText m_XVar;
		void on_XVar_changed();

		/**
		 * Tag filter
		 */
		Gtk::Entry m_tag;

		/**
		 * Hidden file checkbox
		 */
		Gtk::CheckButton *mp_cbHidden;
		void on_hidden_toggled();

		/**
		 * Spike Filter 
		 */
		Gtk::Entry m_minSpikes;
		Gtk::Entry m_maxSpikes;
		Gtk::CheckButton *mp_cbSpikeFilterAbsolute;
		void on_spikeTime_changed();
};
#endif
