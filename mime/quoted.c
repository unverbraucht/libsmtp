/*
  libsmtp is a library to send mail via SMTP
     This part will encode data into quoted-printable format
     
Copyright � 2001 Kevin Read <obsidian@berlios.de>

This software is available under the GNU Lesser Public License as described
in the COPYING file.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free
Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

Kevin Read <obsidian@berlios.de>
Thu Aug 16 2001 */

/* This will only be included when MIME is enabled */

/* #ifndef __G_LIB_H__ */
  #include <glib.h>
/* #endif */

#define LINELEN 72		      /* Encoded line length (max 76) */

#include "../config.h"

#include "libsmtp.h"
#include "libsmtp_mime.h"


/* This function will send data in a part, encoded in quoted printable format.
   It will not perform any checks whatsoever. */

int libsmtp_int_send_quoted (char *libsmtp_int_data, unsigned long int libsmtp_int_length, \
         struct libsmtp_session_struct *libsmtp_session)
{
  int libsmtp_int_temp, libsmtp_int_counter;

  /* These are the input buffer and the output buffer */
  unsigned char libsmtp_int_ogroup[2056], libsmtp_int_obuffer[4];
  unsigned char libsmtp_int_char;
  int libsmtp_int_finished=0, libsmtp_int_outbytes=0, libsmtp_int_width=0;
  int libsmtp_int_copied=0, libsmtp_int_newline=0;
  /* This points into the data stream to the byte we are reading ATM */
  unsigned long int libsmtp_int_data_ptr=0;
  GString *libsmtp_int_gstring=g_string_new (NULL);
  
  /* Lets clear the buffers */
  bzero (libsmtp_int_obuffer, 4);
  bzero (libsmtp_int_ogroup, 2056);
  
  /* The main parsing loop */
  while (!libsmtp_int_finished)
  {
    /* We fetch a character from the input buffer */
    libsmtp_int_char = libsmtp_int_data[libsmtp_int_data_ptr++];

    /* Lets check that we don't read over the end of the input buffer */
    if (libsmtp_int_data_ptr >= libsmtp_int_length)
      libsmtp_int_finished = 1;

    /* We decide which characters to encode and which not */
    switch (libsmtp_int_char)
    {
      /* Don't encode whitespace (except at end of line - we check for that
         below */
      case 9:
      case 32:
        libsmtp_int_obuffer[0]=0;
      break;
      
      /* We need to encode 61 */
      case 61:
        sprintf (libsmtp_int_obuffer, "=%2X", 61); /* OPTIMIZE ME */
      break;
    
      /* Newlines need special observation */
      case '\r':
      case '\n':
        /* Newline characters don't need to be encoded, but if they follow
           whitespace, the whitespace has to be suffixed with = */
        libsmtp_int_obuffer[0]=0;
        libsmtp_int_newline=1;
      break;
      
      default:
        /* All values below 33 need to be encoded except newline/CR and tab
           which we have already dealt with above. All values above 126 needs
           to be encoded as well */
        if ((libsmtp_int_char < 33) || (libsmtp_int_char > 126 ))
        {
          sprintf (libsmtp_int_obuffer, "=%2X", libsmtp_int_char);
        }
        else
        {
          /* The rest can go unencoded */
          libsmtp_int_obuffer[0]=0;
        }
      break;
    }
    
    /* Lets see ... */
    if (libsmtp_int_obuffer[0])
    {
      /* Something encoded */
      strcat (libsmtp_int_ogroup, libsmtp_int_obuffer);
      libsmtp_int_outbytes+=3;
      libsmtp_int_width+=3;

      #ifdef LIBSMTP_QUOTED_DEBUG
        printf ("Got encoded: %c = %s. Group: %s (%d)\n", libsmtp_int_char,
             libsmtp_int_obuffer, libsmtp_int_ogroup, libsmtp_int_outbytes);
      #endif
    }
    else
    {
      /* An unencoded char */
      if (libsmtp_int_newline)
      {
        libsmtp_int_newline=0;
        /* We have to check if the char before us is whitespace */
        if (libsmtp_int_outbytes)
        {
          if (libsmtp_int_ogroup[libsmtp_int_outbytes-1]==' ')
          {
            /* We need to add a = after the whitespace */
            libsmtp_int_ogroup[libsmtp_int_outbytes]= '=';
            libsmtp_int_ogroup[libsmtp_int_outbytes+1]= libsmtp_int_char;
            libsmtp_int_ogroup[libsmtp_int_outbytes+2]=0;
            libsmtp_int_outbytes+=2;
            /* We have a new line */
            libsmtp_int_width=0;
          }
        }
      }
      else
      {
        /* An unencoded character is added */
        libsmtp_int_ogroup[libsmtp_int_outbytes] = libsmtp_int_char;
        libsmtp_int_ogroup[libsmtp_int_outbytes+1] = 0;
        libsmtp_int_outbytes++;
        libsmtp_int_width++;
      }

      #ifdef LIBSMTP_QUOTED_DEBUG
        printf ("Got unencoded: %c = %s. Group: %s (%d)\n", libsmtp_int_char,
             libsmtp_int_obuffer, libsmtp_int_ogroup, libsmtp_int_outbytes);
      #endif
    }

    /* After 72 characters in a line we need a soft linebreak */
    if (libsmtp_int_width > 72 )
    {
      libsmtp_int_ogroup[libsmtp_int_outbytes++]='=';
      libsmtp_int_ogroup[libsmtp_int_outbytes++]='\r';
      libsmtp_int_ogroup[libsmtp_int_outbytes++]='\n';
      libsmtp_int_width=0;
    }
      
    /* If we have more than 2K of data, we send it */
    if (libsmtp_int_outbytes >=2048)
    {
      libsmtp_int_ogroup[libsmtp_int_outbytes]='\0';
      if (libsmtp_int_send_body (libsmtp_int_ogroup, libsmtp_int_outbytes, libsmtp_session))
        return LIBSMTP_ERRORSENDFATAL;
        
      #ifdef LIBSMTP_DEBUG
        printf ("libsmtp_send_quoted: out: %s\n", libsmtp_int_ogroup);
      #endif

      /* We reset the pointer into our outbuffer, too */
      libsmtp_int_outbytes=0;
    }
  }

  /* We send the rest of the data out anyway. It is better to send a linebreak
     here so the next pack won't go over 72 characters a line */

  libsmtp_int_ogroup[libsmtp_int_outbytes++]='\r';
  libsmtp_int_ogroup[libsmtp_int_outbytes++]='\n';
  libsmtp_int_ogroup[libsmtp_int_outbytes]='\0';
  if (libsmtp_int_send_body (libsmtp_int_ogroup, libsmtp_int_outbytes, libsmtp_session))
    return LIBSMTP_ERRORSENDFATAL;
   
  #ifdef LIBSMTP_DEBUG
    printf ("libsmtp_send_quoted: out: %s\n", libsmtp_int_ogroup);
  #endif
  
  return LIBSMTP_NOERR;
}

