/*
  libsmtp is a library to send mail via SMTP
    These are the utility data functions.

Copyright © 2001 Kevin Read <obsidian@berlios.de>

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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../config.h"

#include "../include/libsmtp.h"


/* This function returns a pointer to an allocated libsmtp_session_struct
   All GStrings are initialized. */

struct libsmtp_session_struct *libsmtp_session_initialize (void)
{
  struct libsmtp_session_struct *libsmtp_session;

  /* We use calloc here to clear the memory. GLists are initialized when
     they point to NULL, so it must be cleared. */
  libsmtp_session = (struct libsmtp_session_struct *)calloc (1, sizeof(struct libsmtp_session_struct));

  if (libsmtp_session == NULL)
    return NULL;

  #ifdef WITH_MIME
    libsmtp_session->Parts = NULL;
    libsmtp_session->NumParts = 0;
  #endif

  return libsmtp_session;
}

/* This frees the specified libsmtp_session_struct and all accompanying
   GStrings and GLists */

int libsmtp_free (struct libsmtp_session_struct *libsmtp_session)
{
  int libsmtp_temp;
  struct libsmtp_recipient_struct *recipient_iterator, *next_recipient_iterator;

  /* Lets see if we gotta close the socket */
  if (libsmtp_session->socket)
  {
    close (libsmtp_session->socket);
    libsmtp_session->socket=0;
  }
  recipient_iterator = libsmtp_session->recipients;

  while (recipient_iterator)
  {
  	next_recipient_iterator = recipient_iterator;
  	if (recipient_iterator->response_str)
  		free (recipient_iterator->response_str);
  	free (recipient_iterator);
  	recipient_iterator = next_recipient_iterator;
  }
  libsmtp_session->recipients = NULL;


  if (libsmtp_session->from)
  	free (libsmtp_session->from);

  if (libsmtp_session->subject)
  	free (libsmtp_session->subject);

  /* Ok, lets free the malloced session struct */
  free (libsmtp_session);

  return 0;
}

/* This function sets the environment for the session. At the moment it
   just sets subject and sender address. SSL and auth stuff should be set
   here in the future. */

int libsmtp_set_environment (char *libsmtp_int_From, char *libsmtp_int_Subject,\
      unsigned int libsmtp_int_flags, struct libsmtp_session_struct *libsmtp_session)
{
	uint32_t fromlength = strlen (libsmtp_int_From);
	uint32_t subjectlength = strlen (libsmtp_int_Subject);
  if ((!fromlength) || (!subjectlength))
  {
    libsmtp_session->errorCode = LIBSMTP_BADARGS;
    return LIBSMTP_BADARGS;
  }

  // RFC2822 states a maxmimum line length of 998 octets. We enforce this here
  // (maybe a bit too simply)
  if ((fromlength > 991) || (subjectlength > 988))
  {
    libsmtp_session->errorCode = LIBSMTP_LINETOOLONG;
    return LIBSMTP_LINETOOLONG;
  }

  if (libsmtp_session->from)
  	free (libsmtp_session->from);

  if (libsmtp_session->subject)
  	free (libsmtp_session->subject);

  libsmtp_session->subject = strdup (libsmtp_int_Subject);
  libsmtp_session->from = strdup (libsmtp_int_From);

  return LIBSMTP_NOERR;

}

int libsmtp_ll_add (struct libsmtp_recipient_struct **base, struct libsmtp_recipient_struct *new)
{
	void *next = *base;
	void *last;

	// The base element might already be null
	if (next)
		new->next = next;
	*base = new;
}



int libsmtp_add_recipient (int libsmtp_int_rec_type, char *libsmtp_int_address,
      struct libsmtp_session_struct *libsmtp_session)
{
  struct libsmtp_recipient_struct *new_ll_entry;
  uint32_t addresslength = strlen (libsmtp_int_address);

  /* Lets just check that rec_type isn't an invalid value */
  if ((libsmtp_int_rec_type < 0) || (libsmtp_int_rec_type > LIBSMTP_REC_MAX))
  {
    libsmtp_session->errorCode = LIBSMTP_BADARGS;
    return LIBSMTP_BADARGS;
  }

  /* Zero length string as argument? */
  if (!addresslength)
  {
    libsmtp_session->errorCode = LIBSMTP_BADARGS;
    return LIBSMTP_BADARGS;
  }

  // Enforce RFC 2822 max line length of 998 octets in a simple, brutal way
  if (addresslength > 992)
  {
    libsmtp_session->errorCode = LIBSMTP_LINETOOLONG;
    return LIBSMTP_LINETOOLONG;
  }

	// Check the type parameter for correctness
	if ((libsmtp_int_rec_type != LIBSMTP_REC_TO) &&
			(libsmtp_int_rec_type != LIBSMTP_REC_CC) &&
			(libsmtp_int_rec_type != LIBSMTP_REC_BCC))
	{
		libsmtp_session->errorCode = LIBSMTP_BADARGS;
		return LIBSMTP_BADARGS;
	}

	// Malloc a new linked list entry, which might fail
  new_ll_entry = malloc (sizeof (struct libsmtp_recipient_struct));
  if (!new_ll_entry)
  {
		libsmtp_session->errorCode = LIBSMTP_MALLOCFAIL;
		return (LIBSMTP_MALLOCFAIL);
	}
	else
	{
		// So it worked ok. Copy the address
  	new_ll_entry->address = strdup (libsmtp_int_address);

  	// strdup might fail:
  	if (!new_ll_entry->address)
  	{
  		// Yup, it did :(
  		free (new_ll_entry);
  		libsmtp_session->errorCode = LIBSMTP_MALLOCFAIL;
  		return (LIBSMTP_MALLOCFAIL);
  	}

    // The type is ok, we can set it
  	new_ll_entry->type = libsmtp_int_rec_type;

  	switch (libsmtp_int_rec_type)
  	{
  		case LIBSMTP_REC_TO: libsmtp_session->num_to++; break;
  		case LIBSMTP_REC_CC: libsmtp_session->num_cc++; break;
  		case LIBSMTP_REC_BCC: libsmtp_session->num_bcc++; break;
  	}

  	// Fill in reasonable default values for the rest
  	new_ll_entry->response_str = NULL;
  	new_ll_entry->response_code = 0;

  	libsmtp_ll_add (&(libsmtp_session->recipients), new_ll_entry);
  }

  return LIBSMTP_NOERR;
}
