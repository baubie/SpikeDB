#ifndef UITAGS_H
#define UITAGS_H

#include <gtkmm.h>
#include <vector>

/**
 * Collection of tags you can add or remove.
 */
class uiTags : public Gtk::Fixed {

	public:
		uiTags();
		virtual ~uiTags();

		std::vector<Glib::ustring> tags();
		void tags(std::vector<Glib::ustring> tags);
		void redraw();

	private:

		class Tag : public Gtk::Frame {

			public:
				Tag(Glib::ustring tag);

			private:
				Gtk::HBox m_hbox;
				Gtk::Button m_Close;	
				Gtk::Label m_Value;
		};

		std::vector<Tag*> m_tag_widgets;
		std::vector<Glib::ustring> m_tags;
		Gtk::Button m_AddNew;

		bool on_uiTags_expose(GdkEventExpose* e);
};


#endif
