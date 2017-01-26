void	namespace_init(void *ns_ref, void *ns_id);
int	namespace_register(int nb, void *ref, void *id, __capability void *PCC, __capability void *IDC);
void *	namespace_get_ref(int nb);
void *	namespace_get_id(int nb);
__capability void *	namespace_get_PCC(int nb);
__capability void *	namespace_get_IDC(int nb);

extern void * namespace_ref;
extern void * namespace_id;
