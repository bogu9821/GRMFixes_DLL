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
#include <d3d9.h>
#include <d3dx9math.h>
#include "d3d7types.h"

#pragma comment(lib, "winmm.lib")