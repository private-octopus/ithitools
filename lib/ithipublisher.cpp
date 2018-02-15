#ifdef _WINDOWS
#include "wincompat/dirent.h"
#ifndef ITHI_DEFAULT_FOLDER
#define ITHI_DEFAULT_FOLDER ".\\"
#endif
#ifndef ITHI_FILE_PATH_SEP
#define ITHI_FILE_PATH_SEP "\\"
#endif
#else
#include <dirent.h>
#ifndef ITHI_DEFAULT_FOLDER
#define ITHI_DEFAULT_FOLDER "./"
#endif
#ifndef ITHI_FILE_PATH_SEP
#define ITHI_FILE_PATH_SEP "/"
#endif
#endif
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <algorithm>
#include "ithipublisher.h"



ithipublisher::ithipublisher()
{
}


ithipublisher::~ithipublisher()
{
}
