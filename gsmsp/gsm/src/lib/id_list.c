#include "common.h"
#include "id_list.h"

const char *
id_list_get(struct _id_list *id_list, int id)
{
	struct _id_list *idptr = id_list;

	while (idptr->string != NULL)
	{
		if (idptr->id == id)
			return idptr->string;
		idptr++;
	}

	return "UNKNOWN";
}

