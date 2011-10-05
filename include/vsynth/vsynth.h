// This file is C99

#pragma once

#include <stdlib.h>

#ifdef _MSC_VER
/// Calling convention for public functions in VSynth
# define VSYNTH_API(rettype) rettype __stdcall
/// Definition for public globals in VSynth
# define VSYNTH_EXTERN(type) extern type
/// Declaration helper for building vtable struct declarations
# define VSYNTH_DECLARE_METHOD(rettype, name) rettype (__stdcall *name)
/// Declaration helper for implementing class member functions
# define VSYNTH_IMPLEMENT_METHOD(rettype, name) static rettype __stdcall name
#else
# error Please define VSYNTH_API, VSYNTH_EXTERN, VSYNTH_DECLARE_METHOD and VSYNTH_IMPLEMENT_METHOD for your compiler
// Make sure definitions for new compilers are ABI compatible with existing compilers' definitions
#endif


#ifdef __cplusplus
extern "C" {
#endif


/// The type of frame numbers
typedef unsigned long long int Vs_FrameNumber;
/// FrameNumber value specifying an unbounded count of frames
#define FRAMECOUNT_UNKNOWN ((Vs_FrameNumber)-1)

/// The type of natural times
///
/// Fluff says this should probably be a PTS. That might need further explanation?
/// Maybe some helper functions also?
typedef unsigned long long int Vs_Timestamp;
/// Timestamp value specifying an unbounded duration
#define DURATION_UNKNOWN ((Vs_FrameNumber)-1)

/// Type of Vsynth video frames
typedef struct Vs_Frame *Vs_Frame;
/// Type of an active Vsynth filter
typedef struct Vs_ActiveFilter *Vs_ActiveFilter;
/// Type of a Vsynth filter
typedef struct Vs_Filter *Vs_Filter;


/// Vtable for Frame objects
struct Vs_FrameVirtual {
	/// Deinitialise and deallocate a frame
	///
	/// This should possibly be changed to reference counting.
	VSYNTH_DECLARE_METHOD(void, destroy)(Vs_Frame frame);
	/// Create a complete copy of the frame that can be safely written to
	/// without affecting the original
	VSYNTH_DECLARE_METHOD(Vs_Frame, clone)(Vs_Frame frame);
};
/// Represents a video frame
///
/// @todo This needs some more fields or methods to actually hold pixel data
struct Vs_Frame {
	/// Points to the vtable for this frame object
	struct Vs_FrameVirtual *methods;
	/// Timestamp the frame would appear at during playback
	///
	/// A frame's timestamp is the first moment in time the frame is to be
	/// displayed.
	Vs_Timestamp timestamp;
};

/// Structure for describing supported frame types during filter activation
///
/// Frame types may extend this structure with other relevant fields to describe
/// further constraints.
struct Vs_FrameTypeDescription {
	/// Pointer to the vtable of the frame type, to distinguish the frame type
	struct FrameVirtual *frame_type;
	/// Set by the callee during filter activation, telling whether it supports the format
	///
	/// The filter being activated sets this to zero if it does not supporte the format
	/// or to non-zero if it does support the format.
	int out_supported;
};


/// A character string or blob with memory managed by Vsynth
struct Vs_String {
	/// Length of string in bytes
	size_t len;
	/// Pointer to string character data
	///
	/// If len==0 then this pointer is NULL.
	char *str;
};
/// Type of Vsynth-managed strings
typedef struct Vs_String *Vs_String;

/// Create a new String with undefined contents
VSYNTH_API(Vs_String) Vs_AllocString(size_t len);
/// Create a new String from a nul-terminated string
VSYNTH_API(Vs_String) Vs_MakeString(const char *str);
/// Create a new String from a string with known length
VSYNTH_API(Vs_String) Vs_MakeStringN(const char *str, size_t len);
/// Create a new copy of a String
VSYNTH_API(Vs_String) Vs_CopyString(const Vs_String str);
/// Deallocate the memory used by a String
VSYNTH_API(void) Vs_FreeString(Vs_String str);


/// Vtable for ActiveFilter
struct Vs_ActiveFilterVirtual {
	/// De-initialise and deallocate a filter
	VSYNTH_DECLARE_METHOD(void, destroy)(Vs_ActiveFilter filter);
	/// Return or produce a numbered frame
	///
	/// This function may return NULL. If NULL is returned, the frame number
	/// is past the range this filter can supply and requesting any higher
	/// frame number must also fail.
	///
	/// get_frame(n)->timestamp < get_frame(n+1)->timestamp must always hold.
	///
	/// Requesting frames must be idempotent. The order of requests must not
	/// matter. Requesting the same frame number multiple times must return
	/// identical frames every time, or return NULL every time. The same
	/// active filter instance may be used in multiple threads at one time.
	///
	/// The caller is responsible for destroying the returned Frame object.
	VSYNTH_DECLARE_METHOD(Vs_Frame, get_frame)(Vs_ActiveFilter filter, Vs_FrameNumber n);
	/// Return the maximum number of frames this filter can produce
	///
	/// The number returned by this function may be higher than the actual
	/// number of frames the filter can produce, in case the number of frames
	/// is not known at activation time.
	///
	/// This method may return FRAMECOUNT_UNKNOWN, in which case the actual
	/// number of frames this filter can provide is not known and not bounded.
	VSYNTH_DECLARE_METHOD(Vs_FrameNumber, get_frame_count)(Vs_ActiveFilter filter);
	/// Return the duration of this filter's video stream
	///
	/// This must return either the timestamp of the imaginary frame
	/// following the last frame this filter can supply, or the
	/// DURATION_UNKNOWN constant, in which case the duration of the
	/// video stream is not known.
	VSYNTH_DECLARE_METHOD(Vs_Timestamp, get_duration)(Vs_ActiveFilter filter);
};
/// An activated filter from which frames can be requested
struct Vs_ActiveFilter {
	/// Points to the vtable for the ActiveFilter object
	struct Vs_ActiveFilterVirtual *methods;
	/// Points to the Filter object that produced this active instance
	///
	/// Should be treated const.
	Vs_Filter filter;
};


/// Types a property value can take
enum Vs_PropertyType {
	PROP_FILTER,
	PROP_INT, // signed long long
	PROP_DOUBLE,
	PROP_STRING, // String type
	PROP_FRAMENUMBER,
	PROP_TIMESTAMP,
	// array types too?
};

/// Type of callback function for enumerating all properties on a filter
typedef VSYNTH_DECLARE_METHOD(void, Vs_EnumPropertiesFunc)(const char *name, enum Vs_PropertyType type, void *userdata);
/// Vtable for Filter objects
struct Vs_FilterVirtual {
	/// Increase the reference count to the Filter object
	VSYNTH_DECLARE_METHOD(void, addref)(Vs_Filter filter);
	/// Decrease the reference count to the Filter object
	///
	/// If the reference count reaches zero the object must be deinitialised
	/// and deallocated.
	VSYNTH_DECLARE_METHOD(void, unref)(Vs_Filter filter);
	/// Create a clone of the filter and its property values
	///
	/// The returned clone must have a reference count of 1.
	VSYNTH_DECLARE_METHOD(Vs_Filter, clone)(Vs_Filter filter);
	/// Create an activated instance of the filter
	///
	/// May return NULL if the current property settings are invalid. In that
	/// case the error pointer must be set to a String describing the problem.
	/// The error string is owned by the caller.
	///
	/// The caller must pass an array of pointers to FrameTypeDescription
	/// structures, terminated by a NULL pointer. This array describes the
	/// frame types the activator can accept and any further constrains on
	/// them. The activatee checks each frame type description and sets the
	/// out_supported flag on each depending on whether it can support
	/// outputting that frame type. If it cannot support any of the frame
	/// types it must fail. An active filter should only deliver frame types
	/// it promised during activation.
	VSYNTH_DECLARE_METHOD(Vs_ActiveFilter, activate)(Vs_Filter filter, Vs_String *error, struct Vs_FrameTypeDescription **frametypes);

	/// Enumerate all properties the filter has through the callback function
	VSYNTH_DECLARE_METHOD(void, enum_properties)(Vs_EnumPropertiesFunc callback, void *userdata);
	/// Get a property value of Filter type
	///
	/// The returned Filter object has had addref called and the caller owns
	/// that reference.
	///
	/// May return NULL if the property is not of Filter type, it doesn't exist,
	/// or there is no value assigned.
	VSYNTH_DECLARE_METHOD(Vs_Filter, get_property_filter)(Vs_Filter filter, const char *name);
	/// Get a property value of integer type
	///
	/// The return value is undefined if the property is not of integer type
	/// or it doesn't exist.
	VSYNTH_DECLARE_METHOD(long long, get_property_int)(Vs_Filter filter, const char *name);
	/// Get a property value of double type
	///
	/// The return value is undefined if the property is not of double type or
	/// it doesn't exist.
	VSYNTH_DECLARE_METHOD(double, get_property_double)(Vs_Filter filter, const char *name);
	/// Get a property value of String type
	///
	/// May return NULL if the property is not of String type, it doesn't exist,
	/// or the string is empty.
	///
	/// Returns the actual String object held, the caller must not modify the
	/// returned object. The caller must copy the String if it wants to
	/// manipulate it. (This is for performance reasons, to avoid risking
	/// copying huge strings.)
	VSYNTH_DECLARE_METHOD(Vs_String, get_property_string)(Vs_Filter filter, const char *name);
	/// Get a property value of FrameNumber type
	///
	/// The return value is undefined if the property is not of FrameNumber
	/// type or it doesn't exist.
	VSYNTH_DECLARE_METHOD(Vs_FrameNumber, get_property_framenumber)(Vs_Filter filter, const char *name);
	/// Get a property value of Timestamp type
	///
	/// The return value is undefined if the property is not of Timestamp
	/// type or it doesn't exist.
	VSYNTH_DECLARE_METHOD(Vs_Timestamp, get_property_timestamp)(Vs_Filter filter, const char *name);
	/// Set a property to a Filter value
	///
	/// The value may be NULL.
	///
	/// The callee is responsible for calling addref on the newly assigned
	/// value and calling unref on the old value.
	///
	/// The call may be ignored if the property doesn't exist or is not of
	/// Filter type.
	VSYNTH_DECLARE_METHOD(void, set_property_filter)(Vs_Filter filter, const char *name, Vs_Filter value);
	/// Set a property to an integer value
	///
	/// The call may be ignored if the property doesn't exist or is not of
	/// integer type.
	VSYNTH_DECLARE_METHOD(void, set_property_int)(Vs_Filter filter, const char *name, long long value);
	/// Set a property to a double value
	///
	/// The call may be ignored if the property doesn't exist or is not of
	/// double type.
	VSYNTH_DECLARE_METHOD(void, set_property_double)(Vs_Filter filter, const char *name, double value);
	/// Set a property to a String value
	///
	/// The value may be NULL to set the property to the empty string.
	///
	/// The callee must make a copy of the given String and store that copy.
	///
	/// The call may be ignored if the property doesn't exist or is not of
	/// String type.
	VSYNTH_DECLARE_METHOD(void, set_property_string)(Vs_Filter filter, const char *name, Vs_String value);
	/// Set a property to a FrameNumber value
	///
	/// The call may be ignored if the property doesn't exist or is not of
	/// FrameNumber type.
	VSYNTH_DECLARE_METHOD(void, set_property_framenumber)(Vs_Filter filter, const char *name, Vs_FrameNumber value);
	/// Set a property to a Timestamp value
	///
	/// The call may be ignored if the property doesn't exist or is not of
	/// Timestamp type.
	VSYNTH_DECLARE_METHOD(void, set_property_timestamp)(Vs_Filter filter, const char *name, Vs_Timestamp value);
};
/// A prototype filter which can be configured and activate instances
///
/// A filter prototype must not own active filters and must not request frames.
/// When a filter is activated, it should activate all other filters it
/// references and pass those activated filters to the instance it creates.
struct Vs_Filter {
	/// Point to the vtable for the Filter object
	struct Vs_FilterVirtual *methods;
};


/// A factory for Filter objects of a specific type
struct Vs_FilterFactory {
	/// Programmatic identifier for the filter
	///
	/// The identifier should only consist of characters commonly usable for
	/// names in programming languages.
	const char *identifier;
	/// Descriptive name for the filter
	const char *name;
	/// Copyright string for the filter
	const char *copyright;
	/// Produce a new instance of the filter
	///
	/// The returned Filter object must have a reference count of 1.
	VSYNTH_DECLARE_METHOD(Vs_Filter, produce)(void);
};

/// Type of callback functions for enumerating registered filters
typedef VSYNTH_DECLARE_METHOD(void, Vs_EnumFiltersFunc)(struct Vs_FilterFactory *factory, void *userdata);

/// Register a new filter type with Vsynth
VSYNTH_API(void) Vs_RegisterFilter(struct Vs_FilterFactory *factory);
/// Get the factory for a named filter
///
/// @param name Programmatic identifier for the filter requested
VSYNTH_API(struct Vs_FilterFactory *) Vs_FindFilter(const char *name);
/// Enumerate all currently registered filters
VSYNTH_API(void) Vs_EnumerateFilters(Vs_EnumFiltersFunc callback, void *userdata);


#ifdef __cplusplus
}
#endif
