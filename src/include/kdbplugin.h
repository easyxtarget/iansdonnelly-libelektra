/***************************************************************************
                kdbplugin.h  -  Methods for plugin programing
                             -------------------
 *  begin                : Wed 19 May, 2010
 *  copyright            : (C) 2010 by Markus Raab
 *  email                : elektra@markus-raab.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the BSD License (revised).                      *
 *                                                                         *
 ***************************************************************************/

/*
 * You have to include this file in order to write plugins.
 * You do not need this functions to use elektra otherwise!
 */


#ifndef KDBPLUGIN_H
#define KDBPLUGIN_H

#include <kdb.h>

#define ELEKTRA_QUOTE(x)       #x

#ifdef ELEKTRA_STATIC
	#ifdef ELEKTRA_VARIANT
		#define ELEKTRA_PLUGIN_EXPORT(module) ELEKTRA_PLUGIN_EXPORT2(module, ELEKTRA_VARIANT)
		#define ELEKTRA_PLUGIN_EXPORT2(module, variant) ELEKTRA_PLUGIN_EXPORT3(module, variant)
		#define ELEKTRA_PLUGIN_EXPORT3(module, variant) libelektra_##module##_##variant##_LTX_elektraPluginSymbol(void)
	#else
		#define ELEKTRA_PLUGIN_EXPORT(module) libelektra_##module##_LTX_elektraPluginSymbol(void)
	#endif
#else
        #define ELEKTRA_PLUGIN_EXPORT(module) elektraPluginSymbol(void)
#endif

#ifdef ELEKTRA_VARIANT
	#define ELEKTRA_PLUGIN_FUNCTION(module, function) ELEKTRA_PLUGIN_FUNCTION2(module, ELEKTRA_VARIANT, function)
	#define ELEKTRA_PLUGIN_FUNCTION2(module, variant, function) ELEKTRA_PLUGIN_FUNCTION3(module, variant, function)
	#define ELEKTRA_PLUGIN_FUNCTION3(module, variant, function) libelektra_##module##_##variant##_LTX_elektraPlugin##function
#else
	/**
	 * @brief Declare a plugin's function name suitable for
	 * compilation variants (see doc/tutorials).
	 *
	 * It can be used in the same way as elektraPluginExport().
	 * @see ELEKTRA_PLUGIN_EXPORT
	 *
	 * @ingroup plugin
	 *
	 * @param plugin the name of the plugin
	 * @param which which function it is (open, close, get, set, error)
	 */
	#define ELEKTRA_PLUGIN_FUNCTION(module, function) libelektra_##module##_LTX_elektraPlugin##function
#endif

#ifdef ELEKTRA_VARIANT
	#define ELEKTRA_README(module) ELEKTRA_README2(module, ELEKTRA_VARIANT)
	#define ELEKTRA_README2(module, variant) ELEKTRA_README3(module, variant)
	#define ELEKTRA_README3(module, variant) ELEKTRA_QUOTE(readme_##module##_##variant.c)
#else
	/**
	 * @brief The filename for inclusion of the readme for
	 * compilation variants (see doc/tutorials).
	 *
	 * @ingroup plugin
	 *
	 * @param plugin the name of the plugin
	 */
	#define ELEKTRA_README(module) ELEKTRA_README2(module)
	#define ELEKTRA_README2(module) ELEKTRA_QUOTE(readme_##module.c)
#endif

/**
 * Switches to denote the backend methods. Used in calls to elektraPluginExport().
 *
 * @ingroup backend
 */
typedef enum {
	ELEKTRA_PLUGIN_OPEN=1,		/*!< Next arg is backend for kdbOpen() */
	ELEKTRA_PLUGIN_CLOSE=1<<1,	/*!< Next arg is backend for kdbClose() */
	ELEKTRA_PLUGIN_GET=1<<2,	/*!< Next arg is backend for kdbGet() */
	ELEKTRA_PLUGIN_SET=1<<3,	/*!< Next arg is backend for kdbSet() */
	ELEKTRA_PLUGIN_ERROR=1<<4,	/*!< Next arg is backend for kdbError() */
	ELEKTRA_PLUGIN_END=0		/*!< End of arguments */
} plugin_t;


#ifdef __cplusplus
namespace ckdb {
extern "C" {
#endif

typedef struct _Plugin	Plugin;

Plugin *elektraPluginExport(const char *pluginName, ...);

KeySet *elektraPluginGetConfig(Plugin *handle);
void elektraPluginSetData(Plugin *plugin, void *handle);
void* elektraPluginGetData(Plugin *plugin);


#define PLUGINVERSION "1"


#ifdef __cplusplus
}
}
#endif


#endif
