#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <vsynth/vsynth.h>


#ifdef _MSC_VER
# define INLINE __inline
#else
# define INLINE
#endif


struct FactoryList;
struct LibraryInstance {
	struct FactoryList *factory_list;
	struct TAG_Vs_Library public_interface;
};

INLINE static struct LibraryInstance *getlib(Vs_Library vsynth)
{
	return (struct LibraryInstance *)( ((volatile char *)vsynth) - offsetof(struct LibraryInstance,public_interface) );
}



INLINE VSYNTH_IMPLEMENT_METHOD(Vs_String, AllocString)(size_t len)
{
	Vs_String result = (Vs_String)malloc(sizeof(struct TAG_Vs_String));
	result->len = len;
	if (len > 0)
	{
		result->str = (char *)malloc(len);
	}
	else
	{
		result->str = NULL;
	}
	return result;
}

INLINE VSYNTH_IMPLEMENT_METHOD(Vs_String, MakeStringN)(const char *str, size_t len)
{
	Vs_String result = AllocString(len);
	if (len > 0)
	{
		memcpy(result->str, str, len);
	}
	return result;
}

INLINE VSYNTH_IMPLEMENT_METHOD(Vs_String, MakeString)(const char *str)
{
	return MakeStringN(str, strlen(str));
}

INLINE VSYNTH_IMPLEMENT_METHOD(Vs_String, CopyString)(const Vs_String str)
{
	return MakeStringN(str->str, str->len);
}

INLINE VSYNTH_IMPLEMENT_METHOD(void, FreeString)(Vs_String str)
{
	free(str->str);
	free(str);
}

static struct TAG_Vs_StringAPI StringAPI = {
	AllocString,
	MakeString,
	MakeStringN,
	CopyString,
	FreeString
};



struct FactoryList {
	Vs_FilterFactory *factory;
	struct FactoryList *next;
};

VSYNTH_IMPLEMENT_METHOD(void, RegisterFilter)(Vs_Library vsynth, Vs_FilterFactory *factory)
{
	struct FactoryList *cur;
	struct FactoryList *new_head;
	struct LibraryInstance *v = getlib(vsynth);

	// check it isn't already registered
	for (cur = v->factory_list; cur != NULL; cur = cur->next)
	{
		if (cur->factory == factory)
			return;
	}

	// add it
	new_head = (struct FactoryList *)malloc(sizeof(struct FactoryList));
	new_head->factory = factory;
	new_head->next = v->factory_list;
	v->factory_list = new_head;
}

VSYNTH_IMPLEMENT_METHOD(Vs_FilterFactory *, FindFilter)(Vs_Library vsynth, const char *name)
{
	struct FactoryList *cur;
	struct LibraryInstance *v = getlib(vsynth);
	
	for (cur = v->factory_list; cur != NULL; cur = cur->next)
	{
		if (strcmp(name, cur->factory->identifier) == 0)
			return cur->factory;
	}
	return NULL;
}

VSYNTH_IMPLEMENT_METHOD(void, EnumerateFilters)(Vs_Library vsynth, Vs_EnumFiltersFunc callback, void *userdata)
{
	struct FactoryList *cur;
	struct LibraryInstance *v = getlib(vsynth);
	
	for (cur = v->factory_list; cur != NULL; cur = cur->next)
	{
		callback(cur->factory, userdata);
	}
}


static struct TAG_Vs_FilterRegistry FilterRegistry = {
	RegisterFilter,
	FindFilter,
	EnumerateFilters
};


VSYNTH_API(Vs_Library) Vs_InitLibrary(void)
{
	struct LibraryInstance *v = (struct LibraryInstance *)malloc(sizeof(struct LibraryInstance));

	v->factory_list = NULL;
	v->public_interface.FilterRegistry = &FilterRegistry;
	v->public_interface.String = &StringAPI;

	return &(v->public_interface);
}

VSYNTH_API(void) Vs_FreeLibrary(Vs_Library vsynth)
{
	struct FactoryList *cur, *next;
	struct LibraryInstance *v = getlib(vsynth);

	cur = v->factory_list;
	while (cur != NULL)
	{
		next = cur->next;
		free(cur);
		cur = next;
	}

	free(v);
}

