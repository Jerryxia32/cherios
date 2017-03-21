#define MAX_MSG_B 4
#define MAX_MSG (1 << MAX_MSG_B)
typedef size_t msg_nb_t;

typedef struct
{
	register_t a0; /* GP arguments */
	register_t a1;
	register_t a2;

    void * __capability c3; /* Cap arguments */
    void * __capability c4; /* Cap arguments */
    void * __capability c5; /* Cap arguments */

	register_t v0;  /* method nb */
	register_t v1;  /* syscall nb */
	void * __capability c2;  /* token nb */
	//void *     idc; /* identifier */
}  msg_t;

typedef struct
{
	msg_nb_t start;
	msg_nb_t end;
	msg_nb_t len;
	msg_t msg[MAX_MSG];
}  queue_t;
