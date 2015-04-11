klib_path.o: klib_path.c klib_path.h klib_object.h klib_defs.h klib_string.h
klib_list.o: klib_list.c klib_list.h klib_object.h klib_defs.h
klib_string.o: klib_string.c klib_string.h klib_object.h klib_defs.h klib_wstring.h klib_buffer.h klib_error.h
klib_wstring.o: klib_wstring.c klib_string.h klib_object.h klib_defs.h klib_string.h klib_buffer.h klib_error.h klib_convertutf.h
klib_object.o: klib_object.c klib_object.h klib_defs.h
klib_buffer.o: klib_buffer.c klib_buffer.h klib_object.h klib_object.h klib_defs.h
klib_convertutf.o: klib_convertutf.c klib_convertutf.h
klib_getopt.o: klib_getopt.c klib_getopt.h klib_getoptspec.h klib_log.h klib_error.h
klib_getoptspec.o: klib_getoptspec.c klib_getoptspec.h klib_string.h klib_error.h klib_log.h
klib_log.o: klib_log.h klib_log.c
klib_propsfile.o: klib_propsfile.c klib_propsfile.h klib_assocarray.h
klib_assocarray.o: klib_assocarray.c klib_assocarray.h klib_nvpair.h
klib_nvpair.o: klib_nvpair.c klib_nvpair.h klib_object.h
sxmlc.o: sxmlc.h sxmlc.c
sxmlsearch.o: sxmlcsearch.h sxmlsearch.c
sxmlutils.o: sxmlutils.h sxmlutils.c
klib_xml.o: klib_xml.c sxmlc.h sxmlsearch.c sxmlutils.h
