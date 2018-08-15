

/* This program sends mail via SMTP to a host
   written by Kevin Read <kevin.read@innnet.de> */

#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>

#include "libsmtp.h"


/* libsmtp_sendmail

  return values:
  
  0 ok, mail sent to server
  1 socket couldn't be created
  2 mailserver unknown
  3 connection to mailserver failed
  4 unable to read from socket
  5 mailserver didn't greet correctly (ie its no rfc-conform smtp)
  6 unable to send to socket
  7 mailserver didn't accept our HELO
  8 mailserver didn't accept our MAIL FROM
  9 mailserver didn't accept our RCPT TO
  10 mailserver didn't like us to DATA
  11 mailserver rejected mail after .
  12 From-String out of bounds (200 octets limit)
  13 To-String out of bounds (200 octets limit)
  14 Mailserver-String out of bounds (200 octets limit)
  15 Subject-String out of bound (300 octets limit)
  16 Can't get local hostname
  */
  



int libsmtp_errno ()
{
  return libsmtp_int_errno;
}

void libsmtp_set_debug (int libsmtp_debug_opt, char *libsmtp_tracelog_opt)
{
  if (libsmtp_debug_opt)
  {
    libsmtp_debug=1;
    libsmtp_tracelog=libsmtp_tracelog_opt;
  }
  else
  {
    libsmtp_debug=0;
    libsmtp_tracelog=NULL;
  }
}
  

int libsmtp_sendmail (char *libsmtp_from_opt, char *libsmtp_to_opt, char *libsmtp_mailhost_opt, char *libsmtp_subject_opt, char *libsmtp_body_opt)
{
  int libsmtp_mailsocket;
  char libsmtp_from[201], libsmtp_to[201], libsmtp_mailhost[201], libsmtp_subject[301];
  char libsmtp_myhostname[201];
  struct hostent *hostname;
  struct sockaddr_in partner_ad;
  char libsmtp_buffer[4000];
  int libsmtp_times, libsmtp_temp, fraglength;
  
  if (strlen (libsmtp_from_opt) > 200)
    return 12;
  
  if (strlen (libsmtp_to_opt) > 200)
    return 13;
    
  if (strlen (libsmtp_mailhost_opt) > 200)
    return 14;
  
  if (strlen (libsmtp_subject_opt) > 300)
    return 15;
  
  /* Now copy the strings so as to not accidentially overwrite them... */

  strcpy (libsmtp_from, libsmtp_from_opt);
  strcpy (libsmtp_to, libsmtp_to_opt);
  strcpy (libsmtp_mailhost, libsmtp_mailhost_opt);
  strcpy (libsmtp_subject, libsmtp_subject_opt);

  /* Create the socket */
  libsmtp_mailsocket = socket (PF_INET, SOCK_STREAM, 0);
  
  if (libsmtp_mailsocket < 0)
  {
    libsmtp_int_errno=errno;
    return 1;  /* socket couldn't be created */
  }
  
  if ((hostname=gethostbyname((const char *)libsmtp_mailhost))==NULL)
  {
    libsmtp_int_errno=errno;
    close (libsmtp_mailsocket);
    return 2; /* mailserver unknown */
  }
  
/*  printf ("%s\n", inet_ntoa ( *(struct in_addr *)hostname->h_addr)); */

  partner_ad.sin_family = AF_INET;
  partner_ad.sin_addr = *(struct in_addr *)hostname->h_addr;
  partner_ad.sin_port = htons (25);

  /* Now we make the connection to the smart host on the smtp port */

  if (connect (libsmtp_mailsocket, (struct sockaddr *) &partner_ad, sizeof (partner_ad) ) < 0)
  {
    libsmtp_int_errno=errno;
    close (libsmtp_mailsocket);
    return 3;
  }

  /* Lets hear his greeting */

  if (recv (libsmtp_mailsocket, libsmtp_buffer, 4000, 0)<0)
  {
    libsmtp_int_errno=errno;
    close (libsmtp_mailsocket);
    return 4;
  }
  
  if (libsmtp_debug)
    sprintf (libsmtp_tracelog, ">%s\n", libsmtp_buffer);

  /* Is it really a mail server who's answering? */

  if (!strstr (libsmtp_buffer, "220"))
  {
    close (libsmtp_mailsocket);
    return 5;
  }
  
  /* Ok, lets be nice and tell him our name.
     FIXME: This is still fixed at the moment */

  if (gethostname (libsmtp_myhostname, 300))
  {
    libsmtp_int_errno=errno;
    return 16;
  }

  sprintf (libsmtp_buffer, "HELO %s\n", libsmtp_myhostname);
  
  printf ("<%s\n", libsmtp_buffer);
  
  if (send (libsmtp_mailsocket, libsmtp_buffer, strlen (libsmtp_buffer), 0)<0)
  {
    libsmtp_int_errno=errno;
    close (libsmtp_mailsocket);
    return 6;
  }
  
  /* Will he accept our name? */

  if (recv (libsmtp_mailsocket, libsmtp_buffer, 4000, 0)<0)
  {
    libsmtp_int_errno=errno;
    close (libsmtp_mailsocket);
    return 4;
  }

  if (libsmtp_debug)
    sprintf (libsmtp_tracelog, ">%s\n", libsmtp_buffer);

  if (!strstr (libsmtp_buffer, "250"))
  {
    close (libsmtp_mailsocket);
    return 7;
  }

  /* Ok, he took it, so lets tell him from where the mail's coming */

  sprintf (libsmtp_buffer, "MAIL FROM: %s\n", libsmtp_from);
  
  if (libsmtp_debug)
    sprintf (libsmtp_tracelog, "<%s\n",libsmtp_buffer);

  if (send (libsmtp_mailsocket, libsmtp_buffer, strlen (libsmtp_buffer), 0)<0)
  {
    libsmtp_int_errno=errno;
    close (libsmtp_mailsocket);
    return 6;
  }
  
  /* Is that ok with the smart host? */

  if (recv (libsmtp_mailsocket, libsmtp_buffer, 4000, 0)<0)
  {
    libsmtp_int_errno=errno;
    close (libsmtp_mailsocket);
    return 4;
  }

  if (libsmtp_debug)
    sprintf (libsmtp_tracelog, ">%s\n", libsmtp_buffer);

  if (!strstr (libsmtp_buffer, "250"))
  {
    close (libsmtp_mailsocket);
    return 8;
  }

  /* It seems so. Ok, and now he's to know where to send it. */

  sprintf (libsmtp_buffer, "RCPT TO: %s\n", libsmtp_to);
  
  if (libsmtp_debug)
   sprintf (libsmtp_tracelog, "<%s\n",libsmtp_buffer);

  if (send (libsmtp_mailsocket, libsmtp_buffer, strlen (libsmtp_buffer), 0)<0)
  {
    libsmtp_int_errno=errno;
    close (libsmtp_mailsocket);
    return 6;
  }
  
  /* Is that ok with the smart host? */

  if (recv (libsmtp_mailsocket, libsmtp_buffer, 4000, 0)<0)
  {
    libsmtp_int_errno=errno;
    close (libsmtp_mailsocket);
    return 4;
  }

  if (libsmtp_debug)
    sprintf (libsmtp_tracelog, ">%s\n", libsmtp_buffer);

  if (!strstr (libsmtp_buffer, "250"))
  {
    close (libsmtp_mailsocket);
    return 9;
  }

  /* It seems so. Now we'll start the DATA part */

  sprintf (libsmtp_buffer, "DATA\n");
  
  if (libsmtp_debug)
    sprintf (libsmtp_tracelog, "<%s\n",libsmtp_buffer);

  if (send (libsmtp_mailsocket, libsmtp_buffer, strlen (libsmtp_buffer), 0)<0)
  {
    libsmtp_int_errno=errno;
    close (libsmtp_mailsocket);
    return 6;
  }
  
  /* Is that ok with the smart host? */

  if (recv (libsmtp_mailsocket, libsmtp_buffer, 4000, 0)<0)
  {
    libsmtp_int_errno=errno;
    close (libsmtp_mailsocket);
    return 4;
  }

  if (libsmtp_debug)
    sprintf (libsmtp_tracelog, ">%s\n", libsmtp_buffer);

  if (!strstr (libsmtp_buffer, "354"))
  {
    close (libsmtp_mailsocket);
    return 10;
  }

  /* Time for some subject */

  sprintf (libsmtp_buffer, "Subject: %s\nX-Mailer: C-Mailer routines\n\n", libsmtp_subject);
  
  if (libsmtp_debug)
    sprintf (libsmtp_tracelog, "<%s\n",libsmtp_buffer);

  if (send (libsmtp_mailsocket, libsmtp_buffer, strlen (libsmtp_buffer), 0)<0)
  {
    libsmtp_int_errno=errno;
    close (libsmtp_mailsocket);
    return 6;
  }
  
  /* Good. Now we'll copy each part of the body in 1k chunks */
  
  memset (libsmtp_buffer, 0, 3999);

  libsmtp_times = strlen(libsmtp_body_opt)/2048;

  for (libsmtp_temp=0; libsmtp_temp<libsmtp_times; libsmtp_temp++)
  {
    memcpy (libsmtp_buffer, libsmtp_body_opt+(libsmtp_temp*2048), 2048);
    printf ("\n\n%s\n\n", libsmtp_buffer);

    if (send (libsmtp_mailsocket, libsmtp_buffer, 2048, 0)<0)
    {
      libsmtp_int_errno=errno;
      close (libsmtp_mailsocket);
      return 6;
    }
  }

  fraglength = strlen(libsmtp_body_opt) %2048;

  memcpy (libsmtp_buffer, libsmtp_body_opt+(libsmtp_temp*2048), fraglength);

/*  libsmtp_buffer[strlen(body) % 2048]='\n';
  libsmtp_buffer[strlen(body) % 2048 + 1]='\0'; */
/*  printf ("\n\n%s\n\n", libsmtp_buffer); */

  if (send (libsmtp_mailsocket, libsmtp_buffer, fraglength, 0)<0)
  {
    libsmtp_int_errno = errno;
    close (libsmtp_mailsocket);
    return 6;
  }

  /* Time to end the conversation */
  
  sprintf (libsmtp_buffer, "\n.\n");

  if (send (libsmtp_mailsocket, libsmtp_buffer, 3, 0)<0)
  {
    libsmtp_int_errno=errno;
    close (libsmtp_mailsocket);
    return 6;
  }

  /* Did the smart host get all that ? */

  if (recv (libsmtp_mailsocket, libsmtp_buffer, 4000, 0)<0)
  {
    libsmtp_int_errno=errno;
    close (libsmtp_mailsocket);
    return 4;
  }

  if (libsmtp_debug)
    sprintf (libsmtp_tracelog, ">%s\n", libsmtp_buffer);

  if (!strstr (libsmtp_buffer, "250"))
  {
    close (libsmtp_mailsocket);
    return 11;
  }

  /* Now we say goodbye! */
  
  sprintf (libsmtp_buffer, "QUIT\n");
  
  if (libsmtp_debug)
    sprintf (libsmtp_tracelog, ">%s", libsmtp_buffer);

  if (send (libsmtp_mailsocket, libsmtp_buffer, strlen (libsmtp_buffer), 0)<0)
  {
    libsmtp_int_errno=errno;
    close (libsmtp_mailsocket);
    return 6;
  }

  /* Now will he give us the correct end sequence ? ;-) */

  if (recv (libsmtp_mailsocket, libsmtp_buffer, 4000, 0)<0)
  {
    libsmtp_int_errno=errno;
    close (libsmtp_mailsocket);
    return errno;
  }

  if (libsmtp_debug)
    sprintf (libsmtp_tracelog, ">%s\n", libsmtp_buffer);

  if (!strstr (libsmtp_buffer, "221"))
  {
    close (libsmtp_mailsocket);
    return 0;
  }
  
  close (libsmtp_mailsocket);
  
  return 0;
}
