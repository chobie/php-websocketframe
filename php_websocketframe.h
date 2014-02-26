/*
 * PHP Websocket Frame extension
 * Copyright 2013 Shuhei Tanuma.  All rights reserved.
 *
 * https://github.com/chobie/php-websocketframe
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *   1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef PHP_WEBSOCKETFRAME_H
#define PHP_WEBSOCKETFRAME_H

#define PHP_WEBSOCKETFRAME_EXTNAME "websocketframe"
#define PHP_WEBSOCKETFRAME_VERSION "0.1.0"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "zend.h"
#include "zend_interfaces.h"
#include "zend_exceptions.h"
#include "ext/standard/php_smart_str.h"
#include "ext/spl/spl_exceptions.h"
#include "ext/standard/php_var.h"
#include "ext/standard/php_string.h"
#include "ext/standard/php_incomplete_class.h"
#include "ext/standard/info.h"
#include "ext/standard/php_array.h"
#include "limits.h"
#include <stdlib.h>

/* Define the entry point symbol
 * Zend will use when loading this module
 */
extern zend_module_entry websocketframe_module_entry;
#define phpext_websocketframe_ptr &websocketframe_module_entry

extern zend_class_entry *php_websocket_frame_class_entry;

ZEND_BEGIN_MODULE_GLOBALS(websocketframe)
	int dummy;
ZEND_END_MODULE_GLOBALS(websocketframe)

ZEND_EXTERN_MODULE_GLOBALS(websocketframe)

#ifdef ZTS
#define WFG(v) TSRMG(websocketframe_globals_id, zend_websocketframe_globals *, v)
#else
#define WFG(v) (websocketframe_globals.v)
#endif

#endif /* PHP_WEBSOCKETFRAME_H */
