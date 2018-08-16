#ifdef __cplusplus
 extern "C" {
#endif

#ifndef LIB_SMTP_H

#define LIB_SMTP_H

#include <gmodule.h>

#include <stdint.h>

#define LIBSMTP_BUFFER_SIZE	4096

/* These flags show what the server can do */

#define LIBSMTP_HAS_TLS	1
#define LIBSMTP_HAS_8BIT	2
#define LIBSMTP_HAS_AUTH	4
#define LIBSMTP_HAS_PIPELINING	8
#define LIBSMTP_HAS_SIZE	16
#define LIBSMTP_HAS_DSN		32
#define LIBSMTP_HAS_ETRN	64
#define LIBSMTP_HAS_ENHANCEDSTATUSCODES	128

/* Recipient types for libsmtp_add_recipient */

#define LIBSMTP_REC_MAX	2

#define LIBSMTP_REC_TO	0
#define LIBSMTP_REC_CC	1
#define LIBSMTP_REC_BCC	2

/* SMTP transaction stages */

#define LIBSMTP_NOCONNECT_STAGE	0
#define LIBSMTP_CONNECT_STAGE	1
#define LIBSMTP_GREET_STAGE	2
#define LIBSMTP_HELLO_STAGE	3

#define LIBSMTP_SENDER_STAGE	16
#define LIBSMTP_RECIPIENT_STAGE	17
#define LIBSMTP_DATA_STAGE	18
#define LIBSMTP_HEADERS_STAGE	19
#define LIBSMTP_MIMEHEADERS_STAGE	20
#define LIBSMTP_BODY_STAGE	21

#define LIBSMTP_FINISHED_STAGE	128
#define LIBSMTP_QUIT_STAGE	256

/* Module types */

#define LIBSMTP_BODY_MODULE	0
#define LIBSMTP_MIME_MODULE	1
#define LIBSMTP_HEADER_MODULE	2
#define LIBSMTP_DIALOGUE_MODULE	3

/* These are the error definitions */

/* Error codes below 1024 are fatal errors - the socket will be closed */
#define LIBSMTP_NOERR		0
#define LIBSMTP_SOCKETNOCREATE	1
#define LIBSMTP_HOSTNOTFOUND	2
#define LIBSMTP_CONNECTERR	3
#define LIBSMTP_ERRORREADFATAL	4
#define LIBSMTP_NOTWELCOME	5
#define LIBSMTP_WHATSMYHOSTNAME	6
#define LIBSMTP_ERRORSENDFATAL	7
#define LIBSMTP_WONTACCEPTSENDER	8
#define LIBSMTP_REJECTBODY	9
#define LIBSMTP_WONTACCEPTDATA	10

/* Codes >= 1024 are errors that are not fatal to the whole SMTP session */
#define LIBSMTP_ERRORREAD	1024
#define LIBSMTP_ERRORSEND	1025
#define LIBSMTP_BADARGS		1026
#define LIBSMTP_WONTACCEPTREC	1027
#define LIBSMTP_BADSTAGE	1028
#define LIBSMTP_REJECTQUIT	1029
#define LIBSMTP_MALLOCFAIL 1030
#define LIBSMTP_LINETOOLONG 1031

/* Codes > 2048 are MIME errors and are defined in libsmtp_mime.h */

#define LIBSMTP_UNDEFERR	10000 /* ErrorCode was undefined!! */

/* Ed Goforth <e.goforth@computer.org> */
/* To automatically add an rfc822 compliant date header */
/* LIBSMTP_DATE_STR_FMT is passed to strftime(3) so that a buffer can */
/* be filled out in the correct format */
#define LIBSMTP_DATE_STR_FMT  "%a, %d %b %Y %H:%M:%S %z (%Z)"
/* LIBSMTP_DATA_STR_INIT is passed to g_string_new() so that a buffer of */
/* the correct size can be allocated to store the date string */
#define LIBSMTP_DATE_STR_INIT "Day, dd Mon YYYY hh:mm:ss -xxxx (TZZ)"

/* Encode information about one recipient. Internally a linked list, each
 * entry pointing to the next recipient struct entry */
struct libsmtp_recipient_struct {
	char *address;
	uint8_t type;
	uint16_t response_code;
	char *response_str;
	struct libsmtp_recipient_struct *next;
};

/* This structure defines one libsmtp session */
struct libsmtp_session_struct {
  int serverflags;	/* Server capability flags */
  int socket;		/* socket handle */

  char date[40]; /* rfc822 Date header */

  char *from;	/* From address */

  // A structure recording the participants
  struct libsmtp_recipient_struct *recipients;
  uint32_t num_to;
  uint32_t num_cc;
  uint32_t num_bcc;

  uint32_t num_failed_rec;

  char *subject;	/* Mail subject */
  char lastResponse[512];	/* Last SMTP response string from server */
  int lastResponseCode;	/* Last SMTP response code from server */
  int errorCode;	/* Internal libsmtp error code from last error */
  char *errorModule;	/* Module were error was caused */
  int stage;		/* SMTP transfer stage */

  unsigned int dialogueSent;	/* Number of SMTP dialogue lines sent */
  unsigned int dialogueBytes;	/* Bytes of SMTP dialogue data sent */
  unsigned int headersSent;  	/* Number of header lines sent */
  unsigned int headerBytes;	/* Bytes of header data sent */
  unsigned long int bodyBytes;	/* Bytes of body data sent */

  // Cheap trick to save on mallocs and such:
  // We allocate a shared send and receive buffer
  // This works quite well because RFC2822 enforces a maximum line length anyway
  char buffer[LIBSMTP_BUFFER_SIZE];

  #ifdef WITH_MIME
    GNode *Parts;		/* N-Tree of body parts (MIME stuff) */
    int NumParts;		/* Number of body parts */
    struct libsmtp_part_struct *PartNow;	/* Part we are sending now */
    GNode *PartNowNode;		/* Node of the part we are just sending */
  #endif
};



struct libsmtp_session_struct *libsmtp_session_initialize (void);

int libsmtp_connect (char *, unsigned int, unsigned int, struct libsmtp_session_struct *);

int libsmtp_errno(struct libsmtp_session_struct *);

const char *libsmtp_strerr (struct libsmtp_session_struct *);

int libsmtp_add_recipient (int, char *, struct libsmtp_session_struct *);

int libsmtp_set_environment (char *, char *, unsigned int, struct libsmtp_session_struct *);

int libsmtp_dialogue_send (char *, struct libsmtp_session_struct *);

int libsmtp_dialogue (struct libsmtp_session_struct *);

int libsmtp_header_send (char *, struct libsmtp_session_struct *);

int libsmtp_headers (struct libsmtp_session_struct *);

int libsmtp_body_send_raw (char *, unsigned long int, struct libsmtp_session_struct *);

int libsmtp_body_end (struct libsmtp_session_struct *);

int libsmtp_quit (struct libsmtp_session_struct *);

int libsmtp_close (struct libsmtp_session_struct *);

int libsmtp_free (struct libsmtp_session_struct *);

/* internal functions */

int libsmtp_int_send (uint32_t, struct libsmtp_session_struct *, int);

int libsmtp_int_read (uint32_t *, struct libsmtp_session_struct *, int);

int libsmtp_int_send_body (uint32_t, struct libsmtp_session_struct *);

#endif  /* LIB_SMTP_H */

#ifdef __cplusplus
 }
#endif
