

set(strmbase_srcs
    src/amextra.cpp
    src/amfilter.cpp
    src/amvideo.cpp
    src/arithutil.cpp
    src/combase.cpp
    src/cprop.cpp
    src/ctlutil.cpp
    src/ddmm.cpp
    src/dllentry.cpp
    src/dllsetup.cpp
    src/mtype.cpp
    src/outputq.cpp
    src/perflog.cpp
    src/pstream.cpp
    src/pullpin.cpp
    src/refclock.cpp
    src/renbase.cpp
    src/schedule.cpp
    src/seekpt.cpp
    src/source.cpp
    src/strmctl.cpp
    src/sysclock.cpp
    src/transfrm.cpp
    src/transip.cpp
    src/videoctl.cpp
    src/vtrans.cpp
    src/winctrl.cpp
    src/winutil.cpp
    src/wxdebug.cpp
    src/wxlist.cpp
    src/wxutil.cpp
    )

set(strbase_headers
    src/amextra.h
    src/amfilter.h
    src/cache.h
    src/checkbmi.h
    src/combase.h
    src/cprop.h
    src/ctlutil.h
    src/ddmm.h
    src/dllsetup.h
    src/dxmperf.h
    src/fourcc.h
    src/measure.h
    src/msgthrd.h
    src/mtype.h
    src/outputq.h
    src/perflog.h
    src/perfstruct.h
    src/pstream.h
    src/pullpin.h
    src/refclock.h
    src/reftime.h
    src/renbase.h
    src/schedule.h
    src/seekpt.h
    src/source.h
    src/stdafx.h
    src/streams.h
    src/strmctl.h
    src/sysclock.h
    src/transfrm.h
    src/transip.h
    src/videoctl.h
    src/vtrans.h
    src/winctrl.h
    src/winutil.h
    src/wxdebug.h
    src/wxlist.h
    src/wxutil.h
    )

add_definitions(-D__STDC_CONSTANT_MACROS -DNO_DSHOW_STRSAFE)

include_directories(src)

set(strmbase_link_libs
    strmiids
    uuid
    ole32
    oleaut32
    advapi32
    user32
    winmm
    gdi32
 )

add_library(strmbase STATIC ${strmbase_srcs} ${strbase_headers})
target_link_libraries(strmbase ${strmbase_link_libs})

set(STRMBASE_INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/src CACHE INTERNAL "doc string")
