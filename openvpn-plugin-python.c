/*
 * Copyright (C) 2018 Kaltashkin Eugene <aborche.aborche@gmail.com>
 * Copyright (C) 2019 Boris Lytochkin <lytboris@gmail.com>
 * Copyright (C) 2020 Yandex, LLC
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 51
 * Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

/*
 * This file implements a python interpreter to handle various events though
 * OpenVPN plugin calls.
 *
 * See the README file for build instructions.
 */

#define __EXTENSIONS__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <Python.h>
#include <time.h>

#include "embedpython.h"

#include <openvpn-plugin.h>

#define PLUGIN_NAME "python"

struct hook_item {
	int	hook_id;
	char *hook_name;
};

#define OPENVPN_HOOK(a) { OPENVPN_##a, #a }

const struct hook_item plugin_hooks[] = {
	OPENVPN_HOOK(PLUGIN_UP),
	OPENVPN_HOOK(PLUGIN_DOWN),
	OPENVPN_HOOK(PLUGIN_ROUTE_UP),
	OPENVPN_HOOK(PLUGIN_IPCHANGE),
	OPENVPN_HOOK(PLUGIN_TLS_VERIFY),
	OPENVPN_HOOK(PLUGIN_AUTH_USER_PASS_VERIFY),
	OPENVPN_HOOK(PLUGIN_CLIENT_CONNECT),
	OPENVPN_HOOK(PLUGIN_CLIENT_DISCONNECT),
	OPENVPN_HOOK(PLUGIN_LEARN_ADDRESS),
	OPENVPN_HOOK(PLUGIN_CLIENT_CONNECT_V2),
	OPENVPN_HOOK(PLUGIN_TLS_FINAL),
	OPENVPN_HOOK(PLUGIN_ENABLE_PF),
	OPENVPN_HOOK(PLUGIN_ROUTE_PREDOWN),
};

/* Our context, where we keep our state. */
struct plugin_context {
	plugin_log_t log;
	char *config_param;

	char *plugin_func_names[OPENVPN_PLUGIN_N];
	struct Python_Inventory inv;
};

OPENVPN_EXPORT int
openvpn_plugin_open_v3(const int v3structver,
		       struct openvpn_plugin_args_open_in const *args,
		       struct openvpn_plugin_args_open_return *ret)
{
	struct plugin_context *context = NULL;

	/* Allocate our context */
	context = (struct plugin_context *)calloc(1, sizeof(struct plugin_context));
	if (!context) {
		return OPENVPN_PLUGIN_FUNC_ERROR;
	}
	struct Python_Inventory *inv = &context->inv;

	/* consistent logging */
	plugin_log_t log = args->callbacks->plugin_log;
	context->log = log;

	/* Define plugin types which our script can serve */
	ret->type_mask = 0;

	/* Save parameters for plugin from openvpn config */
	if (args->argv[1])
		context->config_param = strdup(args->argv[1]);

	log(PLOG_DEBUG, PLUGIN_NAME, "openvpn-plugin-" PLUGIN_NAME ": config_param=%s", context->config_param);

	/* Point the global context handle to our newly created context */
	ret->handle = (void *)context;

	/* Init Python interpreter */
	inv->script_dir = context->config_param;
	inv->program_name = "openvpn-plugin-" PLUGIN_NAME;
	inv->script_module = "plugin";
	enum PYTHON_Result load_res = python_init(inv);

	if (load_res != PYTHON_OK) {
		log(PLOG_ERR, PLUGIN_NAME, "Error loading the Python plugin (see stderr)");
		return OPENVPN_PLUGIN_FUNC_ERROR;
	}

	/* Scan module for methods available and register them */
	for (int hook_num = 0; hook_num < OPENVPN_PLUGIN_N; hook_num++) {
		log(PLOG_DEBUG, PLUGIN_NAME, "Looking for function %s in plugin.py", plugin_hooks[hook_num].hook_name);
		if (python_is_function_defined(inv, plugin_hooks[hook_num].hook_name)) {
			context->plugin_func_names[plugin_hooks[hook_num].hook_id] = plugin_hooks[hook_num].hook_name;
			ret->type_mask |= OPENVPN_PLUGIN_MASK(plugin_hooks[hook_num].hook_id);
			log(PLOG_DEBUG, PLUGIN_NAME, "hook %s is enabled", plugin_hooks[hook_num].hook_name);
		} else {
			context->plugin_func_names[plugin_hooks[hook_num].hook_id] = NULL;
			log(PLOG_DEBUG, PLUGIN_NAME, "hook %s is disabled", plugin_hooks[hook_num].hook_name);
		}
	}

	return OPENVPN_PLUGIN_FUNC_SUCCESS;
}

int split_string_by_char(const char* splittee, char sep, char** left, char** right)
{
	size_t len = strnlen(splittee, 1024);
	size_t i;

	for (i = 0; i < len; i++) {
		if (splittee[i] == sep) {
			*left = strndup(splittee, i);
			*right = strndup(splittee + i + 1, len - i - 1);
			break;
		}
	}
	if (i == len) {
		return -1;
	}
	return i;
}

PyObject* env_to_dict(struct openvpn_plugin_args_func_in const *args)
{
	/* Build a Dict out of envp */
	struct plugin_context *context = (struct plugin_context *)args->handle;
	plugin_log_t log = context->log;
	PyObject *envDict = PyDict_New();
	PyObject *dKey, *dValue;

	for (const char **env_item = args->envp; *env_item != NULL; env_item++) {
		char *env_key, *env_value;
		ssize_t res = split_string_by_char(*env_item, '=', &env_key, &env_value);

		if (res < 0) {
			log(PLOG_ERR, PLUGIN_NAME, "Environment variable parse error, = is not found in '%s'", *env_item);
			continue;
		}
		dKey = PyUnicode_FromString(env_key);
		dValue = PyUnicode_FromString(env_value);
		PyDict_SetItem(envDict, dKey, dValue);

		// Python3's Objects/unicodeobject.c PU_FS does not steal the pointer
		free(env_key);
		free(env_value);
	}
	return envDict;
}

PyObject* argv_to_list(struct openvpn_plugin_args_func_in const *args)
{
	/*
	  struct plugin_context *context = (struct plugin_context *)args->handle;
	  plugin_log_t log = context->log;
	*/

	PyObject *argList = PyList_New(0);
	PyObject *dItem;

	for (const char **arg_item = args->argv; *arg_item != NULL; arg_item++) {
		dItem = PyUnicode_FromString(*arg_item);
		PyList_Append(argList, dItem);
	}
	return argList;
}

#define NS_IN_S  1000000000
#define NS_IN_MS 1000000

int get_duration_time(struct timespec *start,
		      struct timespec *stop)
{
	stop->tv_nsec += NS_IN_S * (stop->tv_sec - start->tv_sec);
	return (stop->tv_nsec - start->tv_nsec) / NS_IN_MS;
}

OPENVPN_EXPORT int
openvpn_plugin_func_v3(const int version,
		       struct openvpn_plugin_args_func_in const *args,
		       struct openvpn_plugin_args_func_return *retptr)
{
	struct timespec start_time, stop_time;
	clock_gettime(CLOCK_MONOTONIC, &start_time);

	struct plugin_context *context = (struct plugin_context *)args->handle;
	plugin_log_t log = context->log;

	/* Python function name for calling */
	int PyReturn = OPENVPN_PLUGIN_FUNC_ERROR;

	char *func_name = context->plugin_func_names[args->type];
	if (func_name == NULL) {
		log(PLOG_DEBUG, PLUGIN_NAME, "function was not registered");
		return OPENVPN_PLUGIN_FUNC_ERROR;
	}

	PyObject *envarg = env_to_dict(args);
	PyDict_SetItemString(envarg, "__ARGV", argv_to_list(args));
	int retval = python_from_int(
	    python_call_function(&context->inv, func_name, 1, envarg));
	if (context->inv.last_error == PYTHON_OK) {
		log(PLOG_DEBUG, PLUGIN_NAME, "Result of call: %ld", retval);
		switch (retval) {
		case 0:
			PyReturn = OPENVPN_PLUGIN_FUNC_SUCCESS;
			break;
		case 1:
			PyReturn = OPENVPN_PLUGIN_FUNC_ERROR;
			break;
		case 2:
			PyReturn = OPENVPN_PLUGIN_FUNC_DEFERRED;
			break;
		default:
			PyReturn = OPENVPN_PLUGIN_FUNC_ERROR;
		}
	} else {
		PyErr_Print();
		log(PLOG_ERR, PLUGIN_NAME, python_error(retval));
		log(PLOG_DEBUG, PLUGIN_NAME, "Call failed");
		return OPENVPN_PLUGIN_FUNC_ERROR;
	}

	clock_gettime(CLOCK_MONOTONIC, &stop_time);
	log(PLOG_NOTE, PLUGIN_NAME, "function %s elapsed %d ms", func_name, get_duration_time(&start_time, &stop_time));

	return PyReturn;
}

OPENVPN_EXPORT void
openvpn_plugin_close_v1(openvpn_plugin_handle_t handle)
{
	struct plugin_context *context = (struct plugin_context *)handle;
	python_uninit(&context->inv);
	free(context);
}
