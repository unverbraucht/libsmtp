lib_LIBRARIES = libsmtp.a libsmtp_mime.a

AM_CPPFLAGS = -I$(top_srcdir)/include $(glib_CFLAGS)

libsmtp_mime_adir = mime
libsmtp_mime_a_LIBRARIES = libsmtp_mime.a
libsmtp_mime_a_SOURCES = mime/base64.c mime/comm.c mime/parts.c mime/quoted.c
libsmtp_mime_a_HEADERS = include/libsmtp_mime.h
# libsmtp_mime_a_LIBADD = $(glib_LIBS)

libsmtp_adir = smtp
libsmtp_a_LIBRARIES = libsmtp.a
libsmtp_a_SOURCES = smtp/libsmtp_comm.c smtp/libsmtp_connection.c smtp/libsmtp_data.c smtp/libsmtp_error.c
libsmtp_a_HEADERS = include/libsmtp.h
# libsmtp_a_LIBADD = $(glib_LIBS)

bin_PROGRAMS = examples
examples_SOURCES = examples/smtpmail.c examples/multipart.c examples/singlepart.c
examples_LDADD = $(glib_LIBS) libsmtp.a libsmtp_mime.a

dist_doc_DATA = README