// This file is C99

#pragma once

#include <stdlib.h>

#ifndef VSYNTH_API
# define VSYNTH_API(rettype) __stdcall rettype __declspec(dllimport)
#endif
#define VSYNTH_METHOD __stdcall

#ifdef __cplusplus
# define HEADER_START extern "C" {
# define HEADER_END }
#else
# define HEADER_START
# define HEADER_END
#endif


HEADER_START


/// The type of frame numbers
typedef unsigned long long int FrameNumber;
/// FrameNumber value specifying an unbounded count of frames
#define FRAMECOUNT_UNKNOWN ((FrameNumber)-1)

/// The type of natural times
///
/// Fluff says this should probably be a PTS. That might need further explanation?
/// Maybe some helper functions also?
typedef unsigned long long int Timestamp;
/// Timestamp value specifying an unbounded duration
#define DURATION_UNKNOWN ((FrameNumber)-1)

/// Type of Vsynth video frames
typedef struct Frame *Frame;
/// Type of an active Vsynth filter
typedef struct ActiveFilter *ActiveFilter;
/// Type of a Vsynth filter
typedef struct Filter *Filter;


/// Vtable for Frame objects
struct FrameVirtual {
	/// Deinitialise and deallocate a frame
	///
	/// This should possibly be changed to reference counting.
	void (VSYNTH_METHOD *destroy)(Frame frame);
	/// Create a complete copy of the frame that can be safely written to
	/// without affecting the original
	Frame (VSYNTH_METHOD *clone)(Frame frame);
};
/// Represents a video frame
///
/// @todo This needs some more fields or methods to actually hold pixel data
struct Frame {
	/// Points to the vtable for this frame object
	struct FrameVirtual *methods;
	/// Timestamp the frame would appear at during playback
	///
	/// A frame's timestamp is the first moment in time the frame is to be
	/// displayed.
	Timestamp timestamp;
};

/// Structure for describing supported frame types during filter activation
///
/// Frame types may extend this structure with other relevant fields to describe
/// further constraints.
struct FrameTypeDescription {
	/// Pointer to the vtable of the frame type, to distinguish the frame type
	struct FrameVirtual *frame_type;
	/// Set by the callee during filter activation, telling whether it supports the format
	///
	/// The filter being activated sets this to zero if it does not supporte the format
	/// or to non-zero if it does support the format.
	int out_supported;
};


/// A character string or blob with memory managed by Vsynth
struct String {
	/// Length of string in bytes
	size_t len;
	/// Pointer to string character data
	///
	/// If len==0 then this pointer is NULL.
	char *str;
};
/// Type of Vsynth-managed strings
typedef struct String *String;

/// Create a new String with undefined contents
VSYNTH_API(String) AllocString(size_t len);
/// Create a new String from a nul-terminated string
VSYNTH_API(String) MakeString(const char *str);
/// Create a new String from a string with known length
VSYNTH_API(String) MakeStringN(const char *str, size_t len);
/// Create a new copy of a String
VSYNTH_API(String) CopyString(const String str);
/// Deallocate the memory used by a String
VSYNTH_API(void) FreeString(String str);


/// Vtable for ActiveFilter
struct ActiveFilterVirtual {
	/// De-initialise and deallocate a filter
	void (VSYNTH_METHOD *destroy)(ActiveFilter filter);
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
	Frame (VSYNTH_METHOD *get_frame)(ActiveFilter filter, FrameNumber n);
	/// Return the maximum number of frames this filter can produce
	///
	/// The number returned by this function may be higher than the actual
	/// number of frames the filter can produce, in case the number of frames
	/// is not known at activation time.
	///
	/// This method may return FRAMECOUNT_UNKNOWN, in which case the actual
	/// number of frames this filter can provide is not known and not bounded.
	FrameNumber (VSYNTH_METHOD *get_frame_count)(ActiveFilter filter);
	/// Return the duration of this filter's video stream
	///
	/// This must return either the timestamp of the imaginary frame
	/// following the last frame this filter can supply, or the
	/// DURATION_UNKNOWN constant, in which case the duration of the
	/// video stream is not known.
	Timestamp (VSYNTH_METHOD *get_duration)(ActiveFilter filter);
};
/// An activated filter from which frames can be requested
struct ActiveFilter {
	/// Points to the vtable for the ActiveFilter object
	struct ActiveFilterVirtual *methods;
	/// Points to the Filter object that produced this active instance
	///
	/// Should be treated const.
	Filter filter;
};


/// Types a property value can take
enum PropertyType {
	PROP_FILTER,
	PROP_INT, // signed long long
	PROP_DOUBLE,
	PROP_STRING, // String type
	PROP_FRAMENUMBER,
	PROP_TIMESTAMP,
	// array types too?
};

/// Type of callback function for enumerating all properties on a filter
typedef void (VSYNTH_METHOD *EnumPropertiesFunc)(const char *name, enum PropertyType type, void *userdata);
/// Vtable for Filter objects
struct FilterVirtual {
	/// Increase the reference count to the Filter object
	void (VSYNTH_METHOD *addref)(Filter filter);
	/// Decrease the reference count to the Filter object
	///
	/// If the reference count reaches zero the object must be deinitialised
	/// and deallocated.
	void (VSYNTH_METHOD *unref)(Filter filter);
	/// Create a clone of the filter and its property values
	///
	/// The returned clone must have a reference count of 1.
	Filter (VSYNTH_METHOD *clone)(Filter filter);
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
	ActiveFilter (VSYNTH_METHOD *activate)(Filter filter, String *error, struct FrameTypeDescription **frametypes);

	/// Enumerate all properties the filter has through the callback function
	void (VSYNTH_METHOD *enum_properties)(EnumPropertiesFunc callback, void *userdata);
	/// Get a property value of Filter type
	///
	/// The returned Filter object has had addref called and the caller owns
	/// that reference.
	///
	/// May return NULL if the property is not of Filter type, it doesn't exist,
	/// or there is no value assigned.
	Filter (VSYNTH_METHOD *get_property_filter)(Filter filter, const char *name);
	/// Get a property value of integer type
	///
	/// The return value is undefined if the property is not of integer type
	/// or it doesn't exist.
	long long (VSYNTH_METHOD *get_property_int)(Filter filter, const char *name);
	/// Get a property value of double type
	///
	/// The return value is undefined if the property is not of double type or
	/// it doesn't exist.
	double (VSYNTH_METHOD *get_property_double)(Filter filter, const char *name);
	/// Get a property value of String type
	///
	/// May return NULL if the property is not of String type, it doesn't exist,
	/// or the string is empty.
	///
	/// Returns the actual String object held, the caller must not modify the
	/// returned object. The caller must copy the String if it wants to
	/// manipulate it. (This is for performance reasons, to avoid risking
	/// copying huge strings.)
	String (VSYNTH_METHOD *get_property_string)(Filter filter, const char *name);
	/// Get a property value of FrameNumber type
	///
	/// The return value is undefined if the property is not of FrameNumber
	/// type or it doesn't exist.
	FrameNumber (VSYNTH_METHOD *get_property_framenumber)(Filter filter, const char *name);
	/// Get a property value of Timestamp type
	///
	/// The return value is undefined if the property is not of Timestamp
	/// type or it doesn't exist.
	Timestamp (VSYNTH_METHOD *get_property_timestamp)(Filter filter, const char *name);
	/// Set a property to a Filter value
	///
	/// The value may be NULL.
	///
	/// The callee is responsible for calling addref on the newly assigned
	/// value and calling unref on the old value.
	///
	/// The call may be ignored if the property doesn't exist or is not of
	/// Filter type.
	void (VSYNTH_METHOD *set_property_filter)(Filter filter, const char *name, Filter value);
	/// Set a property to an integer value
	///
	/// The call may be ignored if the property doesn't exist or is not of
	/// integer type.
	void (VSYNTH_METHOD *set_property_int)(Filter filter, const char *name, long long value);
	/// Set a property to a double value
	///
	/// The call may be ignored if the property doesn't exist or is not of
	/// double type.
	void (VSYNTH_METHOD *set_property_double)(Filter filter, const char *name, double value);
	/// Set a property to a String value
	///
	/// The value may be NULL to set the property to the empty string.
	///
	/// The callee must make a copy of the given String and store that copy.
	///
	/// The call may be ignored if the property doesn't exist or is not of
	/// String type.
	void (VSYNTH_METHOD *set_property_string)(Filter filter, const char *name, String value);
	/// Set a property to a FrameNumber value
	///
	/// The call may be ignored if the property doesn't exist or is not of
	/// FrameNumber type.
	void (VSYNTH_METHOD *set_property_framenumber)(Filter filter, const char *name, FrameNumber value);
	/// Set a property to a Timestamp value
	///
	/// The call may be ignored if the property doesn't exist or is not of
	/// Timestamp type.
	void (VSYNTH_METHOD *set_property_timestamp)(Filter filter, const char *name, Timestamp value);
};
/// A prototype filter which can be configured and activate instances
///
/// A filter prototype must not own active filters and must not request frames.
/// When a filter is activated, it should activate all other filters it
/// references and pass those activated filters to the instance it creates.
struct Filter {
	/// Point to the vtable for the Filter object
	struct FilterVirtual *methods;
};


/// A factory for Filter objects of a specific type
struct FilterFactory {
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
	Filter (VSYNTH_METHOD *produce)(void);
};

/// Type of callback functions for enumerating registered filters
typedef void (VSYNTH_METHOD *EnumFiltersFunc)(struct FilterFactory *factory, void *userdata);

/// Register a new filter type with Vsynth
VSYNTH_API(void) RegisterFilter(struct FilterFactory *factory);
/// Get the factory for a named filter
///
/// @param name Programmatic identifier for the filter requested
VSYNTH_API(struct FilterFactory *) FindFilter(const char *name);
/// Enumerate all currently registered filters
VSYNTH_API(void) EnumerateFilters(EnumFiltersFunc callback, void *userdata);

HEADER_END