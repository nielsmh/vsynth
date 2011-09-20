#define VSYNTH_API(rettype) __declspec(dllexport) rettype __stdcall

#include <stdlib.h>
#include <string.h>
#include <vsynth/vsynth.h>


__inline VSYNTH_API(String) AllocString(size_t len)
{
	String result = (String)malloc(sizeof(struct String));
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

__inline VSYNTH_API(String) MakeString(const char *str)
{
	return MakeStringN(str, strlen(str));
}

__inline VSYNTH_API(String) MakeStringN(const char *str, size_t len)
{
	String result = AllocString(len);
	if (len > 0)
	{
		memcpy(result->str, str, len);
	}
	return result;
}

__inline VSYNTH_API(String) CopyString(const String str)
{
	return MakeStringN(str->str, str->len);
}

__inline VSYNTH_API(void) FreeString(String str)
{
	free(str->str);
	free(str);
}



struct FactoryList {
	struct FilterFactory *factory;
	struct FactoryList *next;
};
struct FactoryList *factory_list = NULL;

VSYNTH_API(void) RegisterFilter(struct FilterFactory *factory)
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

VSYNTH_API(struct FilterFactory *) FindFilter(const char *name)
{
	struct FactoryList *cur;
	
	for (cur = factory_list; cur != NULL; cur = cur->next)
	{
		if (strcmp(name, cur->factory->identifier) == 0)
			return cur->factory;
	}
	return NULL;
}

VSYNTH_API(void) EnumerateFilters(EnumFiltersFunc callback, void *userdata)
{
	struct FactoryList *cur;
	
	for (cur = factory_list; cur != NULL; cur = cur->next)
	{
		callback(cur->factory, userdata);
	}
}
