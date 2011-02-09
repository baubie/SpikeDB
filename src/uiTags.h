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

		bool needput;
		bool on_uiTags_expose_event(GdkEventExpose* e);
		void on_tag_deleted(Glib::ustring tag);
		void on_addnew_clicked();
};


#endif
