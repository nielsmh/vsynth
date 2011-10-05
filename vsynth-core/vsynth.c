#include <stdlib.h>
#include <string.h>
#include <vsynth/vsynth.h>


#ifdef _MSC_VER
# define INLINE __inline
#else
# define INLINE
#endif


extern INLINE VSYNTH_API(Vs_String) Vs_AllocString(size_t len)
{
	Vs_String result = (Vs_String)malloc(sizeof(struct Vs_String));
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

extern INLINE VSYNTH_API(Vs_String) Vs_MakeString(const char *str)
{
	return Vs_MakeStringN(str, strlen(str));
}

extern INLINE VSYNTH_API(Vs_String) Vs_MakeStringN(const char *str, size_t len)
{
	Vs_String result = Vs_AllocString(len);
	if (len > 0)
	{
		memcpy(result->str, str, len);
	}
	return result;
}

extern INLINE VSYNTH_API(Vs_String) Vs_CopyString(const Vs_String str)
{
	return Vs_MakeStringN(str->str, str->len);
}

extern INLINE VSYNTH_API(void) Vs_FreeString(Vs_String str)
{
	free(str->str);
	free(str);
}



struct FactoryList {
	struct Vs_FilterFactory *factory;
	struct FactoryList *next;
};
struct FactoryList *factory_list = NULL;

VSYNTH_API(void) Vs_RegisterFilter(struct Vs_FilterFactory *factory)
{
	struct FactoryList *cur;
	struct FactoryList *new_head;

	// check it isn't already registered
	for (cur = factory_list; cur != NULL; cur = cur->next)
	{
		if (cur->factory == factory)
			return;
	}

	// add it
	new_head = (struct FactoryList *)malloc(sizeof(struct FactoryList));
	new_head->factory = factory;
	new_head->next = factory_list;
	factory_list = new_head;
}

VSYNTH_API(struct Vs_FilterFactory *) Vs_FindFilter(const char *name)
{
	struct FactoryList *cur;
	
	for (cur = factory_list; cur != NULL; cur = cur->next)
	{
		if (strcmp(name, cur->factory->identifier) == 0)
			return cur->factory;
	}
	return NULL;
}

VSYNTH_API(void) Vs_EnumerateFilters(Vs_EnumFiltersFunc callback, void *userdata)
{
	struct FactoryList *cur;
	
	for (cur = factory_list; cur != NULL; cur = cur->next)
	{
		callback(cur->factory, userdata);
	}
}
