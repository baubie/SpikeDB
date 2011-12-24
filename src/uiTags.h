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

#ifndef UITAGS_H
#define UITAGS_H

#include <gtkmm.h>
#include <vector>

/**
 * Collection of tags you can add or remove.
 */
class uiTags : public Gtk::Fixed {

	public:
		uiTags(Gtk::Window* parent=NULL);
		virtual ~uiTags();

		bool active;

		std::vector<Glib::ustring> tags();
		void tags(std::vector<Glib::ustring> tags);
		void redraw();
		void clear();
		void set_parent(Gtk::Window* parent);

		bool delete_assist;

		typedef sigc::signal<void,Glib::ustring> type_signal_deleted;
		typedef sigc::signal<bool,Glib::ustring> type_signal_added;

        /**
		 * Signals that a tag has been deleted.
		 */
		type_signal_deleted signal_deleted();

        /**
		 * Signals that a tag has been added.
		 */
		type_signal_added signal_added();


	protected:

		type_signal_deleted m_signal_deleted;
		type_signal_added m_signal_added;

		class Tag : public Gtk::EventBox {

			public:
				Tag(Glib::ustring tag);

				typedef sigc::signal<void,Glib::ustring> type_signal_deleted;
				type_signal_deleted signal_deleted();

			protected:
				type_signal_deleted m_signal_deleted;
				Gtk::Frame m_frame;
				Gtk::HBox m_hbox;
				Gtk::Button m_Close;	
				Gtk::Label m_Value;

				void on_delete_clicked();
		};

		std::vector<Tag*> m_tag_widgets;
		std::vector<Glib::ustring> m_tags;
		Gtk::Button m_AddNew;

		Gtk::Window* m_parent;

		Gtk::Allocation m_a;
		bool needput;
		void on_uiTags_size_allocate(Gtk::Allocation& a);
		void on_tag_deleted(Glib::ustring tag);
		void on_addnew_clicked();
};


#endif
