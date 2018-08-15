/*
  libsmtp is a library to send mail via SMTP
     This is the communication part

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

#include "../config.h"
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "libsmtp.h"

/* Ed Goforth <e.goforth@computer.org> */
/* Have to include time.h to get time_t, time(2) and localtime(3) */
/* This should be moved to configure.in and config.h */
#include <time.h>

/* internal communication functions */

/* Type is one of:
	0 normal body mesg
	1 normal header mesg
	2 normal dialogue mesg

  This function won't return correct libsmtp_gstring_read for dialogue mesgs */

int libsmtp_int_read (uint32_t *buffer_size, struct libsmtp_session_struct *libsmtp_session, int type)
{
  int libsmtp_int_bytes;
  char *libsmtp_int_temp_buffer;

  libsmtp_int_bytes=recv (libsmtp_session->socket, libsmtp_session->buffer, buffer_size, 0);
  if (libsmtp_int_bytes<=0)
  {
    libsmtp_session->errorCode=LIBSMTP_ERRORREADFATAL;
    libsmtp_session->stage=type;
    close (libsmtp_session->socket);
    libsmtp_session->socket=0;
    return LIBSMTP_ERRORREAD;
  }

  #ifdef LIBSMTP_DEBUG
    printf ("DEBUG in read: %s\n", libsmtp_session->buffer);
  #endif

  /* Update statistics */
  switch (type)
  {
    case (0):
      libsmtp_session->bodyBytes+=libsmtp_int_bytes;
      break;

    case (1):
      libsmtp_session->headerBytes+=libsmtp_int_bytes;
      libsmtp_session->headersSent++;
      break;

    case (2):
      libsmtp_session->dialogueBytes+=libsmtp_int_bytes;
      libsmtp_session->dialogueSent++;

      /* Ok, take the first part of the response ... */
      libsmtp_int_temp_buffer = strtok (libsmtp_session->buffer, " ");

      /* and extract the response code */
      libsmtp_session->lastResponseCode = atoi(libsmtp_int_temp_buffer);

      /* Then fetch the rest of the string and save it */
      libsmtp_int_temp_buffer = strtok (NULL, "\0");
      strncpy (libsmtp_session->lastResponse, libsmtp_int_temp_buffer, 511);
      break;
  }
  *buffer_size = libsmtp_int_bytes;
  return LIBSMTP_NOERR;
}

/* Type is one of:
	0 normal body mesg
	1 normal header mesg
	2 normal dialogue mesg */

// FIXME: This is brain-dead. We should loop while sending until all bytes have been sent.
int libsmtp_int_send (uint32_t buffer_size, struct libsmtp_session_struct *libsmtp_session, int type)
{
  int libsmtp_int_bytes;

  #ifdef LIBSMTP_DEBUG
    printf ("DEBUG in send: %s\n", libsmtp_session->buffer);
  #endif

  libsmtp_int_bytes=send (libsmtp_session->socket, libsmtp_session->buffer, buffer_size, 0);
  if (libsmtp_int_bytes<0)
  {
    libsmtp_session->errorCode=LIBSMTP_ERRORSENDFATAL;
    libsmtp_session->stage=type;
    close (libsmtp_session->socket);
    libsmtp_session->socket=0;
    return LIBSMTP_ERRORSEND;
  }
  /* Update statistics */
  switch (type)
  {
    case (0):
      libsmtp_session->bodyBytes+=libsmtp_int_bytes;
      break;
    case (1):
      libsmtp_session->headerBytes+=libsmtp_int_bytes;
      libsmtp_session->headersSent++;
      break;
    case (2):
      libsmtp_session->dialogueBytes+=libsmtp_int_bytes;
      libsmtp_session->dialogueSent++;
      break;
  }

  return LIBSMTP_NOERR;
}

int libsmtp_int_send_body (uint32_t libsmtp_int_length, \
         struct libsmtp_session_struct *libsmtp_session)
{
  int libsmtp_int_bytes;

  #ifdef LIBSMTP_DEBUG
    printf ("DEBUG in bsend : %s\n", libsmtp_session->buffer);
  #endif

  libsmtp_int_bytes=send (libsmtp_session->socket, libsmtp_session->buffer, libsmtp_int_length, 0);
  if (libsmtp_int_bytes<0)
  {
    libsmtp_session->errorCode=LIBSMTP_ERRORSENDFATAL;
    close (libsmtp_session->socket);
    libsmtp_session->socket=0;
    return LIBSMTP_ERRORSEND;
  }
  /* Update statistics */
  libsmtp_session->bodyBytes+=libsmtp_int_bytes;

  return LIBSMTP_NOERR;
}

/* Use this function to make libsmtp run the SMTP dialogue */

int libsmtp_dialogue (struct libsmtp_session_struct *libsmtp_session)
{
  int libsmtp_temp;
  uint32_t buffer_size;
  struct libsmtp_recipient_struct *iterator;

  /* This can only be used if the hello stage is finished, but we haven't
     entered data stage yet */
  if ((libsmtp_session->stage < LIBSMTP_HELLO_STAGE) ||
      (libsmtp_session->stage >= LIBSMTP_DATA_STAGE))
  {
    libsmtp_session->errorCode = LIBSMTP_BADSTAGE;
    return LIBSMTP_BADSTAGE;
  }

  /* First we check if sender, subject and at least one recipient has
     been set */

  if (!libsmtp_session->recipients || !libsmtp_session->subject || !libsmtp_session->from)
  {
    libsmtp_session->errorCode = LIBSMTP_BADARGS;
    return LIBSMTP_BADARGS;
  }

  /* We enter the sender stage now */
  libsmtp_session->stage = LIBSMTP_SENDER_STAGE;

  /* Ok, now lets give him the sender address */
  buffer_size = snprintf (libsmtp_session->buffer, 4095, "MAIL FROM: %s\r\n", \
                      libsmtp_session->from);

  if (libsmtp_int_send (buffer_size, libsmtp_session, 2))
  {
    libsmtp_session->errorCode = LIBSMTP_ERRORSENDFATAL;
    return LIBSMTP_ERRORSENDFATAL;
  }

  /* Now we have to see if he likes it */
  if (libsmtp_int_read (&buffer_size, libsmtp_session, 2))
  {
    libsmtp_session->errorCode = LIBSMTP_ERRORREADFATAL;
    return LIBSMTP_ERRORREADFATAL;
  }

  if (libsmtp_session->lastResponseCode > 299)
  {
    libsmtp_session->errorCode = LIBSMTP_WONTACCEPTSENDER;
    close(libsmtp_session->socket);
    libsmtp_session->socket=0;
    return LIBSMTP_WONTACCEPTSENDER;
  }

  /* We enter the recipient stage now */
  libsmtp_session->stage = LIBSMTP_RECIPIENT_STAGE;

  /* Now we go through all recipients, To: first */
  for (iterator = libsmtp_session->recipients; iterator; iterator = iterator->next)
  {
  	buffer_size = snprintf (libsmtp_session->buffer, 4095, "RCPT TO: %s\r\n", \
                  iterator->address);

    /* Every recipient gets sent to the server */
    if (libsmtp_int_send (buffer_size, libsmtp_session, 2))
    {
      libsmtp_session->errorCode = LIBSMTP_ERRORSENDFATAL;
      return LIBSMTP_ERRORSENDFATAL;
    }

    /* We have to read the servers response of course */
    if (libsmtp_int_read (&buffer_size, libsmtp_session, 2))
    {
      libsmtp_session->errorCode = LIBSMTP_ERRORREADFATAL;
      return LIBSMTP_ERRORREADFATAL;
    }

    /* We write the response code into the response linked
       list so that denial reasons can be seen later */
    iterator->response_code = libsmtp_session->lastResponseCode;

    /* Did he like this one? */
    if (libsmtp_session->lastResponseCode > 299)
    {
      libsmtp_session->errorCode = LIBSMTP_WONTACCEPTREC;
      iterator->response_str = strdup (libsmtp_session->lastResponse);
    }
  }
  return LIBSMTP_NOERR;
}

/* With this function you can send SMTP dialogue strings yourself */

int libsmtp_dialogue_send (char *libsmtp_dialogue_string, \
         struct libsmtp_session_struct *libsmtp_session)
{
	uint32_t buffer_size;

  /* This can only be used if the hello stage is finished, but we haven't
     entered data stage yet */
  if ((libsmtp_session->stage < LIBSMTP_HELLO_STAGE) ||
      (libsmtp_session->stage >= LIBSMTP_DATA_STAGE))
  {
    libsmtp_session->errorCode = LIBSMTP_BADSTAGE;
    return LIBSMTP_BADSTAGE;
  }
  if (!libsmtp_dialogue_string)
  {
    libsmtp_session->errorCode = LIBSMTP_BADARGS;
    return LIBSMTP_BADARGS;
  }

  if (!strlen (libsmtp_dialogue_string))
  {
    libsmtp_session->errorCode = LIBSMTP_BADARGS;
    return LIBSMTP_BADARGS;
  }

  buffer_size = snprintf (libsmtp_session->buffer, 4095, "%\r\n", libsmtp_dialogue_string);

  if (libsmtp_int_send (buffer_size, libsmtp_session, 2))
  {
    libsmtp_session->errorCode = LIBSMTP_ERRORSENDFATAL;
    return LIBSMTP_ERRORSENDFATAL;
  }

  /* We have to read the servers response of course */
  if (libsmtp_int_read (&buffer_size, libsmtp_session, 2))
  {
    libsmtp_session->errorCode = LIBSMTP_ERRORREADFATAL;
    return LIBSMTP_ERRORREADFATAL;
  }

  /* We don't look at the response code here - the app will have to do that */
  return LIBSMTP_NOERR;
}

int libsmtp_int_genrecip (uint32_t type, uint32_t amount, char *command, struct libsmtp_session_struct *libsmtp_session)
{
	uint32_t buffer_size;
	struct libsmtp_recipient_struct *iterator, *iterator_start;
	iterator_start = libsmtp_session->recipients;
	int started = 0;
	char whitespace[3] = "   ";
	char *this_command = command;

	if (!amount)
		return LIBSMTP_NOERR;

	while (iterator)
	{
		if (iterator->type == type)
		{
			if (started)
				this_command = whitespace;
			else
			{
				started = 1;
				this_command = command;
			}
			if (amount > 1)
			{
				buffer_size = snprintf (libsmtp_session->buffer, 4095, "%s%s,\n",
						this_command, iterator->address);
				if (libsmtp_int_send (buffer_size, libsmtp_session, 1))
					return LIBSMTP_ERRORSENDFATAL;
			}
			else
			{
				buffer_size = snprintf (libsmtp_session->buffer, 4095, "%s%s\n",
						this_command, iterator->address);
				if (libsmtp_int_send (buffer_size, libsmtp_session, 1))
					return LIBSMTP_ERRORSENDFATAL;
			}
		}
	}
	return LIBSMTP_NOERR;
}


int libsmtp_int_enter_data (struct libsmtp_session_struct *libsmtp_session)
{
	uint32_t buffer_size;
  /* Maybe we are already in DATA mode so... */
  if (libsmtp_session->stage < LIBSMTP_DATA_STAGE)
  {
    /* Great finality. After this no more dialogue can go on */
    buffer_size = sprintf (libsmtp_session->buffer, "DATA\r\n");

    if (libsmtp_int_send (buffer_size, libsmtp_session, 2))
    {
    	libsmtp_session->errorCode = LIBSMTP_ERRORSENDFATAL;
      return LIBSMTP_ERRORSENDFATAL;
     }

    /* What has he say to a little bit of DATA? */

    if (libsmtp_int_read (&buffer_size, libsmtp_session, 2))
    {
      libsmtp_session->errorCode = LIBSMTP_ERRORREADFATAL;
      return LIBSMTP_ERRORREADFATAL;
    }

    if (libsmtp_session->lastResponseCode != 354)
    {
      libsmtp_session->errorCode = LIBSMTP_WONTACCEPTDATA;
      close(libsmtp_session->socket);
      libsmtp_session->socket=0;
      return LIBSMTP_WONTACCEPTDATA;
    }

    /* We enter the data stage now */
    libsmtp_session->stage = LIBSMTP_HEADERS_STAGE;
  }
  return LIBSMTP_NOERR;
}

/* This function starts the DATA part. No more dialogue stuff can be sent
   from this time on. */
int libsmtp_headers (struct libsmtp_session_struct *libsmtp_session)
{
  int libsmtp_temp, retcode;
  uint32_t buffer_size;
  time_t curr_time; /* for the rfc822 date string */

  /* Are we at the end of the dialogue stage, but haven't sent the
     body yet? */
  if ((libsmtp_session->stage < LIBSMTP_RECIPIENT_STAGE) || \
      (libsmtp_session->stage > LIBSMTP_DATA_STAGE))
  {
    libsmtp_session->errorCode = LIBSMTP_BADSTAGE;
    return LIBSMTP_BADSTAGE;
  }

  // Enter DATA stage if necessary
  retcode = libsmtp_int_enter_data (libsmtp_session);
  if (retcode)
  	return (retcode);

  /* Now we send through all the headers. No more responses will come from
     the mailserver until we end the DATA part. */

  /* Ed Goforth <e.goforth@computer.org> */
  /* According to rfc822, should send the Date: header first */
  curr_time = time(0);
  strftime(libsmtp_session->date, 39,
           LIBSMTP_DATE_STR_FMT, localtime(&curr_time));
  buffer_size = sprintf (libsmtp_session->buffer, "Date: %s\n",
                    libsmtp_session->date);

  if (libsmtp_int_send (buffer_size, libsmtp_session, 1))
  {
  	libsmtp_session->errorCode = LIBSMTP_ERRORSENDFATAL;
    return LIBSMTP_ERRORSENDFATAL;
  }

  /* Then the From: header */
  buffer_size = snprintf (libsmtp_session->buffer, 4095, "From: %s\n",
                    libsmtp_session->from);

  if (libsmtp_int_send (buffer_size, libsmtp_session, 1))
    return LIBSMTP_ERRORSENDFATAL;

  /* Then the Subject: header */
  buffer_size = snprintf (libsmtp_session->buffer, 4095, "Subject: %s\n",
                    libsmtp_session->subject);

  if (libsmtp_int_send (buffer_size, libsmtp_session, 1))
    return LIBSMTP_ERRORSENDFATAL;

  // Send out all To: commands
  if (libsmtp_int_genrecip (LIBSMTP_REC_TO, libsmtp_session->num_to, "To: ", libsmtp_session))
  	return LIBSMTP_ERRORSENDFATAL;

  // Send out all To: commands
  if (libsmtp_int_genrecip (LIBSMTP_REC_CC, libsmtp_session->num_cc, "Cc: ", libsmtp_session))
  	return LIBSMTP_ERRORSENDFATAL;

  return LIBSMTP_NOERR;

}


/* With this function you can send custom headers. */

int libsmtp_header_send (char *libsmtp_header_string, \
       struct libsmtp_session_struct *libsmtp_session)
{
  uint32_t buffer_size;
  int retcode;

  /* Are we at the end of the dialogue stage, but haven't sent the
     DATA yet? */
  if ((libsmtp_session->stage < LIBSMTP_RECIPIENT_STAGE) || \
      (libsmtp_session->stage > LIBSMTP_HEADERS_STAGE))
  {
    libsmtp_session->errorCode = LIBSMTP_BADSTAGE;
    return LIBSMTP_BADSTAGE;
  }

  // Enter DATA stage if necessary
  retcode = libsmtp_int_enter_data (libsmtp_session);
  if (!retcode)
  	return (retcode);

  /* Ok. Lets send these custom headers. */
  buffer_size = strlen (libsmtp_header_string);
  strncpy (libsmtp_session->buffer, libsmtp_header_string, 4095);

  if (libsmtp_int_send (buffer_size, libsmtp_session, 1))
    return LIBSMTP_ERRORSENDFATAL;

  return LIBSMTP_NOERR;
}

/* This function sends raw body data. It can only be used in the appropriate
   stage. The data to be sent has to be formatted according to RFC822 and
   the MIME standards. */

int libsmtp_body_send_raw (char *libsmtp_body_data, unsigned long int libsmtp_int_length, \
            struct libsmtp_session_struct *libsmtp_session)
{
	uint32_t buffer_size, remaining_size = libsmtp_int_length;
  /* Headers should have been sent before body data goes out, but we
     must still be in the body stage at most */
  if ((libsmtp_session->stage < LIBSMTP_HEADERS_STAGE) ||
      (libsmtp_session->stage > LIBSMTP_BODY_STAGE))
  {
    libsmtp_session->errorCode = LIBSMTP_BADSTAGE;
    return LIBSMTP_BADSTAGE;
  }

  /* If we just came from the headers stage, we have to send a blank line
     first */

  /* Headers should have been sent before body data goes out */
  if (libsmtp_session->stage = LIBSMTP_HEADERS_STAGE)
  {
    /* Now let there be a blank line */
    buffer_size = sprintf (libsmtp_session->buffer, "\n");

    if (libsmtp_int_send (buffer_size, libsmtp_session, 1))
      return LIBSMTP_ERRORSENDFATAL;
  }

  /* We now enter the body stage */
  libsmtp_session->stage = LIBSMTP_BODY_STAGE;

  while (remaining_size > 0)
  {
  	buffer_size = remaining_size % 4095;
  	strncpy (libsmtp_session->buffer, libsmtp_body_data, buffer_size);
		if (libsmtp_int_send_body (buffer_size, libsmtp_session))
			return LIBSMTP_ERRORSENDFATAL;

		remaining_size -= buffer_size;
	}

  return LIBSMTP_NOERR;
}



/* This function ends the body part. It can only be used in certain stages */
int libsmtp_body_end (struct libsmtp_session_struct *libsmtp_session)
{
	uint32_t buffer_size;

  /* We need to be in body stage to leave it :) */
  if (libsmtp_session->stage < LIBSMTP_BODY_STAGE)
  {
    libsmtp_session->errorCode = LIBSMTP_BADSTAGE;
    return LIBSMTP_BADSTAGE;
  }

  /* We now enter the finished stage */
  libsmtp_session->stage = LIBSMTP_FINISHED_STAGE;

  /* Now let there be a line with only a dot on it */
  buffer_size = sprintf (libsmtp_session->buffer, "\r\n");
  if (libsmtp_int_send_body (buffer_size, libsmtp_session))
    return LIBSMTP_ERRORSENDFATAL;

  buffer_size = sprintf (libsmtp_session->buffer, ".\r\n");
  if (libsmtp_int_send_body (buffer_size, libsmtp_session))
    return LIBSMTP_ERRORSENDFATAL;

  /* Did you like that body, connisseur? */

  // sleep (2);
  if (libsmtp_int_read (&buffer_size, libsmtp_session, 2))
    return LIBSMTP_ERRORREADFATAL;

  #ifdef LIBSMTP_DEBUG
    printf ("DEBUG: %s\n", libsmtp_session->lastResponse);
  #endif

  if (libsmtp_session->lastResponseCode > 299)
  {
    /* Aaaw no, he didn't. Don't ask me how that can happen... */

    libsmtp_session->errorCode = LIBSMTP_REJECTBODY;
    close (libsmtp_session->socket);
    libsmtp_session->socket=0;
    return LIBSMTP_REJECTBODY;
  }

  return LIBSMTP_NOERR;
}


/* This function ends the SMTP session. It can only be used in certain stages,
   notably in all dialogue modes. */

int libsmtp_quit (struct libsmtp_session_struct *libsmtp_session)
{
	uint32_t buffer_size;

  /* We need to be in body stage to leave it :) */
  if ((libsmtp_session->stage = LIBSMTP_FINISHED_STAGE) || \
      (libsmtp_session->stage < LIBSMTP_DATA_STAGE))
  {

    /* We now enter the quit stage */
    libsmtp_session->stage = LIBSMTP_QUIT_STAGE;

  buffer_size = sprintf (libsmtp_session->buffer, "QUIT\r\n");
  if (libsmtp_int_send (buffer_size, libsmtp_session, 1))
    return LIBSMTP_ERRORSENDFATAL;

    /* I hope thats okay with him :) */
    if (libsmtp_int_read (&buffer_size, libsmtp_session, 2))
      return LIBSMTP_ERRORREADFATAL;

    if (libsmtp_session->lastResponseCode > 299)
    {
      /* He says it isn't, but who cares... */
      libsmtp_session->errorCode = LIBSMTP_REJECTQUIT;
      close (libsmtp_session->socket);
      libsmtp_session->socket=0;
      libsmtp_session->stage=LIBSMTP_NOCONNECT_STAGE;
      return LIBSMTP_REJECTQUIT;
    }
    else
    {
      /* Babe, I'm gonne leave you... */

      libsmtp_session->errorCode = LIBSMTP_NOERR;
      close (libsmtp_session->socket);
      libsmtp_session->socket=0;
      libsmtp_session->stage=LIBSMTP_NOCONNECT_STAGE;
      return LIBSMTP_NOERR;
    }
  }

  /* Wrong stage, dude ! */
  libsmtp_session->errorCode = LIBSMTP_BADSTAGE;
  return LIBSMTP_BADSTAGE;
}
