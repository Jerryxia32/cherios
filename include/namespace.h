void namespace_init(void *ns_ref);
int	namespace_register(int nb, void *ref, void * __capability PCC, void * __capability IDC);
void *	namespace_get_ref(int nb);
void * __capability	namespace_get_PCC(int nb);
void * __capability	namespace_get_IDC(int nb);

extern void * namespace_ref;
