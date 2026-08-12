# Find EpollShim
# Once done, this will define
#
#   EPOLLSHIM_FOUND - System has EpollShim
#   EPOLLSHIM_INCLUDE_DIRS - The EpollShim include directories
#   EPOLLSHIM_LIBRARIES - The libraries needed to use EpollShim

find_path(EPOLLSHIM_INCLUDE_DIRS NAMES sys/epoll.h sys/timerfd.h HINTS /usr/local/include/libepoll-shim)
find_library(EPOLLSHIM_LIBRARIES NAMES epoll-shim libepoll-shim HINTS /usr/local/lib)

if (EPOLLSHIM_INCLUDE_DIRS AND EPOLLSHIM_LIBRARIES)
	set(EPOLLSHIM_FOUND TRUE)
endif (EPOLLSHIM_INCLUDE_DIRS AND EPOLLSHIM_LIBRARIES)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(EpollShim DEFAULT_MSG EPOLLSHIM_LIBRARIES EPOLLSHIM_INCLUDE_DIRS)
mark_as_advanced(EPOLLSHIM_INCLUDE_DIRS EPOLLSHIM_LIBRARIES)
