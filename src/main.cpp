#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#ifdef HAVE_CONFIG_H
#include <ac_config.h>
#endif

#ifdef MAC
#include <unistd.h>
#include <libgen.h> // For dirname
#include <mach-o/dyld.h> // For _NSGetExecutablePath
#endif

#include "globals.h"
#include "resources/global_files.h"
#include "resources/music.h"
#include "sdl_master.h"
#include <SDL/SDL_mixer.h>

#include "ecs/thing.h"

#undef main

static void install_runtime_logging()
{
    char log_path[PATH_MAX];
    strncpy(log_path, "/tmp/lineage_client.log", sizeof(log_path) - 1);
    log_path[sizeof(log_path) - 1] = '\0';
#ifdef MAC
    char pathbuf[PATH_MAX];
    uint32_t bufsize = sizeof(pathbuf);
    if (_NSGetExecutablePath(pathbuf, &bufsize) == 0) {
        char pathcopy[PATH_MAX];
        strncpy(pathcopy, pathbuf, sizeof(pathcopy) - 1);
        pathcopy[sizeof(pathcopy) - 1] = '\0';
        char* exec_dir = dirname(pathcopy); // .../Assets/Lineage.app/Contents/MacOS
        if (exec_dir != NULL) {
            snprintf(log_path, sizeof(log_path), "%s/../../../lineage_client.log", exec_dir);
        }
    }
#endif
    FILE *out = freopen(log_path, "a", stdout);
    if (out != NULL) {
        setvbuf(stdout, NULL, _IOLBF, 0);
    }
    FILE *err = freopen(log_path, "a", stderr);
    if (err != NULL) {
        setvbuf(stderr, NULL, _IOLBF, 0);
    }
    time_t now = time(NULL);
    printf("\n===== Lineage start %ld =====\n", (long)now);
    printf("Runtime log path: %s\n", log_path);
}

static void fatal_signal_handler(int sig)
{
    printf("FATAL SIGNAL: %d\n", sig);
    fflush(stdout);
    fflush(stderr);
    signal(sig, SIG_DFL);
    raise(sig);
}

static void install_signal_handlers()
{
    signal(SIGABRT, fatal_signal_handler);
    signal(SIGSEGV, fatal_signal_handler);
    signal(SIGBUS, fatal_signal_handler);
    signal(SIGILL, fatal_signal_handler);
}

/**
This function is an osx specific function that changes the current directory
to the resources folder within the .app folder structure
This minimizes the changes required for the osx version when accessing resources
*/
void change_working_directory()
{
#ifdef MAC
    char pathbuf[PATH_MAX];
    uint32_t bufsize = sizeof(pathbuf);
    if (_NSGetExecutablePath(pathbuf, &bufsize) == 0) {
        char pathcopy[PATH_MAX];
        strncpy(pathcopy, pathbuf, sizeof(pathcopy) - 1);
        pathcopy[sizeof(pathcopy) - 1] = '\0';
        char* exec_dir = dirname(pathcopy); // .../Lineage.app/Contents/MacOS

        char resources_dir[PATH_MAX];
        snprintf(resources_dir, sizeof(resources_dir), "%s/../Resources", exec_dir); // .../Lineage.app/Contents/Resources

        if (chdir(resources_dir) == 0) {
            printf("Changed working directory to: %s\n", resources_dir);
        } else {
            perror("chdir failed");
        }
    } else {
        fprintf(stderr, "Failed to get executable path\n");
    }
#endif
}

/** Entry point of the program. Initializes SDL and starts the sdl_master object.
*/
int main(int argc, char* argv[])
{
    install_runtime_logging();
    install_signal_handlers();
    printf("main argc=%d\n", argc);
    for (int i = 0; i < argc; i++) {
        printf("argv[%d]=%s\n", i, argv[i]);
    }

    char current_cwd[PATH_MAX];
    if (getcwd(current_cwd, sizeof(current_cwd)) != NULL) {
        printf("Initial CWD: %s\n", current_cwd);
    } else {
        perror("getcwd error");
    }

    change_working_directory();

    if (getcwd(current_cwd, sizeof(current_cwd)) != NULL) {
        printf("After chdir CWD: %s\n", current_cwd);
    } else {
        perror("getcwd error after chdir");
    }

#ifdef WINDOWS
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0)
    {
		printf("WSAStartup failed!\n");
		//perror("WSAStartup failed!\n");
	}
	#endif
	if (SDL_Init(SDL_INIT_EVERYTHING) == -1)
	{
		printf("Failed to start SDL: %s\n", SDL_GetError());
		return 1;
	}
	printf("SDL_Init(SDL_INIT_EVERYTHING) ok\n");
	
	SDL_EnableUNICODE(1);
	
	Thing g = all_the_things.spawn();
	games.make_new_game(g);
	
	
	sdl_master graphics;
	printf("sdl_master constructed\n");
	global_files *all_files = new global_files();
	sprite::load_generic_sprite_data();
	printf("sprite::load_generic_sprite_data done\n");
	graphics.create_client();
	printf("graphics.create_client done\n");
	graphics.process_events();
	printf("graphics.process_events returned\n");
	delete all_files;
	SDL_VideoQuit();
	SDL_Quit();
	printf("Exiting now\n");
	return 0;
}
