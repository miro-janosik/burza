// -----
// Program: Burza
// Autor: Miro Janosik
// Vytvorene pre MC Srdiecko Trencin
// -----


#include <gtkmm.h>
#include "burza.h"

Glib::RefPtr<Gtk::Application> app;

int global_argc;
char **global_argv;

void on_app_activate()
{
	main2(global_argc, global_argv);
}

int main(int argc, char **argv)
{	
    app = Gtk::Application::create("org.gtkmm.burza");
    global_argc = argc;
    global_argv = argv;
	Burza::Centrala::Instance();
    Burza::Centrala::SetApp(app);

	app->signal_activate().connect([] () { on_app_activate(); });
    return app->run(argc, argv);
}
