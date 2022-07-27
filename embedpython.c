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

#include <stdbool.h>

#include "embedpython.h"


int python_init(struct Python_Inventory* inv)
{
	inv->initialized = false;
	PyStatus status;
	PyPreConfig preconfig;
	PyPreConfig_InitPythonConfig(&preconfig);
	status = Py_PreInitialize(&preconfig);
	if (PyStatus_Exception(status)) {
		Py_ExitStatusException(status);
	}

	wchar_t* program_name = Py_DecodeLocale(inv->program_name, NULL);
	Py_SetProgramName(program_name);
	Py_Initialize();

	PyObject *sys = PyImport_ImportModule("sys");
	PyObject *path = PyObject_GetAttrString(sys, "path");
	PyList_Append(path, PyUnicode_FromString(inv->script_dir));
	Py_XDECREF(sys);
	Py_XDECREF(path);

	PyObject *tb_module_name = PyUnicode_FromString("traceback");
	if (NULL == PyImport_Import(tb_module_name)) {
		PyErr_Print();
		inv->last_error = PYTHON_IMPORT_FAIL;
		return PYTHON_IMPORT_FAIL;
	}
	Py_XDECREF(tb_module_name);

	PyObject *module_name = PyUnicode_FromString(inv->script_module);
	if (module_name == NULL) {
		inv->last_error = PYTHON_GENERIC_ERROR;
		return PYTHON_GENERIC_ERROR;
	}

	inv->module = PyImport_Import(module_name);
	if (inv->module == NULL) {
		PyErr_Print();
		inv->last_error = PYTHON_IMPORT_FAIL;
		return PYTHON_IMPORT_FAIL;
	}

	PyObject* script_globals = PyObject_GetAttrString(inv->module, GLOBALS_KEY);
	if (NULL == script_globals) {
		inv->globals = PyDict_New();
		PyObject_SetAttrString(inv->module, GLOBALS_KEY, inv->globals);
	} else {
		inv->globals = script_globals;
	}

	inv->initialized = true;
	inv->last_error = PYTHON_OK;
	return PYTHON_OK;
}

int python_uninit(struct Python_Inventory* inv)
{
	inv->initialized = false;

	Py_XDECREF(inv->globals);
	Py_XDECREF(inv->module);

	Py_Finalize();

	inv->last_error = PYTHON_OK;
	return PYTHON_OK;
}

// Accepts a list of PyObject*'s which can be created
// from char* and int/long with python_str and python_int respectively.
PyObject* python_call_function(struct Python_Inventory* inv, const char* name, int numargs, ...)
{
	if (!Py_IsInitialized()) {
		inv->last_error = PYTHON_NOT_INITIALIZED;
		return NULL;
	}

	PyObject *function = PyObject_GetAttrString(inv->module, name);
	if (function == NULL) {
		inv->last_error = PYTHON_UNKNOWN_FUNCTION;
		return NULL;
	}

	PyObject* args;
	if (0 == numargs) {
		args = PyTuple_New(0);
	} else {
		args = PyTuple_New(numargs);
		va_list arguments;
		va_start(arguments, numargs);
		for (int i = 0; i < numargs; i++) {
			PyTuple_SetItem(args, i, va_arg(arguments, PyObject*));
		}
		va_end(arguments);
	}
	// Note: has to be cast and Py_DECREF'd later with one of the unboxers
	// (python_from_int, python_from_str).
	PyObject* res = PyObject_CallObject(function, args);

	Py_XDECREF(args); Py_XDECREF(function);
	PyObject* err = PyErr_Occurred();
	if (err != NULL) {
		PyErr_Print();
		inv->last_error = PYTHON_RUNTIME_EXCEPTION;
		Py_XDECREF(res);
		Py_XDECREF(err);
		return NULL;
	}
	inv->last_error = PYTHON_OK;
	return res;
}

bool python_is_function_defined(struct Python_Inventory* inv, const char* name)
{
	if (!Py_IsInitialized()) {
		inv->last_error = PYTHON_NOT_INITIALIZED;
		return NULL;
	}

	if (!PyObject_HasAttrString(inv->module, name)) {
		return false;
	}
	PyObject* func = PyObject_GetAttrString(inv->module, name);
	return PyCallable_Check(func);
}

PyObject* python_str(const char* arg)
{
	PyObject* res = PyUnicode_FromString(arg);
	assert(NULL != res);
	return res;
}

PyObject* python_int(const long long int arg)
{
	PyObject* res = PyLong_FromLongLong(arg);
	assert(NULL != res);
	return res;
}

long long int python_from_int(PyObject* arg)
{
	long long int result = PyLong_AsLongLong(arg);
	Py_XDECREF(arg);
	return result;
}

const char* python_from_str(PyObject* arg)
{
	const char* result = PyUnicode_AsUTF8(arg);
	Py_XDECREF(arg);
	return result;
}

const char* python_error(const enum PYTHON_Result res)
{
	switch (res) {
	case PYTHON_IMPORT_FAIL:
		return "Python importing error";
	case PYTHON_INIT_FAIL:
		return "Python interpreter initialization failed";
	case PYTHON_NOT_INITIALIZED:
		return "Python interpterer was not initialized properly";
	case PYTHON_UNINIT_FAIL:
		return "Python interpreter unitialization failed";
	case PYTHON_UNKNOWN_FUNCTION:
		return "unknown function name";
	case PYTHON_RUNTIME_EXCEPTION:
		return "runtime Python exception raised";
	case PYTHON_OK:
		return "no error";
	case PYTHON_GENERIC_ERROR:
	default:
		return "generic PYTHON error";
	}
}
