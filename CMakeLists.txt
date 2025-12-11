cmake_minimum_required(VERSION 3.16)

project(gimg)

################################################################################
# Source groups
################################################################################

set(inc
    "gimg_bmp.h"
    "gimg_dds.h"
    #"gimg_etc1.h"
    #"gimg_etc2.h"
    "gimg_etcpack.h"
    "gimg_export.h"
    "gimg_hgt.h"
    "gimg_import.h"
    "gimg_jpg.h"
    "gimg_math.h"
    "gimg_png.h"
    "gimg_ppm.h"
    "gimg_pvr.h"
    #"gimg_rg_etc1.h"
    "gimg_tga.h"
    "gimg_tiff.h"
    "gimg_typedef.h"
    "gimg_utility.h"
)
source_group("inc" FILES ${inc})

set(src
    "gimg_bmp.c"
    "gimg_dds.c"
    #"gimg_etc1.c"
    #"gimg_etc2.c"
    #"gimg_etcpack.cpp"
    "gimg_export.c"
    "gimg_hgt.c"
    "gimg_import.c"
    "gimg_jpg.c"
    "gimg_png.c"
    "gimg_ppm.c"
    "gimg_pvr.c"
    #"gimg_rg_etc1.cpp"
    "gimg_tga.c"
    "gimg_tiff.c"
    "gimg_utility.c"
)
source_group("src" FILES ${src})

set(ALL_FILES
    ${inc}
    ${src}
)

add_library(gimg STATIC ${ALL_FILES})

target_include_directories(${PROJECT_NAME} PRIVATE external/fs external/stb external/logger external/libjpeg-turbo/src external/libpng external/libtiff/libtiff)
