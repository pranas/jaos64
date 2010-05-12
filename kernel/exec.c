#include "exec.h"

uint64_t exec(char* filename)
{
    // find file
	dir_entry* file = find_file(filename);

	// not found? exit
	if (!file) return 0;
	
    clean_user_space();

    // let's load it!	
    void* entry = load_executable(file->filename);
    
    // can't load but already cleaned everything :(
    //if (!entry) exit();
    
    // jump
    switch_to_user_mode(entry);
}