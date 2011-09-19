#define VSYNTH_API(rettype) __declspec(dllexport) rettype __stdcall

#include "stdframe.h"
#include <stdlib.h>
#include <assert.h>

static void VSYNTH_METHOD Stdframe_destroy(Frame frame)
{
	StandardFrame sf = Stdframe_Get(frame);
	assert(sf != NULL);

	free(sf->data_baseptr);
	free(sf);
}

static Frame VSYNTH_METHOD Stdframe_clone(Frame frame)
{
	StandardFrame sf = Stdframe_Get(frame);
	assert(sf != NULL);

	// todo
	return NULL;
}

struct StandardFrameVirtual stdframe_vtable = {
	{
		Stdframe_destroy,
		Stdframe_clone
	},
};


VSYNTH_API(StandardFrame) Stdframe_New(int pixfmt, size_t width, size_t height)
{
	StandardFrame frame = (StandardFrame)malloc(sizeof(struct StandardFrame));
	// todo: alloc pixels
	return frame;
}

VSYNTH_API(StandardFrame) Stdframe_Get(Frame frame)
{
	if (frame->methods == &stdframe_vtable.base)
	{
		return (StandardFrame)frame;
	}
	else
	{
		return NULL;
	}
}
