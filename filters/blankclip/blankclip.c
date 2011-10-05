#include <vsynth/vsynth.h>
#include <vsynth/stdframe.h>
#include <stdint.h>
#include <assert.h>


struct BlankclipFilter {
	struct Vs_Filter base;
	size_t refcount;
	size_t width, height;
	uint32_t color;
	Vs_Timestamp frame_duration;
	Vs_FrameNumber length;
};

extern struct Vs_FilterVirtual blankclip_vtable;

static __inline struct BlankclipFilter * GetBlankclip(Vs_Filter filter)
{
	if (filter->methods == &blankclip_vtable)
		return (struct BlankclipFilter *)filter;
	else
		return NULL;
}



VSYNTH_IMPLEMENT_METHOD(Vs_Filter, blankclip_new)(void)
{
	struct BlankclipFilter *f = (struct BlankclipFilter *)malloc(sizeof(struct BlankclipFilter));
	f->base.methods = &blankclip_vtable;
	f->refcount = 1;
	f->width = 0;
	f->height = 0;
	f->color = 0;
	f->frame_duration = 0;
	f->length = 0;
	return &f->base;
}

struct Vs_FilterFactory blankclip_factory = {
	"blankclip",
	"Blank frame source",
	"Public domain",
	blankclip_new
};


VSYNTH_IMPLEMENT_METHOD(void, blankclip_addref)(Vs_Filter filter)
{
	struct BlankclipFilter *bf = GetBlankclip(filter);
	bf->refcount++;
}

VSYNTH_IMPLEMENT_METHOD(void, blankclip_unref)(Vs_Filter filter)
{
	struct BlankclipFilter *bf = GetBlankclip(filter);
	assert(bf->refcount > 0);
	bf->refcount--;
	
	if (bf->refcount == 0)
	{
		free(bf);
	}
}

VSYNTH_IMPLEMENT_METHOD(Vs_Filter, blankclip_clone)(Vs_Filter filter)
{
	struct BlankclipFilter *bf = GetBlankclip(filter);
	struct BlankclipFilter *nf = GetBlankclip(blankclip_new());
	nf->width = bf->width;
	nf->height = bf->width;
	nf->color = bf->color;
	nf->frame_duration = bf->frame_duration;
	nf->length = bf->length;
	return &nf->base;
}

static Vs_ActiveFilter FailActivate(Vs_String *error, const char *msg)
{
	*error = Vs_MakeString(msg);
	return NULL;
}
VSYNTH_IMPLEMENT_METHOD(Vs_ActiveFilter, blankclip_activate)(Vs_Filter filter, Vs_String *error, struct Vs_FrameTypeDescription **frametypes)
{
	struct Vs_StandardFrameTypeDescription *sfd;
	enum Vs_StdframePixelFormat *pf;

	int allow_xrgb8 = 0, allow_argb8 = 0, allow_xrgb16 = 0, allow_argb16 = 0;

	struct BlankclipFilter *f = GetBlankclip(filter);
	if (f->width < 1) return FailActivate(error, "Width is less than 1");
	if (f->height < 1) return FailActivate(error, "Height is less than 1");
	if (f->frame_duration == 0) return FailActivate(error, "No frame duration is set");
	if (f->length == 0) return FailActivate(error, "No output length given");

	for (; *frametypes; frametypes++)
	{
		if ((*frametypes)->frame_type == (struct FrameVirtual *)&Vs_stdframe_vtable)
		{
			sfd = (struct Vs_StandardFrameTypeDescription *)*frametypes;
			sfd->base.out_supported = 1;
			if (sfd->minwidth > f->width  || sfd->maxwidth < f->width || sfd->minheight > f->height || sfd->maxheight < f->height)
			{
				sfd->base.out_supported = 0;
			}
			else if (sfd->width_modulo && (f->width % sfd->width_modulo != 0))
			{
				sfd->base.out_supported = 0;
			}
			else if (sfd->height_modulo && (f->height % sfd->height_modulo != 0))
			{
				sfd->base.out_supported = 0;
			}
			else if (sfd->pixfmts)
			{
				for (pf = sfd->pixfmts; *pf != STDPIXFMT_MAX; pf++)
				{
					switch (*pf)
					{
					case STDPIXFMT_XRGB8:
						allow_xrgb8 = 1;
						break;
					case STDPIXFMT_ARGB8:
						allow_argb8 = 1;
						break;
					case STDPIXFMT_XRGB16:
						allow_xrgb16 = 1;
						break;
					case STDPIXFMT_ARGB16:
						allow_argb16 = 1;
						break;
					}
				}
				if (!(allow_xrgb8 | allow_argb8 | allow_xrgb16 | allow_argb16))
				{
					sfd->base.out_supported = 0;
				}
			}
			else
			{
				allow_xrgb8 = allow_argb8 = allow_xrgb16 = allow_argb16 = 1;
			}

			if (sfd->base.out_supported)
			{
				sfd->minwidth = sfd->maxwidth = f->width;
				sfd->minheight = sfd->maxheight = f->height;
				sfd->allow_pixfmt_change = 0;
				sfd->allow_resolution_change = 0;
			}
		}
		else
		{
			(*frametypes)->out_supported = 0;
		}
	}

	if (allow_xrgb8 | allow_argb8 | allow_xrgb16 | allow_argb16)
	{
		// found a supported pixel format
		// todo: create the instance!
	}
	
	return NULL;
}

VSYNTH_IMPLEMENT_METHOD(void, blankclip_enum_properties)(Vs_EnumPropertiesFunc callback, void *userdata)
{
	callback("width", PROP_INT, userdata);
	callback("height", PROP_INT, userdata);
	callback("color", PROP_INT, userdata);
	callback("framedur", PROP_TIMESTAMP, userdata);
	callback("length", PROP_FRAMENUMBER, userdata);
}

VSYNTH_IMPLEMENT_METHOD(Vs_Filter, blankclip_get_property_filter)(Vs_Filter filter, const char *name)
{
	return NULL; // no filter properties
}

VSYNTH_IMPLEMENT_METHOD(long long, blankclip_get_property_int)(Vs_Filter filter, const char *name)
{
	struct BlankclipFilter *f = GetBlankclip(filter);
	if (strcmp(name, "width") == 0)
		return f->width;
	if (strcmp(name, "height") == 0)
		return f->height;
	if (strcmp(name, "color") == 0)
		return f->color;
	return 0;
}

VSYNTH_IMPLEMENT_METHOD(double, blankclip_get_property_double)(Vs_Filter filter, const char *name)
{
	return 0; // no double properties
}

VSYNTH_IMPLEMENT_METHOD(Vs_String, blankclip_get_property_string)(Vs_Filter filter, const char *name)
{
	return NULL; // no string properties
}

VSYNTH_IMPLEMENT_METHOD(Vs_FrameNumber, blankclip_get_property_framenumber)(Vs_Filter filter, const char *name)
{
	struct BlankclipFilter *f = GetBlankclip(filter);
	if (strcmp(name, "length") == 0)
		return f->length;
	return 0;
}

VSYNTH_IMPLEMENT_METHOD(Vs_Timestamp, blankclip_get_property_timestamp)(Vs_Filter filter, const char *name)
{
	struct BlankclipFilter *f = GetBlankclip(filter);
	if (strcmp(name, "framedur") == 0)
		return f->frame_duration;
	return 0;
}

VSYNTH_IMPLEMENT_METHOD(void, blankclip_set_property_filter)(Vs_Filter filter, const char *name, Vs_Filter value)
{
	// no filter properties
}

VSYNTH_IMPLEMENT_METHOD(void, blankclip_set_property_int)(Vs_Filter filter, const char *name, long long value)
{
	struct BlankclipFilter *f = GetBlankclip(filter);
	if (strcmp(name, "width") == 0)
		f->width = (size_t)value;
	else if (strcmp(name, "height") == 0)
		f->height = (size_t)value;
	else if (strcmp(name, "color") == 0)
		f->color = (uint32_t)value;
}

VSYNTH_IMPLEMENT_METHOD(void, blankclip_set_property_double)(Vs_Filter filter, const char *name, double value)
{
	// no double properties
}

VSYNTH_IMPLEMENT_METHOD(void, blankclip_set_property_string)(Vs_Filter filter, const char *name, Vs_String value)
{
	// no string properties
}

VSYNTH_IMPLEMENT_METHOD(void, blankclip_set_property_framenumber)(Vs_Filter filter, const char *name, Vs_FrameNumber value)
{
	struct BlankclipFilter *f = GetBlankclip(filter);
	if (strcmp(name, "length") == 0)
		f->length = value;
}

VSYNTH_IMPLEMENT_METHOD(void, blankclip_set_property_timestamp)(Vs_Filter filter, const char *name, Vs_Timestamp value)
{
	struct BlankclipFilter *f = GetBlankclip(filter);
	if (strcmp(name, "framedur") == 0)
		f->frame_duration = value;
}


struct Vs_FilterVirtual blankclip_vtable = {
	blankclip_addref,
	blankclip_unref,
	blankclip_clone,
	blankclip_activate,
	blankclip_enum_properties,
	blankclip_get_property_filter,
	blankclip_get_property_int,
	blankclip_get_property_double,
	blankclip_get_property_string,
	blankclip_get_property_framenumber,
	blankclip_get_property_timestamp,
	blankclip_set_property_filter,
	blankclip_set_property_int,
	blankclip_set_property_double,
	blankclip_set_property_string,
	blankclip_set_property_framenumber,
	blankclip_set_property_timestamp
};


#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

BOOL WINAPI DllMain(__in  HINSTANCE hinstDLL, __in  DWORD fdwReason, __in  LPVOID lpvReserved)
{
	if (fdwReason == DLL_PROCESS_ATTACH)
	{
		Vs_RegisterFilter(&blankclip_factory);
	}
	else if (fdwReason == DLL_PROCESS_DETACH)
	{
		// unregister?
	}

	return TRUE;
}
#endif
