/*
  just a small test app for libsmtp
   
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



/*******************************************************************
 * This is a demonstration for libsmtp - it is not intended to be  *
 * good coding style - it's artificial. You have been warned :)    *
 * All good coders, cover your eyes now ;)			   *
 *******************************************************************/

#include <glib.h>

#include "libsmtp.h"

int main(void)
{
  int test_temp=0;
  
  /* This struct holds all session data. You need one per mail server
     connection */
  struct libsmtp_session_struct *mailsession;
  
  /* We need these to look through the GLists later on */
  GList *temp_glist;
  GList *temp_glist_address;

  /* This mallocs the structs mem and initializes variables in it */
  mailsession = libsmtp_session_initialize();

  /* Set session environment (from address, subject) */
  libsmtp_set_environment ("libsmtp-test@hotmail.com","libsmtp Test", 0, mailsession);
  
  /* Now we add some recipients */
  libsmtp_add_recipient (LIBSMTP_REC_TO, "obsidian@berlios.de", mailsession);
  libsmtp_add_recipient (LIBSMTP_REC_TO, "libsmtp-test@obsidian.de", mailsession);
  libsmtp_add_recipient (LIBSMTP_REC_CC, "libsmtp-test@hotmail.com", mailsession);
  libsmtp_add_recipient (LIBSMTP_REC_CC, "steve.balmer@microsoft.com", mailsession);
  libsmtp_add_recipient (LIBSMTP_REC_BCC, "we_love_you@backstreetboys.com", mailsession);
  libsmtp_add_recipient (LIBSMTP_REC_BCC, "i_love_you_too@backsideboys.com", mailsession);

  /* This starts the SMTP connection */
  if (libsmtp_connect ("mail.obsidian.de",0,0,mailsession))
  {
    printf ("An error occured while connecting:\n%s\nLast Response:%s\n", \
      libsmtp_strerr (mailsession), mailsession->LastResponse->str);
    printf ("\nAll in all %u bytes of dialogue data (%u lines) were sent.\n",
      mailsession->DialogueBytes, mailsession->DialogueSent);
    printf ("%u bytes of header data (%u lines) were sent.\n",
       mailsession->HeaderBytes, mailsession->HeadersSent);
    printf ("%lu bytes of body data were sent.\n", mailsession->BodyBytes);

    return mailsession->ErrorCode;
  }
    
  /* This will conduct the SMTP dialogue */
  if (libsmtp_dialogue (mailsession))
  {
    printf ("An error occured while conducting the SMTP dialogue:\n%s\nLast Response:%s\n", \
      libsmtp_strerr (mailsession), mailsession->LastResponse->str);
    printf ("\nAll in all %u bytes of dialogue data (%u lines) were sent.\n",
      mailsession->DialogueBytes, mailsession->DialogueSent);
    printf ("%u bytes of header data (%u lines) were sent.\n",
       mailsession->HeaderBytes, mailsession->HeadersSent);
    printf ("%lu bytes of body data were sent.\n", mailsession->BodyBytes);

    return mailsession->ErrorCode;
  }
  
  /* Now lets send the headers - you can send your own headers too */
  if (libsmtp_headers (mailsession))
  {
    printf ("An error occured while sending header data:\n%s\nLast Response:%s\n", \
      libsmtp_strerr (mailsession), mailsession->LastResponse->str);
    printf ("\nAll in all %u bytes of dialogue data (%u lines) were sent.\n",
      mailsession->DialogueBytes, mailsession->DialogueSent);
    printf ("%u bytes of header data (%u lines) were sent.\n",
       mailsession->HeaderBytes, mailsession->HeadersSent);
    printf ("%lu bytes of body data were sent.\n", mailsession->BodyBytes);

    return mailsession->ErrorCode;
  }

  /* This sends a line of message body */
  if (libsmtp_body_send_raw ("Bla!!", strlen ("Bla!!"), mailsession))
  {
    printf ("An error occured while sending the body:\n%s\nLast Response:%s\n", \
      libsmtp_strerr (mailsession), mailsession->LastResponse->str);
    printf ("\nAll in all %u bytes of dialogue data (%u lines) were sent.\n",
      mailsession->DialogueBytes, mailsession->DialogueSent);
    printf ("%u bytes of header data (%u lines) were sent.\n",
       mailsession->HeaderBytes, mailsession->HeadersSent);
    printf ("%lu bytes of body data were sent.\n", mailsession->BodyBytes);

    return mailsession->ErrorCode;
  }

  /* This ends the body part */
  if (libsmtp_body_end (mailsession))
  {
    printf ("An error occured while sending the body:\n%s\nLast Response:%s\n", \
      libsmtp_strerr (mailsession), mailsession->LastResponse->str);
    printf ("\nAll in all %u bytes of dialogue data (%u lines) were sent.\n",
      mailsession->DialogueBytes, mailsession->DialogueSent);
    printf ("%u bytes of header data (%u lines) were sent.\n",
       mailsession->HeaderBytes, mailsession->HeadersSent);
    printf ("%lu bytes of body data were sent.\n", mailsession->BodyBytes);

    return mailsession->ErrorCode;
  }
  
  /* This ends the connection gracefully */
  if (libsmtp_quit (mailsession))
  {
    /* Actually there can't be an error here... :) */
    printf ("An error occured while closing the connection:\n%s\nLast Response:%s\n", \
      libsmtp_strerr (mailsession), mailsession->LastResponse->str);
    printf ("\nAll in all %u bytes of dialogue data (%u lines) were sent.\n",
      mailsession->DialogueBytes, mailsession->DialogueSent);
    printf ("%u bytes of header data (%u lines) were sent.\n",
       mailsession->HeaderBytes, mailsession->HeadersSent);
    printf ("%lu bytes of body data were sent.\n", mailsession->BodyBytes);

    return mailsession->ErrorCode;
  }

  /* Ok, lets print some stats */
  printf ("Mail sent successfully.\nLast Response:%s\n", \
      mailsession->LastResponse->str);
  printf ("\nAll in all %u bytes of dialogue data (%u lines) were sent.\n",
      mailsession->DialogueBytes, mailsession->DialogueSent);
  printf ("%u bytes of header data (%u lines) were sent.\n",
       mailsession->HeaderBytes, mailsession->HeadersSent);
  printf ("%lu bytes of body data were sent.\n", mailsession->BodyBytes);
  

  /* This lists the contents of the struct delivery response codes */
  printf ("The mail could not be delivered to the following recipients:\n");
  
  if (mailsession->NumFailedTo)
  {
    for (test_temp=0; test_temp<g_list_length (mailsession->ToResponse); test_temp++)
    {
      temp_glist = g_list_nth (mailsession->ToResponse, test_temp);
      if (atoi (temp_glist->data) > 299)
      {
        temp_glist_address = g_list_nth (mailsession->To, test_temp);
        printf ("%s: %s\n", temp_glist_address->data, temp_glist->data);
      }
    }
  }
  
  if (mailsession->NumFailedCC)
  {
    for (test_temp=0; test_temp<g_list_length (mailsession->CCResponse); test_temp++)
    {
      temp_glist = g_list_nth (mailsession->CCResponse, test_temp);
      if (atoi (temp_glist->data) > 299)
      {
        temp_glist_address = g_list_nth (mailsession->CC, test_temp);
        printf ("%s: %s\n", temp_glist_address->data, temp_glist->data);
      }
    }
  }

  if (mailsession->NumFailedBCC)
  {
    for (test_temp=0; test_temp<g_list_length (mailsession->BCCResponse); test_temp++)
    {
      temp_glist = g_list_nth (mailsession->BCCResponse, test_temp);
      if (atoi (temp_glist->data) > 299)
      {
        temp_glist_address = g_list_nth (mailsession->BCC, test_temp);
        printf ("%s: %s\n", temp_glist_address->data, temp_glist->data);
      }
    }
  }    
  
  /* Free the allocated struct mem */
  free (mailsession);
  return 0;
}