#include "gimg_utility.h"
#include "gimg_typedef.h"

#include <string.h>
#include <ctype.h>		// tolower
#include <assert.h>

enum GIMG_FILE 
gimg_file_type(const char* filepath) {
	int n = strlen(filepath);
	assert(filepath[n-4] == '.');
	if (tolower(filepath[n-3]) == 'p' && 
		tolower(filepath[n-2]) == 'n' && 
		tolower(filepath[n-1]) == 'g') {
		return FILE_PNG;
	} else if (tolower(filepath[n-3]) == 'j' && 
		       tolower(filepath[n-2]) == 'p' && 
			   tolower(filepath[n-1]) == 'g') {
		return FILE_JPG;
	} else if (tolower(filepath[n-3]) == 'b' && 
		       tolower(filepath[n-2]) == 'm' && 
			   tolower(filepath[n-1]) == 'p') {
		return FILE_BMP;
	} else {
		return FILE_INVALID;
	}
}