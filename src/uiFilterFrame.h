#ifndef UIFILTERFRAME_H
#define UIFILTERFRAME_H

#include <gtkmm.h>
#include <iostream>

class uiFilterFrame {

	public:
		uiFilterFrame(const Glib::RefPtr<Gtk::Builder>& refGlade);
        virtual ~uiFilterFrame();

		int minFiles();
		void minFiles(int set);

		int XVar();

		typedef sigc::signal<void> type_signal_changed;
		type_signal_changed signal_changed();

	protected:
		type_signal_changed m_signal_changed;

		/**
		 * Parent window
		 */
		Gtk::Object *parent;

		/**
		 * Glad reference
		 */
        Glib::RefPtr<Gtk::Builder> m_refGlade;

		/**
		 * VBoxFilter
		 */
		Gtk::VBox *mp_VBoxFilter;	

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

};
					  
#endif
