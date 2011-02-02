#include <gtkmm.h>

class uiFilterFrame : public Gtk::Frame {

	public:
		uiFilterFrame(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& refGlade)

	private:
		Gtk::SpinButton* m_pMinFiles;
		Gtk::ComboBox* m_pTypeFilter;

}
					  
