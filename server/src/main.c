 #include <Ecore.h>

 int main(int argc, const char **argv)
 {
    ecore_init();
    ecore_app_args_set(argc, argv);
    ecore_main_loop_begin();
    ecore_shutdown();
    return 0;
 }
