#define MAX_MSG_B 4
#define MAX_MSG (1 << MAX_MSG_B)
typedef size_t msg_nb_t;

typedef struct
{
	register_t a0; /* GP arguments */
	register_t a1;
	register_t a2;

    __capability void *c3; /* Cap arguments */
    __capability void *c4; /* Cap arguments */
    __capability void *c5; /* Cap arguments */

	register_t v0;  /* method nb */
	register_t v1;  /* syscall nb */
	__capability void *c2;  /* token nb */
	//void *     idc; /* identifier */
}  msg_t;

typedef struct
{
	msg_nb_t start;
	msg_nb_t end;
	msg_nb_t len;
	msg_t msg[MAX_MSG];
}  queue_t;
