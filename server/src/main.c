#include <Ecore.h>
#include "custom.h" 

static void custom_init()
{
	sample_custom_init();
}

static void custom_exit()
{

}

 int main(int argc, const char **argv)
 {
	ecore_init();
	
	//user custom init function
	//add Timer or IPC
	custom_init();

	ecore_app_args_set(argc, argv);
	ecore_main_loop_begin();

	//user custom shutdown function
	//release user structre and etc...	
	custom_exit();


	ecore_shutdown();
	return 0;
 }
