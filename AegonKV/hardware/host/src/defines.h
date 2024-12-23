/*
 * (c) Copyright 2019 Xilinx, Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
#ifndef TITAN_HARDWARE__DEFINES_H_
#define TITAN_HARDWARE__DEFINES_H_

#include <fcntl.h> /* For O_RDWR */
#include <cmath>
#include <cstdint>
#include <sys/stat.h>
#include <ctime>
#include <unistd.h> /* For open(), creat() */

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include "xcl2.hpp"

#ifndef MAX_INPUT_LENGTH
#define MAX_INPUT_LENGTH (4)
#endif

#ifndef MAX_DATA_SIZE
 #define MAX_DATA_SIZE (136 * 1024 * 1024)
//#define MAX_DATA_SIZE (32 * 1024)
#endif

#ifndef MAX_INPUT_BITMAP_SIZE
#define MAX_INPUT_BITMAP_SIZE (136 * 1024 / 8)
#endif

#ifndef OUTPUT_META_SIZE
#define OUTPUT_META_SIZE (4)
#endif

#endif  // TITAN_HARDWARE__DEFINES_H_
