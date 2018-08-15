
#ifndef LIB_SMTP_H

#define LIB_SMTP_H

int libsmtp_int_errno;
int libsmtp_debug=0;
char *libsmtp_tracelog;

int libsmtp_errno();

void libsmtp_set_debug(int, char *);

int libsmtp (char *, char *, char *, char *, char *);

#endif