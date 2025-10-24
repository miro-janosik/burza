#include "EditMessageBox.h"
#include "burza.h"

bool EditMessageBox::initialized = false;
// template<> EditMessageBox* Singleton<EditMessageBox> because of unix g++ compiler
template<> EditMessageBox* Singleton<EditMessageBox>::instance = NULL;

bool EditMessageBox::ShowDialog( std::string message, std::string& edit )
{
    assert( instance != NULL );
    assert( instance->initialized );

    instance->controls.labelText->set_text( message );
    instance->controls.entryText->set_text( edit );

    auto dialog = instance->controls.dialog;
    Burza::Centrala::GetApp()->add_window(*dialog);
    dialog->set_visible(true);
    int res = instance->resultButtonIndex;

    edit = instance->controls.entryText->get_text();
    instance->controls.dialog->hide();

    return res == 0;
}
