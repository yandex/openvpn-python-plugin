/*
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

#ifndef __PYTHON_H_ALSO_THE_GAME__
#define __PYTHON_H_ALSO_THE_GAME__

#include <stdbool.h>

#include <Python.h>

#define GLOBALS_KEY "_GLOBALS"

enum PYTHON_Result {
	PYTHON_OK = 0,
	PYTHON_GENERIC_ERROR,
	PYTHON_INIT_FAIL,
	PYTHON_UNINIT_FAIL,
	PYTHON_IMPORT_FAIL,
	PYTHON_NOT_INITIALIZED,
	PYTHON_UNKNOWN_FUNCTION,
	PYTHON_RUNTIME_EXCEPTION,
};

struct Python_Inventory {
	char *script_dir;
	char *script_module;
	char *program_name;
	PyObject *globals;
	PyObject *module;
	enum PYTHON_Result last_error;
	bool initialized;
} Python_Inventory;

int python_init(struct Python_Inventory *inv);

PyObject* python_call_function(struct Python_Inventory *inv, const char* name, int numargs, ...);

bool python_is_function_defined(struct Python_Inventory *inv, const char* name);

int python_uninit(struct Python_Inventory *inv);

const char* python_error(const enum PYTHON_Result res);

PyObject* python_int(long long int);
PyObject* python_str(const char*);
long long int python_from_int(PyObject*);
const char* python_from_str(PyObject*);

#endif
