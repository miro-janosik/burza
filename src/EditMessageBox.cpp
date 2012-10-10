#include "EditMessageBox.h"

bool EditMessageBox::initialized = false;
// template<> EditMessageBox* Singleton<EditMessageBox> because of unix g++ compiler
template<> EditMessageBox* Singleton<EditMessageBox>::instance = NULL;
