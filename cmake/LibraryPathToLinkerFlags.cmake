function(library_path_to_linker_flags LD_FLAGS LIB_PATHS)
  foreach(L ${LIB_PATHS})
    get_filename_component(DIR  ${L} PATH)
    get_filename_component(LIBFILE ${L} NAME_WE)
    STRING(REGEX REPLACE "^lib" "" FILE ${LIBFILE})

    if (${L} MATCHES "[.]framework$")
        set(FILE_OPT "-framework ${FILE}")
        set(DIR_OPT  "-F${DIR}")
    else()
        set(FILE_OPT "-l${FILE}")
        set(DIR_OPT  "-L${DIR}")
    endif()

    if ("${DIR}" STREQUAL "" OR "${DIR}" STREQUAL "${LASTDIR}")
        set (DIR_OPT "")
    endif()

    set(LASTDIR ${DIR})

    set(${LD_FLAGS} ${${LD_FLAGS}} ${DIR_OPT} ${FILE_OPT} PARENT_SCOPE)
    string (REPLACE ";" " " ${LD_FLAGS} "${${LD_FLAGS}}")
  endforeach()
endfunction()
