
#ifndef __GSMSP_ID_LIST_H__
#define __GSMSP_ID_LIST_H__ 1

struct _id_list
{
	int id;
	const char *string;
};

const char *id_list_get(struct _id_list *id_list, int id);

#endif /* !__GSMSP_COMMON_H__ */


