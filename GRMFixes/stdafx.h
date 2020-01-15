#pragma once

#include "targetver.h"

//#define WIN32_LEAN_AND_MEAN 
#include <windows.h>
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <algorithm>
#include "detours.h"
#include <map>
#include <unordered_map>
#include <string>
#include <fstream>
#include "d3d7.h"
#include <thread>
#include <mutex>
#include <math.h>
#include <cassert>

#pragma comment(lib, "winmm.lib")